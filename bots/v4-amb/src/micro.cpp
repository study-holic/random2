// Farming micro: the ring wait (v3-quick give-up), squad positions (melee adjacent, ranged at distance 3 once a
// melee ally is adjacent, everyone on the weakest camp, stuns and damage spells cast when ready), and v2's solo play.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

using cambc::vec2;
using cambc::team;

static constexpr int kGiveUpWait = 12;     // v3-quick: rounds at a respawn ring before giving up
static constexpr int kTankWait = 20;       // ranged: rounds to wait for a melee ally to engage

// v2: attack camps within reach (spell range is 4, so farther camps are skipped before any ability query).
static bool fight_any(const TurnView& v) {
    for (const auto& c : v.camps) {
        if (tile_distance(v.pos, c.camp.spawn_position()) > 4) continue;
        if (cast_on_camp(c.camp)) { attack_if_in_range(c.camp); return true; }
        if (attack_if_in_range(c.camp)) { reset_and_attack_again(c.camp); return true; }
    }
    return false;
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
        if (idx < 0 || a == me || roster_squad[idx] != squad || (mem[std::min(idx, kSquadClaimByte - 1)] & 0x7f) != target + 1) continue;
        const int gap = tile_distance(v.pos, a.position());
        if (gap >= 4 && gap <= 12 && min_distance(a.position(), groups[target].tiles) > mine + 2) { behind = true; break; }
    }
    if (!behind || ++wait_cohesion > 6) { if (!behind) wait_cohesion = 0; return false; }
    return true;
}

void Bot::farm(const TurnView& v, const std::vector<cambc::champion>& allies) {
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
        if (wait_rounds > kGiveUpWait || contested) {
            exclude_group(target, v.round);
            wait_rounds = 0;
            target = choose_target(v, -1);
            return;
        }
        if (!g.ring.is_set(v.pos)) move_towards_closest(static_block, camp_static, g.ring);
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

void Bot::farm_squad(const TurnView& v, const std::vector<cambc::champion>& allies, const LiveCamp* prey) {
    auto& me = cambc::self();
    const auto& g = groups[target];
    if (prey == nullptr) {   // camps out of view: approach without stepping on the spawn tiles
        if (!g.ring.is_set(v.pos)) move_towards_closest(static_block, camp_static, g.ring);
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
    } else {
        engaged = false; if (true) { engaged = true; }
        for (const auto& a : allies)
            if (!(a == me) && tile_distance(a.position(), pp) <= 1) { engaged = true; break; }
        if (engaged) {
            wait_tank = 0;
            const int range = std::max(1, static_cast<int>(me.attack_range()));
            goals = (tiles_around(pp, range) & ~tiles_around(pp, range - 1)) & ~g.mask & ~static_block;
            cambc::bitboard others; others.clear();   // out of reach of the other live monsters when possible
            for (const auto& c : v.camps)
                if (&c != prey && camp_strength(c.camp) > 0) others |= tiles_around(c.camp.spawn_position(), kMonsterRange);
            if (!(goals & ~others).is_empty()) goals &= ~others;
            if (goals.is_empty()) goals = tiles_around(pp, range) & ~live;
        } else {
            bool melee_coming = false;
            for (const auto& a : allies) {
                const int idx = roster_index(a);
                if (idx < 0 || a == me || roster_squad[idx] != squad || !roster_melee[idx]) continue;
                if (a.health() >= a.max_health() * 0.5f && tile_distance(a.position(), pp) <= 15) { melee_coming = true; break; }
            }
            if (++wait_tank > (melee_coming ? kTankWait : 3)) {
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
