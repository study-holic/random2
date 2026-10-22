// EXP-073 centre control (the rank-1 team on corners): map test at draft, group filter, and the centre fight rule.
// A centre map has an altar within 6 tiles of the map centre and at least 2 gem-cluster camp groups within 10 tiles
// of that altar. There every champion farms only those groups, banks at that altar, waits beside respawning groups
// within 8 tiles of it, and attacks any enemy champion within 8 tiles of it (never leaving a 10-tile leash).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include "draft.h"
#include <algorithm>
#include <array>
#include <cstdint>

using cambc::vec2;
using cambc::team;

static constexpr int kMagic = 0xC7;
static constexpr int kCentreAltarDist = 6, kCentreGroupDist = 10, kHoldRadius = 8, kFoeRadius = 8, kJoinRadius = 6, kLeash = 10;
static constexpr float kBreakOff = 0.4f;

// Pure map test: the altar within 6 of the map centre with the most gem-cluster groups within 10 of it (at least 2).
static bool centre_pick(int w, int h, const std::vector<vec2>& altars, const std::vector<std::vector<vec2>>& gem_groups, vec2& out) {
    const vec2 mid(w / 2, h / 2);
    int best_n = 1, best_d = 1 << 20;
    bool found = false;
    for (auto a : altars) {
        const int dc = tile_distance(a, mid);
        if (dc > kCentreAltarDist) continue;
        int n = 0;
        for (const auto& tiles : gem_groups) if (min_distance(a, tiles) <= kCentreGroupDist) ++n;
        if (n > best_n || (n == best_n && found && dc < best_d)) { best_n = n; best_d = dc; out = a; found = true; }
    }
    return found;
}

// Draft turn: decide from the map and store {magic, flag, altar x, altar y} in draft memory.
CentreDraft centre_draft_decision() {
    static bool done = false;
    static CentreDraft result;
    if (done) return result;
    done = true;
    try {
        std::vector<std::vector<vec2>> gem_groups;
        for (auto g : cambc::map::camp_groups()) {
            bool gem = false;
            for (const auto& c : g.camps()) gem |= c.is_gem_cluster();
            if (gem) gem_groups.push_back(g.spawn_tiles().positions());
        }
        vec2 altar(0, 0);
        const bool yes = centre_pick(cambc::map::width(), cambc::map::height(), cambc::map::altars().positions(), gem_groups, altar);
        result = {yes, altar};
        const std::array<std::uint8_t, 4> bytes{static_cast<std::uint8_t>(kMagic), static_cast<std::uint8_t>(yes ? 1 : 0),
            static_cast<std::uint8_t>(altar.x), static_cast<std::uint8_t>(altar.y)};
        cambc::draft_memory = bytes;
    } catch (...) {}
    return result;
}

// Game init: read the draft decision (or recompute from the cached groups), then mark the groups near the altar.
void Bot::setup_centre() {
    centre_map = false;
    centre_altar = -1;
    bool yes = false;
    vec2 want(0, 0);
    const auto& m = cambc::draft_memory;
    if (m.size() >= 4 && std::to_integer<int>(m[0]) == kMagic) {
        yes = std::to_integer<int>(m[1]) == 1;
        want = vec2(std::to_integer<int>(m[2]), std::to_integer<int>(m[3]));
    } else {
        std::vector<std::vector<vec2>> gem_groups;
        for (const auto& g : groups) if (g.has_gem_cluster) gem_groups.push_back(g.tiles);
        yes = centre_pick(cambc::map::width(), cambc::map::height(), cambc::map::altars().positions(), gem_groups, want);
    }
    if (!yes) return;
    for (std::size_t i = 0; i < altars.size(); ++i) if (altars[i] == want) centre_altar = static_cast<int>(i);
    if (centre_altar < 0) return;
    centre_pos = want;
    map_w = cambc::map::width();
    map_h = cambc::map::height();
    squad_capable = false;   // EXP-077: shared bytes 6 and 7 carry the stack, so squad claims are off
    centre_group.assign(groups.size(), 0);
    int n = 0;
    for (std::size_t i = 0; i < groups.size(); ++i)
        if (!groups[i].monolith && !groups[i].squad_target && min_distance(want, groups[i].tiles) <= kCentreGroupDist) { centre_group[i] = 1; ++n; }
    centre_map = n > 0;
}

