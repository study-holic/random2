// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <span>
#include <variant>
#include <vector>
namespace va {
// v5 S1 economy controller (tools/cloud/v5-s1-economy.md): task allocation with time windows by a sequential
// single-item auction over the shared-memory claim bytes (Koenig et al. 2006; Nunes and Gini 2015, TeSSI).
//
// Model. Champion i, task j = capture(group g). Value rate
//   u_ij = [ (G_j(1 - rho) + lambda O_j) / k_j - rho (C_i + B_i) ] / ( tau_ij + max(0, e_j - t - tau_ij) + s_j(k_j) )
//   G_j, O_j  group gems and gold (SDK camp_group values); k_j the coalition the group needs (below); the share 1/k_j
//             is i's part of a joint clear (rewards split among capturers, arena test 4; only one captures here)
//   tau_ij    walking rounds from i to the group (distance field per group, built after init, staged by fuel)
//   e_j       respawn round (refresh_group from vision, or capture round + respawn); 0 = believed alive
//   s_j(k)    clear time HP_j / (k dps_i) + 5 capture rounds (GameRules capture: 5 uncontested rounds)
//   rho       death/failure risk: camp part from the damage the coalition takes while clearing (nearest-champion
//             targeting, arena test 2) against its health; enemy part from enemy champions seen near the group
//   C_i       cost of a death: 60 rounds x income per champion-round (banked / (champions x round), floored at the map
//             prior sum_j G_j / (respawn_j + 5) / (2 x champions)) + lambda x 50 gold to the killer
//   B_i       carried gems (lost on death)
// Coalition size k_j: smallest k with (sum strength_j) x HP_j / (k dps) <= P_S1_RISK_RATIO x k x H_i, i.e. the damage
// taken while k champions clear stays under the ratio of their pooled health; light groups have k = 1.
// Claims: byte a = task id (group + 1, low 6 bits) | ETA bucket << 6 (0: <= 10 rounds, 1: <= 25, 2: <= 50, 3: more).
// Decision rule, on each champion's turn in fixed order: take the best u_ij over at most P_S1_CANDIDATES groups
// (pre-ranked by Chebyshev distance) among groups that are unclaimed, or joinable (claimants < k_j), or claimed with
// an ETA bucket at least 2 worse than ours (outbid); keep the current task unless u_new > P_S1_HYST x u_cur.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <span>
#include <variant>

using cambc::vec2;
using cambc::team;

static int camp_hp(const cambc::camp& c) {   // engine 1.0 designs (FACTS: wolf 1350, elk 2400, clusters 120, monolith 20000)
    if (c.is_wolf()) return 1350;
    if (c.is_elk()) return 2400;
    if (c.is_monolith()) return 20000;
    if (c.is_golem()) return 5000;
    return 120;
}

int eta_bucket(int eta) { return eta <= 10 ? 0 : eta <= 25 ? 1 : eta <= 50 ? 2 : 3; }

// Static per-group numbers and the map prior (called once at the end of init).
void Bot::s1_init() {
    double supply = 0;
    for (auto& g : groups) {
        g.hp_total = 0; g.str_total = 0;
        g.bx0 = g.bx1 = g.tiles[0].x; g.by0 = g.by1 = g.tiles[0].y;
        for (auto t : g.tiles) { g.bx0 = std::min(g.bx0, t.x); g.bx1 = std::max(g.bx1, t.x); g.by0 = std::min(g.by0, t.y); g.by1 = std::max(g.by1, t.y); }
        for (const auto& c : g.group.camps()) { g.hp_total += camp_hp(c); g.str_total += camp_strength(c); }
        g.gold = static_cast<int>(g.group.gold_value());
        g.respawn = static_cast<int>(g.group.max_respawn_timer());
        if (!g.monolith && g.gems > 0) supply += static_cast<double>(g.gems) / (g.respawn + 5);
    }
    const int n = std::max<int>(1, static_cast<int>(roster.size()));
    s1_prior = supply / (2.0 * n);
    s1_tbl.assign(groups.size(), {});
    s1_w = cambc::map::width();
    s1_field_ok.assign(groups.size(), 0);
    s1_foe_round.assign(groups.size(), -1000);
    s1_foe_n.assign(groups.size(), 0);
    s1_tables_next = 0;
}

// Distance tables: one field per group (roots = its spawn tiles), built while this turn has fuel to spare.
void Bot::s1_tables_step() {
    if (!initialised || s1_tables_next >= static_cast<int>(groups.size())) return;
    cambc::bitboard walls = static_block, seen;
    int built = 0;
    while (s1_tables_next < static_cast<int>(groups.size()) && cambc::fuel_remaining() > static_cast<std::uint64_t>(P_S1_TABLE_FUEL)) {
        const int i = s1_tables_next++;
        if (groups[i].monolith) continue;
        if (built++ >= 1) { --s1_tables_next; break; }   // one table per turn (a 100x100 field can cost more than the reserve)
        const auto f0 = cambc::fuel_remaining();
        try {   // computed by the engine, then copied once into wasm memory: lookups are then plain reads (get() is a host call)
            cambc::distance_field f;
            f.compute(walls & ~groups[i].mask, groups[i].mask, seen);
            s1_tbl[i] = f.export_linear_buffer();
            s1_field_ok[i] = 1;
        } catch (...) {}
#if P_S1_FUELDBG
        std::cout << "FT g" << i << " cost" << (f0 - cambc::fuel_remaining()) << std::endl;
#endif
    }
}

int Bot::s1_tau(int i, vec2 pos) const {
    if (s1_field_ok[i]) {
        const std::size_t k = static_cast<std::size_t>(pos.y) * s1_w + pos.x;
        if (k < s1_tbl[i].size()) {
            const int d = s1_tbl[i][k];
            return d != cambc::distance_field::unreachable ? d : (1 << 12);
        }
    }
    return (min_distance(pos, groups[i].tiles) * 5) / 4;   // Chebyshev x 1.25 until the table exists
}

// Enemy sightings near groups (risk term).
void Bot::s1_note_foes(const TurnView& v, const std::vector<cambc::champion>& enemies) {
    if (enemies.empty() || (v.round + ally_index) % 4 != 0 || cambc::fuel_remaining() < 60000) return;   // every 4th round (40-round memory), never on expensive turns
    std::vector<vec2> foes;   // positions read once (each position() is a host call: 34-68k fuel per turn on islands)
    foes.reserve(enemies.size());
    for (const auto& e : enemies) foes.push_back(e.position());
    for (std::size_t i = 0; i < groups.size(); ++i) {
        if (tile_distance(v.pos, groups[i].tiles[0]) > 16 || min_distance(v.pos, groups[i].tiles) > 14) continue;
        int n = 0;
        for (auto f : foes) if (min_distance(f, groups[i].tiles) <= 6) ++n;
        if (n > 0) { s1_foe_round[i] = v.round; s1_foe_n[i] = n; }
    }
}

int Bot::s1_coalition(int i) const {
    const auto& g = groups[i];
    if (g.str_total <= 0) return 1;
    auto& me = cambc::self();
    const double dps = std::max(10.0, static_cast<double>(me.strength()) * P_S1_DPS_MULT);
    const double h = std::max(300.0, static_cast<double>(me.max_health()));
    const double need = g.str_total * static_cast<double>(g.hp_total) / (dps * h * P_S1_RISK_RATIO);   // k^2 >= need
    int k = 1;
    while (k * k < need && k < 8) ++k;
    return k;
}

// u_ij of group i for this champion; `claim` = claimants other than me; eta receives tau + wait. k_fixed > 0 overrides
// the coalition size (P_S1_AUCTION 2: the squad that clears it together).
// Champion and team numbers for u_ij, read once per round (each read is a host call).
void Bot::s1_ctx_refresh(int round) const {
    if (s1_ctx_round == round) return;
    s1_ctx_round = round;
    auto& me = cambc::self();
    s1_ctx_dps = std::max(10.0, static_cast<double>(me.strength()) * P_S1_DPS_MULT);
    s1_ctx_hp = static_cast<double>(me.health());
    s1_ctx_maxhp = static_cast<double>(me.max_health());
    s1_ctx_bag = static_cast<double>(me.gem_count());
    s1_ctx_banked = static_cast<double>(cambc::global_submitted_gems(team::ally));
}

int Bot::s1_eta_of(int i, const TurnView& v) const {
    const int tau = s1_tau(i, v.pos);
    return tau + std::max(0, groups[i].ready_round - v.round - tau);
}

double Bot::s1_value(int i, const TurnView& v, int claim, int& eta, int k_fixed) const {
    const auto& g = groups[i];
    s1_ctx_refresh(v.round);
    const int tau = s1_tau(i, v.pos);
    const int wait = std::max(0, g.ready_round - v.round - tau);
    eta = tau + wait;
    const int k = k_fixed > 0 ? k_fixed : std::max(1, std::max(s1_coalition(i), claim + 1));
    const double dps = s1_ctx_dps;
    const double s = static_cast<double>(g.hp_total) / (dps * k) + 5.0;
    const double h = std::max(300.0, P_S1_RISK_MAXHP ? s1_ctx_maxhp : s1_ctx_hp);   // retreat at 30-45 % already guards a hurt champion
    const double ratio = g.str_total * (s - 5.0) / (h * k);
    double rho_c = std::clamp((ratio - P_S1_RISK_RATIO) / P_S1_RISK_RATIO, 0.0, 1.0);
    double rho_e = 0;
    if (v.round - s1_foe_round[i] <= P_S1_FOE_MEMORY) rho_e = std::min(0.8, 0.25 * s1_foe_n[i]);
    const double rho = 1.0 - (1.0 - rho_c) * (1.0 - rho_e);
    const int nch = std::max<int>(1, static_cast<int>(roster.size()));
    const double banked = s1_ctx_banked;
    const double income = std::max(s1_prior, banked / (nch * std::max(1, v.round)));
    const double cost = 60.0 * income + P_S1_LAMBDA * 50.0;
    const double bag = s1_ctx_bag;
    const double gain = (g.gems * (1.0 - rho) + P_S1_LAMBDA * g.gold) / k - rho * (cost + bag);
    return gain / (P_S1_TRAVEL_W * tau + P_S1_WAIT_W * wait + s);   // travel weighted: a far task also leaves the champion far from the next one
}

// Claimants of group i other than me, and the best (lowest) ETA bucket among them.
int Bot::s1_claimants(const std::array<std::uint8_t, 8>& mem, int i, int& best_bucket) const {
    int n = 0; best_bucket = 4;
    for (int k = 0; k < kSquadClaimByte; ++k) {
        if (k == ally_index || (mem[k] & 0x3F) != i + 1) continue;
        ++n; best_bucket = std::min(best_bucket, mem[k] >> 6);
    }
    return n;
}

// Best group by u among at most P_S1_CANDIDATES candidates; -1 when none has positive value.
int Bot::s1_pick(const TurnView& v, int exclude, double* best_u) {
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    std::array<std::pair<int, int>, 64> cand{};
    int nc = 0;
    for (int i = 0; i < static_cast<int>(groups.size()) && nc < 64; ++i) {
        const auto& g = groups[i];
        if (g.monolith || g.excluded_until > v.round || i == exclude) continue;
        if (g.gems <= 0 && P_S1_LAMBDA * g.gold <= 0) continue;
        const int d = min_distance(v.pos, g.tiles);
        const int wait = std::max(0, g.ready_round - v.round - d);
        cand[nc++] = {d + wait, i};
    }
    const int keep = std::min(nc, static_cast<int>(P_S1_CANDIDATES));
    std::partial_sort(cand.begin(), cand.begin() + keep, cand.begin() + nc);
    int best = -1, best_eta = 0; double bu = 0;
    for (int c = 0; c < keep; ++c) {
        const int i = cand[c].second;
        int bb = 4;
        const int claim = s1_claimants(mem, i, bb);
        int eta = 0;
        const double u = s1_value(i, v, claim, eta) + 0.01 * s1_salt_noise(ally_index, i);
        if (claim > 0 && i != target) {
            const int k = s1_coalition(i);
            const bool joinable = groups[i].heavy && claim < k;
            const bool outbid = eta_bucket(eta) <= bb - 2;
            if (!joinable && !outbid) continue;
        }
        if (u > bu) { bu = u; best = i; best_eta = eta; }
    }
    if (best_u) *best_u = bu;
    if (best >= 0) s1_eta = best_eta;
    return best;
}

// Per-turn re-auction with hysteresis (called from resolve_target when not holding a capture).
void Bot::s1_reauction(const TurnView& v) {
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    double u_cur = -1e9;
    if (target >= 0) {
        int bb = 4;
        const int claim = s1_claimants(mem, target, bb);
        int eta = 0;
        u_cur = s1_value(target, v, claim, eta);
        s1_eta = eta;
        // outbid: another champion holds my light task with an ETA bucket 2 better than mine
        if (claim > 0 && !groups[target].heavy && bb <= eta_bucket(eta) - 2) u_cur = -1e9;
    }
    double u_new = 0;
    const int best = s1_pick(v, -1, &u_new);
    if (best >= 0 && best != target && (target < 0 || u_new > P_S1_HYST * u_cur || u_cur <= 0)) {
        target = best; wait_rounds = 0; wait_tank = 0; assemble_wait = 0;
    } else if (target >= 0) {
        int bb = 4; int eta = 0;
        s1_value(target, v, s1_claimants(mem, target, bb), eta);
        s1_eta = eta;
    }
}

// Capture order among claimants of `group` in view within 8 tiles (lowest capture_key captures).
int Bot::s1_capture_rank(const std::vector<cambc::champion>& allies, int group) const {
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    auto& me = cambc::self();
    int rank = 0;
    for (const auto& a : allies) {
        if (a == me) continue;
        const int idx = roster_index(a);
        if (idx < 0 || capture_key[idx] >= capture_key[ally_index]) continue;
        if ((mem[std::min(idx, kSquadClaimByte - 1)] & 0x3F) != group + 1) continue;
        if (min_distance(a.position(), groups[group].tiles) > 8) continue;
        ++rank;
    }
    return rank;
}

// Assemble before waking a heavy group: wait outside its reach until k_j claimants (or all claimants, at most the team)
// stand within 6 tiles; at most P_S1_ASSEMBLE_WAIT rounds. True when this turn was used to wait.
bool Bot::s1_assemble(const TurnView& v, const std::vector<cambc::champion>& allies) {
    const auto& g = groups[target];
    if (!g.heavy || g.ready_round > v.round) return false;
    for (const auto& c : v.camps) if (c.group == target && awake_tiles().is_set(c.camp.spawn_position())) return false;   // already fighting
    const int k = s1_coalition(target);
    if (k <= 1) return false;
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    auto& me = cambc::self();
    int near = min_distance(v.pos, g.tiles) <= 6 ? 1 : 0, bb = 4;
    const int claim = 1 + s1_claimants(mem, target, bb);
    for (const auto& a : allies) {
        if (a == me) continue;
        const int idx = roster_index(a);
        if (idx < 0 || (mem[std::min(idx, kSquadClaimByte - 1)] & 0x3F) != target + 1) continue;
        if (min_distance(a.position(), g.tiles) <= 6) ++near;
    }
    const int need = std::min(k, claim);   // every claimant, up to the coalition size
    if (near >= need || ++assemble_wait > P_S1_ASSEMBLE_WAIT) return false;
    // stage: within 6 of the group, outside camp reach (4 for elk, +1 margin)
    cambc::bitboard reach; reach.clear();
    for (auto t : g.tiles) reach |= tiles_around(t, 5);
    cambc::bitboard stage; stage.clear();
    for (auto t : g.tiles) stage |= tiles_around(t, 7);
    stage &= ~reach & ~static_block & ~camp_static;
    if (!stage.is_set(v.pos)) move_towards_closest(static_block, camp_static, stage);
    return true;
}
} // namespace va
