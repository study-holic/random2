// EXP-086 hostile-only reactions (P_HOSTILE). EXP-071/072 treated every visible enemy as a threat, so harmless starter
// farmers cut uncontested pace. Here an enemy champion is hostile only after it has been seen to hurt or to close in:
//  (a) a visible ally (or this champion) lost health while the enemy was within its attack range + 1 of the victim and
//      no live camp stood within 3 of the victim (camp damage excluded);
//  (b) the enemy is within 6 and has moved closer to this champion's current tile on each of the last two turns.
// Hostility lasts P_HOSTILE_MEMORY rounds and is kept per champion from its own vision (shared bytes are all taken).
// Reactions run only with a hostile in view: R1 bank-first, R2 not alone (hostile_layer); R3 no trickle (hostile_wait).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#if P_HOSTILE

using cambc::vec2;
using cambc::team;

static constexpr int kMemory = P_HOSTILE_MEMORY, kBag = P_HOSTILE_BAG, kSafe = P_HOSTILE_SAFE;
static constexpr int kWait = P_HOSTILE_RESPAWN_WAIT, kStick = 40, kNear = 8;

static int nearest_to(vec2 p, const std::vector<vec2>& pts) {
    int d = 1 << 20;
    for (auto t : pts) d = std::min(d, tile_distance(p, t));
    return d;
}

bool Bot::hostile_group(int index, int round) const {
    return index >= 0 && index < static_cast<int>(hg8.size()) && round - hg8[index] < kMemory;
}

