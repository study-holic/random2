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
// S2 quick wins (tools/cloud/v5-s2-quickwins.md, results/v5-s2.md): altar locks, the tribute gate and gold spending.
// Every behaviour sits behind its own switch (params.h P_S2_*); with all three at 0 this file changes nothing.
#include <cambc.h>
#include "bot.h"
#include "params.h"
#include "util.h"

using cambc::vec2;
using cambc::team;
using CT = cambc::champion_type;

// Altar nearest the enemy spawn (by Chebyshev distance to the spawn tiles' centre), computed once.
int Bot::s2_enemy_altar() {
    if (s2_enemy_altar_idx != -2) return s2_enemy_altar_idx;
    s2_enemy_altar_idx = -1;
    try {
        long sx = 0, sy = 0, n = 0;
        for (auto t : cambc::map::spawn_region(team::enemy).positions()) { sx += t.x; sy += t.y; ++n; }
        if (n == 0) return -1;
        const vec2 c{static_cast<int>(sx / n), static_cast<int>(sy / n)};
        int best = 1 << 20;
        for (std::size_t i = 0; i < altars.size(); ++i) {
            const int d = tile_distance(c, altars[i]);
            if (d < best) { best = d; s2_enemy_altar_idx = static_cast<int>(i); }
        }
    } catch (...) {}
    return s2_enemy_altar_idx;
}

// P_S2_ALTAR_LOCK (c). Model: a lock is worth the enemy turns it destroys. Value of locking altar i now:
//   V_i = 3 if an enemy is channelling a tribute at i (the channel is cancelled: GameRules l.195, and 3 attacks to clear),
//         2 if an enemy tributed at i within the last 150 rounds (where they bank), 1 if i is the altar nearest their spawn,
//         0 otherwise; place the lock on argmax V_i over unlocked altars within 2 tiles when V > 0.
// Source: task allocation by marginal value (operations research); the "ward the enemy's route" habit of MOBA support play.
// When an enemy is channelling at an altar 3..2+k tiles away (k = its remaining channel turns, ceil(gems/10)) and we are
// healthy, step towards that altar's 2-ring so the lock lands before the tribute completes. Returns true when it moved.
bool Bot::s2_lock(const TurnView& v, const std::vector<cambc::champion>& enemies) {
    if (!P_S2_ALTAR_LOCK || altars.empty()) return false;
    auto& me = cambc::self();
    std::vector<int> foe_trib(altars.size(), 0);   // per altar: max gems of an enemy channelling there (+1), 0 = none
    for (const auto& e : enemies) {
        bool chan = false;
        try { chan = e.is_submitting_gems(); } catch (...) {}
        if (!chan) continue;
        int g = 0;
        try { g = static_cast<int>(e.gem_count()); } catch (...) {}
        for (std::size_t i = 0; i < altars.size(); ++i)
            if (tile_distance(e.position(), altars[i]) <= cambc::constants::submit_gems_range) {
                foe_trib[i] = std::max(foe_trib[i], g + 1); s2_trib_altar = static_cast<int>(i); s2_trib_round = v.round;
            }
    }
    int have = 0;
    try { have = static_cast<int>(me.consumables()[cambc::consumable_type::altar_lock].count()); } catch (...) {}
    if (have <= 0) return false;
    const int ea = s2_enemy_altar();
    int best = -1, best_v = 0, chase = -1, chase_d = 1 << 20;
    for (std::size_t i = 0; i < altars.size(); ++i) {
        const int d = tile_distance(v.pos, altars[i]);
        if (d > 2) {
            const int k = (foe_trib[i] + 8) / 10;   // remaining channel turns, at most ceil(gems / 10)
            if (foe_trib[i] > 0 && d <= 2 + k && d < chase_d) { chase = static_cast<int>(i); chase_d = d; }
            continue;
        }
        const int val = foe_trib[i] > 0 ? 3 : (static_cast<int>(i) == s2_trib_altar && v.round - s2_trib_round <= 150) ? 2
                      : static_cast<int>(i) == ea ? 1 : 0;
        if (val <= best_v) continue;
        bool locked = true;
        try { locked = cambc::vision::has_altar_lock(altars[i]); } catch (...) {}
        if (!locked) { best = static_cast<int>(i); best_v = val; }
    }
    if (best >= 0) {
        try {
            if (me.consumables()[cambc::consumable_type::altar_lock].use(altars[best])) { ++s2_locks_placed; DBG(" S2LOCK"); }
        } catch (...) {}
        return false;   // placing a lock is not the turn's move or attack
    }
    if (chase >= 0 && me.can_move() && me.health() >= me.max_health() * 0.5f) {
        if (move_towards_closest(static_block, camp_static, tiles_around(altars[chase], 2))) { DBG(" S2LOCKCHASE"); return true; }
    }
    return false;
}

