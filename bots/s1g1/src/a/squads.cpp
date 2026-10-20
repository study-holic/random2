// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
namespace va {
// S-08 squads: squad assignment by champion type, squad claims in shared memory, target choice (v2 scoring with
// v3-quick's own-half-first rule), and the capture rule (lowest-id squad member captures, the rest move on).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <span>
#include <variant>

using cambc::vec2;
using CT = cambc::champion_type;

static constexpr int kRespawnTime = P_RESPAWN_TIME;      // rules: respawn_time 60 plus the 5-round capture
static constexpr int kClaimSlack = P_CLAIM_SLACK;       // a squad claim stays valid while its group respawns within this many rounds
static constexpr int kExcludeRounds = P_EXCLUDE_ROUNDS;
static constexpr int kClaimBonus = P_CLAIM_BONUS;          // score bonus (tiles) for the squad's claimed group
static constexpr bool kCaptureByHealth = P_CAPTURE_BY_HEALTH;   // false: lowest id captures (spec); true: lowest starting health

// Squads by type (spec): with 5 or more champions Vendetta, Issac and Cassi form squad 0 and the rest squad 1.
// When the draft produced something else and a squad lacks a melee champion or has fewer than two members,
// melee champions (highest max health first) are dealt alternately to the two squads, then the ranged ones.
void Bot::assign_squads() {
    const int n = static_cast<int>(roster.size());
    roster_squad.assign(n, 0);
    roster_melee.assign(n, false);
    for (int i = 0; i < n; ++i) roster_melee[i] = roster[i].attack_range() <= 1;
    if (n >= P_SQUAD_MIN_ROSTER) {
        for (int i = 0; i < n; ++i) {
            const auto t = roster[i].champion_name();
            roster_squad[i] = (t == CT::vendetta || t == CT::issac || t == CT::cassi) ? 0 : 1;
        }
        int size[2] = {0, 0}, melee[2] = {0, 0};
        for (int i = 0; i < n; ++i) { ++size[roster_squad[i]]; if (roster_melee[i]) ++melee[roster_squad[i]]; }
        if (size[0] < P_SQUAD_MIN_SIZE || size[1] < P_SQUAD_MIN_SIZE || melee[0] == 0 || melee[1] == 0) {
            std::vector<int> order(n);
            std::iota(order.begin(), order.end(), 0);
            std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
                if (roster_melee[a] != roster_melee[b]) return static_cast<bool>(roster_melee[a]);
                return roster[a].max_health() > roster[b].max_health();
            });
            for (int k = 0; k < n; ++k) roster_squad[order[k]] = k % 2;
        }
    }
    capture_key.assign(n, 0);
    for (int i = 0; i < n; ++i)
        capture_key[i] = kCaptureByHealth ? static_cast<int>(roster[i].max_health()) * 8 + i : i;
    squad = roster_squad[ally_index];
    is_melee = roster_melee[ally_index];
    squad_capable = false;
    int members = 0;
    for (int i = 0; i < n; ++i) if (roster_squad[i] == squad) { ++members; if (roster_melee[i]) squad_capable = true; }
    if (members < P_ELK_MIN_SQUAD) squad_capable = false;
    squad_size = std::max(1, members);
}

int Bot::roster_index(const cambc::champion& c) const {
    for (std::size_t i = 0; i < roster.size(); ++i) if (roster[i] == c) return static_cast<int>(i);
    return -1;
}

int Bot::read_squad_claim() const {
    std::uint8_t b = 0;
    cambc::shared_memory.read(kSquadClaimByte + squad, std::span{&b, 1});
    return static_cast<int>(b) - 1;
}

void Bot::write_squad_claim(int group) {
    const std::uint8_t b = static_cast<std::uint8_t>(group + 1);
    cambc::shared_memory.write(kSquadClaimByte + squad, std::span<const std::uint8_t>{&b, 1});
}

bool Bot::is_squad_group(int index) const {
    return index >= 0 && squad_capable && groups[index].squad_target;
}

bool Bot::claim_usable(int claim, int round, int exclude) const {
    if (claim < 0 || claim >= static_cast<int>(groups.size()) || claim == exclude) return false;
    const auto& g = groups[claim];
    return !g.monolith && g.excluded_until <= round && g.ready_round <= round + kClaimSlack;
}

