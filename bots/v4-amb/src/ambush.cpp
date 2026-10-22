// EXP-020 (v4-amb): ambush opening on maps where the enemy's first camp group is close. All champions walk as one
// stack to the tile beside that group nearest to us (via a waypoint on our side of the map), focus one enemy
// champion at a time while any is within 8 tiles of the stack, then return to v4.2 behaviour when the ambush ends.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include <algorithm>
#include <bit>
#include <iostream>
#include <span>

using cambc::vec2;
using cambc::team;

static constexpr int kMaxD = 25;          // ambush on when the first group is within this many tiles
static constexpr int kEndRound = 120;
static constexpr int kQuietRounds = 10;   // no enemy visible this long after arrival: over
static constexpr int kFocusRange = 8;
static constexpr float kLow = 0.3f;
static constexpr int kStackRadius = 2;    // no member further than this from the stack centre
static constexpr int kPaceGap = 2;        // hold when this much closer to the goal than the rearmost member
static constexpr int kMaxHold = 6;
static constexpr int kIdleLimit = 10;     // travel step failed this many rounds: give up the ambush
static constexpr int kStamp = 64;         // shared round stamps are kStamp + round / 2

std::uint8_t Bot::shared_byte(int i) const {
    std::uint8_t b = 0;
    cambc::shared_memory.read(i, std::span{&b, 1});
    return b;
}

void Bot::set_shared_byte(int i, std::uint8_t value) {
    cambc::shared_memory.write(i, std::span<const std::uint8_t>{&value, 1});
}

static long dist2(vec2 a, vec2 b) {
    const long dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// D = distance from our spawn centre to the enemy's first group (the group nearest to the enemy spawn). On when
// D <= 25 and the team has 4 or 5 champions. Staging tile: the free tile beside that group nearest to us.
void Bot::ambush_init(vec2 ally_centre) {
    int first = -1;
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
        if (groups[i].monolith) continue;
        if (first < 0 || groups[i].dist_enemy < groups[first].dist_enemy) first = i;
    }
    if (first >= 0) ambush_d = min_distance(ally_centre, groups[first].tiles);
    // Picks: the Cassi and Issac pick yields two champions.
    bool cassi = false, issac = false;
    for (const auto& r : roster) {
        cassi |= r.champion_name() == cambc::champion_type::cassi;
        issac |= r.champion_name() == cambc::champion_type::issac;
    }
    const int n = static_cast<int>(roster.size());
    const int picks = n - (cassi && issac ? 1 : 0);
    const bool on = first >= 0 && ambush_d <= kMaxD && (picks == 4 || picks == 5);
    if (ally_index == 0)
        std::cout << "v4-amb ambush " << (on ? "ON" : "off") << " D=" << ambush_d << " picks=" << picks << " champions=" << n << "\n";
    if (!on) return;
    int best = 1 << 20;
    bool found = false;
    for (auto t : groups[first].ring.positions()) {
        if (static_block.is_set(t) || camp_static.is_set(t)) continue;
        const int d = tile_distance(ally_centre, t);
        if (d < best) { best = d; ambush_stage = t; found = true; }
    }
    if (!found) return;
    enemy_roster = cambc::champions(team::enemy);
    enemy_spawn = cambc::map::spawn_region(team::enemy);
    // Enemy spawn centre (same integer mean as init).
    long sx = 0, sy = 0, cnt = 0;
    for (auto t : enemy_spawn.positions()) { sx += t.x; sy += t.y; ++cnt; }
    const vec2 ec = cnt ? vec2(static_cast<int>(sx / cnt), static_cast<int>(sy / cnt)) : vec2(0, 0);
    // Waypoint: walking from the staging tile back towards our spawn, the first free tile that is on our side of the
    // map centre (closer to our spawn than to theirs). Going there first keeps the route on our half.
    ambush_way_done = true;
    for (int k = 0; k <= 40; ++k) {
        const vec2 p(ambush_stage.x + (ally_centre.x - ambush_stage.x) * k / 40,
                     ambush_stage.y + (ally_centre.y - ambush_stage.y) * k / 40);
        if (dist2(p, ally_centre) > dist2(p, ec)) continue;
        if (static_block.is_set(p) || camp_static.is_set(p)) continue;
        if (tile_distance(p, ally_centre) > 3) { ambush_way = p; ambush_way_done = false; }
        break;
    }
    ambush_group = first;
    ambush_enabled = true;
}

