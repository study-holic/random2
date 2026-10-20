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
// map_contest.cpp (w9): Monolith siege. Contest's only gem source is its two Monoliths (17,31) and (32,18) (200 gems
// each, 20,000 health; every other camp is a gold cluster), so v2 ends 0-0 on gems. Casters poke them from range 4
// (siege.cpp). P_SIEGE_DRAFT 1 drafts the caster-heavy C&I + Prism + Ryth + Stellar + Tavi (top-50 contest draft,
// 72% over 25 games, intel/meta-2026-10-18.md section 3a); 0 keeps v2's list.
#include "plan.h"
#include "bot.h"
#include "siege.h"
#include "util.h"
#include "params.h"

#ifndef P_SIEGE_CONTEST
#define P_SIEGE_CONTEST 0   // FIX_SIEGE_OFF: engine 1.0 monolith range 5 > siege cast range 4 (125 deaths on the ladder)
#endif
#ifndef P_SIEGE_ESCORT
#define P_SIEGE_ESCORT 1
#endif
#ifndef P_SIEGE_TANK
#define P_SIEGE_TANK 1
#endif
#ifndef P_SIEGE_SAFE
#define P_SIEGE_SAFE 6
#endif
#ifndef P_SIEGE_CASSI_FARMS
#define P_SIEGE_CASSI_FARMS 0
#endif
#ifndef P_SIEGE_DRAFT
#define P_SIEGE_DRAFT 1
#endif

// w1 CONTEST_SIEGE2 (P_CONTEST_SIEGE2): a team siege session on the Monolith nearer our spawn. Real-team contest games
// (data/real/v4-panel micron-A, deerpark-A): the only gems are the two Monoliths (200 each); opponents take them with an
// HP-stacked tank (Ryth 5,400-5,900 max HP) while v4 farms gold and scores 0. Measured rules used here
// (results/arena-1020.md, maps-arena p14): an awake camp hits the NEAREST champion, so melee champions adjacent shield
// the ranged ones at 3; a Monolith does not regenerate (19,950 HP unchanged for 380 rounds), so sessions add up.
// Sessions start every P_SIEGE_PERIOD rounds (every champion sees the same clock: no messages needed); a champion
// joins at >= 90 % health, gathers 7 tiles out, engages, and leaves below P_SIEGE_LEAVE of its health or when fewer
// than 2 allies stay engaged or 2+ enemy champions are within 6.
#ifndef P_CONTEST_SIEGE2
#define P_CONTEST_SIEGE2 0   // off: vs v3b-e1 133 gems a game (v4 67) but 15.7 deaths (v4 9.3); vs v4 contest 0/6 (gold tiebreak)
#endif
#ifndef P_SIEGE_PERIOD
#define P_SIEGE_PERIOD 60
#endif
#ifndef P_SIEGE_FIRST
#define P_SIEGE_FIRST 120   // first session round (gold farming and shards first)
#endif
#ifndef P_SIEGE_LEAVE
#define P_SIEGE_LEAVE 0.35f
#endif
#ifndef P_SIEGE_TANK_HP
#define P_SIEGE_TANK_HP 2500   // melee max health before it joins (opponents tank with 5,400; a base 1,200 Ryth died in 5 hits)
#endif
#ifndef P_SIEGE_HP_BUILD
#define P_SIEGE_HP_BUILD 1     // melee champions buy Power shards of health with all their gold on contest
#endif
#ifndef P_SIEGE_FOES
#define P_SIEGE_FOES 1   // enemy champions within 6 that end a session (2 cost 7.5 champion deaths a game vs v3b-e1)
#endif
#ifndef P_SIEGE_GATHER
#define P_SIEGE_GATHER 25   // rounds a session waits at the gathering tile
#endif

