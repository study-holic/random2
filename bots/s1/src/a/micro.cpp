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
// Farming micro: the ring wait (v3-quick give-up), squad positions (melee adjacent, ranged at distance 3 once a
// melee ally is adjacent, everyone on the weakest camp, stuns and damage spells cast when ready), and v2's solo play.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

using cambc::vec2;
using cambc::team;

static constexpr int kGiveUpWait = P_GIVE_UP_WAIT;     // v3-quick: rounds at a respawn ring before giving up
static constexpr int kTankWait = P_TANK_WAIT;       // ranged: rounds to wait for a melee ally to engage

// v2: attack camps within reach (spell range is 4, so farther camps are skipped before any ability query).
// FIX_FIGHT_CACHE (P_FIGHT_CACHE): same decisions as cast_on_camp per camp, but each ability's valid_tiles() and the
// P_SAVE_CC enemy check are queried once per call instead of once per camp (farm_solo peaked at 76k fuel).
#ifndef P_FIGHT_CACHE
#define P_FIGHT_CACHE 1
#endif
static bool fight_any(const TurnView& v) {
#if P_FIGHT_CACHE
    auto& me = cambc::self();
    const bool can_cast = me.can_cast();
    std::array<cambc::bitboard, 2> valid{};
    std::array<bool, 2> usable{false, false};
    bool prepared = false;
    const auto abs = me.abilities();
    for (const auto& c : v.camps) {
        if (tile_distance(v.pos, c.camp.spawn_position()) > 4) continue;
        if (can_cast) {
            if (!prepared) {
                prepared = true;
                int near = -1;   // enemy_champion_near(), queried only when a ready CC ability needs it
                for (std::size_t i = 0; i < abs.size(); ++i) {
                    const auto& ab = abs[i];
                    if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_camps()) continue;
                    if (P_SAVE_CC && ab.is_cc()) { if (near < 0) near = enemy_champion_near() ? 1 : 0; if (near) continue; }
                    try { valid[i] = ab.valid_tiles(); usable[i] = true; } catch (...) {}
                }
            }
            bool cast = false;
            for (std::size_t i = 0; i < abs.size() && !cast; ++i) {
                if (!usable[i] || !valid[i].is_set(c.camp.position())) continue;
                try { if (abs[i].use(c.camp)) { DBG(" CAST"); cast = true; } } catch (...) {}
            }
            if (cast) { attack_if_in_range(c.camp); return true; }
        }
        if (attack_if_in_range(c.camp)) { reset_and_attack_again(c.camp); return true; }
    }
    return false;
#else
    for (const auto& c : v.camps) {
        if (tile_distance(v.pos, c.camp.spawn_position()) > 4) continue;
        if (cast_on_camp(c.camp)) { attack_if_in_range(c.camp); return true; }
        if (attack_if_in_range(c.camp)) { reset_and_attack_again(c.camp); return true; }
    }
    return false;
#endif
}

bool Bot::engaged_at_squad_target(const TurnView& v) const {
    if (!is_melee || !is_squad_group(target)) return false;
    for (const auto& c : v.camps)
        if (c.group == target && tile_distance(v.pos, c.camp.spawn_position()) <= 1) return true;
    return false;
}

// Cohesion: while travelling to a squad group, hold when a squad member bound for the same group is 4 to 12 tiles
// away and further from it, so a lone enemy meets the whole squad. At most 6 rounds in a row.
bool Bot::wait_for_squad(const TurnView& v, const std::vector<cambc::champion>& allies) {
    const int mine = min_distance(v.pos, groups[target].tiles);
    if (mine <= 5) { wait_cohesion = 0; return false; }
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    auto& me = cambc::self();
    bool behind = false;
    for (const auto& a : allies) {
        const int idx = roster_index(a);
        if (idx < 0 || a == me || roster_squad[idx] != squad || (mem[std::min(idx, kSquadClaimByte - 1)] & 0x3F) != target + 1) continue;
        const int gap = tile_distance(v.pos, a.position());
        if (gap >= 4 && gap <= 12 && min_distance(a.position(), groups[target].tiles) > mine + 2) { behind = true; break; }
    }
    if (!behind || ++wait_cohesion > P_COHESION_MAX) { if (!behind) wait_cohesion = 0; return false; }
    return true;
}