// Squad groups go through the squad claim: while the claim is usable it is the only squad group a member may pick;
// solo groups are free for whoever is nearest and unclaimed. Scores follow v2 (distance, wait, gem value) with
// v3-quick's own half first before round 400.
int Bot::choose_target(const TurnView& v, int exclude) {
    if (P_S1_AUCTION == 1) return s1_pick(v, exclude, nullptr);   // S1: auction replaces the scoring below
#if P_S1_FUELDBG
    const auto fct0 = cambc::fuel_remaining();
#endif
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    squad_live = 1;   // S1 hybrid coalition: me plus squad mates alive (non-zero claim byte; 0 = dead, bot.cpp)
    if (P_S1_SQUAD_LIVE)
        for (int k = 0; k < static_cast<int>(roster.size()); ++k)
            if (k != ally_index && roster_squad[k] == squad && mem[std::min(k, kSquadClaimByte - 1)] != 0) ++squad_live;
    if (!P_S1_SQUAD_LIVE) squad_live = squad_size;
    const int round = v.round;
    if (P_OPENING_ASSIGN) {   // EXP-079: own first group during the opening
        const int a = assigned_group(v, exclude, mem.data());
        if (a >= 0) return a;
    }
    if (cluster_opening(round)) {   // EXP-030: harmless gem groups first in the opening
        const int c = choose_cluster(v, exclude, mem.data());
        if (c >= 0) return c;
    }
    const int my_claim = static_cast<int>(mem[kSquadClaimByte + squad]) - 1;
    const int other_claim = static_cast<int>(mem[kSquadClaimByte + 1 - squad]) - 1;
    const bool claim_ok = squad_capable && claim_usable(my_claim, round, exclude);
    // EXP-018: groups contested in the last 30 rounds are skipped while another live group exists on our half.
    bool alt_own_half = false;
    for (int i = 0; i < static_cast<int>(groups.size()) && !alt_own_half; ++i) {
        const auto& g = groups[i];
        alt_own_half = !g.monolith && g.excluded_until <= round && i != exclude && !contested(i, round)
            && g.dist_ally <= g.dist_enemy && g.ready_round <= round + 30;
    }
#if P_S1_FUELDBG
    const auto fct1 = cambc::fuel_remaining();
#endif
    int best = -1; double best_score = 1e18;
    // S1 hybrid scratch on the stack (no heap allocation): u per group once per call, the candidate set (u only for the
    // P_S1_CANDIDATES best by v4's cheap score, ranked by bounding-box distance), exact distances once per call.
    constexpr int kMaxG = 64;
    const int ng = std::min<int>(static_cast<int>(groups.size()), kMaxG);
    std::array<double, kMaxG> u_cache; u_cache.fill(-1e18);
    std::array<char, kMaxG> u_cand; u_cand.fill(1);
    std::array<int, kMaxG> d_cache; d_cache.fill(-1);
    auto dist = [&](int i) { if (i >= kMaxG) return min_distance(v.pos, groups[i].tiles); if (d_cache[i] < 0) d_cache[i] = min_distance(v.pos, groups[i].tiles); return d_cache[i]; };
    if (P_S1_AUCTION == 2) {
        std::array<std::pair<int, int>, kMaxG> cs; int ncs = 0;
        for (int i = 0; i < ng; ++i) {
            const auto& g = groups[i];
            if (g.monolith || g.excluded_until > round || i == exclude) continue;
            const int d = g.box_distance(v.pos);
            cs[ncs++] = {d + 2 * std::max(0, g.ready_round - round - d), i};
        }
        if (ncs > P_S1_CANDIDATES) {
            std::nth_element(cs.begin(), cs.begin() + P_S1_CANDIDATES, cs.begin() + ncs);
            u_cand.fill(0);
            for (int c = 0; c < P_S1_CANDIDATES; ++c) u_cand[cs[c].second] = 1;
        }
    }
#if P_S1_FUELDBG
    const auto fct2 = cambc::fuel_remaining();
#endif
    for (int pass = 0; pass < 3 && best < 0; ++pass) {
        const bool skip_contested = alt_own_half && pass < 2;
        if (pass == 2 && !alt_own_half && !P_THREAT_LAYER) break;   // pass 2 only retries with contested groups allowed
        const bool own_half_only = round < P_OWN_HALF_ROUNDS && pass == 0;
        for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
            const auto& g = groups[i];
            if (g.monolith || g.excluded_until > round || i == exclude) continue;
            if (skip_contested && contested(i, round)) continue;
            if (P_THREAT_LAYER && pass < 2 && leashed(i, round)) continue;   // EXP-071 hard leash; pass 2 is the fallback
            if (own_half_only && g.dist_enemy < g.dist_ally && !(claim_ok && i == my_claim)) continue;
            if (is_squad_group(i)) {
                if (i == other_claim) continue;
                if (claim_ok && i != my_claim) continue;
            } else if (P_S1_AUCTION == 2) {   // S1: a claimed light group is taken only by outbidding (ETA bucket 2 better)
                int bb = 4;
                if (s1_claimants(mem, i, bb) > 0) {
                    if (!P_S1_OUTBID) continue;
                    if (eta_bucket(s1_eta_of(i, v)) > bb - 2) continue;
                }
            } else {
                bool taken = false;
                for (int k = 0; k < kSquadClaimByte; ++k) if (k != ally_index && (mem[k] & 0x3F) == i + 1) taken = true;
                if (taken) continue;
            }
            const bool cand = i >= kMaxG || u_cand[i];
            const int d = P_S1_AUCTION == 2 ? (cand ? dist(i) : g.box_distance(v.pos)) : min_distance(v.pos, g.tiles);
            const int wait = std::max(0, g.ready_round - round - d);
            double score = d + P_WAIT_WEIGHT * wait - 0.05 * g.gems;
            if (P_S1_AUCTION == 2) {   // S1: score = -u_ij (value rate), coalition = the squad for squad groups
                int eta = 0;
                if (!cand) score = 2000.0 + d;   // outside the candidate set: after every candidate, nearest first
                else {
                    double u = i < kMaxG ? u_cache[i] : -1e18;
                    if (u < -1e17) { u = s1_value(i, v, 0, eta, is_squad_group(i) ? squad_live : 1); if (i < kMaxG) u_cache[i] = u; }   // once per call (3 passes)
                    score = u > 0 ? -u * P_S1_U_SCALE : 1000.0 + d;   // no positive value: last resort, nearest first
                }
            }
            score += s1_salt_noise(ally_index, i);   // measurement salt (0 at salt 0)
            if (claim_ok && i == my_claim) score -= kClaimBonus;   // squads stay together
            if (round >= P_OWN_HALF_ROUNDS) score += 0.5 * std::max(0, g.dist_ally - g.dist_enemy);
            if (score < best_score) { best_score = score; best = i; }
        }
    }
    if (best >= 0 && is_squad_group(best) && !claim_ok) write_squad_claim(best);
