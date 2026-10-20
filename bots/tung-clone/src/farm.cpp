// Squad farming for the Tung clone: shared squad claims, own-half targeting, tank and ranged positions,
// a single capturer per group, and spell casting on the camp being fought.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

using cambc::vec2;
using CT = cambc::champion_type;

static constexpr int kRespawnTime = 60;      // rules: respawn_time (60) plus the 5-round capture
static constexpr int kClaimSlack = 25;       // a claim stays valid while its group respawns within this many rounds

static bool melee_type(CT t) {
    return t == CT::vendetta || t == CT::varg || t == CT::issac || t == CT::ryth || t == CT::ren || t == CT::marcella;
}

// Tank priority inside a squad: lower is tankier (Vendetta and Varg first, then the next melee ally).
static int tank_rank(CT t) {
    switch (t) {
        case CT::vendetta: return 0;
        case CT::varg: return 1;
        case CT::issac: return 2;
        case CT::ryth: return 3;
        default: return 4;
    }
}

int Bot::read_claim() const {
    std::array<std::uint8_t, 8> m{};
    cambc::shared_memory.read(0, std::span{m.data(), m.size()});
    return static_cast<int>(m[squad]) - 1;
}

void Bot::write_claim(int group) {
    const std::uint8_t b = static_cast<std::uint8_t>(group + 1);
    cambc::shared_memory.write(squad, std::span<const std::uint8_t>{&b, 1});
}

// The squad's tank: the highest-priority melee ally that is above 35 per cent health (this champion always counts
// itself, so it keeps the role until its own retreat triggers).
bool Bot::tank_role(const std::vector<cambc::champion>& allies) const {
    if (!is_melee) return false;
    auto& me = cambc::self();
    const int mine = tank_rank(me.champion_name());
    for (const auto& a : allies) {
        if (a == me) continue;
        const auto t = a.champion_name();
        if (!melee_type(t) || squad_of(t, ally_count) != squad) continue;
        if (a.health() < a.max_health() * 0.35f) continue;
        if (tank_rank(t) < mine) return false;
    }
    return true;
}

// Rank of this champion among the squad members standing near the group (0 = lowest id = the capturer).
int Bot::capture_rank(const std::vector<cambc::champion>& allies, int group) const {
    auto& me = cambc::self();
    int rank = 0;
    for (const auto& a : allies) {
        if (a == me) continue;
        if (squad_of(a.champion_name(), ally_count) != squad) continue;
        if (min_distance(a.position(), groups[group].tiles) > 8) continue;
        int their = 99;
        for (std::size_t i = 0; i < roster.size(); ++i) if (roster[i] == a) their = static_cast<int>(i);
        if (their < ally_index) ++rank;
    }
    return rank;
}

// True when every camp tile of the group is in view and no camp of it is alive.
bool Bot::group_cleared(const TurnView& v, int index) const {
    const auto& g = groups[index];
    for (auto t : g.tiles) if (!v.visible.is_set(t)) return false;
    for (const auto& c : v.camps) if (c.group == index) return false;
    return true;
}

// Nearest own-half group not claimed by another squad. Respawning groups are skipped unless their timer is below the
// travel time; when nothing else is left the group that respawns first is chosen.
int Bot::choose_target(const TurnView& v, int exclude) {
    std::array<std::uint8_t, 8> claims{};
    cambc::shared_memory.read(0, std::span{claims.data(), claims.size()});
    bool any_own = false, any_tie = false;
    for (const auto& g : groups) if (!g.monolith) { any_own |= g.own_half; any_tie |= g.tie_half; }
    int best = -1, soonest = -1;
    int best_score = 1 << 20, soonest_ready = 1 << 20;
    for (std::size_t i = 0; i < groups.size(); ++i) {
        const auto& g = groups[i];
        if (g.monolith || static_cast<int>(i) == exclude) continue;
        if (any_own ? !g.own_half : (any_tie && !(g.own_half || g.tie_half))) continue;
        bool taken = false;
        for (int s = 0; s < 8; ++s) if (s != squad && claims[s] == i + 1 && s < 2) taken = true;
        if (taken) continue;
        const int d = min_distance(v.pos, g.tiles);
        const int wait = std::max(0, g.ready_round - v.round);
        if (wait > std::min(d, kClaimSlack)) {
            if (g.ready_round < soonest_ready) { soonest_ready = g.ready_round; soonest = static_cast<int>(i); }
            continue;
        }
        if (d < best_score) { best_score = d; best = static_cast<int>(i); }
    }
    return best >= 0 ? best : soonest;
}

