// EXP-071 threat layer (P_THREAT_LAYER): survive and bank. A dying carrier's bag goes to the killer and a stun
// interrupts a tribute or a recall (GameRules.md, "When a champion dies" and the channel rules), so a carrier walks to
// an altar with no enemy within P_SAFE_DIST, else to allies or the spawn, and never starts a tribute with an enemy
// that close. Outnumbered champions regroup at allies (with hysteresis); contested groups are leashed (leashed()).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>

using cambc::vec2;
using cambc::team;

static constexpr int kRadius = P_THREAT_RADIUS, kBag = P_THREAT_BAG, kSafe = P_SAFE_DIST;
static constexpr int kLeash = P_LEASH_ALLIES, kRegroupRounds = P_REGROUP_ROUNDS;
static constexpr int kRegroupMin = 3;   // a regroup lasts at least this many rounds (EXP-004, E19: no oscillation)

static int nearest_to(vec2 p, const std::vector<vec2>& pts) {
    int d = 1 << 20;
    for (auto t : pts) d = std::min(d, tile_distance(p, t));
    return d;
}

// A group with an enemy seen near it in the last P_LEASH_MEMORY rounds is entered only with P_LEASH_ALLIES allies
// within 6 tiles (allies6 is refreshed at the start of every turn).
bool Bot::leashed(int index, int round) const {
    return P_THREAT_LAYER && (P_THREAT_MODE & 4) && index >= 0 && round - foe_seen[index] < P_LEASH_MEMORY && allies6 < kLeash;
}

// Walk to the closer of the nearest other ally (a tile beside it) and the spawn, keeping out of the enemies' safe
// radius when a goal tile outside it exists. 0 = moved, 1 = already at the goal, 2 = no step.
int Bot::step_to_friends(const std::vector<vec2>& foes, const std::vector<cambc::champion>& allies) {
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
    for (auto f : foes) zone |= tiles_around(f, kSafe);
    const cambc::bitboard open = goals & ~zone;
    if (!open.is_empty()) goals = open;
    if (goals.is_set(pos)) return 1;
    return move_towards_closest(static_block, camp_static, goals) ? 0 : 2;
}

// No ally and no spawn reachable: step to a tile that raises the distance to the nearest enemy.
bool Bot::flee_step(const TurnView& v, const std::vector<vec2>& foes) {
    if (foes.empty()) return false;
    const int cur = nearest_to(v.pos, foes);
    cambc::bitboard goals; goals.clear();
    for (auto t : tiles_around(v.pos, 2).positions())
        if (nearest_to(t, foes) > cur) goals.set(t, true);
    goals &= ~static_block & ~camp_static & ~danger_zone(v);
    goals.set(v.pos, false);
    return !goals.is_empty() && move_towards_closest(static_block, camp_static, goals);
}

bool Bot::threat_layer(const TurnView& v, const std::vector<cambc::champion>& enemies,
                       const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    const int round = v.round;
    std::vector<vec2> foes, all_foes;   // within the threat radius / every visible enemy
    for (const auto& e : enemies) {
        if (e.health() <= 0) continue;
        const vec2 p = e.position();
        all_foes.push_back(p);
        if (tile_distance(v.pos, p) <= kRadius) foes.push_back(p);
    }
    int friends4 = 0;
    for (const auto& a : allies) if (!(a == me) && tile_distance(v.pos, a.position()) <= 4) ++friends4;
    if (!foes.empty()) last_foe_round = round;
    const int gems = static_cast<int>(me.gem_count());

    // Fight in place when an enemy is in attack range (regroup hold, bag hold): enemy_response attacks and casts.
    auto hold = [&]() {
        for (auto f : all_foes) if (me.attack_tiles().is_set(f)) { enemy_response(v, enemies, allies); break; }
        return true;
    };

    // 2. Bank-first. Walk to an altar with no enemy within P_SAFE_DIST; when none exists, only survive (leave the
    // enemy's safe radius towards allies or the spawn) and otherwise play as v4-13. The flee/hold time of one bag is
    // capped, so a carrier cannot ping-pong at the edge of vision for ever.
    if (gems == 0) { bag_fled = false; flee_budget = 0; bank_altar = -1; }
    const bool latched = bag_fled && round - last_foe_round < kRegroupRounds;
    if ((P_THREAT_MODE & 1) && gems >= kBag && flee_budget < 8 * kRegroupRounds && (!foes.empty() || latched)) {
        ++flee_budget;
        if (foes.empty()) return hold();   // latched: the enemies just left view
        const int range = static_cast<int>(cambc::constants::submit_gems_range);
        const bool me_safe = nearest_to(v.pos, all_foes) > kSafe;
        auto usable = [&](std::size_t i) { return lock_until[i] <= round && altar_skip_until[i] <= round; };
        int pick = -1, pick_d = 1 << 20;
        if (bank_altar >= 0 && usable(bank_altar) && nearest_to(altars[bank_altar], all_foes) > 3) {   // sticky choice
            pick = bank_altar;
            pick_d = tile_distance(v.pos, altars[pick]);
        } else {
            for (std::size_t i = 0; i < altars.size(); ++i) {
                if (!usable(i) || nearest_to(altars[i], all_foes) <= kSafe) continue;
                const int d = tile_distance(v.pos, altars[i]);
                if (d < pick_d) { pick_d = d; pick = static_cast<int>(i); }
            }
            bank_altar = pick;
        }
        if (pick >= 0) {
            banking = true;
            if (carry_since < 0) carry_since = round;
            holding = -1;
            target = -1;
            if (pick_d <= range) {
                if (me_safe) { bag_fled = false; return bank(v, enemies, allies); }
            } else if (move_towards_closest(static_block, camp_static, tiles_around(altars[pick], range))) {
                bag_fled = false;
                return true;
            }
        }
        if (me_safe) return false;   // no safe altar and no enemy close: v4-13 rules (farm on, bank by its own triggers)
        bag_fled = true;
        holding = -1;
        const int r = step_to_friends(foes, allies);
        if (r == 0) return true;
        if (r == 1) return hold();
        return flee_step(v, foes);   // false: the enemy rules take the turn
    }

    // 3. Regroup: outnumbered within the radius.
    if (!(P_THREAT_MODE & 2)) return false;
    if (regrouping) {
        const bool done = round - regroup_start >= kRegroupMin
            && (friends4 >= kLeash || round - last_foe_round >= kRegroupRounds);
        if (done) regrouping = false;
    }
    if (!regrouping && static_cast<int>(foes.size()) > friends4 + 1) { regrouping = true; regroup_start = round; }
    if (regrouping) {
        holding = -1;
        const int r = step_to_friends(foes, allies);
        if (r == 0) return true;
        if (r == 1) return hold();
    }
    return false;
}