#if P_S1_FUELDBG
    std::cout << "FC r" << round << " a" << ally_index << " choose" << (fct0 - cambc::fuel_remaining()) << " pre" << (fct0 - fct1) << " cand" << (fct1 - fct2) << std::endl;
#endif
    return best;
}

// Give up on a group for a while; a squad claim on it is cleared so the squad picks again.
void Bot::exclude_group(int index, int round) {
    if (index < 0) return;
    groups[index].excluded_until = round + kExcludeRounds;
    if (is_squad_group(index) && read_squad_claim() == index) write_squad_claim(-1);
}

// True when every spawn tile of the group is in view and no camp of it is alive.
bool Bot::group_cleared(const TurnView& v, int index) const {
    const auto& g = groups[index];
    if (!(g.mask & ~v.visible).is_empty()) return false;
    for (const auto& c : v.camps) if (c.group == index) return false;
    return true;
}

// Number of squad members near the group that capture before this one (0 = this champion captures). The order is
// lowest starting health first (ties by id), so the ranged member captures and banks while the tank stays on.
int Bot::capture_rank(const std::vector<cambc::champion>& allies, int group) const {
    auto& me = cambc::self();
    int rank = 0;
    for (const auto& a : allies) {
        if (a == me) continue;
        const int idx = roster_index(a);
        if (idx < 0 || roster_squad[idx] != squad || capture_key[idx] >= capture_key[ally_index]) continue;
        if (min_distance(a.position(), groups[group].tiles) > 8) continue;
        ++rank;
    }
    return rank;
}

// EXP-081: lowest health fraction among visible melee squad mates (the tanks), -1 when none is in view.
float Bot::squad_tank_frac(const std::vector<cambc::champion>& allies) const {
    float low = -1.0f;
    for (const auto& a : allies) {
        const int idx = roster_index(a);
        if (idx < 0 || a == cambc::self() || roster_squad[idx] != squad || !roster_melee[idx]) continue;
        const float f = a.health() / a.max_health();
        if (low < 0.0f || f < low) low = f;
    }
    return low;
}

