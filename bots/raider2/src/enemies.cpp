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

    if (P_RANGED_KITE) kite_step(v, enemies, static_block, camp_static);
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

// hunter: hunt mode. Start when an enemy champion is visible within 12 tiles and at least 2 allies (this one included)
// stand within 6 tiles of this champion. Target: most carried gems, then lowest health. Keep hunting while an enemy
// was seen in the last 5 rounds (walk to the last known spot when none is visible). Break off below 40 per cent health.
bool Bot::hunt(const TurnView& v, const std::vector<cambc::champion>& enemies, const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    if (me.health() < me.max_health() * 0.4f) { hunting = false; return false; }
    const cambc::champion* prey = nullptr;
    for (const auto& e : enemies) {
        if (e.health() <= 0 || tile_distance(v.pos, e.position()) > 12) continue;
        if (prey == nullptr) { prey = &e; continue; }
        const int ga = carried(e), gb = carried(*prey);
        if (ga != gb ? ga > gb : e.health() < prey->health()) prey = &e;
    }
    if (prey != nullptr) {
        if (!hunting) {
            int near = 0;
            for (const auto& a : allies) if (tile_distance(v.pos, a.position()) <= 6) ++near;   // includes this champion
            if (!is_raider && near < 2) return false;
            hunting = true;
        }
        hunt_seen = v.round;
        hunt_pos = prey->position();
    } else if (!hunting || v.round - hunt_seen > 5) {
        hunting = false;
        return false;
    }
    if (prey == nullptr) {   // out of sight: walk to the last known spot
        if (tile_distance(v.pos, hunt_pos) <= 1) { hunting = false; return false; }
        auto goals = tiles_around(hunt_pos, 1);
        return move_towards_closest(static_block, camp_static, goals);
    }
    const int range = std::max(1, static_cast<int>(me.attack_range()));
    if (!me.attack_tiles().is_set(prey->position())) {
        auto goals = tiles_around(prey->position(), range);
        goals.set(prey->position(), false);
        if (range >= 2) {   // ranged: stand at full range when a free tile exists
            auto ring = goals & ~tiles_around(prey->position(), range - 1);
            if (!ring.is_empty()) goals = ring;
        }
        move_towards_closest(static_block, camp_static, goals);
    }
    if (prey->is_visible() && me.attack_tiles().is_set(prey->position())) {
        cast_on_enemy(*prey);
        if (!prey->is_visible()) return true;   // a spell killed it
        if (attack_if_in_range(*prey)) reset_and_attack_again(*prey);
    }
    return true;
}

// raider2: hunt when prey is in sight, else walk to the next group of the enemy half and wait there (30 rounds), then
// the next. Groups with damaging camps are approached to 4..5 tiles (outside monster range 3 plus margin).
bool Bot::raid(const TurnView& v, const std::vector<cambc::champion>& enemies, const std::vector<cambc::champion>& allies) {
    if (hunt(v, enemies, allies)) return true;
    if (patrol_order.empty()) return false;
    const auto& g = groups[patrol_order[patrol_idx % patrol_order.size()]];
    cambc::bitboard near, far, goals;
    near.clear(); far.clear();
    for (auto t : g.tiles) { near |= tiles_around(t, 3); far |= tiles_around(t, 5); }
    if (g.cluster) goals = g.ring; else goals = far & ~near;
    goals &= ~static_block & ~camp_static;
    if (goals.is_empty()) { ++patrol_idx; return false; }
    if (goals.is_set(v.pos)) {
        if (patrol_since == 0) patrol_since = v.round;
        if (v.round - patrol_since >= 30) { ++patrol_idx; patrol_since = 0; }
        return true;
    }
    patrol_since = 0;
    return move_towards_closest(static_block, camp_static, goals);
}