void Bot::farm(const TurnView& v, const std::vector<cambc::champion>& allies) {
    if (rows_sched) { farm_rows(v, allies); return; }   // EXP-093
    leave_contested(v);   // EXP-018
    if (target < 0) { fight_any(v); return; }
    const auto& g = groups[target];
    if (holding == target) {   // capture: stand on a spawn tile until the reward is paid
        if (!g.mask.is_set(v.pos)) move_towards_closest(static_block, camp_static, g.mask);
        return;
    }
    if (g.ready_round > 0 && g.ready_round > v.round - 3) {
        // Respawning: an occupied spawn tile delays the whole group's respawn, so wait beside the tiles. Give up after
        // 12 rounds, or at once when an enemy champion is within 6 tiles of the group (v3-quick).
        ++wait_rounds;
        bool contested = false;
        for (auto e : cambc::vision::champion_tiles(team::enemy).positions())
            if (min_distance(e, g.tiles) <= 6) { contested = true; break; }
        // P_CLUSTER_WAIT: a harmless cluster is waited for up to the wait plus the walk (wait_rounds counts the walk too).
        const int give_up = (P_CLUSTER_WAIT > 0 && harmless_now(target, v.round)) ? P_CLUSTER_WAIT + P_CLUSTER_REACH : kGiveUpWait;
        if (wait_rounds > give_up || contested) {
            exclude_group(target, v.round);
            wait_rounds = 0;
            target = choose_target(v, -1);
            return;
        }
        if (!g.ring().is_set(v.pos)) move_towards_closest(static_block, camp_static, g.ring());
        fight_any(v);
        return;
    }
    wait_rounds = 0;
    if (!is_squad_group(target)) { farm_solo(v, nullptr); return; }
    if (wait_for_squad(v, allies)) return;

    // Prey: the weakest live camp that can hurt us; harmless clusters only when nothing else is left.
    const LiveCamp* prey = nullptr;
    for (const auto& c : v.camps) {
        if (c.group != target) continue;
        const bool hurts = camp_strength(c.camp) > 0;
        const bool prey_hurts = prey != nullptr && camp_strength(prey->camp) > 0;
        if (prey == nullptr || hurts > prey_hurts || (hurts == prey_hurts && c.health < prey->health)) prey = &c;
    }
    farm_squad(v, allies, prey);
}

// P_TANK_ROTATE_HP: this melee champion is below the rotate fraction and a melee squad mate within 2 tiles has a health
// fraction at least 0.3 higher.
bool Bot::fresher_mate_near(const TurnView& v, const std::vector<cambc::champion>& allies) const {
    auto& me = cambc::self();
    const float mine = me.health() / me.max_health();
    if (mine >= static_cast<float>(P_TANK_ROTATE_HP)) return false;
    for (const auto& a : allies) {
        const int idx = roster_index(a);
        if (idx < 0 || a == me || roster_squad[idx] != squad || !roster_melee[idx]) continue;
        if (tile_distance(a.position(), v.pos) > 2) continue;
        if (a.health() / a.max_health() >= mine + 0.3f) return true;
    }
    return false;
}

