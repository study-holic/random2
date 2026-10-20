// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <span>
#include <string>
#include <variant>
#include <vector>
namespace va {
// s3_contest.cpp (v5 S3): contest plays. Contest's only gem sources are two Monoliths, (17,31) and (32,18), one
// spawn tile each, 20,000 health, strength 200 at range 5, 200 gems, respawn 150 (data/maps21.json, decoded camps).
// v4 dealt them 0 damage against the top 10 (results/v5-s3-diag.md section 6), farmed gold clusters and died 48.7
// times a game. Model (results/v5-s3.md has the formulas):
//  (b) pack siege: the Monolith hits the NEAREST champion, ties alternate (results/arena-1020.md, arena 2), and an
//      asleep camp never attacks. So the team gathers on the 8 tiles at distance 1 before the first hit (asleep:
//      free), then all hit: each tied champion takes 200/k a round. A champion leaves at h < 200*steps_out/k + 100
//      (steps_out = 6 - d when alone, 1 otherwise), heals (recall) and returns at h >= P_S3_ENTER of max; health
//      does not regenerate, so the siege resumes where it stopped (top-10 replays: 190-424 round first sieges).
//      Capture: the ally with the highest health within 6 holds the spawn tile; the bag (200) banks by the default
//      bank() at a centre altar, escorted (shared bytes 6-7, as siege.cpp).
//  (a) interdiction: an enemy channelling a tribute (is_submitting_gems) with >= P_S3_INT_MIN gems in view: place
//      an altar lock if carried and within 2 tiles of its altar (a new lock cancels the channel), else cast a stun or
//      damage ability on it, else attack, else walk to it. A stun or the carrier's death cancels a tribute and a
//      kill transfers the bag (FACTS, Tribute and Death lines); damage alone does not cancel.
// The champion that does neither falls through to v4's behaviour.
#include "s3_contest.h"
#include "bot.h"
#include "util.h"
#include "params.h"

using cambc::vec2;
using cambc::team;

#ifndef P_S3_MONO
#define P_S3_MONO 1           // (b) pack siege on contest's Monoliths
#endif
#ifndef P_S3_INTERDICT
#define P_S3_INTERDICT 1      // (a) stun, lock or kill an enemy tribute channel
#endif
#ifndef P_S3_LOCK
#define P_S3_LOCK 1           // (a) carry one altar lock (10 gold) bought at spawn
#endif
#ifndef P_S3_ESCORT
#define P_S3_ESCORT 1         // (b) walk beside an ally carrying >= 100 gems
#endif
#ifndef P_S3_PACK
#define P_S3_PACK 2           // (b) healthy allies within 10 of the Monolith before the pack goes in (4: STAGE-bound, t26)
#endif
#ifndef P_S3_PACK_WAIT
#define P_S3_PACK_WAIT 40     // (b) rounds staged before a pack of 2 goes in anyway
#endif
#ifndef P_S3_ENTER
#define P_S3_ENTER 0.6f       // (b) health fraction needed to (re-)enter the siege
#endif
#ifndef P_S3_LEAVE_MIN
#define P_S3_LEAVE_MIN 0.2f   // (b) floor of the leave threshold, fraction of max health
#endif
#ifndef P_S3_FOE_R
#define P_S3_FOE_R 5          // (b) an enemy champion this close hands the turn to v4's fight logic
#endif
#ifndef P_S3_INT_MIN
#define P_S3_INT_MIN 30       // (a) smallest enemy bag worth interdicting
#endif
#ifndef P_S3_INT_REACH
#define P_S3_INT_REACH 10     // (a) tiles we walk to reach a channelling carrier
#endif
#ifndef P_S3_INT_HP
#define P_S3_INT_HP 0.35f     // (a) health fraction needed to go for the carrier
#endif
#ifndef P_S3_GEAR
#define P_S3_GEAR 1           // (b) contest build (melee: Jungle then Durability emblems of health) and a gear gate
#endif
#ifndef P_S3_SIEGE_ROUND
#define P_S3_SIEGE_ROUND 250  // (b) an ungeared melee joins the siege from this round anyway
#endif
#ifndef P_S3_TRACE
#define P_S3_TRACE 0
#endif

namespace S3C {
namespace {

struct Mono {
    explicit Mono(const cambc::camp& c) : camp(c) {}
    cambc::camp camp;
    int group = -1;
    vec2 pos{};
    float hp = 20000.0f;
    int ready = 0;      // estimated round it is alive again (0 = assume alive)
    // Host-computed distance fields over walls (cambc::distance_field::compute): exit = steps to a tile at distance
    // >= 6 (out of range 5); ring = steps to a tile at distance 1. Built once, in init.
    cambc::distance_field fexit, fring;
    bool built = false;
    int exit_at(vec2 t) const {
        if (!built || tile_distance(t, pos) >= 6) return 0;
        try { const int v = fexit.get(t); return v == cambc::distance_field::unreachable ? 0 : v; } catch (...) { return 0; }
    }
    int ring_at(vec2 t) const {
        if (!built || tile_distance(t, pos) > 8) return 255;
        try { const int v = fring.get(t); return v >= 255 ? 255 : v; } catch (...) { return 255; }
    }
};

bool g_on = false;
std::vector<Mono> g_m;
vec2 g_home{};
int g_wait = 0;          // rounds waited on the ring for the pack
bool g_healing = false;  // left the siege: heal before re-entering
int g_cur = -1;          // current target index (sticky until it is respawning)
bool g_go = false;       // committed to the current wave (sticky: ally counts flicker with vision through walls)
int g_alone = 0;         // consecutive turns committed with no healthy ally within 10 of the Monolith

void trace(const char* what, int round, const Bot& b, int extra = 0) {
    if (!P_S3_TRACE) return;
    auto& me = cambc::self();
    const auto p = me.position();
    std::cout << "S3 r" << round << " u" << b.ally_index << " " << what << " hp" << static_cast<int>(me.health()) << " g" << me.gem_count()
              << " x" << extra << " p" << p.x << "," << p.y << " ca" << me.can_attack() << me.can_move() << " f" << cambc::fuel_remaining() << '\n';
}

bool cast_stun_or_damage(const cambc::champion& foe) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (int pass = 0; pass < 2; ++pass)   // stuns first (they cancel the channel), then damage
        for (const auto& ab : me.abilities()) {
            try {
                if (!ab.is_ready() || !ab.can_target_enemies()) continue;
                if (pass == 0 ? !ab.is_cc() : !(ab.is_damage() || ab.is_aoe_damage())) continue;
                if (ab.valid_tiles().is_set(foe.position()) && ab.use(foe)) return true;
            } catch (...) {}
        }
    return false;
}

bool place_lock(const Bot& b, vec2 carrier) {
    auto& me = cambc::self();
    try {
        const auto& lock = me.consumables()[cambc::consumable_type::altar_lock];
        if (lock.count() == 0) return false;
        for (auto a : b.altars) {
            if (tile_distance(a, carrier) > cambc::constants::submit_gems_range || tile_distance(a, me.position()) > 2) continue;
            bool ours = false;
            for (const auto& l : cambc::vision::altar_locks(team::ally)) if (l.position() == a) ours = true;
            if (!ours && lock.use(a)) return true;
        }
    } catch (...) {}
    return false;
}

// (a) Interdiction. Returns true when the turn is used.
bool interdict(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies) {
    auto& me = cambc::self();
    if (me.health() < me.max_health() * P_S3_INT_HP || me.gem_count() >= 50) return false;
    const cambc::champion* best = nullptr; int best_d = 1 << 20;
    for (const auto& e : enemies) {
        try {
            if (!e.is_visible() || !e.is_submitting_gems() || e.gem_count() < static_cast<std::uint32_t>(P_S3_INT_MIN)) continue;
            const int d = tile_distance(v.pos, e.position());
            if (d <= P_S3_INT_REACH && d < best_d) { best_d = d; best = &e; }
        } catch (...) {}
    }
    if (!best) return false;
    const cambc::champion foe = *best;
    if (P_S3_LOCK && place_lock(b, foe.position())) { trace("LOCK", v.round, b, best_d); return true; }
    if (cast_stun_or_damage(foe)) { trace("INT-CAST", v.round, b, best_d); attack_if_in_range(foe); return true; }
    if (attack_if_in_range(foe)) { reset_and_attack_again(foe); trace("INT-ATK", v.round, b, best_d); return true; }
    const int reach = std::max(1, static_cast<int>(me.attack_range()));
    auto goals = tiles_around(foe.position(), reach) & ~b.static_block & ~cambc::vision::champion_tiles();
    if (move_towards_closest_nozone(b.static_block, b.camp_static, goals)) {
        if (cast_stun_or_damage(foe) || attack_if_in_range(foe)) {}
        trace("INT-MOVE", v.round, b, best_d);
        return true;
    }
    return false;
}

// Walk to the nearest tile outside the Monolith's range 5 (its zone is not a wall for this move).
// Build both distance fields of Monolith m (host-computed; the first try, a hand-written BFS, coincided with fuel
// exhaustion on its turns).
void build_fields(Mono& m) {
    const auto walls = cambc::map::walls() | cambc::map::holes() | cambc::map::altars();
    auto blocked = walls;
    blocked.set(m.pos, true);
    cambc::bitboard visited;
    const auto out = tiles_around(m.pos, 9) & ~tiles_around(m.pos, 5) & ~walls;
    m.fexit.compute(blocked, out, visited, 16);
    auto ring = tiles_around(m.pos, 1) & ~walls;
    ring.set(m.pos, false);
    m.fring.compute(blocked, ring, visited, 16);
    m.built = true;
}

// Exit step: the free neighbour with the fewest steps out of range (BFS field over walls, built in init); contest's
// pockets have corners where "farther from the Monolith" is a dead end (trace, 20 Oct: (29,15) beside (32,18)).
void exit_range(const Bot& b, const Mono& m) {
    auto& me = cambc::self();
    const vec2 p = me.position();
    if (!me.can_move()) return;
    const auto block = b.static_block | cambc::map::walls() | cambc::vision::champion_tiles() | cambc::vision::camp_tiles();
    vec2 best = p; int bk = m.exit_at(p) * 100 - tile_distance(p, m.pos);
    for (auto t : (tiles_around(p, 1) & ~block).positions()) {
        const int k = m.exit_at(t) * 100 - tile_distance(t, m.pos);
        if (k < bk) { bk = k; best = t; }
    }
    if (!(best == p)) { try { if (me.move_absolute(best)) return; } catch (...) {} }
    move_towards_closest(b.static_block, b.camp_static, b.ally_spawn);
}

using I = cambc::item_type;
int have(I shard, I emblem) {
    const auto owned = cambc::self().item_count();
    return static_cast<int>(owned[shard]) + 5 * static_cast<int>(owned[emblem]);
}
// Gear gate (Pantheon's contest tanks at round 214: Durability emblem of health + Jungle emblem, 1,400-2,200 health,
// 126 damage a Monolith hit instead of 200 = 0.7 x 0.9). Ranged champions stand behind the tanks: no gate.
bool geared() {
    auto& me = cambc::self();
    if (!P_S3_GEAR || me.attack_range() > 1.0f) return true;
    return have(I::jungle_shard_of_health, I::jungle_emblem_of_health) >= 5 && have(I::durability_shard_of_health, I::durability_emblem_of_health) >= 5;
}

// Greedy step down the ring field (towards distance 1) to a free tile; false when no neighbour is better.
bool step_field(const Bot& b, const Mono& m, const cambc::bitboard& occupied) {
    auto& me = cambc::self();
    if (!me.can_move()) return false;
    const vec2 p = me.position();
    const auto block = b.static_block | occupied | cambc::vision::camp_tiles();
    vec2 best = p; int bk = m.ring_at(p) * 4 + tile_distance(p, m.pos);
    for (auto t : (tiles_around(p, 1) & ~block).positions()) {
        const int r = m.ring_at(t);
        if (r == 255) continue;
        const int k = r * 4 + tile_distance(t, m.pos);
        if (k < bk) { bk = k; best = t; }
    }
    if (best == p) return false;
    try { return me.move_absolute(best); } catch (...) { return false; }
}

int group_state(Bot& b, const Mono& m, int& timer) {   // 0 alive, 1 reward available, 2 respawning, -1 unknown
    try {
        const auto s = b.groups[m.group].group.state();
        if (std::holds_alternative<cambc::map::camp_group_reward_available>(s)) return 1;
        if (auto r = std::get_if<cambc::map::camp_group_respawning>(&s)) { timer = r->timer; return 2; }
        return 0;
    } catch (...) { return -1; }
}

// (b) Pack siege. Returns true when the turn is used.
bool siege(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    if (me.gem_count() > 0 || g_m.empty()) return false;
    bool foe_close = false;   // checked after the leave rule: v4's retreat cannot path out of a Monolith zone
    for (const auto& e : enemies) if (tile_distance(e.position(), v.pos) <= P_S3_FOE_R) foe_close = true;

    // Refresh what is in view.
    for (auto& m : g_m) {
        if (!v.visible.is_set(m.pos)) continue;
        try { if (m.camp.is_visible() && m.camp.health() > 0) { m.hp = m.camp.health(); m.ready = 0; continue; } } catch (...) {}
        int timer = 0;
        const int st = group_state(b, m, timer);
        if (st == 2) { m.ready = v.round + timer; m.hp = 20000.0f; }
    }
    // Target: alive by arrival; engaged (lower health) first, then nearer our spawn, then the one allies claim.
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    int best = -1; double best_s = 1e18;
    for (int k = 0; k < static_cast<int>(g_m.size()); ++k) {
        const auto& m = g_m[k];
        const int d = tile_distance(v.pos, m.pos);
        if (m.ready > v.round + d) continue;
        int mates = 0;
        for (int s = 0; s < kSquadClaimByte; ++s) if (s != b.ally_index && mem[s] == m.group + 1) ++mates;
        const bool foes_there = m.group < static_cast<int>(b.foe_seen.size()) && v.round - b.foe_seen[m.group] <= 30;
        const double sc = 30.0 * (m.hp / 20000.0) + tile_distance(g_home, m.pos) - 10.0 * mates + (foes_there ? 40.0 : 0.0) - (k == g_cur ? 15.0 : 0.0);
        if (sc < best_s) { best_s = sc; best = k; }
    }
    // Sticky: keep the current target while it is alive by arrival (ally counts and foe sightings flicker with vision).
    if (g_cur >= 0 && g_cur < static_cast<int>(g_m.size()) && g_m[g_cur].ready <= v.round + tile_distance(v.pos, g_m[g_cur].pos)) best = g_cur;
    g_cur = best;
    if (best < 0) return false;
    auto& m = g_m[best];
    const int d = tile_distance(v.pos, m.pos);
    b.target = m.group;
    const float hp = me.health(), mx = me.max_health();

    // Dead in view with the reward pending: capture.
    if (v.visible.is_set(m.pos)) {
        bool alive = false;
        try { alive = m.camp.is_visible() && m.camp.health() > 0; } catch (...) {}
        if (!alive) {
            int timer = 0;
            const int st = group_state(b, m, timer);
            if (st != 1) { b.target = -1; return false; }
            auto& g = b.groups[m.group];
            if (g.mask.is_set(v.pos)) { trace("CAPHOLD", v.round, b); return true; }
            bool other = false;
            for (const auto& a : allies) {
                if (a == me || !a.is_visible()) continue;
                const int ad = tile_distance(a.position(), m.pos);
                if (ad > 6) continue;
                if (a.health() > hp || (a.health() == hp && b.roster_index(a) < b.ally_index)) { other = true; break; }
            }
            if (!other || d > 6) { move_towards_closest_nozone(b.static_block, b.camp_static, g.mask); trace("CAPGO", v.round, b, d); return true; }
            auto guard = tiles_around(m.pos, 2) & ~g.mask & ~b.static_block;
            if (!guard.is_set(v.pos)) move_towards_closest_nozone(b.static_block, b.camp_static, guard);
            return true;
        }
    }

    if (!geared() && v.round < P_S3_SIEGE_ROUND && !g_healing) { b.target = -1; trace("FARM", v.round, b); return false; }   // farm gold first (v4)
    // Pack: allies (me included) within 10 of the Monolith; others_near: other allies at least as near as me (the
    // Monolith hits the nearest champion, ties alternate).
    int pack = (d <= 10 && hp >= mx * P_S3_ENTER) ? 1 : 0, others_near = 0, nearer = 0, alive_allies = 1;
    for (const auto& a : allies) {
        if (a == me || !a.is_visible()) continue;
        const int ad = tile_distance(a.position(), m.pos);
        if (ad <= 10 && a.health() >= a.max_health() * P_S3_ENTER) ++pack;   // radius > staging band 7-8: no flip-flop
        if (ad <= d) ++others_near;
        if (ad < d) ++nearer;
    }
    for (const auto& a : b.roster) { try { if (!(a == me) && a.is_alive()) ++alive_allies; } catch (...) {} }
    bool awake = false;
    try { awake = m.camp.is_visible() && m.camp.state().is_awake(); } catch (...) {}
    // Leave rule (top-10 sieges: hitters at distance 1, many recalls, 0-4 deaths): the Monolith hits the nearest
    // champion, ties alternate, and walking out of contest's walled pockets takes up to 5 steps. So a hurt champion
    // that is not the nearest recalls in place (5 turns; only damage interrupts it); at distance 1 with another ally
    // also at 1, it steps back one tile first (then it is not the nearest); the last one at distance 1, or the
    // nearest anywhere in range, walks out, so it leaves early: h < 200 * steps_out + 150.
    int at1 = d <= 1 ? 1 : 0;
    for (const auto& a : allies) if (!(a == me) && a.is_visible() && tile_distance(a.position(), m.pos) <= 1) ++at1;
    const bool shielded = d >= 2 ? nearer >= 1 : at1 >= 2;   // someone else is (or after one step back will be) nearer
    auto recall_or_back = [&]() {
        if (d >= 2 && nearer >= 1) { try { me.recall(); trace("RECALL-IN", v.round, b, nearer); return; } catch (...) {} }
        if (d <= 1 && at1 >= 2) {
            auto back = tiles_around(m.pos, 2) & ~tiles_around(m.pos, 1) & tiles_around(v.pos, 1) & ~b.static_block & ~cambc::vision::champion_tiles();
            if (move_towards_closest_nozone(b.static_block, b.camp_static, back)) { trace("STEPBACK", v.round, b, at1); return; }
        }
        exit_range(b, m);
        trace("LEAVE", v.round, b, nearer);
    };
    if (d <= 5 && awake) {
        const float leave = shielded ? std::max(mx * P_S3_LEAVE_MIN, 350.0f) : 200.0f * std::max(1, m.exit_at(v.pos)) + 150.0f;
        if (hp < leave) { g_healing = true; g_go = false; recall_or_back(); return true; }
    }
    if (g_healing || hp < mx * P_S3_ENTER) {
        if (hp >= mx * 0.9f) { g_healing = false; }   // = P_SPAWN_LEAVE_HP: v4 releases the spawn heal-wait at 90 %
        else {
            g_healing = true;
            g_go = false;
            if (b.ally_spawn.is_set(v.pos)) return false;   // v4's spawn heal-wait
            if (d <= 5 && awake) { recall_or_back(); return true; }
            if (d > 5 && min_distance(v.pos, b.spawn_tiles) > 6) {
                bool foe8 = false;
                for (const auto& e : enemies) if (tile_distance(e.position(), v.pos) <= 8) foe8 = true;
                if (!foe8) { try { me.recall(); trace("RECALL", v.round, b); return true; } catch (...) {} }
            }
            // Walk home around the Monolith's range (the way home can pass its pocket: LEAVE/HEAL loops, 20 Oct).
            const auto blk = b.static_block | tiles_around(m.pos, 5);
            if (blk.is_set(v.pos) || !move_towards_closest(blk, b.camp_static, b.ally_spawn)) move_towards_closest(b.static_block, b.camp_static, b.ally_spawn);
            trace("HEAL", v.round, b, d);
            return true;
        }
    }

    if (foe_close) { trace("FOE", v.round, b, d); return false; }   // fights belong to v4's enemy response
    // Entry. The Monolith hits the nearest champion, so walking in while it is awake costs 200 a step to whoever
    // leads. Awake: walk in only behind a healthy ally already at distance 1 (it stays the nearest). Asleep (asleep
    // camps never attack): the pack gathers in its bands (melee at 1, ranged at 2-3) and the first hit waits for
    // the tanks (below).
    const bool ranged = me.attack_range() > 1.0f;
    int tank1 = 0, melee = ranged ? 0 : 1;   // at1 (allies at distance 1, me included) comes from the leave rule
    for (const auto& a : allies) {
        if (a == me || !a.is_visible()) continue;
        const int ad = tile_distance(a.position(), m.pos);
        if (ad <= 1 && a.health() >= a.max_health() * 0.5f) ++tank1;
    }
    for (const auto& a : b.roster) { try { if (!(a == me) && a.is_alive() && a.attack_range() <= 1.0f) ++melee; } catch (...) {} }
    const int need = std::min(P_S3_PACK, alive_allies);
    const bool in_band = ranged ? (d >= 2 && d <= 3) : d <= 1;
    if (!awake && (pack >= need || (g_wait >= P_S3_PACK_WAIT && pack >= 2))) g_go = true;
    g_alone = (g_go && pack <= 1) ? g_alone + 1 : 0;
    if (g_alone >= 3) { g_go = false; g_alone = 0; }
    bool go = awake ? (in_band || tank1 > 0) : (g_go || d <= 3);
    const auto occupied = cambc::vision::champion_tiles();   // one query a turn (fuel)
    if (!go) {
        // Stage at distance 6-8: inside the box, a greedy step on the exit or ring field; outside it, travel.
        if (d < 6) exit_range(b, m);
        else if (d > 8) {
            auto goals = tiles_around(m.pos, 8) & ~tiles_around(m.pos, 5) & ~b.static_block & ~occupied;
            move_towards_closest_nozone(b.static_block, b.camp_static, goals);
        }
        if (tile_distance(me.position(), m.pos) <= 8) ++g_wait;
        trace("STAGE", v.round, b, pack);
        return true;
    }
    // First hit only with the tanks in place: min(2, melee alive) champions at distance 1 (ranged at 2-3 are then
    // never the nearest).
    const bool may_hit = awake || at1 >= std::max(1, std::min(2, melee)) || g_wait >= P_S3_PACK_WAIT;
    if (!awake && d <= 3) ++g_wait;
    // A ranged champion moves OR attacks in a turn (GameRules l.50), so it attacks first when in range. Casts only
    // within 4 (every ability's range here), since each cast check reads valid_tiles (fuel).
    bool hit = false;
    if (may_hit) {
        if (d <= 4) hit = cast_on_camp(m.camp);
        if (ranged && attack_if_in_range(m.camp)) { reset_and_attack_again(m.camp); hit = true; }
    }
    if (!(ranged && hit)) {
        const bool there = ranged ? (d >= 2 && d <= 3) : d <= 1;
        if (!there) {
            if (d > 8 || m.ring_at(v.pos) == 255) {
                auto goals = tiles_around(m.pos, 1) & ~b.static_block & ~occupied;
                goals.set(m.pos, false);
                move_towards_closest_nozone(b.static_block, b.camp_static, goals);
            } else step_field(b, m, occupied);
        }
        if (may_hit) {
            const int d2 = tile_distance(me.position(), m.pos);
            if (!hit && d2 <= 4) hit = cast_on_camp(m.camp);
            if (attack_if_in_range(m.camp)) { reset_and_attack_again(m.camp); hit = true; }
        }
    }
    if (hit) g_wait = 0;
    trace(hit ? "HIT" : may_hit ? "RING" : "GATHER", v.round, b, static_cast<int>(m.hp));
    return true;
}

bool escort(Bot& b, const TurnView& v, std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    std::array<std::uint8_t, 2> cb{};
    cambc::shared_memory.read(kSquadClaimByte, std::span{cb.data(), cb.size()});
    if (me.gem_count() >= 100) {
        const std::array<std::uint8_t, 2> w{static_cast<std::uint8_t>(v.pos.x + 100), static_cast<std::uint8_t>(v.pos.y + 100)};
        cambc::shared_memory.write(kSquadClaimByte, std::span<const std::uint8_t>{w.data(), w.size()});
        return false;
    }
    if (cb[0] < 100 || cb[1] < 100) return false;
    const vec2 cp(cb[0] - 100, cb[1] - 100);
    bool seen = false;
    for (const auto& a : allies) {
        try { if (a.is_visible() && !(a == me) && tile_distance(a.position(), cp) <= 3 && a.gem_count() >= 100) { seen = true; break; } } catch (...) {}
    }
    if (!seen && (v.visible.is_set(cp) || cp == v.pos)) {   // stale: carrier gone, robbed or done
        const std::array<std::uint8_t, 2> z{0, 0};
        cambc::shared_memory.write(kSquadClaimByte, std::span<const std::uint8_t>{z.data(), z.size()});
        return false;
    }
    if (me.health() < me.max_health() * 0.5f || me.gem_count() > 0) return false;
    const int dc = tile_distance(v.pos, cp);
    if (dc > 30) return false;
    if (dc > 2) {
        auto goals = tiles_around(cp, 2) & ~tiles_around(cp, 0) & ~b.static_block;
        move_towards_closest(b.static_block, b.camp_static, goals);
        trace("ESCORT", v.round, b, dc);
        return true;
    }
    return false;   // beside the carrier: v4's fight logic guards
}

}  // namespace

void enable() { g_on = true; }

void init(Bot& b) {
    if (!g_on) return;
    int sx = 0, sy = 0;
    for (auto t : b.spawn_tiles) { sx += t.x; sy += t.y; }
    if (!b.spawn_tiles.empty()) g_home = vec2(sx / static_cast<int>(b.spawn_tiles.size()), sy / static_cast<int>(b.spawn_tiles.size()));
    for (int i = 0; i < static_cast<int>(b.groups.size()); ++i) {
        if (!b.groups[i].monolith) continue;
        for (const auto& c : b.groups[i].group.camps()) {
            if (!c.is_monolith()) continue;
            Mono m(c);
            m.group = i;
            m.pos = c.spawn_position();
            g_m.push_back(m);
        }
    }
}

bool pre(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies) {
    if (!g_on) return false;
    if (P_S3_TRACE >= 2) std::cout << "S3F r" << v.round << " u" << b.ally_index << " pre " << cambc::fuel_remaining() << '\n';
    for (auto& m : g_m)   // one Monolith's fields a turn, while fuel allows
        if (!m.built) { if (v.round >= 2 && cambc::fuel_remaining() > 60000) { build_fields(m); trace("BUILD", v.round, b); } break; }
    if (P_S3_INTERDICT && interdict(b, v, enemies)) return true;
    if (P_S3_ESCORT && escort(b, v, allies)) return true;
    if (P_S3_MONO && siege(b, v, enemies, allies)) return true;
    trace("V4", v.round, b);
    return false;
}

bool buy(Bot& b) {
    (void)b;
    if (!g_on) return false;
    auto& me = cambc::self();
    if (P_S3_LOCK) {
        try {
            if (me.consumables()[cambc::consumable_type::altar_lock].count() == 0 && me.can_buy(cambc::consumable_type::altar_lock))
                me.buy(cambc::consumable_type::altar_lock);
        } catch (...) {}
    }
    if (!P_S3_GEAR || me.attack_range() > 1.0f) return false;   // ranged: v4's build
    // Melee contest build: Jungle shards of health to the emblem (-30 % camp damage), then Durability (-10 % damage),
    // then Power shards of health. Shards cost 30 and five matching combine (FACTS, Shop).
    const std::uint32_t price = I::jungle_shard_of_health.cost();
    int j = have(I::jungle_shard_of_health, I::jungle_emblem_of_health), du = have(I::durability_shard_of_health, I::durability_emblem_of_health);
    std::uint32_t gold = me.gold_count();
    int bj = 0, bd = 0, bp = 0;
    while (gold >= price) {
        if (j < 5) { ++j; ++bj; } else if (du < 5) { ++du; ++bd; } else ++bp;
        gold -= price;
    }
    auto go = [&](I item, int n) { try { if (n > 0 && me.can_buy(item, n)) me.buy(item, n); } catch (...) {} };
    go(I::jungle_shard_of_health, bj);
    go(I::durability_shard_of_health, bd);
    go(I::power_shard_of_health, bp);
    return true;
}

}  // namespace S3C
} // namespace va