// Memory update, once per turn, before any reaction. Changes no behaviour by itself.
void Bot::hostile_update(const TurnView& v, const std::vector<cambc::champion>& enemies,
                         const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    const int round = v.round;
    if (enemy_roster.empty()) {
        try { enemy_roster = cambc::champions(team::enemy); } catch (...) {}
        if (enemy_roster.size() > static_cast<std::size_t>(kHsMax)) enemy_roster.erase(enemy_roster.begin() + kHsMax, enemy_roster.end());
        hg8.assign(groups.size(), -1000);
        hg10.assign(groups.size(), -1000);
        hs_hp_round.fill(-1000);
        for (int i = 0; i < kHsMax; ++i) { hs_rnd[i] = {-1000, -1000}; hs_pos[i] = {vec2(0, 0), vec2(0, 0)}; }
    }
    if (hs_dead) { hs_dead = false; hs_pending = true; hs_wait_start = -1; hs_wait_target = -1; hb_until = -1; hb_altar = -1; }
    allies6 = 0;
    for (const auto& a : allies) if (!(a == me) && tile_distance(v.pos, a.position()) <= 6) ++allies6;

    // Enemy slots of the visible champions (index k into `enemies` is kept for the attack-range query).
    std::array<int, 8> slot{}, ek{};
    std::array<vec2, 8> epos{};
    int n = 0;
    for (std::size_t q = 0; q < enemies.size(); ++q) {
        if (n >= 8) break;
        const auto& e = enemies[q];
        if (e.health() <= 0) continue;
        int idx = -1;
        for (std::size_t k = 0; k < enemy_roster.size(); ++k) if (enemy_roster[k] == e) { idx = static_cast<int>(k); break; }
        if (idx < 0) {
            if (enemy_roster.size() >= static_cast<std::size_t>(kHsMax)) continue;
            enemy_roster.push_back(e);
            idx = static_cast<int>(enemy_roster.size()) - 1;
        }
        slot[n] = idx;
        ek[n] = static_cast<int>(q);
        epos[n] = e.position();
        ++n;
    }

    // (a) an ally lost health with an enemy champion in reach and no live camp beside the victim.
    for (const auto& a : allies) {
        const int ai = roster_index(a);
        if (ai < 0 || ai >= 8) continue;
        const float hp = a.health();
        const bool lost = n > 0 && hs_hp_round[ai] == round - 1 && hp < hs_hp[ai] - 0.5f;
        hs_hp[ai] = hp;
        hs_hp_round[ai] = round;
        if (!lost) continue;
        const vec2 victim = a.position();
        int in_reach[8], m = 0;
        for (int k = 0; k < n; ++k) {
            int range = 1;
            try { range = std::max(1, static_cast<int>(enemies[ek[k]].attack_range())); } catch (...) {}
            if (tile_distance(epos[k], victim) <= range + 1) in_reach[m++] = k;
        }
        if (m == 0) continue;
        if (!(tiles_around(victim, 3) & cambc::vision::camp_tiles()).is_empty()) continue;   // camp damage
        for (int j = 0; j < m; ++j) hs_until[slot[in_reach[j]]] = round + kMemory;
    }

    // (b) within 6 and closer to this tile on each of the last two turns; then record this sighting.
    hs_foes.clear();
    hs_near = 1 << 20;
    // Credible pre-contact threat (EXP-087 narrowing): an enemy closing on me counts only when I am not locally
    // superior (enemies within 8 >= allies within 6, self excluded, + 1) and either I carry a bag or two or more
    // enemies are within 8. A lone farmer walking toward a shared group no longer triggers anything.
    int enemies8 = 0;
    for (int k = 0; k < n; ++k) if (tile_distance(epos[k], v.pos) <= 8) ++enemies8;
    const bool credible_ok = enemies8 >= allies6 + 1 && (me.gem_count() >= kBag || enemies8 >= 2);
    for (int k = 0; k < n; ++k) {
        const int i = slot[k];
        const int d = tile_distance(epos[k], v.pos);
        if (credible_ok && d <= 6 && hs_rnd[i][0] == round - 1 && hs_rnd[i][1] == round - 2
            && d < tile_distance(hs_pos[i][0], v.pos) && tile_distance(hs_pos[i][0], v.pos) < tile_distance(hs_pos[i][1], v.pos))
            hs_until[i] = round + kMemory;
        hs_pos[i][1] = hs_pos[i][0]; hs_rnd[i][1] = hs_rnd[i][0];
        hs_pos[i][0] = epos[k]; hs_rnd[i][0] = round;
        if (hs_until[i] > round) {
            hs_foes.push_back(epos[k]);
            hs_near = std::min(hs_near, d);
            for (std::size_t g = 0; g < groups.size(); ++g) {
                const int gd = min_distance(epos[k], groups[g].tiles);
                if (gd <= 8) hg8[g] = round;
                if (gd <= 10) hg10[g] = round;
            }
        }
    }
}

// Walk to the closer of the nearest other ally (a tile beside it) and the spawn, keeping out of the hostile safe radius
// when a goal tile outside it exists. 0 = moved, 1 = already at the goal, 2 = no step.
int Bot::hs_step_to_friends(const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    const vec2 pos = me.position();
    int best = min_distance(pos, spawn_tiles);
    cambc::bitboard goals = ally_spawn;
    for (const auto& a : allies) {
        if (a == me) continue;
        const int d = tile_distance(pos, a.position());
        if (d < best) { best = d; goals = tiles_around(a.position(), 1); goals.set(a.position(), false); }
    }
    cambc::bitboard zone; zone.clear();
    for (auto f : hs_foes) zone |= tiles_around(f, kSafe);
    const cambc::bitboard open = goals & ~zone;
    if (!open.is_empty()) goals = open;
    if (goals.is_set(pos)) return 1;
    return move_towards_closest(static_block, camp_static, goals) ? 0 : 2;
}

