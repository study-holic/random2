// w3 dispatcher copy: system headers at global scope, bot code in namespace vb
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <variant>
#include <vector>
namespace vb {
// EXP-121 expanse stack runner. On expanse (80x80, three picks) the Varg works the elk+gem stacks (an Elk beside three
// gem clusters in a column, 108 gems) of the column nearer its spawn, one stack after another: it attacks the middle gem
// cluster while it lives (Varg's splash hits the elk and the other clusters), then the weakest camp that hurts, lets the
// existing capture logic (holding) collect the group, banks through bank() and moves to the next stack. When no stack of
// its column is alive or due within P_RUN_DUE rounds it hands the turn back to the v10 flow.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#include <cmath>
#if P_RUN_DBG
#include <iostream>
#endif

using cambc::vec2;

static constexpr int kRespawnTime = P_RESPAWN_TIME;

// Stacks: groups of four camps with an Elk and gem clusters. The column (x below or above 40) with the smaller mean
// distance from our spawn centre is ours. Only for the Varg on 80x80 (expanse; Cassi & Issac count as two champions, so the roster has four).
void Bot::init_runner() {
#if P_RUN_DBG
    std::cout << "RUNNER try roster" << roster.size() << " me" << int(cambc::self().champion_name()) << " " << cambc::map::width() << "x" << cambc::map::height() << " groups" << groups.size() << std::endl;
#endif
    if (cambc::self().champion_name() != cambc::champion_type::varg) return;
    if (cambc::map::width() != 80 || cambc::map::height() != 80) return;
    std::vector<int> side[2];
    long dist[2] = {0, 0};
    for (std::size_t i = 0; i < groups.size(); ++i) {
        const auto& g = groups[i];
        if (!g.heavy || g.monolith || !g.has_gem_cluster || g.tiles.size() != 4) continue;
        const int s = g.tiles[0].x < 40 ? 0 : 1;
        side[s].push_back(static_cast<int>(i));
        dist[s] += g.dist_ally;
    }
    int pick = -1;
    for (int s = 0; s < 2; ++s)
        if (!side[s].empty() && (pick < 0 || dist[s] * static_cast<long>(side[pick].size()) < dist[pick] * static_cast<long>(side[s].size()))) pick = s;
#if P_RUN_DBG
    for (const auto& g : groups) if (g.heavy) std::cout << "HEAVY tiles" << g.tiles.size() << " x" << g.tiles[0].x << " y" << g.tiles[0].y << " gem" << g.has_gem_cluster << " mono" << g.monolith << std::endl;
#endif
    if (pick < 0) return;
    for (int idx : side[pick]) {
        // The middle gem cluster is the camp within one tile of every other camp of the group.
        const auto camps = groups[idx].group.camps();
        vec2 mid = groups[idx].tiles[0];
        int best = -1;
        for (const auto& c : camps) {
            if (!c.is_gem_cluster()) continue;
            int near = 0;
            for (const auto& o : camps) if (tile_distance(c.spawn_position(), o.spawn_position()) <= 1) ++near;
            if (near > best) { best = near; mid = c.spawn_position(); }
        }
        stack_ids.push_back(idx);
        stack_mid.push_back(mid);
    }
    runner = !stack_ids.empty();
    stack_elk_hp.assign(stack_ids.size(), 2400.0f);
#if P_RUN_DBG
    std::cout << "RUNNER init side" << pick << " stacks";
    for (std::size_t k = 0; k < stack_ids.size(); ++k) std::cout << " g" << stack_ids[k] << "(mid " << stack_mid[k].x << "," << stack_mid[k].y << ")";
    std::cout << std::endl;
#endif
}

int Bot::stack_slot(int group) const {
    for (std::size_t k = 0; k < stack_ids.size(); ++k) if (stack_ids[k] == group) return static_cast<int>(k);
    return -1;
}

// EXP-124: the runner is inside the reach of a hurting camp of its stack (a visit in progress).
bool Bot::runner_visit(const TurnView& v) const {
    if (stack_slot(target) < 0) return false;
    for (const auto& c : v.camps)
        if (c.group == target && camp_strength(c.camp) > 0 && tile_distance(v.pos, c.camp.spawn_position()) <= kMonsterRange + 1) return true;
    return false;
}

// EXP-124: elk rounds left of the target stack at the champion's Strength (one attack per round).
static int elk_rounds_left(float elk_hp, float strength) {
    if (elk_hp <= 0) return 0;
    return static_cast<int>(std::ceil(elk_hp / std::max(1.0f, strength)));
}

// EXP-124 finish-or-leave: below 250 health with more elk work left than the health can pay for, step out and recall.
bool Bot::runner_leave(const TurnView& v) const {
    const int k = stack_slot(target);
    if (k < 0) return false;
    float elk_hp = 0;
    for (const auto& c : v.camps) if (c.group == target && c.camp.is_elk()) elk_hp = c.health;
    const float hp = cambc::self().health();
    const int rem = elk_rounds_left(elk_hp, cambc::self().strength());
    return hp < 250.0f && rem * 25.0f > hp - 60.0f;
}

// Sets target to the stack to work and settles capture holds. False hands the turn to the v10 flow.
bool Bot::runner_resolve(const TurnView& v) {
    auto& me = cambc::self();
    run_heal_needed = false;
    const int round = v.round;
    if (holding >= 0) {
        if (stack_slot(holding) < 0) return false;   // a v10 hold: not ours
        bool alive = false;
        for (const auto& c : v.camps) if (c.group == holding) alive = true;
        refresh_group(holding, round, v.visible);
        const bool paid = static_cast<int>(me.gem_count()) > run_hold_gems || groups[holding].ready_round > round;
        if (alive || paid) {
            if (paid && !alive) {
                ++run_captures;
                { const int hk = stack_slot(holding); if (hk >= 0 && hk < static_cast<int>(stack_elk_hp.size())) stack_elk_hp[hk] = 2400.0f; }
                if (groups[holding].ready_round <= round) groups[holding].ready_round = round + kRespawnTime;
#if P_RUN_DBG
                std::cout << "CAPTURE r" << round << " g" << holding << " n" << run_captures << " bag" << int(me.gem_count()) << std::endl;
#endif
            }
            holding = -1;
        } else if (round - hold_since > 20) {
            exclude_group(holding, round);
            holding = -1;
            target = -1;
        } else { target = holding; return true; }
    }
    if (me.gem_count() > 0) return false;   // a bag goes to the altar first (v10 bank rules)
    int best = -1; double best_score = 1e18;
    for (int idx : stack_ids) {
        const auto& g = groups[idx];
        if (g.excluded_until > round || g.ready_round > round + P_RUN_DUE) continue;
        const double score = min_distance(v.pos, g.tiles) - (idx == target ? 8 : 0) + 0.3 * std::max(0, g.ready_round - round);
        if (score < best_score) { best_score = score; best = idx; }
    }
    if (best < 0) { if (stack_slot(target) >= 0) target = -1; return false; }
#if P_RUN_GATE
    // EXP-124 start gate: a visit begins only with health for the remaining elk (last seen, 2400 when unseen); otherwise the normal flow.
    if (!(best == target && runner_visit(v))) {
        const int bk = stack_slot(best);
        float elk_hp = stack_elk_hp[bk];
        for (const auto& c : v.camps) if (c.group == best && c.camp.is_elk()) { elk_hp = c.health; break; }
        const bool respawning = groups[best].ready_round > round;
        const float gate = std::min((elk_rounds_left(respawning ? 2400.0f : elk_hp, me.strength()) + 4) * 25.0f + 150.0f, 0.9f * me.max_health());   // EXP-124b: capped at the spawn heal level
        if (me.health() < gate) {
            run_heal_needed = true;   // EXP-124b: play_turn_inner sends the runner home to heal when no enemy champion is in view
#if P_RUN_DBG
            if (round % 20 == 0) std::cout << "GATEFAIL r" << round << " hp" << int(me.health()) << " gate" << int(gate) << " str" << int(me.strength()) << " elk" << int(elk_hp) << std::endl;
#endif
            return false;
        }
    }
    for (const auto& c : v.camps) { const int ck = stack_slot(c.group); if (ck >= 0 && c.camp.is_elk()) stack_elk_hp[ck] = c.health; }
#endif
    if (best != target) { target = best; wait_rounds = 0; }
    if (groups[target].ready_round <= round && group_cleared(v, target)) {
        holding = target; hold_since = round; run_hold_gems = static_cast<int>(me.gem_count());
    }
    return true;
}

void Bot::runner_farm(const TurnView& v, const std::vector<cambc::champion>& allies) {
    const int k = stack_slot(target);
    if (k < 0) { farm(v, allies); return; }
    const auto& g = groups[target];
    if (holding == target) {   // stand on a spawn tile until the reward is paid
        if (!g.mask.is_set(v.pos)) move_towards_closest(static_block, camp_static, g.mask);
        return;
    }
    if (g.ready_round > v.round) {   // due soon: wait beside the tiles without blocking the respawn
        if (!g.ring.is_set(v.pos)) move_towards_closest(static_block, camp_static, g.ring);
        return;
    }
#if P_RUN_DBG
    { const bool vis = runner_visit(v);
      if (vis && !run_in_visit) { ++run_visits; std::cout << "VISIT r" << v.round << " n" << run_visits << " hp" << int(cambc::self().health()) << " str" << int(cambc::self().strength()) << " g" << target << std::endl; }
      run_in_visit = vis; }
#endif
    const LiveCamp* prey = nullptr;
    for (const auto& c : v.camps) {
        if (c.group != target) continue;
        if (c.camp.spawn_position() == stack_mid[k]) { prey = &c; break; }
        const bool hurts = camp_strength(c.camp) > 0;
        const bool prey_hurts = prey != nullptr && camp_strength(prey->camp) > 0;
        if (prey == nullptr || hurts > prey_hurts || (hurts == prey_hurts && c.health < prey->health)) prey = &c;
    }
#if P_RUN_DBG
    if (min_distance(v.pos, g.tiles) <= 10) {
        std::cout << "RUN r" << v.round << " pos" << v.pos.x << "," << v.pos.y << " hp" << int(cambc::self().health()) << " g" << target
                  << " ret" << retreating << " camps";
        for (const auto& c : v.camps) if (c.group == target) std::cout << " (" << c.camp.spawn_position().x << "," << c.camp.spawn_position().y << ":" << int(c.health) << ")";
        std::cout << std::endl;
    }
#endif
    farm_squad(v, allies, prey);
}

// EXP-124b heal trip: the start gate failed; with no enemy champion in view walk out of camp reach and recall (damage interrupts it),
// or walk in when the spawn is near. The spawn heals the champion to 90 per cent (existing HEALWAIT), which meets the capped gate.
bool Bot::runner_heal_trip(const TurnView& v, const std::vector<cambc::champion>& enemies) {
    auto& me = cambc::self();
    if (!enemies.empty() || ally_spawn.is_set(v.pos)) return false;
    if (min_distance(v.pos, spawn_tiles) <= P_NEAR_SPAWN && move_towards_closest(static_block, camp_static, ally_spawn)) return true;
    if (incoming_damage(v, v.pos) > 0) return move_towards_closest(static_block, camp_static, ~danger_zone(v));
    try { me.recall(); } catch (...) { return false; }
#if P_RUN_DBG
    std::cout << "HEALTRIP r" << v.round << " hp" << int(me.health()) << std::endl;
#endif
    return true;
}
} // namespace vb