// P_S2_TRIBUTE_GATE (d). A started tribute is lost (gems kept, turns wasted) to a stun or a new lock, and the bag is lost
// to a kill; damage alone does not cancel it (GameRules l.181/229). With k = max(1, ceil(gems/10)) channel turns:
//   stun threat: a visible enemy with a ready CC ability within 4 + 1 tiles (cast range 4, one step), and k >= 2;
//   lock threat: the enemy has placed locks this game, an enemy is within 2 + 1 tiles of the altar, and k >= 2;
//   kill threat: sum over visible enemies e of str_e * max(0, k + 1 - max(0, d_e - r_e)) + 2 * focus_e (ready damage
//                ability within 5) >= our health, with no ally adjacent;
//   start the tribute iff none of the three holds. (Expected-loss rule from decision analysis; terms are engine rules.)
bool Bot::s2_tribute_ok(const TurnView& v, const std::vector<cambc::champion>& enemies,
                        const std::vector<cambc::champion>& allies, vec2 altar, int gems) {
    if (!P_S2_TRIBUTE_GATE) return true;
    auto& me = cambc::self();
    const int k = std::max(1, (gems + 9) / 10);
    for (const auto& l : lock_until) if (l > 0) s2_foe_locks = true;
    bool ally_adj = false;
    for (const auto& a : allies) if (!(a == me) && tile_distance(a.position(), v.pos) <= 1) ally_adj = true;
    double dmg = 0;
    for (const auto& e : enemies) {
        if (e.health() <= 0) continue;
        const int d = tile_distance(e.position(), v.pos);
        bool cc_ready = false, dmg_ready = false;
        try {
            for (const auto& ab : e.abilities()) {
                bool ready = false;
                try { ready = ab.cooldown() <= 0; } catch (...) { ready = true; }   // unknown cooldown: assume ready
                if (ready && ab.is_cc()) cc_ready = true;
                if (ready && (ab.is_damage() || ab.is_aoe_damage())) dmg_ready = true;
            }
        } catch (...) {}
        if (k >= 2 && cc_ready && d <= 5) { DBG(" S2GATE-STUN"); return false; }
        if (k >= 2 && s2_foe_locks && tile_distance(e.position(), altar) <= 3) { DBG(" S2GATE-LOCK"); return false; }
        int r = 1;
        try { r = std::max(1, static_cast<int>(e.attack_range())); } catch (...) {}
        dmg += e.strength() * std::max(0, k + 1 - std::max(0, d - r));
        if (dmg_ready && d <= 5) dmg += 2.0 * e.focus();
    }
    if (!ally_adj && dmg >= me.health()) { DBG(" S2GATE-KILL"); return false; }
    return true;
}

// P_S2_SPEND_NOW: recall to shop when the gold completes the current emblem (or buys 5 shards). Model: the shop trip costs
// T = 5 recall turns (+ the walk back), and buys g = floor(gold / 30) shards of permanent stat growth; trip iff
//   gold >= 30 * max(2, shards left to the emblem) and the recall cannot be interrupted (no enemy within 8, no incoming
//   damage), nothing is carried (a bag goes to the altar first), and the last trip was 40+ rounds ago.
// Source: MOBA "back timing" (recall when the gold buys a completed item); recall/shop rules FACTS.md.
bool Bot::s2_shop_recall(const TurnView& v, const std::vector<cambc::champion>& enemies, double incoming, bool at_spawn) {
    if (!P_S2_SPEND_NOW || retreating || holding >= 0 || incoming > 0 || remote_shop) { s2_walk_until = 0; return false; }
    if (at_spawn) { s2_walk_until = 0; return false; }
    auto& me = cambc::self();
    if (me.gem_count() > 0 || v.round - s2_last_trip < 40 || v.round >= 1850) return false;
    if (v.round < s2_walk_until) {   // walking a near trip (spawn within 8): keep going unless an enemy closes in
        for (const auto& e : enemies) if (tile_distance(e.position(), v.pos) <= 5) { s2_walk_until = 0; return false; }
        if (move_towards_closest(static_block, camp_static, ally_spawn)) return true;
        s2_walk_until = 0; return false;
    }
    if (me.gem_count() > 0 || v.round - s2_last_trip < 40 || v.round >= 1850) return false;
    for (const auto& e : enemies) if (tile_distance(e.position(), v.pos) <= 8) return false;
    const int shards = static_cast<int>(me.gold_count() / 30);
    if (shards < std::max(2, s2_to_emblem) && shards < 5) return false;
    s2_last_trip = v.round;
    const int home = min_distance(v.pos, spawn_tiles);
    if (home < 8) {   // close: walking is no slower than the 5-turn recall and cannot be interrupted by damage
        s2_walk_until = v.round + home + 2; DBG(" S2SHOPWALK");
        return move_towards_closest(static_block, camp_static, ally_spawn);
    }
    try { me.recall(); } catch (...) { return false; }
    DBG(" S2SHOP");
    return true;
}
} // namespace va
