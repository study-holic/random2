// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <span>
#include <variant>
#include <vector>
namespace va {
// P_W_WATCHDOG (tools/cloud/sweep35.md task 3): a progress watchdog. Model: a Lyapunov-style progress certificate
// (Khalil, Nonlinear Systems, ch. 4) run as an embedded watchdog timer. For the champion's objective o:
//   potential Phi = Chebyshev distance to the altar while banking, to the target group's spawn tiles while travelling
//   (distance > 4), or the remaining HP of the group's visible camps while clearing (distance <= 4);
//   progress event = Phi falls below its best since the objective was taken, gems or gold change (pickup, tribute,
//   purchase), health rises on our spawn (healing), an enemy champion is within 5 and blows were exchanged (a fight, handled elsewhere), a known
//   respawn wait of <= 30 rounds at the target, or a step onto a tile not among the last 6 positions;
//   S = rounds since the last progress event; trigger when S > W (12 travel, 20 clearing, 10 within 3 of the altar).
// Escalation, one level per trigger (reset by progress): 1 soft hazards for W rounds (Monolith zones, siege block and
// threat detours dropped from pathing); 2 release the objective and block it 60 rounds (group excluded_until, altar
// altar_skip_until); 3 drop the bank decision and take the nearest open group, claims ignored, for 60 rounds when target
// choice returns none (wd_after_target; rushbait_v2: three champions never left spawn with no target); 4 walk to the own
// spawn (fixed escape) for 8 rounds. Cost: a few hundred fuel; skipped below 20,000 fuel. Source: Copilot, consult 1946.
#include <cambc.h>
#include "bot.h"
#include "params.h"
#include "util.h"

int g_wd_soft_until = 0;   // util.cpp: soft hazards while round < this
static const cambc::bitboard& wd_enemy_spawn() { static const cambc::bitboard b = cambc::map::spawn_region(cambc::team::enemy); return b; }

bool Bot::wd_tick(const TurnView& v) {
    if (!P_W_WATCHDOG || cambc::fuel_remaining() < 20000) return false;
    auto& me = cambc::self();
    const int r = v.round;
    if (r < wd_escape_until) {
        if (ally_spawn.is_set(v.pos)) { wd_escape_until = 0; return false; }
        return move_towards_closest(static_block, camp_static, ally_spawn);
    }
    const int gems = static_cast<int>(me.gem_count()), gold = static_cast<int>(me.gold_count());
    const float hp = me.health();
    // Level 3 walk: the threat layer and enemy response can hold a champion beside idle enemies before target choice
    // runs (rushbait_v2), so the watchdog walks it to the forced group itself until within 4, unless it is being hit.
    if (wd_force >= 0 && r < wd_force_until && groups[wd_force].excluded_until <= r && !(wd_hp >= 0 && hp < wd_hp - 0.5f)
        && min_distance(v.pos, groups[wd_force].tiles) > 4) {
        wd_hp = hp; wd_last = r;
        cambc::bitboard goal; goal.clear();
        for (auto t : groups[wd_force].tiles) goal.set(t, true);
        if (move_towards_closest(static_block, camp_static, tiles_around(groups[wd_force].tiles[0], 4) | goal)) return true;
    }
    int mode = 0, obj = -1, W = 12;
    double phi = 0;
    bool wait_ok = false;
    if (banking && gems > 0 && !altars.empty()) {
        int best = 1 << 20;
        for (std::size_t i = 0; i < altars.size(); ++i) { const int d = tile_distance(v.pos, altars[i]); if (d < best) { best = d; obj = static_cast<int>(i); } }
        mode = 3; phi = best; W = best <= 3 ? 10 : 12;
    } else if (target >= 0 && target < static_cast<int>(groups.size())) {
        const int d = min_distance(v.pos, groups[target].tiles);
        obj = target;
        if (d > 4) { mode = 1; phi = d; W = 12; }
        else {
            mode = 2; W = 20;
            for (const auto& c : v.camps) if (c.group == target) phi += c.health;
            if (phi <= 0 && groups[target].ready_round > r && groups[target].ready_round - r <= 30) wait_ok = true;
        }
    }
    bool prog = wait_ok;
    if (mode != wd_mode || obj != wd_obj) { prog = true; wd_mode = mode; wd_obj = obj; wd_best = phi; }
    if (phi < wd_best - 1e-6) { wd_best = phi; prog = true; }
    if (gems != wd_gems || gold != wd_gold) prog = true;
    if (hp > wd_hp + 0.5f && ally_spawn.is_set(v.pos)) prog = true;
    // A fresh tile (not among the last 6 positions) is progress in every mode: Chebyshev Phi does not fall on a detour
    // round a wall, and reading that as a stall released the altar for every carrier on shortcut side B (traced).
    bool fresh = true;
    for (const auto& q : wd_trail) if (q == v.pos) { fresh = false; break; }
    if (fresh) prog = true;
    wd_trail[wd_trail_i++ % wd_trail.size()] = v.pos;
    if (!prog && (attacked || std::abs(hp - wd_hp) > 0.5f))   // a fight: an enemy within 5 and blows exchanged (an idle enemy
        for (auto e : cambc::vision::champion_tiles(cambc::team::enemy).positions())   // beside our spawn froze 3 champions on rushbait_v2)
            if (tile_distance(v.pos, e) <= 5 && !wd_enemy_spawn().is_set(e)) { prog = true; break; }   // an enemy on its spawn heals every turn
    wd_gems = gems; wd_gold = gold; wd_hp = hp; wd_pos = v.pos;
    if (prog) { wd_last = r; wd_level = 0; return false; }
    if (r - wd_last <= W) return false;
    wd_last = r;
    wd_level = std::min(wd_level + 1, 4);
    ++wd_triggers;
    DBG(" WD" + std::to_string(wd_level));
    switch (wd_level) {
        case 1: g_wd_soft_until = r + W; break;
        case 2: case 3:
            if (mode == 3 && obj >= 0 && obj < static_cast<int>(altar_skip_until.size())) altar_skip_until[obj] = r + 60;
            if ((mode == 1 || mode == 2) && obj >= 0) { groups[obj].excluded_until = std::max(groups[obj].excluded_until, r + 60); target = -1; holding = -1; }
            if (wd_level == 3) {   // nearest feasible task: the nearest open non-Monolith group, claims ignored, held 60 rounds
                banking = false; target = -1; holding = -1;
                int best = -1, bd = 1 << 20;
                for (std::size_t i = 0; i < groups.size(); ++i) {
                    if (groups[i].monolith || groups[i].excluded_until > r || static_cast<int>(i) == obj) continue;
                    const int d = min_distance(v.pos, groups[i].tiles);
                    if (d < bd) { bd = d; best = static_cast<int>(i); }
                }
                if (best >= 0) { wd_force = best; wd_force_until = r + 60; }
            }
            break;
        default:
            wd_escape_until = r + 8;
            return move_towards_closest(static_block, camp_static, ally_spawn);
    }
    return false;
}
void Bot::wd_after_target(const TurnView& v) {
    if (!P_W_WATCHDOG || target >= 0 || wd_force < 0) return;
    if (v.round >= wd_force_until || groups[wd_force].excluded_until > v.round) { wd_force = -1; return; }
    target = wd_force;
}
} // namespace va