// Per-turn bookkeeping of an alive champion: kills, enemies seen, arrival, and the four end conditions.
void Bot::ambush_update(const TurnView& v, const std::vector<cambc::champion>& enemies,
                        const std::vector<cambc::champion>& allies) {
    if (!ambush_active()) return;
    auto& me = cambc::self();
    const int round = v.round;
    std::uint8_t flags = shared_byte(kByteFlags);
    if (flags & 0x80) { ambush_over = true; return; }
    const std::uint8_t flags_in = flags;
    for (std::size_t i = 0; i < enemy_roster.size() && i < 7; ++i) {
        bool alive = true;
        try { alive = enemy_roster[i].is_alive(); } catch (...) {}
        if (!alive) ambush_killed |= static_cast<std::uint8_t>(1u << i);
    }
    if (!enemies.empty()) {
        for (const auto& e : enemies)
            for (std::size_t i = 0; i < enemy_roster.size() && i < 7; ++i)
                if (enemy_roster[i] == e) flags |= static_cast<std::uint8_t>(1u << i);
        set_shared_byte(kByteLastSeen, static_cast<std::uint8_t>(kStamp + std::min(round, 250) / 2));
    }
    if (ambush_arrived && shared_byte(kByteArrived) == 0)
        set_shared_byte(kByteArrived, static_cast<std::uint8_t>(kStamp + std::min(round, 250) / 2));

    // End conditions.
    const unsigned seen = flags & 0x7f;
    bool over = false;
    if (seen != 0 && (seen & ~static_cast<unsigned>(ambush_killed)) == 0) over = true;   // every enemy seen killed once
    const int arrived = shared_byte(kByteArrived);
    if (arrived != 0) {
        const int last_seen = shared_byte(kByteLastSeen);
        const int last = std::max(2 * (arrived - kStamp), last_seen ? 2 * (last_seen - kStamp) : 0);
        if (round - last >= kQuietRounds) over = true;
    }
    if (round >= kEndRound) over = true;
    int low = me.health() < me.max_health() * kLow ? 1 : 0;
    for (const auto& a : allies) if (!(a == me) && a.health() < a.max_health() * kLow) ++low;
    for (const auto& r : roster) {
        if (r == me) continue;
        bool alive = true;
        try { alive = r.is_alive(); } catch (...) {}
        if (!alive) ++low;
    }
    if (low >= 2) over = true;
    if (over) { flags |= 0x80; ambush_over = true; }
    if (flags != flags_in) set_shared_byte(kByteFlags, flags);
}

static vec2 stack_centre(const TurnView& v, const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    long sx = v.pos.x, sy = v.pos.y, n = 1;
    for (const auto& a : allies) {
        if (a == me || a.health() <= 0) continue;
        sx += a.position().x; sy += a.position().y; ++n;
    }
    return vec2(static_cast<int>(sx / n), static_cast<int>(sy / n));
}

// Returns true when the turn is used: focus fire while an enemy champion is within 8 tiles of the stack centre,
// otherwise the walk to the staging tile. After arrival (and with no enemy near) the normal flow farms the group.
bool Bot::ambush_turn(const TurnView& v, const std::vector<cambc::champion>& enemies,
                      const std::vector<cambc::champion>& allies) {
    const vec2 centre = stack_centre(v, allies);
    const cambc::champion* foe = nullptr;
    int foe_d = 99, foe_idx = 99;
    for (const auto& e : enemies) {
        if (e.health() <= 0 || enemy_spawn.is_set(e.position())) continue;
        const int d = tile_distance(centre, e.position());
        if (d > kFocusRange) continue;
        int idx = 98;
        for (std::size_t i = 0; i < enemy_roster.size(); ++i) if (enemy_roster[i] == e) { idx = static_cast<int>(i); break; }
        const bool better = foe == nullptr || e.health() < foe->health()
            || (e.health() == foe->health() && (d < foe_d || (d == foe_d && idx < foe_idx)));
        if (better) { foe = &e; foe_d = d; foe_idx = idx; }
    }
    if (foe != nullptr) { ambush_focus(v, *foe, centre); return true; }
    if (ambush_arrived) return false;
    return ambush_travel(v, allies);
}

