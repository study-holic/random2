// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#ifndef P_MONO_ZONE_ALWAYS
#define P_MONO_ZONE_ALWAYS 1
#endif
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
#include "util.h"
#include "bot.h"
#include "params.h"
#include "siege.h"
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
    const bool tight = Siege::mono_tight();   // w9 siege: a Monolith hits at its range 3 only (no +1 margin)
    for (const auto& c : v.camps)
        if (tile_distance(pos, c.camp.spawn_position()) <= camp_range(c.camp) + ((tight && c.camp.is_monolith()) ? 0 : 1)) sum += camp_strength(c.camp);
    return sum;
}

cambc::bitboard danger_zone(const TurnView& v) {
    cambc::bitboard zone; zone.clear();
    const bool tight = Siege::mono_tight();   // w9 siege: see incoming_damage
    for (const auto& c : v.camps)
        if (camp_strength(c.camp) > 0) zone |= tiles_around(c.camp.spawn_position(), camp_range(c.camp) + ((tight && c.camp.is_monolith()) ? 0 : 1));
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
    DBG(" ATK(hp" + std::to_string(int(target.health())) + ")");
    return true;
}

// Use an attack reset right after a basic attack so the champion attacks twice.
void reset_and_attack_again(const cambc::unit& target) {
    auto& me = cambc::self();
    if (!me.can_cast()) return;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_attack_reset() || !ab.is_ready()) continue;
        try {
            DBG(" RESET");
            if (ab.use() && target.is_visible() && target.health() > 0 && me.can_attack()
                && me.attack_tiles().is_set(target.position()))
                { me.attack(target); DBG(" ATK2"); }
        } catch (...) {}
        return;
    }
}

// P_SAVE_CC: an enemy champion within 8 tiles of this champion.
bool enemy_champion_near() {
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
            if (ab.valid_tiles().is_set(camp.position()) && ab.use(camp)) { DBG(" CAST"); return true; }
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
    if (!camp_near && m3_on(M3_SHIELDENG, me.champion_name()))   // v3b item 3: shield at engage (enemy champion within 4)
        for (auto e : cambc::vision::champion_tiles(cambc::team::enemy).positions())
            if (tile_distance(v.pos, e) <= 4) { camp_near = true; break; }
    if (!camp_near) return;
    for (const auto& ab : me.abilities())
        if (ab.is_shield() && ab.is_ready()) { try { ab.use(); } catch (...) {} break; }
}

// Awake camp tiles in vision, queried once per round (shared by the Monolith zone and P_S1_BANK_REACH).
const cambc::bitboard& awake_tiles() {
    static int zr = -1; static cambc::bitboard awake;
    if (zr != static_cast<int>(cambc::round_number())) { zr = static_cast<int>(cambc::round_number()); awake = cambc::vision::awake_camp_tiles(); }
    return awake;
}

std::vector<std::int8_t> g_camp_rng;   // P_S1_THREAT_COST: reach of the camp spawned on each tile (0 none)
int g_camp_w = 0;
cambc::bitboard g_threat_exempt;       // tiles of the group being fought (its camps are the prey, not a threat)

// Reach of awake damaging camps in vision, minus the exempt group: wolves 3, elks 4, monoliths 5 (arena test 1).
static cambc::bitboard threat_zone() {
    cambc::bitboard z; z.clear();
    if (g_camp_rng.empty()) return z;
    for (auto p : awake_tiles().positions()) {
        const int r = g_camp_rng[p.y * g_camp_w + p.x];
        if (r > 0 && !g_threat_exempt.is_set(p)) z |= tiles_around(p, r);
    }
    return z;
}

std::vector<MonoZone> g_mono_list;
bool g_mono_zone_on = false;

