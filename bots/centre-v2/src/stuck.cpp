// EXP-018 (v4.2): contested-group memory and the leave rule, and the stuck detector, ported from v4.1 (EXP-017).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>

using cambc::vec2;
using cambc::team;

static constexpr int kContestRadius = P_CONTEST_RADIUS;      // enemy champions within 4 tiles of a spawn tile
static constexpr int kContestRounds = P_CONTEST_ROUNDS;     // skip a contested group for 30 rounds after the sighting
static constexpr float kFinishHealth = P_FINISH_HEALTH;  // finish the current camp of a contested group below 30 per cent
static constexpr int kStuckExclude = P_STUCK_EXCLUDE;      // still rounds before the target group and altar are excluded
static constexpr int kStuckWalk = P_STUCK_WALK;         // still rounds before the walk towards our spawn
static constexpr int kStuckExcludeRounds = P_STUCK_EXCLUDE_ROUNDS;
static constexpr int kStuckWalkTiles = P_STUCK_WALK_TILES;

void Bot::update_contested(const TurnView& v, const std::vector<cambc::champion>& enemies) {
    if (enemies.size() < 2) return;
    std::vector<vec2> foes;
    for (const auto& e : enemies) foes.push_back(e.position());
    for (auto& g : groups) {
        int n = 0;
        for (auto f : foes) if (min_distance(f, g.tiles) <= kContestRadius) ++n;
        if (n >= 2) g.contested_round = v.round;
    }
}

bool Bot::contested(int index, int round) const {
    return index >= 0 && round - groups[index].contested_round < kContestRounds;
}

// Target contested: finish the current camp only when it is below 30 per cent health, else move to another group
// (when one exists). The current camp is the nearest live camp of the target within 4 tiles.
bool Bot::leave_contested(const TurnView& v) {
    if (centre_map || target < 0 || holding == target || !contested(target, v.round)) return false;   // EXP-073: the centre is fought for
    const LiveCamp* cur = nullptr; int best = 5;
    for (const auto& c : v.camps) {
        if (c.group != target) continue;
        const int d = tile_distance(v.pos, c.camp.spawn_position());
        if (d < best) { best = d; cur = &c; }
    }
    if (cur != nullptr) {
        float mx = 0;
        try { mx = cur->camp.max_health(); } catch (...) {}
        if (mx > 0 && cur->health < kFinishHealth * mx) return false;
    }
    const int next = choose_target(v, target);
    if (next < 0) return false;
    exclude_group(target, v.round);
    target = next;
    wait_rounds = 0;
    wait_tank = 0;
    return true;
}

// Runs at the start of every turn: rounds alive at the same position, not recalling, tributing, capturing,
// teleporting or attacking (last turn), and not healing on the spawn.
void Bot::update_stuck() {
    auto& me = cambc::self();
    if (!me.is_alive()) { still_rounds = 0; walk_home = 0; last_pos = vec2(-1, -1); return; }
    const vec2 p = me.position();
    const bool healing = ally_spawn.is_set(p) && me.health() < me.max_health();
    const bool busy = me.is_recalling() || me.is_submitting_gems() || me.is_teleporting() || holding >= 0 || attacked || healing
        || (centre_map && me.gem_count() == 0 && tile_distance(p, centre_pos) <= 8);   // EXP-073: holding the centre is not stuck
    if (busy || !(p == last_pos)) still_rounds = 0; else ++still_rounds;
    last_pos = p;
}

// At 15 still rounds: exclude the target group and the current altar for 60 rounds, clear the target, re-pick.
// At 30: walk 5 tiles towards our spawn region, then re-pick. Returns true when the turn is used.
bool Bot::handle_stuck(const TurnView& v) {
    auto& me = cambc::self();
    if (walk_home > 0) {
        --walk_home;
        const bool done = walk_home == 0 || tile_distance(walk_from, v.pos) >= kStuckWalkTiles || ally_spawn.is_set(v.pos);
        if (!done && move_towards_closest(static_block, camp_static, ally_spawn)) return true;
        walk_home = 0;
        target = choose_target(v, -1);
        return false;
    }
    if (still_rounds == kStuckExclude) {
        if (target >= 0) {
            groups[target].excluded_until = v.round + kStuckExcludeRounds;
            if (is_squad_group(target) && read_squad_claim() == target) write_squad_claim(-1);
        }
        if (me.gem_count() > 0 && !altars.empty()) {   // the altar this bag was heading to: the nearest usable one
            int best = -1, best_d = 1 << 20;
            for (std::size_t i = 0; i < altars.size(); ++i) {
                const int d = tile_distance(v.pos, altars[i]) + (lock_until[i] > v.round ? 1000 : 0);
                if (d < best_d) { best_d = d; best = static_cast<int>(i); }
            }
            if (best >= 0) altar_skip_until[best] = v.round + kStuckExcludeRounds;
        }
        target = -1;
        wait_rounds = 0;
        wait_tank = 0;
        target = choose_target(v, -1);
        return false;
    }
    if (still_rounds >= kStuckWalk) {
        still_rounds = 0;
        walk_from = v.pos;
        walk_home = kStuckWalkTiles * 2;
        banking = false;
        carry_since = v.round;
        target = -1;
        return move_towards_closest(static_block, camp_static, ally_spawn);
    }
    return false;
}

// Altar submit tiles not locked and not stuck-skipped; falls back to unlocked, then to every altar.
cambc::bitboard Bot::altar_goal_tiles(int round) const {
    cambc::bitboard goals; goals.clear();
    for (int pass = 0; pass < 3 && goals.is_empty(); ++pass)
        for (std::size_t i = 0; i < altars.size(); ++i) {
            const bool unlocked = lock_until[i] <= round, unskipped = altar_skip_until[i] <= round;
            if (pass == 2 || (unlocked && (pass == 1 || unskipped)))
                goals |= tiles_around(altars[i], cambc::constants::submit_gems_range);
        }
    return goals;
}