// EXP-081: nearest group on our half that nobody claims (champion bytes and both squad claims), solo-farmable
// (P_SQUAD_FALLBACK 1) or also an unclaimed pack (2); Monoliths and Elks never.
int Bot::fallback_group(const TurnView& v) {
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    int best = -1, best_d = 1 << 20;
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
        const auto& g = groups[i];
        if (g.monolith || g.heavy || i == target || g.excluded_until > v.round || g.dist_ally > g.dist_enemy) continue;
        if (g.squad_target && P_SQUAD_FALLBACK < 2) continue;
        if (contested(i, v.round)) continue;
        bool taken = false;
        for (int k = 0; k < 8; ++k) if (k != ally_index && (mem[k] & 0x3F) == i + 1) taken = true;
        if (taken) continue;
        const int d = min_distance(v.pos, g.tiles);
        if (g.ready_round > v.round + d + 10 || d > 30) continue;
        if (d < best_d) { best_d = d; best = i; }
    }
    return best;
}

// Decide this turn's target: hold a capture (20-round timeout), leave a cleared squad group (the lowest-id member
// stays), follow the squad claim, or choose afresh.
void Bot::resolve_target(const TurnView& v, const std::vector<cambc::champion>& allies) {
    if (rows_sched) { resolve_rows(v, allies); return; }   // EXP-093
    const int round = v.round;
    if (holding >= 0) {
        bool contested = false;
        for (const auto& c : v.camps) if (c.group == holding) contested = true;
        refresh_group(holding, round, v.visible);   // respawning means the capture has been paid
        if (contested || groups[holding].ready_round > round) {
            holding = -1;
        } else if (round - hold_since > 20) {
            exclude_group(holding, round);
            holding = -1;
            target = -1;
        } else { target = holding; return; }
    }
    if (P_SQUAD_FALLBACK && fb_origin >= 0) {   // EXP-081: back to the squad group when the tank is healthy again
        if (groups[fb_origin].ready_round > round || groups[fb_origin].excluded_until > round || is_squad_group(target)) fb_origin = -1;
        else if (squad_tank_frac(allies) >= 0.7f && claim_usable(fb_origin, round, -1)) {
            target = fb_origin; fb_origin = -1; wait_rounds = 0; wait_tank = 0;
            return;
        }
    }
    if (is_squad_group(target) && groups[target].ready_round <= round && group_cleared(v, target)) {
        const int cleared = target;
        const int rank = P_S1_AUCTION == 1 ? s1_capture_rank(allies, cleared) : capture_rank(allies, cleared);
        const int next = choose_target(v, cleared);
        bool stay = rank == 0 || next < 0;
        if (!stay && !rows && !P_S1_ONE_CAPTURER && rank == 1 && groups[cleared].wolves >= 3 && min_distance(groups[cleared].tiles[0], groups[next].tiles) > 5)
            stay = true;   // three-wolf pack with the next group far: the second member also captures
        if (stay) { holding = cleared; hold_since = round; return; }
        groups[cleared].ready_round = round + kRespawnTime;
        if (P_S1_AUCTION != 1 && read_squad_claim() == cleared) write_squad_claim(is_squad_group(next) ? next : -1);
        target = next;
        return;
    }
    if (P_S1_AUCTION == 1) {   // S1: cleared groups and holds handled above; otherwise re-auction with hysteresis
        if (target >= 0 && groups[target].excluded_until > round) target = -1;
        s1_reauction(v);
        return;
    }
    if (target >= 0 && groups[target].excluded_until > round) target = -1;
    if (P_S1_AUCTION && target < 0 && banking && cambc::self().gem_count() > 0) return;   // S1 fuel: bank() clears the target each banking turn; choose after the tribute
    if (P_S1_AUCTION && cambc::fuel_remaining() < static_cast<std::uint64_t>(P_S1_CHOOSE_FUEL)) return;   // S1 fuel: an expensive turn defers the choice one round
    if (target < 0 || (groups[target].ready_round && groups[target].ready_round > round + 10 + (P_CLUSTER_WAIT > 0 && harmless_now(target, round) ? P_CLUSTER_WAIT + P_CLUSTER_REACH : 0))) {
        target = choose_target(v, -1);
        wait_rounds = 0;
        wait_tank = 0;
        return;
    }
    // A squad group target follows the squad claim when the squad moved on.
    if (is_squad_group(target) && !(cluster_opening(round) && harmless_now(target, round))) {   // EXP-030
        const int claim = read_squad_claim();
        if (claim != target && claim_usable(claim, round, -1)) { target = claim; wait_rounds = 0; wait_tank = 0; }
    }
}
} // namespace va