// Hold tiles beside a respawning group: the ring part within 8 tiles of the altar when there is one.
cambc::bitboard Bot::centre_hold_goals(const cambc::bitboard& ring) const {
    if (!centre_map) return ring;
    const auto near = ring & tiles_around(centre_pos, kHoldRadius);
    return near.is_empty() ? ring : near;
}

// Idle near the altar (no target): stay within 3 tiles of it.
void Bot::centre_idle(const TurnView& v) {
    const cambc::vec2 at = stack_on ? stack_tile : centre_pos;
    if (tile_distance(v.pos, at) <= (stack_on ? 1 : 3)) return;
    move_towards_closest(static_block, camp_static, tiles_around(at, stack_on ? 1 : 3) & ~static_block);
}

static int carried_gems(const cambc::champion& c) {
    try { return static_cast<int>(c.gem_count()); } catch (...) { return 0; }
}

// Centre fight: an enemy champion within 8 tiles of the altar is attacked by every champion within 6 of the altar
// (or within 5 tiles of that enemy while inside the 10-tile leash). Focus: most carried gems, then lowest health.
bool Bot::centre_fight(const TurnView& v, const std::vector<cambc::champion>& enemies, const std::vector<cambc::champion>& allies) {
    if (!centre_map || enemies.empty() || retreating) return false;
    auto& me = cambc::self();
    if (me.health() < me.max_health() * kBreakOff || ally_spawn.is_set(v.pos)) return false;
    if (tile_distance(v.pos, centre_pos) > kLeash) return false;
    const cambc::champion* foe = nullptr;
    int foe_gems = 0;
    for (const auto& e : enemies) {
        if (e.health() <= 0 || tile_distance(e.position(), centre_pos) > kFoeRadius) continue;
        const int g = carried_gems(e);
        if (foe == nullptr || g > foe_gems || (g == foe_gems && e.health() < foe->health())) { foe = &e; foe_gems = g; }
    }
    if (foe == nullptr) return false;
    if (tile_distance(v.pos, centre_pos) > kJoinRadius && tile_distance(v.pos, foe->position()) > 5) return false;
    const bool in_range = me.attack_tiles().is_set(foe->position());
    if (!in_range) {
        // EXP-076 local-advantage gate: allies within 4 (self included) must match the enemies within 6 of the foe.
        int mine = 1, theirs = 0;
        for (const auto& a : allies) if (!(a == me) && tile_distance(a.position(), v.pos) <= 4) ++mine;
        for (const auto& e : enemies) if (e.health() > 0 && tile_distance(e.position(), foe->position()) <= 6) ++theirs;
        if (mine + P_CENTRE_ADV < theirs) { centre_idle(v); return true; }

        const int range = std::max(1, static_cast<int>(me.attack_range()));
        auto goals = tiles_around(foe->position(), range) & tiles_around(centre_pos, kLeash) & ~static_block;
        goals.set(foe->position(), false);
        if (goals.is_empty() || !move_towards_closest(static_block, camp_static, goals)) return false;
        if (attack_if_in_range(*foe)) reset_and_attack_again(*foe);
        return true;
    }
    cast_on_enemy(*foe);
    const bool gone = P_ALLY_VISION_FIX && !foe->is_visible();
    if (!gone && attack_if_in_range(*foe)) reset_and_attack_again(*foe);
    return true;
}

// EXP-076: on a centre map a champion more than 10 tiles from the altar with an enemy in view walks back to within 8
// of it (no recall or tribute is in progress here: play_turn_inner returns earlier for those). A retreating
// champion keeps its walk home.
bool Bot::centre_recentre(const TurnView& v, const std::vector<cambc::champion>& enemies) {
    if (!centre_map || enemies.empty() || retreating || ally_spawn.is_set(v.pos)) return false;
    if (tile_distance(v.pos, centre_pos) <= kLeash) return false;
    return move_towards_closest(static_block, camp_static, tiles_around(centre_pos, kHoldRadius) & ~static_block);
}

// EXP-077: a group is a legal target on a centre map when it lies within 10 of the altar and, while a stack is
// active, within 6 of the stack tile.
bool Bot::centre_ok(int group) const {
    if (!centre_map) return true;
    if (!centre_group[group]) return false;
    return !stack_on || min_distance(stack_tile, groups[group].tiles) <= 6;
}
