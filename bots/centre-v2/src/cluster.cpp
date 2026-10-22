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
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
        const auto& g = groups[i];
        if (g.monolith || g.excluded_until > round || i == exclude || !harmless_now(i, round)) continue;
        if (centre_map && !centre_group[i]) continue;   // EXP-073
        if (!centre_map && contested(i, round)) continue;   // contested-group avoidance kept
        bool taken = false;
        for (int k = 0; k < kSquadClaimByte; ++k) if (k != ally_index && mem[k] == i + 1) taken = true;
        if (taken) continue;
        const int d = min_distance(v.pos, g.tiles);
        if (d > kClusterReach || g.ready_round > round + d) continue;   // alive by the time we arrive
        if (d < best_d || (d == best_d && g.gems > best_gems)) { best = i; best_d = d; best_gems = g.gems; }
    }
    return best;
}

bool Bot::opening_bank(const TurnView& v, int gems) const {
    if (!cluster_opening(v.round) || gems < kOpeningBankNear) return false;
    for (auto a : altars) if (tile_distance(v.pos, a) <= kOpeningAltarNear) return true;
    return false;
}