// Walk to the waypoint, then the staging tile, at the pace of the rearmost visible member.
bool Bot::ambush_travel(const TurnView& v, const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    if (tile_distance(v.pos, ambush_stage) <= 3) { ambush_arrived = true; return false; }
    if (!ambush_way_done && tile_distance(v.pos, ambush_way) <= 2) ambush_way_done = true;
    const vec2 goal = ambush_way_done ? ambush_stage : ambush_way;
    int mine = tile_distance(v.pos, goal), rear = mine;
    for (const auto& a : allies) {
        if (a == me || a.health() <= 0) continue;
        rear = std::max(rear, tile_distance(a.position(), goal));
    }
    if (mine + kPaceGap <= rear && ambush_hold < kMaxHold) { ++ambush_hold; return true; }
    ambush_hold = 0;
    bool moved = false;
    if (me.can_move()) moved = move_towards_closest(static_block, camp_static, tiles_around(goal, 1));
    if (!moved && me.can_move()) {
        if (!ambush_way_done) ambush_way_done = true;   // waypoint unreachable: go straight to the staging tile
        else if (++ambush_idle >= kIdleLimit) set_shared_byte(kByteFlags, shared_byte(kByteFlags) | 0x80);
    } else {
        ambush_idle = 0;
    }
    return true;
}

// Focus fire on `foe`: members under 30 per cent step behind the stack; the others close in but stay within 2
// tiles of the stack centre. Every ready damage or stun spell, then basic attacks (with attack resets).
void Bot::ambush_focus(const TurnView& v, const cambc::champion& foe, vec2 centre) {
    auto& me = cambc::self();
    const vec2 fp = foe.position();
    const bool low = me.health() < me.max_health() * kLow;
    const int range = std::max(1, static_cast<int>(me.attack_range()));
    if (me.can_move()) {
        const auto blocked = static_block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles() | camp_static;
        auto disc = tiles_around(centre, kStackRadius) & ~blocked;
        if (tile_distance(v.pos, centre) <= kStackRadius) disc.set(v.pos, true);
        if (low) {
            vec2 best = v.pos;
            int best_foe = tile_distance(v.pos, fp), best_ctr = tile_distance(v.pos, centre);
            for (auto t : disc.positions()) {
                if (tile_distance(t, v.pos) > 1) continue;
                const int df = tile_distance(t, fp), dc = tile_distance(t, centre);
                if (df > best_foe || (df == best_foe && dc < best_ctr)) { best = t; best_foe = df; best_ctr = dc; }
            }
            if (best != v.pos) { try { me.move_absolute(best); } catch (...) {} }
        } else if (!me.attack_tiles().is_set(fp)) {
            auto goals = tiles_around(fp, range) & disc;
            goals.set(fp, false);
            if (range > 1 && !(goals & ~tiles_around(fp, range - 1)).is_empty()) goals &= ~tiles_around(fp, range - 1);
            if (goals.is_empty()) {   // nothing in range inside the stack disc: the disc tile nearest to the target
                vec2 best = v.pos;
                int best_d = 1 << 20;
                for (auto t : disc.positions()) {
                    const int d = tile_distance(t, fp) * 8 + tile_distance(t, v.pos);
                    if (d < best_d) { best_d = d; best = t; }
                }
                if (best != v.pos) goals.set(best, true);
            }
            if (!goals.is_empty()) move_towards_closest(static_block, camp_static, goals);
        }
    }
    for (int k = 0; k < 2; ++k) if (!cast_on_enemy(foe)) break;
    if (attack_if_in_range(foe)) reset_and_attack_again(foe);
}
