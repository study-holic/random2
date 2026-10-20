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
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
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
    int best = -1; double best_score = 1e18;
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
            } else {
                bool taken = false;
                for (int k = 0; k < kSquadClaimByte; ++k) if (k != ally_index && mem[k] == i + 1) taken = true;
                if (taken) continue;
            }
            const int d = min_distance(v.pos, g.tiles);
            const int wait = std::max(0, g.ready_round - round - d);
            double score = d + P_WAIT_WEIGHT * wait - 0.05 * g.gems;
            if (claim_ok && i == my_claim) score -= kClaimBonus;   // squads stay together
            if (round >= P_OWN_HALF_ROUNDS) score += 0.5 * std::max(0, g.dist_ally - g.dist_enemy);
            if (score < best_score) { best_score = score; best = i; }
        }
    }
    if (best >= 0 && is_squad_group(best) && !claim_ok) write_squad_claim(best);
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
        for (int k = 0; k < 8; ++k) if (k != ally_index && mem[k] == i + 1) taken = true;
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
        const int rank = capture_rank(allies, cleared);
        const int next = choose_target(v, cleared);
        bool stay = rank == 0 || next < 0;
        if (!stay && !rows && rank == 1 && groups[cleared].wolves >= 3 && min_distance(groups[cleared].tiles[0], groups[next].tiles) > 5)
            stay = true;   // three-wolf pack with the next group far: the second member also captures
        if (stay) { holding = cleared; hold_since = round; return; }
        groups[cleared].ready_round = round + kRespawnTime;
        if (read_squad_claim() == cleared) write_squad_claim(is_squad_group(next) ? next : -1);
        target = next;
        return;
    }
    if (target >= 0 && groups[target].excluded_until > round) target = -1;
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
