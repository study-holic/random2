// Enemy champions (coordinator rules after EXP-013a): v2's attack-in-range stays, focused on one enemy; every member
// near an ally under attack hits the same lowest-health enemy with spells and attacks; a lone champion facing two
// or more enemies walks to the nearest ally or the spawn; never chase more than 2 tiles from the farmed camp.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include <algorithm>

using cambc::vec2;

// Damage or stun spells on an enemy champion in spell range.
bool cast_on_enemy(const cambc::champion& foe) {
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
    for (const auto& a : allies) if (!(a == me) && tile_distance(v.pos, a.position()) <= 4) ++friends4;
    for (const auto& e : enemies) if (tile_distance(v.pos, e.position()) <= 4) ++foes4;

    // Alone and outnumbered: walk to the nearest ally or the spawn, whichever is closer.
    if (friends4 == 0 && foes4 >= 2) {
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

    // Focus: enemies within 2 tiles of this champion or within 3 tiles of an ally within 4 (the ally is under
    // attack); the lowest-health one is the common target.
    const cambc::champion* foe = nullptr;
    for (const auto& e : enemies) {
        if (e.health() <= 0) continue;
        bool threat = tile_distance(v.pos, e.position()) <= 2 || me.attack_tiles().is_set(e.position());
        for (const auto& a : allies)
            if (!threat && !(a == me) && tile_distance(v.pos, a.position()) <= 4 && tile_distance(a.position(), e.position()) <= 3)
                threat = true;
        if (threat && (foe == nullptr || e.health() < foe->health())) foe = &e;
    }
    if (foe == nullptr) return false;

    const bool in_range = me.attack_tiles().is_set(foe->position());
    if (in_range) {
        cast_on_enemy(*foe);
        if (attack_if_in_range(*foe)) { reset_and_attack_again(*foe); return true; }
    }
    // Chase at most 2 tiles, and never further than 2 tiles from the camp being farmed.
    const bool farming = target >= 0 && min_distance(v.pos, groups[target].tiles) <= 4;
    const bool near_camp = !farming || min_distance(foe->position(), groups[target].tiles) <= 3;
    if (!in_range && near_camp && tile_distance(v.pos, foe->position()) <= 2 + std::max(1, static_cast<int>(me.attack_range())) && !retreating) {
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
