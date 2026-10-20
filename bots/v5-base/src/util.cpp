#include "util.h"
#include "bot.h"
#include "params.h"
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

double incoming_damage(const TurnView& v, vec2 pos) {
    double sum = 0;
    for (const auto& c : v.camps)
        if (tile_distance(pos, c.camp.spawn_position()) <= kMonsterRange + 1) sum += camp_strength(c.camp);
    return sum;
}

cambc::bitboard danger_zone(const TurnView& v) {
    cambc::bitboard zone; zone.clear();
    for (const auto& c : v.camps)
        if (camp_strength(c.camp) > 0) zone |= tiles_around(c.camp.spawn_position(), kMonsterRange + 1);
    return zone;
}

void keep_visible(std::vector<cambc::champion>& list) {
    std::vector<cambc::champion> kept;
    kept.reserve(list.size());
    for (const auto& c : list) if (c.is_visible()) kept.push_back(c);
    list = std::move(kept);
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

// P_SAVE_CC: an enemy champion within 8 tiles of this champion.
static bool enemy_champion_near() {
    const cambc::vec2 p = cambc::self().position();
    for (auto e : cambc::vision::champion_tiles(cambc::team::enemy).positions())
        if (tile_distance(p, e) <= 8) return true;
    return false;
}

bool cast_on_camp(const cambc::camp& camp) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_camps()) continue;
        if (P_SAVE_CC && ab.is_cc() && enemy_champion_near()) continue;   // keep the stun for the champion fight
        try {
            if (ab.valid_tiles().is_set(camp.position()) && ab.use(camp)) return true;
        } catch (...) {}
    }
    return false;
}

void cast_shield(const TurnView& v) {
    auto& me = cambc::self();
    if (!me.can_cast() || me.champion_name() != cambc::champion_type::varg) return;
    bool camp_near = me.health() < me.max_health() * static_cast<float>(P_VARG_SHIELD_HP);
    for (const auto& c : v.camps)
        if (camp_strength(c.camp) > 0 && tile_distance(v.pos, c.camp.spawn_position()) <= 4) { camp_near = true; break; }
    if (!camp_near) return;
    for (const auto& ab : me.abilities())
        if (ab.is_shield() && ab.is_ready()) { try { ab.use(); } catch (...) {} break; }
}

bool move_towards_closest(const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                          cambc::bitboard goals) {
    auto& me = cambc::self();
    const vec2 pos = me.position();
    if (!me.can_move() || goals.is_set(pos)) return false;
    auto blocked = static_block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles()
        | (camp_static & ~goals) | cambc::vision::teleport_tiles();
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
