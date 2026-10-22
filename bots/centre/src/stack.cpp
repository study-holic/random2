// EXP-077 synchronized stack (centre maps only). The lowest-id live champion (the leader) publishes in shared bytes 6
// and 7 (free on centre maps: squad claims are off there): byte 6 = stack tile offset from the altar, nibbles
// (dx + 7, dy + 7), 0 = no stack; byte 7 = focus enemy offset from the altar, nibbles (dx + 7, dy + 7), 0xFF = none.
// The stack runs while an enemy champion was within 10 tiles of the altar in the last 8 rounds (leader's sight).
// Followers: D isolated rule, C ranged exposure to stunners, B engage only as a stack (focus in range of 3 of us).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#include <array>
#include <optional>
#include <cstdint>
#include <span>

using cambc::vec2;
using cambc::team;
using CT = cambc::champion_type;

static constexpr int kStackNear = 10, kStackHold = 4, kStackRadius = 6, kMinFoe = 5, kFocusMax = 7, kBurstRounds = 3, kLeash2 = 10;

static bool has_stun(const cambc::champion& e) {
    try {
        const auto t = e.champion_name();
        return t == CT::prism || t == CT::ren || t == CT::tavi || t == CT::ryth;
    } catch (...) { return false; }
}
static int carried(const cambc::champion& c) {
    try { return static_cast<int>(c.gem_count()); } catch (...) { return 0; }
}
static bool alive_handle(const cambc::champion& c) {
    try { return c.is_alive(); } catch (...) { return false; }
}

// Leader election and publication; runs every turn of every live champion, only the leader writes.
void Bot::centre_publish(const std::vector<cambc::champion>& enemies) {
    int lead = -1;
    for (std::size_t i = 0; i < roster.size() && lead < 0; ++i) if (alive_handle(roster[i])) lead = static_cast<int>(i);
    if (lead != ally_index) return;
    const int round = static_cast<int>(cambc::round_number());
    std::array<std::uint8_t, 2> out{0, 0xFF};
    std::array<std::uint8_t, 2> old{0, 0xFF};
    cambc::shared_memory.read(6, std::span{old.data(), 2});
    int cx = 0, cy = 0, n = 0;
    bool near = false;
    for (const auto& e : enemies) {
        if (e.health() <= 0) continue;
        const vec2 p = e.position();
        if (tile_distance(p, centre_pos) <= kStackNear) near = true;
        if (tile_distance(p, centre_pos) <= 14) { cx += p.x; cy += p.y; ++n; }
    }
    if (near) stack_until = round + kStackHold;
    if (round > stack_until) { cambc::shared_memory.write(6, std::span<const std::uint8_t>{out.data(), 2}); focus_set = false; return; }
    // Stack tile: keep the published one while valid, else the free tile within 6 of the altar, at least 5 from every
    // enemy, nearest to the point 3 tiles from the altar on the side away from the enemy centroid.
    auto passable = [&](vec2 t) {
        return t.x >= 0 && t.y >= 0 && t.x < map_w && t.y < map_h && !static_block.is_set(t) && !camp_static.is_set(t);
    };
    auto foe_dist = [&](vec2 t) {
        int d = 99;
        for (const auto& e : enemies) if (e.health() > 0) d = std::min(d, tile_distance(e.position(), t));
        return d;
    };
    vec2 tile = centre_pos;
    bool keep = false;
    if (old[0] != 0) {
        tile = vec2(centre_pos.x + (old[0] >> 4) - 7, centre_pos.y + (old[0] & 15) - 7);
        keep = passable(tile) && foe_dist(tile) >= kMinFoe && tile_distance(tile, centre_pos) <= kStackRadius;
    }
    if (!keep) {
        vec2 P = centre_pos;
        if (n > 0) {
            const int dx = centre_pos.x - cx / n, dy = centre_pos.y - cy / n;
            const int m = std::max(1, std::max(std::abs(dx), std::abs(dy)));
            P = vec2(centre_pos.x + dx * 3 / m, centre_pos.y + dy * 3 / m);
        }
        int best = 1 << 20;
        for (int x = centre_pos.x - kStackRadius; x <= centre_pos.x + kStackRadius; ++x)
            for (int y = centre_pos.y - kStackRadius; y <= centre_pos.y + kStackRadius; ++y) {
                const vec2 t(x, y);
                if (!passable(t)) continue;
                const int score = tile_distance(t, P) * 2 + (foe_dist(t) >= kMinFoe ? 0 : 100);
                if (score < best) { best = score; tile = t; }
            }
    }
    out[0] = static_cast<std::uint8_t>(((tile.x - centre_pos.x + 7) << 4) | (tile.y - centre_pos.y + 7));
    // Focus: keep the current one while visible and within 7 of the altar; else most carried gems, then lowest health.
    const cambc::champion* foe = nullptr;
    if (focus_set && focus_unit)
        for (const auto& e : enemies)
            if (e == *focus_unit && e.health() > 0 && tile_distance(e.position(), centre_pos) <= kFocusMax) { foe = &e; break; }
    if (foe == nullptr) {
        int fg = -1;
        for (const auto& e : enemies) {
            if (e.health() <= 0 || tile_distance(e.position(), centre_pos) > kFocusMax) continue;
            const int g = carried(e);
            if (foe == nullptr || g > fg || (g == fg && e.health() < foe->health())) { foe = &e; fg = g; }
        }
    }
    if (foe != nullptr) {
        focus_unit = *foe;
        focus_set = true;
        const vec2 p = foe->position();
        out[1] = static_cast<std::uint8_t>(((p.x - centre_pos.x + 7) << 4) | (p.y - centre_pos.y + 7));
    } else focus_set = false;
    cambc::shared_memory.write(6, std::span<const std::uint8_t>{out.data(), 2});
}

