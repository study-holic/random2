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
// s2.cpp: S2 quick wins, altar control and the tribute gate (tools/cloud/v5-s2-quickwins.md tasks 2 and 3).
//
// Lock model (deny-the-objective, as a tower-dive denial in MOBA play): a lock placed on altar a cancels every enemy
// tribute channel there and blocks new ones until 3 basic attacks remove it (GameRules l.193-199). Value of placing
// on a:  V(a) = B_a (bag of an enemy channelling at a now, lost time ceil(B_a/10)+3 turns)  >  V_pre(a) = 3 enemy turns
// on the altar they are likely to use. Decision rule: place on a channelling altar within 2 tiles at once; step towards
// one within P_S2_LOCK_CHASE tiles; else place on the pre-lock altar (last seen enemy tribute, otherwise the altar
// nearest the enemy spawn) when passing within 2 tiles. Cost: 10 gold and no action (placement is a consumable use).
//
// Tribute gate (stopping rule): start a channel of T = max(1, ceil(g/10)) turn ends unless an enemy within reach can end
// it: a ready stun in range 4 + 1 step (stun cancels, GameRules l.181), a lock carrier within 3 of the altar, or
// sum of reachable enemy damage x (T + 1) >= my health (death drops the bag). Damage alone never cancels it, so the
// gate never fires for damage below the kill line. After P_S2_GATE_MAX gated turns the tribute starts anyway.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include "s2.h"
#include <algorithm>
#include <vector>

using cambc::vec2;
using cambc::team;

#ifndef P_S2_LOCK_CHASE
#define P_S2_LOCK_CHASE 4            // tiles from an enemy-channelled altar within which a lock carrier walks in
#endif
#ifndef P_S2_GATE_MAX
#define P_S2_GATE_MAX 3              // gated turns in a row before the tribute starts anyway
#endif
#ifndef P_S2_FUEL_FLOOR
#define P_S2_FUEL_FLOOR 45000        // S2 checks are skipped when less fuel than this is left in the turn
#endif
static constexpr int kLockRange = 2;   // GameRules l.193: placed within 2 tiles of an altar

void Bot::s2_buy_lock() {
    if (!P_S2_ALTAR_LOCK || static_cast<int>(cambc::round_number()) < P_S2_LOCK_FROM) return;
    auto& me = cambc::self();
    using C = cambc::consumable_type;
    try { if (me.can_buy(C::altar_lock)) { me.buy(C::altar_lock); s2_has_lock = true; } } catch (...) {}
}

static bool altar_locked(const std::vector<cambc::altar_lock>& locks, vec2 a) {
    for (const auto& l : locks) if (l.position() == a) return true;
    return false;
}

