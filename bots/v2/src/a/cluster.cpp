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
#include <variant>
#include <vector>
namespace va {
// EXP-030: cluster-first opening. On maps with three or more gem-cluster groups, for the first 120 rounds every
// champion takes the nearest harmless gem group (pure clusters, or a mixed group seen with only its clusters alive)
// that is alive by arrival, unclaimed and within 25 tiles; wolves and Elks only when none exists. Clusters deal no
// damage, so no squad is needed and champions spread one per group. Banking in the window: 36, or 22 beside an altar.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <cstdint>

static constexpr int kOpeningRounds = P_OPENING_ROUNDS;
static constexpr int kClusterReach = P_CLUSTER_REACH;     // tiles from the champion
static constexpr int kClusterWait = P_CLUSTER_WAIT;       // P_CLUSTER_WAIT: accept a cluster that respawns this long after arrival
static constexpr int kMixedMemory = P_MIXED_MEMORY;       // rounds a "mixed group with only clusters alive" sighting stays valid
static constexpr int kMixedLook = P_MIXED_LOOK;        // mixed groups within this many tiles are checked
static constexpr int kOpeningBankNear = P_OPENING_BANK_NEAR;  // bag that is banked when the nearest altar is within 4 tiles
static constexpr int kOpeningAltarNear = P_OPENING_ALTAR_NEAR;

bool Bot::cluster_opening(int round) const { return cluster_map && round <= kOpeningRounds; }

bool Bot::harmless_now(int index, int round) const {
    const auto& g = groups[index];
    return g.cluster || (g.has_gem_cluster && !g.monolith && round - g.cluster_seen <= kMixedMemory);
}

// A dead camp is not visible while its spawn tile is: a mixed group in full view with no damaging camp visible and
// a gem cluster visible is harmless now.
void Bot::note_mixed_clusters(const TurnView& v) {
    if (!cluster_opening(v.round)) return;
    for (auto& g : groups) {
        if (g.cluster || !g.has_gem_cluster || g.monolith || g.ready_round > v.round) continue;
        if (min_distance(v.pos, g.tiles) > kMixedLook || !(g.mask & ~v.visible).is_empty()) continue;
        bool damaging = false, gem = false;
        for (const auto& c : g.group.camps()) {
            if (!c.is_visible()) continue;
            if (c.is_gem_cluster()) gem = true;
            else if (!c.is_gold_cluster()) damaging = true;
        }
        if (gem && !damaging) g.cluster_seen = v.round;
    }
}

int Bot::choose_cluster(const TurnView& v, int exclude, const std::uint8_t* mem) {
    const int round = v.round;
    int best = -1, best_d = 1 << 20, best_gems = 0;
    bool best_alive = false;
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
        const auto& g = groups[i];
        if (g.monolith || g.excluded_until > round || i == exclude || !harmless_now(i, round)) continue;
        if (contested(i, round)) continue;   // contested-group avoidance kept
        if (P_THREAT_LAYER && leashed(i, round)) continue;   // EXP-071 hard leash
        bool taken = false;
        for (int k = 0; k < kSquadClaimByte; ++k) if (k != ally_index && mem[k] == i + 1) taken = true;
        if (taken) continue;
        const int d = min_distance(v.pos, g.tiles);
        if (d > kClusterReach || g.ready_round > round + d + kClusterWait) continue;   // alive by arrival (or within P_CLUSTER_WAIT after)
        const bool alive = g.ready_round <= round + d;   // alive on arrival wins over distance
        if (best < 0 || (alive && !best_alive) || (alive == best_alive && (d < best_d || (d == best_d && g.gems > best_gems))))
            { best = i; best_d = d; best_gems = g.gems; best_alive = alive; }
    }
    return best;
}

bool Bot::opening_bank(const TurnView& v, int gems) const {
    if (!cluster_opening(v.round) || gems < kOpeningBankNear) return false;
    for (auto a : altars) if (tile_distance(v.pos, a) <= kOpeningAltarNear) return true;
    return false;
}

// EXP-079: opening assignment. At init, the groups on our half that one champion can farm alone (no Monolith, no
// Elk or wolf pack that needs a squad) are sorted by walking distance from our spawn; the champion at roster slot k
// gets the k-th, so no two champions open on the same group. Cluster maps only (lanes and rows keep their openings).
void Bot::init_opening_assign() {
    open_group = -1;
    if (!cluster_map) return;
    const cambc::bitboard terrain = cambc::map::walls() | cambc::map::holes() | cambc::map::altars();
    cambc::distance_field field;
    cambc::bitboard reach;
    field.compute(terrain, ally_spawn, reach);
    std::vector<std::pair<int, int>> order;   // (walking distance, group index)
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
        const auto& g = groups[i];
        if (g.monolith || g.heavy || g.squad_target || g.dist_enemy - g.dist_ally < P_OPENING_ASSIGN_MARGIN) continue;
        int best = 1 << 20;
        for (auto t : g.tiles) {
            for (int dx = -1; dx <= 1; ++dx)   // the spawn tile itself may be blocked: use it or a neighbour
                for (int dy = -1; dy <= 1; ++dy) {
                    const cambc::vec2 q(t.x + dx, t.y + dy);
                    if (q.x < 0 || q.y < 0 || q.x >= cambc::map::width() || q.y >= cambc::map::height() || !reach.is_set(q)) continue;
                    best = std::min(best, static_cast<int>(field.get(q)));
                }
        }
        if (best < (1 << 20)) order.push_back({best, i});
    }
    if (static_cast<int>(order.size()) < P_OPENING_ASSIGN_MIN_GROUPS) return;   // EXP-080/083: too few safe groups, v4-13 logic
    std::sort(order.begin(), order.end());
    if (ally_index < static_cast<int>(order.size())) open_group = order[ally_index].second;
}

// The assigned group while this champion has not collected yet, the round limit has not passed and the group is
// usable (alive by arrival, not excluded, not claimed by another champion, not contested); -1 hands over to the
// normal choice.
int Bot::assigned_group(const TurnView& v, int exclude, const std::uint8_t* mem) {
    const int i = open_group;
    if (i < 0 || open_done || v.round > P_OPENING_ASSIGN_ROUNDS || i == exclude) return -1;
    const auto& g = groups[i];
    if (g.excluded_until > v.round || contested(i, v.round)) return -1;
    if (P_THREAT_LAYER && leashed(i, v.round)) return -1;
    for (int k = 0; k < kSquadClaimByte; ++k) if (k != ally_index && mem[k] == i + 1) return -1;
    if (g.ready_round > v.round + min_distance(v.pos, g.tiles) + kClusterWait) return -1;
    return i;
}
} // namespace va
