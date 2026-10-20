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
// Enemy champions (coordinator rules after EXP-013a): v2's attack-in-range stays, focused on one enemy; every member
// near an ally under attack hits the same lowest-health enemy with spells and attacks; a lone champion facing two
// or more enemies walks to the nearest ally or the spawn; never chase more than 2 tiles from the farmed camp.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>

using cambc::vec2;

static constexpr int kFleeFoes = P_FLEE_FOES, kFleeRadius = P_FLEE_RADIUS;
static constexpr int kChaseTiles = P_CHASE_TILES, kChaseFromCamp = P_CHASE_FROM_CAMP;
static constexpr float kEngageMinHp = static_cast<float>(P_ENGAGE_MIN_HP);

// P_FOCUS_RULE: is `a` a better focus target than `b`? 0 lowest health; 1 most carried gems (tie lowest health);
// 2 nearest (tie lowest health).
static int carried(const cambc::champion& c) {
    try { return static_cast<int>(c.gem_count()); } catch (...) { return 0; }
}
static bool better_focus(const cambc::champion& a, const cambc::champion& b, vec2 pos) {
    if (P_FOCUS_RULE == 1) {
        const int ga = carried(a), gb = carried(b);
        if (ga != gb) return ga > gb;
    } else if (P_FOCUS_RULE == 2) {
        const int da = tile_distance(pos, a.position()), db = tile_distance(pos, b.position());
        if (da != db) return da < db;
    }
    return a.health() < b.health();
}

// P_RANGED_KITE: a ranged champion beside an enemy melee champion steps to a free tile at distance 2 to 3 from it
// (outside camp danger zones) before attacking. Returns true when it moved.
static bool kite_step(const TurnView& v, const std::vector<cambc::champion>& enemies, const cambc::bitboard& static_block,
                      const cambc::bitboard& camp_static) {
    auto& me = cambc::self();
    if (me.attack_range() < 2 || !me.can_move()) return false;
    cambc::bitboard goals = tiles_around(v.pos, 1);
    bool adjacent = false;
    for (const auto& e : enemies) {
        if (e.health() <= 0 || e.attack_range() > 1) continue;
        const int d = tile_distance(v.pos, e.position());
        if (d <= 1) {
            adjacent = true;
            goals &= tiles_around(e.position(), 3);
        }
        goals &= ~tiles_around(e.position(), 1);
    }
    if (!adjacent) return false;
    goals &= ~static_block & ~camp_static & ~danger_zone(v) & ~cambc::vision::champion_tiles() & ~cambc::vision::camp_tiles();
    goals.set(v.pos, false);
    if (goals.is_empty()) return false;
    return move_towards_closest(static_block, camp_static, goals);
}

// Damage or stun spells on an enemy champion in spell range.
static bool cast_on_enemy(const cambc::champion& foe) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_enemies()) continue;
        try {
            if (ab.valid_tiles().is_set(foe.position()) && ab.use(foe)) return true;
        } catch (...) {}
    }
    return false;
}