void Bot::farm_squad(const TurnView& v, const std::vector<cambc::champion>& allies, const LiveCamp* prey) {
    auto& me = cambc::self();
    const auto& g = groups[target];
    if (prey == nullptr) {   // camps out of view: approach without stepping on the spawn tiles
        if (!g.ring().is_set(v.pos)) move_towards_closest(static_block, camp_static, g.ring());
        return;
    }
    const vec2 pp = prey->camp.spawn_position();
    cambc::bitboard live; live.clear();
    for (const auto& c : v.camps) live.set(c.camp.spawn_position(), true);
    cambc::bitboard goals;
    bool engaged = true;
    if (is_melee) {
        goals = tiles_around(pp, 1) & ~g.mask & ~static_block;   // off the spawn tiles so the capture is not split
        if (goals.is_empty()) goals = tiles_around(pp, 1) & ~live;
        if (P_TANK_ROTATE_HP > 0.0 && camp_strength(prey->camp) > 0 && fresher_mate_near(v, allies)) {
            // P_TANK_ROTATE_HP: hurt tank yields the adjacent ring to a fresher melee mate (camps hit the adjacent one).
            auto off = (tiles_around(pp, camp_range(prey->camp)) & ~tiles_around(pp, 1)) & ~g.mask & ~static_block;
            if (!off.is_empty()) goals = off;
        }
    } else {
        engaged = false; if (true) { engaged = true; }
        for (const auto& a : allies)
            if (!(a == me) && tile_distance(a.position(), pp) <= 1) { engaged = true; break; }
        if (engaged) {
            wait_tank = 0;
            const int range = rows_sched ? 3 : std::max(1, static_cast<int>(me.attack_range()));   // EXP-095: rows ranged stand at 3 (the attack guard below is 4)
            goals = (tiles_around(pp, range) & ~tiles_around(pp, range - 1)) & ~g.mask & ~static_block;
            cambc::bitboard others; others.clear();   // out of reach of the other live monsters when possible
            for (const auto& c : v.camps)
                if (&c != prey && camp_strength(c.camp) > 0) others |= tiles_around(c.camp.spawn_position(), camp_range(c.camp));
            if (!(goals & ~others).is_empty()) goals &= ~others;
#if P_STAND_SIGHT
            if (goals.count() <= 12) {   // FIX_STAND_SIGHT: keep stand tiles that see the prey (cost capped)
                cambc::bitboard seen; seen.clear();
                for (auto t : goals.positions()) { try { if (me.visible_tiles_from(t).is_set(pp)) seen.set(t, true); } catch (...) {} }
                goals = seen;
            }
#endif
            if (goals.is_empty()) goals = tiles_around(pp, range) & ~live;
        } else {
            bool melee_coming = false;
            for (const auto& a : allies) {
                const int idx = roster_index(a);
                if (idx < 0 || a == me || roster_squad[idx] != squad || !roster_melee[idx]) continue;
                if (a.health() >= a.max_health() * 0.5f && tile_distance(a.position(), pp) <= 15) { melee_coming = true; break; }
            }
            const int waited = ++wait_tank;
            if (P_SQUAD_FALLBACK) {   // EXP-081: use the wait on a free group
                const float tf = squad_tank_frac(allies);
                if (waited > P_SQUAD_FALLBACK_WAIT || (tf >= 0.0f && tf < 0.4f)) {
                    const int fb = fallback_group(v);
                    if (fb >= 0) { fb_origin = target; target = fb; wait_tank = 0; wait_rounds = 0; return; }
                }
            }
            if (waited > (melee_coming ? kTankWait : P_TANK_WAIT_SOLO)) {
                exclude_group(target, v.round);
                wait_tank = 0;
                target = -1;
                return;
            }
            goals = tiles_around(pp, 8) & ~danger_zone(v);
            if (goals.is_empty()) goals = tiles_around(pp, 5) & ~tiles_around(pp, 4);
        }
    }
    goals &= ~live;
    DBG(" goals" + std::to_string(goals.count()) + (goals.is_set(v.pos) ? "(on)" : "") + " eng" + std::to_string(engaged) + " dpp" + std::to_string(tile_distance(v.pos, pp)));
    const bool moved = !goals.is_set(v.pos) && move_towards_closest(static_block, camp_static, goals);
    if (!engaged || tile_distance(v.pos, pp) > 4) return;
    bool cast = false;
    if (!moved) cast = cast_on_camp(prey->camp);
    if (attack_if_in_range(prey->camp)) { reset_and_attack_again(prey->camp); return; }
    if (cast) return;
    for (const auto& c : v.camps)
        if (attack_if_in_range(c.camp)) { reset_and_attack_again(c.camp); return; }
}

// v2 solo play: fight what is in reach, close in on the nearest live camp of the target, else stand on it to capture.
void Bot::farm_solo(const TurnView& v, const LiveCamp*) {
    auto& me = cambc::self();
    if (fight_any(v)) return;
    const auto& g = groups[target];
    const LiveCamp* prey = nullptr; int best_d = 99;
    for (const auto& c : v.camps) {
        if (c.group != target) continue;
        const int d = tile_distance(v.pos, c.camp.spawn_position());
        if (d < best_d) { best_d = d; prey = &c; }
    }
    if (prey != nullptr) {
        auto reach = tiles_around(prey->camp.spawn_position(), std::max(1, static_cast<int>(me.attack_range())));
        reach.set(prey->camp.spawn_position(), false);
        if (!reach.is_set(v.pos)) move_towards_closest(static_block, camp_static, reach);
    } else {
        move_towards_closest(static_block, camp_static, g.mask);
    }
    fight_any(v);
}
} // namespace va