namespace {
enum class S2 { farm, gather, engage, leave };
struct Siege2 {
    int group = -1; cambc::vec2 pos{0, 0}; cambc::bitboard stage; S2 st = S2::farm; int session = -1;
} g2;

int engaged_allies(const std::vector<cambc::champion>& allies, cambc::vec2 m) {
    auto& me = cambc::self(); int n = 0;
    for (const auto& a : allies) { try { if (!(a == me) && a.is_visible() && a.health() > 0 && tile_distance(a.position(), m) <= 4) ++n; } catch (...) {} }
    return n;
}

void s2_init(Bot& b) {
    int best = 1 << 20;
    for (std::size_t i = 0; i < b.groups.size(); ++i) {
        const auto& g = b.groups[i];
        if (!g.monolith) continue;
        for (const auto& c : g.group.camps()) if (c.is_monolith()) {
            const int d = g.dist_ally;
            if (d < best) { best = d; g2.group = static_cast<int>(i); g2.pos = c.spawn_position(); }
        }
    }
    if (g2.group < 0) return;
    // gathering tiles: distance 7 from the Monolith, walkable, the half nearer our spawn
    cambc::bitboard ring = tiles_around(g2.pos, 7) & ~tiles_around(g2.pos, 6) & ~b.static_block & ~b.camp_static;
    int dmin = 1 << 20;
    for (auto t : ring.positions()) dmin = std::min(dmin, min_distance(t, b.spawn_tiles));
    g2.stage.clear();
    for (auto t : ring.positions()) if (min_distance(t, b.spawn_tiles) <= dmin + 4) g2.stage.set(t, true);
    std::cout << "siege2 mono " << g2.pos.x << "," << g2.pos.y << " g" << g2.group << " stage" << g2.stage.count() << '\n';
}

const cambc::camp* s2_mono(const TurnView& v) {
    for (const auto& c : v.camps) if (c.group == g2.group && c.camp.is_monolith()) return &c.camp;
    return nullptr;
}

bool s2_move(Bot& b, const cambc::bitboard& goals) {
    set_mono_zone_skip(true);
    const bool r = move_towards_closest(b.static_block, b.camp_static, goals);
    set_mono_zone_skip(false);
    return r;
}

bool s2_pre(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies) {
    if (g2.group < 0) return false;
    auto& me = cambc::self();
    const auto& g = b.groups[g2.group];
    if (b.holding == g2.group || me.gem_count() > 0) { g2.st = S2::farm; return false; }   // capture / bank: default code
    // reward on the ground: go stand on the Monolith tile (the default capture code takes over once holding)
    bool reward = false;
    if (v.visible.is_set(g2.pos)) { try { reward = std::holds_alternative<cambc::map::camp_group_reward_available>(g.group.state()); } catch (...) {} }
    if (reward && tile_distance(v.pos, g2.pos) <= 8) { b.holding = g2.group; b.target = g2.group; g2.st = S2::farm; return false; }
    const float hp = me.health() / me.max_health();
    int foes6 = 0;
    for (const auto& e : enemies) { try { if (e.health() > 0 && tile_distance(e.position(), v.pos) <= 6) ++foes6; } catch (...) {} }
    const int session = v.round >= P_SIEGE_FIRST ? (v.round - P_SIEGE_FIRST) / P_SIEGE_PERIOD : -1;
    const bool mono_known_dead = g.ready_round > v.round;
    switch (g2.st) {
    case S2::farm:
        if (session >= 0 && session != g2.session && !mono_known_dead && hp >= 0.9f && foes6 == 0 && !b.retreating
            && (!b.is_melee || me.max_health() >= P_SIEGE_TANK_HP)) { g2.st = S2::gather; g2.session = session; }
        else return false;
        [[fallthrough]];
    case S2::gather: {
        if (foes6 >= P_SIEGE_FOES || mono_known_dead) { g2.st = S2::farm; return false; }
        const int since = v.round - (P_SIEGE_FIRST + g2.session * P_SIEGE_PERIOD);
        int near = 0;
        for (const auto& a : allies) { try { if (!(a == me) && a.is_visible() && min_distance(a.position(), std::vector<cambc::vec2>{g2.pos}) <= 8) ++near; } catch (...) {} }
        if (!g2.stage.is_set(v.pos) && tile_distance(v.pos, g2.pos) > 7) { s2_move(b, g2.stage); return true; }
        if (hp < 0.9f) { g2.st = S2::farm; return false; }
        if (since < P_SIEGE_GATHER && near < 3) return true;   // wait for the squad
        if (near < 2 && !b.is_melee) { g2.st = S2::farm; return false; }   // ranged without a squad: give up (a tank with P_SIEGE_TANK_HP goes alone)
        g2.st = S2::engage;
        [[fallthrough]];
    }
    case S2::engage: {
        const bool lone = engaged_allies(allies, g2.pos) < 1 && tile_distance(v.pos, g2.pos) <= 5;
        // leave with enough health to walk out: two Monolith hits plus a margin (measured: 200 a hit), at least P_SIEGE_LEAVE
        // leave with the health to walk out: if no visible champion is as near to the Monolith as we are, every step until
        // distance 6 is a hit (measured: 200 each, nearest champion only); otherwise P_SIEGE_LEAVE of max health.
        const int dm = tile_distance(v.pos, g2.pos);
        bool nearest = true;
        for (auto t : cambc::vision::champion_tiles().positions()) if (!(t == v.pos) && tile_distance(t, g2.pos) <= dm) { nearest = false; break; }
        const float leave_hp = nearest ? std::max(P_SIEGE_LEAVE * me.max_health(), (7 - dm) * 200.0f + 60.0f) : P_SIEGE_LEAVE * me.max_health();
        if (me.health() < leave_hp || foes6 >= P_SIEGE_FOES || mono_known_dead || (lone && !b.is_melee)) { g2.st = S2::leave; }
        else {
            const cambc::camp* m = s2_mono(v);
            if (m == nullptr) { s2_move(b, tiles_around(g2.pos, 3) & ~tiles_around(g2.pos, 0)); return true; }
            const int range = b.is_melee ? 1 : 3;
            if (!b.is_melee) {   // ranged: only behind a melee ally standing next to the Monolith (it is the nearest, so it is hit)
                bool tank = false;
                for (const auto& a : allies) { try { if (!(a == me) && a.is_visible() && a.attack_range() <= 1 && tile_distance(a.position(), g2.pos) <= 1) tank = true; } catch (...) {} }
                if (!tank) { if (tile_distance(v.pos, g2.pos) <= 6) s2_move(b, g2.stage); return true; }
            }
            cambc::bitboard goals = tiles_around(g2.pos, range) & ~tiles_around(g2.pos, range - 1) & ~b.camp_static & ~b.static_block;
            if (!goals.is_set(v.pos) && !me.attack_tiles().is_set(g2.pos)) s2_move(b, goals);
            const bool cast = cast_on_camp(*m);
            if (attack_if_in_range(*m)) reset_and_attack_again(*m);
            (void)cast;
            return true;
        }
        [[fallthrough]];
    }
    case S2::leave:
        // walk out of the Monolith's reach (through its zone if need be); a blocked step or an enemy in reach hands the
        // turn back to the default code (a champion stuck here was killed by an Issac without fighting back)
        if (tile_distance(v.pos, g2.pos) <= 6 && foes6 == 0 && s2_move(b, b.ally_spawn)) return true;
        g2.st = S2::farm; b.retreating = hp < 0.9f; return false;   // out of range: the default retreat heals at the spawn
    }
    return false;
}
bool s2_buy(Bot& b) {
    if (!P_SIEGE_HP_BUILD || !b.is_melee) return false;
    auto& me = cambc::self();
    const auto item = cambc::item_type::power_shard_of_health;
    const unsigned n = me.gold_count() / item.cost();
    if (n == 0) return true;
    try { if (me.can_buy(item, n)) me.buy(item, n); } catch (...) {}
    return true;
}
}  // namespace

namespace Plan {

void setup_contest(MapPlan& p) {
    if (P_CONTEST_SIEGE2) { p.init = s2_init; p.pre = s2_pre; p.buy = s2_buy; return; }
    if (!P_SIEGE_CONTEST) return;
    Siege::Options o; o.on = true; o.escort = P_SIEGE_ESCORT; o.tank_capture = P_SIEGE_TANK; o.safe_tribute = P_SIEGE_SAFE; o.cassi_farms = P_SIEGE_CASSI_FARMS;
    Siege::enable(o);
    using D = cambc::champion_draft;
    if (P_SIEGE_DRAFT == 1) p.draft = {D::stellar, D::tavi, D::prism, D::cassi_and_issac, D::ryth};
    else if (P_SIEGE_DRAFT == 2) p.draft = {D::stellar, D::tavi, D::prism, D::ren, D::willow};
    p.init = Siege::init;
    p.pre = Siege::pre;
    p.mid = Siege::mid;
    p.buy = Siege::buy;
}

}  // namespace Plan
} // namespace va