bool Bot::enemy_response(const TurnView& v, const std::vector<cambc::champion>& enemies,
                         const std::vector<cambc::champion>& allies) {
    if (enemies.empty()) return false;
    auto& me = cambc::self();
    int friends4 = 0, foes4 = 0;
    for (const auto& a : allies) if (!(a == me) && tile_distance(v.pos, a.position()) <= kFleeRadius) ++friends4;
    for (const auto& e : enemies) if (tile_distance(v.pos, e.position()) <= kFleeRadius) ++foes4;

    // Alone and outnumbered: walk to the nearest ally or the spawn, whichever is closer.
    if (friends4 == 0 && foes4 >= kFleeFoes) {
        int best = min_distance(v.pos, spawn_tiles);
        cambc::bitboard goals = ally_spawn;
        for (const auto& a : allies) {
            if (a == me) continue;
            const int d = tile_distance(v.pos, a.position());
            if (d < best) { best = d; goals = tiles_around(a.position(), 1); }
        }
        move_towards_closest(static_block, camp_static, goals);
        return true;
    }

    // P_ENGAGE_MIN_HP: too hurt to fight back: retreat mode and walk home (the fight goes on if no step exists).
    if (kEngageMinHp > 0.0f && me.health() < me.max_health() * kEngageMinHp && !ally_spawn.is_set(v.pos)) {
        retreating = true;
        holding = -1;
        if (move_towards_closest(static_block, camp_static, ally_spawn)) return true;
    }

    // Focus: enemies within 2 tiles of this champion or within 3 tiles of an ally within 4 (the ally is under
    // attack); the lowest-health one is the common target.
    const cambc::champion* foe = nullptr;
    for (const auto& e : enemies) {
        if (e.health() <= 0) continue;
        bool threat = tile_distance(v.pos, e.position()) <= 2 || me.attack_tiles().is_set(e.position());
        for (const auto& a : allies)
            if (!threat && !(a == me) && tile_distance(v.pos, a.position()) <= 4 && tile_distance(a.position(), e.position()) <= 3)
                threat = true;
        if (threat && (foe == nullptr || better_focus(e, *foe, v.pos))) foe = &e;
    }
    if (foe == nullptr) return false;

    const auto m3t = me.champion_name();
    const bool foe_vend = foe->champion_name() == cambc::champion_type::vendetta;
    const int m3d = tile_distance(v.pos, foe->position());
    // v3b item 4: dashes in fights. Ranged (Vesper): adjacent melee foe -> dash to distance >= 3, then attack.
    if (m3_on(M3_DASH, m3t) && me.attack_range() > 1 && foe->attack_range() <= 1 && m3d <= 1 && me.can_cast()) {
        for (const auto& ab : me.abilities()) {
            if (!ab.is_dash() || !ab.is_ready()) continue;
            cambc::bitboard goals = ab.valid_tiles() & ~tiles_around(foe->position(), 2) & ~static_block & ~camp_static & ~danger_zone(v) & ~cambc::vision::champion_tiles() & ~cambc::vision::camp_tiles();
            goals &= tiles_around(foe->position(), 3);
            vec2 bestt = v.pos; int bd = -1;
            for (auto t : goals.positions()) { const int dd = tile_distance(t, foe->position()); if (dd > bd) { bd = dd; bestt = t; } }
            if (bd >= 3) { try { if (ab.use(bestt)) { if (attack_if_in_range(*foe)) reset_and_attack_again(*foe); return true; } } catch (...) {} }
            break;
        }
    }
    if ((P_RANGED_KITE || m3_on(M3_KITE, m3t)) && !(P_M3 && !(P_M3_OFF & M3_KITE) && foe_vend)) kite_step(v, enemies, static_block, camp_static);
    // v3b item 2: damage/stun spells from full range before moving.
    if (m3_on(M3_EARLYCAST, m3t)) cast_on_enemy(*foe);
    // v3b item 4: melee with a dash, foe 2..3 away -> dash next to it.
    if (m3_on(M3_DASH, m3t) && me.attack_range() <= 1 && m3d >= 2 && m3d <= 3 && me.can_cast()) {
        for (const auto& ab : me.abilities()) {
            if (!ab.is_dash() || !ab.is_ready()) continue;
            cambc::bitboard goals = ab.valid_tiles() & tiles_around(foe->position(), 1) & ~static_block & ~camp_static & ~danger_zone(v) & ~cambc::vision::champion_tiles() & ~cambc::vision::camp_tiles();
            for (auto t : goals.positions()) { try { if (ab.use(t)) break; } catch (...) {} }
            break;
        }
    }
    const bool in_range = me.attack_tiles().is_set(foe->position());
    bool foe_gone = false;
    if (in_range) {
        cast_on_enemy(*foe);
        if (attack_if_in_range(*foe)) { reset_and_attack_again(*foe); return true; }
        if (P_ALLY_VISION_FIX) foe_gone = !foe->is_visible();   // a spell killed it: its handle now throws 97
    }
    // Chase at most 2 tiles, and never further than 2 tiles from the camp being farmed.
    const bool farming = target >= 0 && min_distance(v.pos, groups[target].tiles) <= 4;
    const bool near_camp = foe_gone || !farming || min_distance(foe->position(), groups[target].tiles) <= kChaseFromCamp;
    if (!foe_gone && !in_range && near_camp && tile_distance(v.pos, foe->position()) <= kChaseTiles + std::max(1, static_cast<int>(me.attack_range())) && !retreating) {
        const int range = std::max(1, static_cast<int>(me.attack_range()));
        auto goals = tiles_around(foe->position(), range);
        goals.set(foe->position(), false);
        if (move_towards_closest(static_block, camp_static, goals)) {
            if (attack_if_in_range(*foe)) reset_and_attack_again(*foe);
            return true;
        }
    }
    // v2: anything in range.
    for (const auto& e : enemies)
        if (attack_if_in_range(e)) { reset_and_attack_again(e); return true; }
    return false;
}
} // namespace va