bool Bot::hostile_layer(const TurnView& v, const std::vector<cambc::champion>& enemies,
                        const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    const int round = v.round, gems = static_cast<int>(me.gem_count());
    if (gems == 0) { hb_until = -1; hb_altar = -1; }
    const bool near8 = hs_near <= kNear;
    const bool sticky = hb_until > round && gems > 0;
    const int d_foe = nearest_to(v.pos, hs_foes);

    // Fight in place at a safe spot (the enemy rules attack and cast); the turn is used.
    auto hold = [&]() { enemy_response(v, enemies, allies); return true; };

    // R1 bank-first.
    if (gems >= kBag && (near8 || sticky)) {
        if (hb_until <= round) hb_until = round + kStick;
        const int range = static_cast<int>(cambc::constants::submit_gems_range);
        auto usable = [&](std::size_t i) { return lock_until[i] <= round && altar_skip_until[i] <= round; };
        int pick = -1, pick_d = 1 << 20;
        if (hb_altar >= 0 && usable(hb_altar) && nearest_to(altars[hb_altar], hs_foes) > kSafe) {   // sticky choice
            pick = hb_altar;
            pick_d = tile_distance(v.pos, altars[pick]);
        } else {
            for (std::size_t i = 0; i < altars.size(); ++i) {
                if (!usable(i) || nearest_to(altars[i], hs_foes) <= kSafe + 1) continue;
                const int d = tile_distance(v.pos, altars[i]);
                if (d < pick_d) { pick_d = d; pick = static_cast<int>(i); }
            }
            hb_altar = pick;
        }
        if (pick >= 0) {
            banking = true;
            if (carry_since < 0) carry_since = round;
            holding = -1;
            target = -1;
            if (pick_d <= range) {
                if (d_foe > kSafe) return bank(v, enemies, allies);   // never start a tribute with a hostile within SAFE
            } else if (move_towards_closest(static_block, camp_static, tiles_around(altars[pick], range))) {
                return true;
            }
        }
        if (d_foe > kSafe) return false;   // nothing hostile close: the usual rules
        holding = -1;
        const int r = hs_step_to_friends(allies);
        if (r == 0) return true;
        if (r == 1) return hold();
        return false;   // no step: the enemy rules take the turn
    }

    // R2 not alone: the target group had a hostile near it recently and nobody is beside this champion.
    if (near8 && allies6 == 0 && holding < 0 && target >= 0 && hostile_group(target, round) && !retreating) {
        hs_avoid = true;
        const int next = choose_target(v, target);
        hs_avoid = false;
        if (next >= 0 && next != target && !hostile_group(next, round)) {
            exclude_group(target, round);
            target = next;
            wait_rounds = 0;
            wait_tank = 0;
            return false;
        }
        bool in_reach = false;
        for (auto f : hs_foes) if (me.attack_tiles().is_set(f)) in_reach = true;
        if (!in_reach) {
            int best = 1 << 20;
            cambc::bitboard goals; goals.clear();
            for (const auto& a : allies) {
                if (a == me) continue;
                const int d = tile_distance(v.pos, a.position());
                if (d < best) { best = d; goals = tiles_around(a.position(), 1); goals.set(a.position(), false); }
            }
            if (best < (1 << 20) && move_towards_closest(static_block, camp_static, goals)) return true;
        }
    }
    return false;
}

// R3 no trickle: after a respawn, when a hostile was seen within 10 of the next target in the last P_HOSTILE_MEMORY
// rounds, stay on the spawn until an ally is within 6 or P_HOSTILE_RESPAWN_WAIT rounds pass.
bool Bot::hostile_wait(const TurnView& v) {
    if (!hs_pending) return false;
    const int round = v.round;
    if (!ally_spawn.is_set(v.pos)) { hs_pending = false; return false; }
    if (hs_wait_start < 0) {
        hs_wait_start = round;
        hs_wait_target = target >= 0 ? target : choose_target(v, -1);
    }
    const bool trouble = hs_wait_target >= 0 && hs_wait_target < static_cast<int>(hg10.size()) && round - hg10[hs_wait_target] < kMemory;
    if (round - hs_wait_start >= kWait || allies6 >= 1 || !trouble) { hs_pending = false; return false; }
    still_rounds = 0;   // waiting is not being stuck (EXP-018)
    return true;
}
#endif