bool Bot::s2_lock_turn(const TurnView& v, const std::vector<cambc::champion>& enemies) {
    if (!P_S2_ALTAR_LOCK || !s2_has_lock || altars.empty()) return false;
    if (cambc::fuel_remaining() < P_S2_FUEL_FLOOR) return false;   // fuel guard: lanes/fracture fights ran out at 100k
    {   // no altar within reach: nothing to place or chase (no SDK calls; the enemy loop cost 8-10k in lanes fights)
        int near = 1 << 20;
        for (auto a : altars) near = std::min(near, tile_distance(v.pos, a));
        if (near > std::max(P_S2_LOCK_CHASE, kLockRange)) return false;
    }
    auto& me = cambc::self();
    // Enemy channels in view: altar index of the nearest one, if any.
    int chan = -1, chan_d = 1 << 20;
    for (const auto& e : enemies) {
        vec2 ep;
        try { ep = e.position(); } catch (...) { continue; }   // one live query per enemy (was one per altar: 12k fuel in lanes fights)
        if (tile_distance(ep, v.pos) > 8) continue;
        for (std::size_t i = 0; i < altars.size(); ++i) {
            if (tile_distance(ep, altars[i]) > cambc::constants::submit_gems_range) continue;
            bool sub = false;
            try { sub = e.is_submitting_gems(); } catch (...) {}
            if (!sub) continue;
            s2_enemy_altar = static_cast<int>(i);
            const int d = tile_distance(v.pos, altars[i]);
            if (d < chan_d) { chan_d = d; chan = static_cast<int>(i); }
        }
    }
    int pre = s2_enemy_altar;
    if (pre < 0) {   // altar nearest the enemy spawn centre
        int best = 1 << 20;
        for (std::size_t i = 0; i < altars.size(); ++i) {
            const int d = tile_distance(altars[i], init_enemy_centre);
            if (d < best) { best = d; pre = static_cast<int>(i); }
        }
    }
    const int goal = (chan >= 0 && chan_d <= P_S2_LOCK_CHASE) ? chan
        : (pre >= 0 && tile_distance(v.pos, altars[pre]) <= kLockRange) ? pre : -1;
    if (P_S2_LOCK_LOG > 1) std::cout << "LKD r" << v.round << " chan" << chan << " d" << chan_d << " pre" << pre << " dp" << (pre >= 0 ? tile_distance(v.pos, altars[pre]) : -1) << '\n';
    if (goal < 0) return false;
    if (tile_distance(v.pos, altars[goal]) <= kLockRange) {
        std::vector<cambc::altar_lock> locks;
        try { locks = cambc::vision::altar_locks(); } catch (...) {}
        if (altar_locked(locks, altars[goal])) return false;
        try {
            const auto& c = me.consumables()[cambc::consumable_type::altar_lock];
            if (c.count() == 0) { s2_has_lock = false; return false; }
            if (c.use(altars[goal])) {
                s2_has_lock = false;
                if (P_S2_LOCK_LOG) std::cout << "LK r" << v.round << (goal == chan ? " chan" : " pre") << '\n';
            }
        } catch (const std::exception& ex) { s2_has_lock = false; if (P_S2_LOCK_LOG > 1) std::cout << "LKX " << ex.what() << '\n'; }
        catch (...) { s2_has_lock = false; }
        return false;   // the placement takes no move or attack: the turn goes on
    }
    // Walk in on a channel within reach (only healthy and not carrying a bag worth more than the denial).
    if (goal == chan && me.health() >= me.max_health() * 0.5f && me.gem_count() < 20 && me.can_move())
        return move_towards_closest(static_block, camp_static, tiles_around(altars[goal], kLockRange));
    return false;
}

// True: do not start the tribute this turn.
bool Bot::s2_tribute_gate(const TurnView& v, const std::vector<cambc::champion>& enemies, vec2 altar, int gems) {
    if (!P_S2_TRIBUTE_GATE || cambc::fuel_remaining() < P_S2_FUEL_FLOOR) return false;
    auto& me = cambc::self();
    const int ticks = std::max(1, (gems + 9) / 10);
    float dmg = 0;
    bool stun = false, lock = false;
    try {
    for (const auto& e : enemies) {
        const vec2 ep = e.position();
        const int d = tile_distance(ep, v.pos);
        if (d <= 5) {
            try {
                for (const auto& ab : e.abilities()) if (ab.is_cc() && ab.cooldown() <= 1) stun = true;
            } catch (...) {}
            if (me.champion_name() == cambc::champion_type::marcella) stun = false;   // stun immune
        }
        if (d <= 4) {
            float f = 0, s = 0;
            try { f = e.focus(); s = e.strength(); } catch (...) {}
            dmg += s + 1.5f * f;
        }
        if (tile_distance(ep, altar) <= kLockRange + 1) {
            bool carries = true;   // unknown inventory counts as carrying (5 of 8 top-10 teams buy locks)
            try { carries = e.consumables()[cambc::consumable_type::altar_lock].count() > 0; } catch (...) {}
            lock |= carries;
        }
    }
    } catch (...) { return false; }
    const bool kill = dmg * static_cast<float>(ticks + 1) >= me.health();
    if (!(stun || lock || kill) || s2_gate_turns >= P_S2_GATE_MAX) { s2_gate_turns = 0; return false; }
    ++s2_gate_turns;
    return true;
}
// P_S2_SPEND_NOW detour: no bag, P_S2_SHOP_GOLD or more gold, the spawn within P_S2_SHOP_DIST and no enemy in view: walk
// in to shop. Cost 2 x distance turns; gain 3+ shards now instead of at the next death or retreat.
bool Bot::s2_shop_detour(const TurnView& v, const std::vector<cambc::champion>& enemies) {
    if (!P_S2_SPEND_NOW || holding >= 0 || retreating || !enemies.empty()) return false;
    auto& me = cambc::self();
    if (me.gem_count() > 0 || me.gold_count() < P_S2_SHOP_GOLD || !me.can_move()) return false;
    if (min_distance(v.pos, spawn_tiles) > P_S2_SHOP_DIST) return false;
    return move_towards_closest(static_block, camp_static, ally_spawn);
}
} // namespace va