// Decide which group this champion works on this turn: hold a capture, leave a cleared group (the lowest-id squad
// member stays to capture), or follow the squad claim in shared memory.
void Bot::resolve_target(const TurnView& v, const std::vector<cambc::champion>& allies) {
    const int round = v.round;
    const int old_target = target, old_hold = holding;
    resolve_inner(v, allies);
}

void Bot::resolve_inner(const TurnView& v, const std::vector<cambc::champion>& allies) {
    const int round = v.round;
    if (holding >= 0) {
        bool contested = false;
        for (const auto& c : v.camps) if (c.group == holding) contested = true;
        refresh_group(holding, round, v.visible);                      // respawning means the capture reward has been paid
        if (contested || groups[holding].ready_round > round) {
            holding = -1;
        } else { target = holding; return; }
    }
    if (target >= 0 && groups[target].ready_round <= round && group_cleared(v, target)) {
        const int cleared = target;
        const int rank = capture_rank(allies, cleared);
        const int next = choose_target(v, cleared);
        bool stay = rank == 0 || next < 0;
        if (!stay && rank == 1 && groups[cleared].wolves >= 3 && min_distance(groups[cleared].tiles[0], groups[next].tiles) > 5)
            stay = true;                                    // three-wolf pack: the second member also captures
        groups[cleared].ready_round = round + 5 + kRespawnTime;
        if (stay) { holding = cleared; groups[cleared].ready_round = 0; return; }
        write_claim(next);
        target = next;
        return;
    }
    const int claim = read_claim();
    const bool valid = claim >= 0 && claim < static_cast<int>(groups.size())
        && groups[claim].ready_round <= round + kClaimSlack;
    if (valid) { target = claim; return; }
    target = choose_target(v, -1);
    if (target >= 0) write_claim(target);
}

void Bot::farm(const TurnView& v, bool is_tank, const std::vector<cambc::champion>& allies) {
    (void)is_tank;
    if (target < 0) return;
    auto& me = cambc::self();
    const auto& g = groups[target];

    if (holding == target) {                                // capture: stand on a spawn tile until the reward is paid
        if (!g.mask.is_set(v.pos)) move_towards_closest(static_block, camp_static, g.mask);
        return;
    }
    if (g.ready_round > 0 && g.ready_round > v.round - 3) {  // respawning: wait beside the tiles, not on them
        if (!g.ring.is_set(v.pos)) move_towards_closest(static_block, camp_static, g.ring);
        return;
    }

    // Prey: the weakest live camp that can hurt us; harmless clusters only when nothing else is left.
    const LiveCamp* prey = nullptr;
    for (const auto& c : v.camps) {
        if (c.group != target) continue;
        const bool better = prey == nullptr
            || (camp_strength(c.camp) > 0) > (camp_strength(prey->camp) > 0)
            || ((camp_strength(c.camp) > 0) == (camp_strength(prey->camp) > 0) && c.health < prey->health);
        if (better) prey = &c;
    }
    if (prey == nullptr) { move_towards_closest(static_block, camp_static, g.mask); return; }

    const vec2 pp = prey->camp.spawn_position();
    const int range = std::max(1, static_cast<int>(me.attack_range()));
    cambc::bitboard goals;
    if (is_melee) {
        goals = tiles_around(pp, 1);                         // tank and melee allies stand adjacent
    } else {
        bool engaged = false;                                // ranged allies wait outside the danger zone for the tank
        for (const auto& a : allies)
            if (!(a == me) && tile_distance(a.position(), pp) <= 1) { engaged = true; break; }
        if (engaged) {
            goals = tiles_around(pp, range) & ~tiles_around(pp, range - 1);
            cambc::bitboard others; others.clear();          // keep out of reach of the other live monsters when possible
            for (const auto& c : v.camps)
                if (&c != prey && camp_strength(c.camp) > 0) others |= tiles_around(c.camp.spawn_position(), 3);
            if (!(goals & ~others).is_empty()) goals &= ~others;
        } else {
            goals = tiles_around(pp, 8) & ~danger_zone(v);
            if (goals.is_empty()) goals = tiles_around(pp, 5) & ~tiles_around(pp, 4);
        }
    }
    for (const auto& c : v.camps) goals.set(c.camp.spawn_position(), false);
    const bool in_position = goals.is_set(v.pos);
    const bool moved = !in_position && move_towards_closest(static_block, camp_static, goals);

    if (tile_distance(v.pos, pp) > 4 && !moved) return;
    bool attacked = false;
    if (!moved && tile_distance(v.pos, pp) <= 4) attacked = cast_on_camp(prey->camp);
    if (attack_if_in_range(prey->camp)) { reset_and_attack_again(prey->camp); return; }
    if (attacked) return;
    for (const auto& c : v.camps)
        if (attack_if_in_range(c.camp)) { reset_and_attack_again(c.camp); return; }
}
