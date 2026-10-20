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
//   purchase), health rises on our spawn (healing), an enemy champion is within 5 (a fight, handled elsewhere), a known
//   respawn wait of <= 30 rounds at the target, or (no objective) any change of position;
//   S = rounds since the last progress event; trigger when S > W (12 travel, 20 clearing, 10 within 3 of the altar).
// Escalation, one level per trigger (reset by progress): 1 soft hazards for W rounds (Monolith zones, siege block and
// threat detours dropped from pathing); 2 release the objective and block it 60 rounds (group excluded_until, altar
// altar_skip_until); 3 drop the bank decision and take the nearest feasible task (resolve_target); 4 walk to the own
// spawn (fixed escape) for 8 rounds. Cost: a few hundred fuel; skipped below 20,000 fuel. Source: Copilot, consult 1946.
#include <cambc.h>
#include "bot.h"
#include "params.h"
#include "util.h"

int g_wd_soft_until = 0;   // util.cpp: soft hazards while round < this

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
    if (mode == 0 && !(v.pos == wd_pos)) prog = true;
    if (!prog)
        for (auto e : cambc::vision::champion_tiles(cambc::team::enemy).positions())
            if (tile_distance(v.pos, e) <= 5) { prog = true; break; }
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
            if (wd_level == 3) { banking = false; target = -1; holding = -1; }
            break;
        default:
            wd_escape_until = r + 8;
            return move_towards_closest(static_block, camp_static, ally_spawn);
    }
    return false;
}
} // namespace va
