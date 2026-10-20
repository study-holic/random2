#include "util.h"
#include <algorithm>

using cambc::vec2;

int tile_distance(vec2 a, vec2 b) { return static_cast<int>(a.chebyshev_distance(b)); }

cambc::bitboard tiles_around(vec2 position, int radius) {
    cambc::bitboard tiles; tiles.clear(); tiles.draw_chebyshev(position, radius); return tiles;
}

// Strength by camp kind (rules table); avoids a design query per camp.
int camp_strength(const cambc::camp& c) {
    if (c.is_elk()) return 60;
    if (c.is_wolf()) return 20;
    if (c.is_monolith()) return 200;
    return 0;
}

int min_distance(vec2 pos, const std::vector<vec2>& tiles) {
    int d = 1 << 20;
    for (auto t : tiles) d = std::min(d, tile_distance(pos, t));
    return d;
}

bool attack_if_in_range(const cambc::unit& target) {
    auto& me = cambc::self();
    if (!me.can_attack() || !target.is_visible() || target.health() <= 0 || !me.attack_tiles().is_set(target.position()))
        return false;
    me.attack(target);
    return true;
}

// Use an attack reset right after a basic attack so the champion attacks twice.
void reset_and_attack_again(const cambc::unit& target) {
    auto& me = cambc::self();
    if (!me.can_cast()) return;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_attack_reset() || !ab.is_ready()) continue;
        try {
            if (ab.use() && target.is_visible() && target.health() > 0 && me.can_attack()
                && me.attack_tiles().is_set(target.position()))
                me.attack(target);
        } catch (...) {}
        return;
    }
}

// Cast a damaging or stunning spell on a camp in spell range; these spells forbid walking this turn.
bool cast_on_camp(const cambc::camp& camp) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_camps()) continue;
        if (ab.is_cc() && !ab.is_damage()) {
            try { if (ab.damage_amount() <= 0) continue; } catch (...) { continue; }
        }
        try {
            if (ab.valid_tiles().is_set(camp.position()) && ab.use(camp)) return true;
        } catch (...) {}
    }
    return false;
}

bool move_towards_closest(const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                          cambc::bitboard goals, const cambc::bitboard* avoid) {
    auto& me = cambc::self();
    const vec2 pos = me.position();
    if (!me.can_move() || goals.is_set(pos)) return false;
    auto blocked = static_block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles()
        | (camp_static & ~goals) | cambc::vision::teleport_tiles();
    if (avoid != nullptr) blocked |= (*avoid & ~goals);
    blocked.set(pos, false);
    goals &= ~blocked | cambc::vision::camp_tiles();
    if (goals.is_empty()) return false;
    if (goals.count() == 1 && !blocked.is_set(goals.positions()[0])) return me.move_towards(goals.positions()[0]);
    const auto path_walls = blocked & ~goals;
    cambc::distance_field field; cambc::bitboard visited;
    field.compute_to(path_walls, goals, tiles_around(pos, 0), visited);
    auto step = pos;
    for (auto tile : blocked.reachable_tiles(step, me.base_stats().base_move_speed).positions())
        if (field.get(tile) < field.get(step)) step = tile;
    return step != pos && me.move_absolute(step);
}
