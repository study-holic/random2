// v5 stack: cohesion (leader holds for laggards, followers keep within 2 tiles and never run ahead), the stack bank
// trigger (72 gems in total or 36 on one member) and the escort of non-carriers, plus shared-byte helpers.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include <algorithm>
#include <span>

using cambc::vec2;

static constexpr int kStackBagTotal = 72;
static constexpr int kStackBagOne = 36;
static constexpr int kLeaderHold = 6;     // rounds the leader waits for a laggard
static constexpr int kFollowerHold = 8;   // rounds a follower waits when ahead of the leader
static constexpr int kBankTimeout = 80;

std::uint8_t Bot::shared_byte(int i) const {
    std::uint8_t b = 0;
    cambc::shared_memory.read(i, std::span{&b, 1});
    return b;
}

void Bot::set_shared_byte(int i, std::uint8_t value) {
    cambc::shared_memory.write(i, std::span<const std::uint8_t>{&value, 1});
}

// A stack member we can use as a reference: healthy, or at least not stuck healing on the spawn.
static bool usable_member(const cambc::champion& a, const cambc::bitboard& spawn) {
    if (a.health() < a.max_health() * 0.5f) return false;
    return !(spawn.is_set(a.position()) && a.health() < a.max_health() * 0.9f);
}

// Cohesion while travelling to `dest` (tiles). True when the turn is used (hold, or a step towards the leader).
bool Bot::stay_with_stack(const TurnView& v, const std::vector<cambc::champion>& allies, const std::vector<vec2>& dest) {
    auto& me = cambc::self();
    // The lowest-index healthy member leads; on a bank run the member with the most gems leads.
    int leader = ally_index;
    vec2 leader_pos = v.pos;
    int leader_gems = stack_banking ? static_cast<int>(me.gem_count()) : 0;
    bool laggard = false;
    for (const auto& a : allies) {
        if (a == me) continue;
        const int idx = roster_index(a);
        if (idx < 0 || !usable_member(a, ally_spawn)) continue;
        const int g = stack_banking ? static_cast<int>(a.gem_count()) : 0;
        if (g > leader_gems || (g == leader_gems && idx < leader)) { leader = idx; leader_pos = a.position(); leader_gems = g; }
        if (tile_distance(v.pos, a.position()) > 2) laggard = true;
    }
    if (leader == ally_index) {
        if (!laggard) { wait_cohesion = 0; return false; }
        return ++wait_cohesion <= kLeaderHold;
    }
    if (tile_distance(v.pos, leader_pos) > 2) {
        wait_cohesion = 0;
        move_towards_closest(static_block, camp_static, tiles_around(leader_pos, 1));
        return true;
    }
    if (min_distance(v.pos, dest) < min_distance(leader_pos, dest) && ++wait_cohesion <= kFollowerHold) return true;
    if (min_distance(v.pos, dest) >= min_distance(leader_pos, dest)) wait_cohesion = 0;
    return false;
}

// Stack bank trigger from what this champion sees (the stack stays within a few tiles, so everyone agrees).
void Bot::update_stack_bank(const TurnView& v, const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    int total = static_cast<int>(me.gem_count()), top = total;
    for (const auto& a : allies) {
        if (a == me) continue;
        const int g = static_cast<int>(a.gem_count());
        total += g;
        top = std::max(top, g);
    }
    if (!stack_banking) {
        if (total >= kStackBagTotal || top >= kStackBagOne) { stack_banking = true; bank_since = v.round; }
    } else if (total == 0 || v.round - bank_since > kBankTimeout) {
        stack_banking = false;
    }
}

// Non-carrier during a bank run: shadow the member with the most gems and stay near it while it tributes.
bool Bot::escort(const TurnView& v, const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    const cambc::champion* carrier = nullptr;
    int best = 0;
    for (const auto& a : allies) {
        if (a == me) continue;
        const int g = static_cast<int>(a.gem_count());
        if (g > best) { best = g; carrier = &a; }
    }
    if (carrier == nullptr) return false;
    if (tile_distance(v.pos, carrier->position()) > 2) {
        move_towards_closest(static_block, camp_static, tiles_around(carrier->position(), 1));
    }
    return true;   // within 2 tiles: stay with the carrier
}