// The stack turn. Returns true when the turn is used.
bool Bot::centre_stack_turn(const TurnView& v, const std::vector<cambc::champion>& enemies,
                            const std::vector<cambc::champion>& allies) {
    stack_on = false;
    if (!centre_map) return false;
    auto& me = cambc::self();
    std::array<std::uint8_t, 2> b{0, 0xFF};
    cambc::shared_memory.read(6, std::span{b.data(), 2});
    bool local_near = false;
    for (const auto& e : enemies) if (e.health() > 0 && tile_distance(e.position(), centre_pos) <= kStackNear) local_near = true;
    stack_on = b[0] != 0 || local_near;
    if (stack_on) ++diag_stack;
    if (holding >= 0) ++diag_hold;
    if (!stack_on) return false;
    vec2 S = centre_pos;
    if (b[0] != 0) S = vec2(centre_pos.x + (b[0] >> 4) - 7, centre_pos.y + (b[0] & 15) - 7);
    stack_tile = S;
    if (retreating || ally_spawn.is_set(v.pos)) return false;
    if (holding < 0 && target >= 0 && min_distance(S, groups[target].tiles) > kStackRadius) target = -1;   // farm within 6 of the stack only

    int n4 = 0, n8 = 0, near_foe = 99;
    for (const auto& a : allies) {
        if (a == me) continue;
        const int d = tile_distance(a.position(), v.pos);
        if (d <= 8) ++n8;
        if (d <= 4) ++n4;
    }
    for (const auto& e : enemies) if (e.health() > 0) near_foe = std::min(near_foe, tile_distance(e.position(), v.pos));
    const int gems = static_cast<int>(me.gem_count());
    const cambc::bitboard goal_s = tiles_around(S, 1) & ~static_block & ~camp_static;
    // D: isolated with an enemy within 8: go to the stack tile and do nothing else (a bag nearer the altar than the foe is tributed).
    if (n8 == 0 && near_foe <= 8) ++d_streak; else d_streak = 0;
    if (n8 == 0 && near_foe <= 8 && d_streak <= 10) {
        ++diag_d;
        if (gems > 0 && tile_distance(v.pos, centre_pos) < near_foe) {
            if (tile_distance(v.pos, centre_pos) <= cambc::constants::submit_gems_range) {
                try { me.submit_gems(altars[centre_altar]); } catch (...) {}
                return true;
            }
            move_towards_closest(static_block, camp_static, tiles_around(altars[centre_altar], cambc::constants::submit_gems_range));
            return true;
        }
        if (!goal_s.is_set(v.pos)) move_towards_closest(static_block, camp_static, goal_s);
        return true;
    }
    if (gems > 0 && (banking || gems >= P_BANK_BAG)) return false;   // EXP-078: carriers bank (bank() sets the flag)
    // Focus enemy (from the leader's byte, matched by position in our own sight).
    const cambc::champion* foe = nullptr;
    if (b[0] != 0 && b[1] != 0xFF) {
        const vec2 fp(centre_pos.x + (b[1] >> 4) - 7, centre_pos.y + (b[1] & 15) - 7);
        for (const auto& e : enemies) if (e.health() > 0 && e.position() == fp) { foe = &e; break; }
    }
    const int range = std::max(1, static_cast<int>(me.attack_range()));
    const bool committed = foe != nullptr && v.round <= engaged_until;
    // C: a ranged champion stays out of every enemy stunner's range 4 (except in a burst on a target already in range).
    if (range >= 3 && me.can_move()) {
        cambc::bitboard safe = tiles_around(v.pos, 1) & ~static_block & ~camp_static;
        bool exposed = false;
        for (const auto& e : enemies)
            if (e.health() > 0 && has_stun(e) && tile_distance(e.position(), v.pos) <= 4) { exposed = true; safe &= ~tiles_around(e.position(), 4); }
        const bool burst_ok = committed && me.attack_tiles().is_set(foe->position());
        if (exposed && !burst_ok) {
            safe.set(v.pos, false);
            if (safe.is_empty()) safe = goal_s;
            if (move_towards_closest(static_block, camp_static, safe)) { ++diag_c; return true; }
        }
    }
    // B: engage only as a stack.
    if (foe != nullptr) {
        int cnt = me.attack_tiles().is_set(foe->position()) ? 1 : 0;
        for (const auto& a : allies) {
            if (a == me) continue;
            if (tile_distance(a.position(), foe->position()) <= std::max(1, static_cast<int>(a.attack_range()))) ++cnt;
        }
        (void)cnt;
        ++diag_foe;
        const bool ready = n4 >= 2;
        const bool in_range = me.attack_tiles().is_set(foe->position());
        const bool fire = committed || (ready && in_range);
        if (ready && in_range && v.round > engaged_until) { engaged_until = v.round + kBurstRounds; ++diag_burst; }
        if (fire && in_range) {
            ++diag_attack;
            cast_on_enemy(*foe);
            const bool gone = P_ALLY_VISION_FIX && !foe->is_visible();
            if (!gone) { cast_on_enemy(*foe); if (attack_if_in_range(*foe)) reset_and_attack_again(*foe); }
            return true;
        }
        if (n4 >= 2) { ++diag_ready; diag_dsum += tile_distance(v.pos, foe->position()); if (in_range) ++diag_inr; }
        if (fire || ready) {   // close in together: only with at least two allies at hand
            auto goals = tiles_around(foe->position(), range) & tiles_around(centre_pos, kLeash2) & ~static_block;
            goals.set(foe->position(), false);
            if (!goals.is_empty() && !in_range) { if (move_towards_closest(static_block, camp_static, goals)) return true; ++diag_fail; }
        }
    }
    // Defensive hit: an enemy already in range while two allies are at hand.
    if (n4 >= 2)
        for (const auto& e : enemies)
            if (e.health() > 0 && attack_if_in_range(e)) { reset_and_attack_again(e); return true; }
    // Join the stack at once when farther than 6 from it; inside 6 the caller farms (targets are within 6 of the stack).
    if (tile_distance(v.pos, S) > kStackRadius && move_towards_closest(static_block, camp_static, goal_s)) return true;
    return false;
}