static bool g_zone_skip = false;
static bool move_towards_closest_impl(const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                                      cambc::bitboard goals, bool& zone_used);
// FIX_MONO_ZONE: avoid Monolith ranges when a path exists; when the zone leaves no step, walk through it.
bool move_towards_closest(const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                          cambc::bitboard goals) {
    bool zone_used = false;
    if (move_towards_closest_impl(static_block, camp_static, goals, zone_used)) return true;
    if (P_S1_NO_SELF_BLOCK && P_S1_NSB_WALK && camp_static.is_set(cambc::self().position())) {
        // P_S1_NO_SELF_BLOCK: standing inside a block of spawn tiles (concentrated_wealth group 0 is 5x5), every
        // neighbour is a spawn tile and so a wall: the champion froze there and held the respawn (902 rounds in one
        // game). Walk out over the dead camps' tiles (live camps stay blocked through vision::camp_tiles).
        cambc::bitboard none; none.clear();
        if (move_towards_closest_impl(static_block, none, goals, zone_used)) return true;
    }
    if (!zone_used) return false;
    {   // goals that lie inside a zone stay unwanted: only retry when some goal outside every zone could not be reached
        cambc::bitboard z; z.clear();
        for (const auto& mz : g_mono_list) z |= mz.zone;
        if ((goals & ~z).is_empty()) {
            // P_S1_BANK_REACH: every goal inside a zone (boss_battle's only altar sits within 5 of two Monoliths) is
            // entered when no Monolith covering a goal is awake; asleep camps never attack (arena test 1).
            if (!P_S1_BANK_REACH) return false;
            for (const auto& mz : g_mono_list) if (!(mz.zone & goals).is_empty() && awake_tiles().is_set(mz.pos)) return false;
        }
    }
    g_zone_skip = true;
    const bool r = move_towards_closest_impl(static_block, camp_static, goals, zone_used);
    g_zone_skip = false;
    return r;
}
static bool move_towards_closest_impl(const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                                      cambc::bitboard goals, bool& zone_used) {
    auto& me = cambc::self();
    const vec2 pos = me.position();
    if (!me.can_move() || goals.is_set(pos)) return false;
    auto blocked = static_block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles()
        | (camp_static & ~goals) | cambc::vision::teleport_tiles();
    if (g_mono_zone_on && !g_zone_skip) {   // FIX_MONO_ZONE: a visible AWAKE Monolith's range is a wall (asleep camps do not attack: GameRules, monster details)
        const cambc::bitboard& awake = awake_tiles();
        for (const auto& mz : g_mono_list)
            if ((P_MONO_ZONE_ALWAYS || awake.is_set(mz.pos)) && !mz.zone.is_set(pos)) { blocked |= mz.zone; zone_used = true; }
    }
    if (const auto* sb = Siege::block()) blocked |= *sb;   // w9 siege: never step within 3 of a live Monolith
    blocked.set(pos, false);
    goals &= ~blocked | cambc::vision::camp_tiles();
    if (goals.is_empty()) return false;
    cambc::bitboard threat; threat.clear();
    if (P_S1_THREAT_COST && !g_zone_skip) { threat = threat_zone() & ~goals & ~blocked; threat.set(pos, false); }
    if (threat.is_empty() && goals.count() == 1 && !blocked.is_set(goals.positions()[0])) { const bool ok = me.move_towards(goals.positions()[0]); DBG(" MT1" + std::string(ok ? "ok" : "NO")); return ok; }
    const auto path_walls = blocked & ~goals;
    cambc::distance_field field; cambc::bitboard visited;
    field.compute_to(path_walls, goals, tiles_around(pos, 0), visited);
    auto step = pos;
    bool detour = false;
    if (!threat.is_empty()) {   // P_S1_THREAT_COST: a path outside the threat reach is taken when it costs at most P_S1_DETOUR extra tiles
        cambc::distance_field f2; cambc::bitboard vis2;
        f2.compute_to(path_walls | threat, goals, tiles_around(pos, 0), vis2);
        const int d1 = field.get(pos), d2 = f2.get(pos);
        if (d2 != cambc::distance_field::unreachable && (d1 == cambc::distance_field::unreachable || d2 <= d1 + P_S1_DETOUR)) {
            detour = true;
            for (auto tile : (blocked | threat).reachable_tiles(step, me.base_stats().base_move_speed).positions())
                if (f2.get(tile) < f2.get(step)) step = tile;
        }
    }
    if (!detour)
    for (auto tile : blocked.reachable_tiles(step, me.base_stats().base_move_speed).positions())
        if (field.get(tile) < field.get(step)) step = tile;
    const bool ok = step != pos && me.move_absolute(step);
    DBG(" MV(" + std::to_string(step.x) + "," + std::to_string(step.y) + (ok ? ")ok" : ")NO") + " g" + std::to_string(goals.count()));
    return ok;
}
} // namespace va
