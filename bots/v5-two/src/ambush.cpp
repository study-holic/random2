// v5-ambush: the stack walks to the enemy half and wipes the enemy champions one at a time (single shared focus
// target), then returns to v5-stack behaviour. Active only when kAmbush is true (bot.h).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include <algorithm>
#include <bit>
#include <vector>

using cambc::vec2;
using cambc::team;

static constexpr int kNoSightRounds = 120;     // no enemy champion seen by then: fall back to the plain stack
static constexpr int kQuietRounds = 10;        // wipe over after a kill and this long without an enemy in view
static constexpr int kUnseenRounds = 3;        // focus target out of vision this long: choose again
static constexpr float kLowHealth = 0.3f;

int Bot::ambush_state() const { return shared_byte(kByteState) & 3; }

bool Bot::ambush_on() const { return kAmbush && initialised && ambush_state() != 2; }

// The group on the enemy half nearest to the enemy spawn region (where their champions farm first).
void Bot::ambush_init(const vec2&) {
    if (!kAmbush) return;
    int best = -1;
    for (int pass = 0; pass < 2 && best < 0; ++pass)
        for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
            const auto& g = groups[i];
            if (g.monolith || (pass == 0 && g.dist_enemy >= g.dist_ally)) continue;
            if (best < 0 || g.dist_enemy < groups[best].dist_enemy) best = i;
        }
    ambush_group = best;
}

// Seen / kill / focus bookkeeping in shared memory; ends the wipe when the coordinator's conditions hold.
void Bot::ambush_bookkeeping(const TurnView& v, const std::vector<cambc::champion>& enemies,
                             const std::vector<cambc::champion>& allies) {
    if (!kAmbush) return;
    const int round = v.round;
    std::uint8_t st = shared_byte(kByteState);
    int state = st & 3;
    if (state == 2) return;
    auto& me = cambc::self();
    auto end_wipe = [&]() { set_shared_byte(kByteState, 2); };

    // Two or more members under 30 per cent: the stack walks home (shared call) and the wipe is over.
    int low = me.health() < me.max_health() * kLowHealth ? 1 : 0;
    for (const auto& a : allies) if (!(a == me) && a.health() > 0 && a.health() < a.max_health() * kLowHealth) ++low;
    const int call = shared_byte(kByteHome);
    if (low >= 2 && !enemies.empty()) {
        const int stamp = std::min(254, round / 2) + 1;
        set_shared_byte(kByteHome, static_cast<std::uint8_t>(stamp));
        home_stamp = stamp;
        home_call = true;
        end_wipe();
        return;
    }
    if (call != 0 && call != home_stamp) { home_stamp = call; home_call = true; end_wipe(); return; }

    if (!enemies.empty()) {
        set_shared_byte(kByteSeen, static_cast<std::uint8_t>(std::min(255, round / 2)));
        if (state == 0) { state = 1; st = static_cast<std::uint8_t>((st & ~3) | 1); set_shared_byte(kByteState, st); }
    }
    if (state == 0 && round >= kNoSightRounds) { end_wipe(); return; }

    // Focus target: dead (a kill), still in view, or out of view for kUnseenRounds.
    std::uint8_t mask = shared_byte(kByteKills);
    int f = static_cast<int>((st >> 2) & 15) - 1;
    if (f >= 0 && f < static_cast<int>(enemy_roster.size())) {
        bool dead = false;
        try { dead = !enemy_roster[f].is_alive(); } catch (...) {}
        const cambc::champion* seen = nullptr;
        for (const auto& e : enemies) if (e == enemy_roster[f]) { seen = &e; break; }
        bool clear = false;
        if (dead) {
            mask = static_cast<std::uint8_t>(mask | (1u << f));
            clear = true;
        } else if (seen != nullptr && seen->health() > 0) {
            focus_unseen = 0; focus_seen = true;
            focus_last = seen->position();
            focus_last_hp = seen->health() / std::max(1.0f, static_cast<float>(seen->max_health()));
            if (enemy_spawn.is_set(focus_last)) clear = true;   // healing on its spawn: unreachable
        } else if (focus_seen && ++focus_unseen >= kUnseenRounds) {
            if (focus_last_hp < 0.2f) mask = static_cast<std::uint8_t>(mask | (1u << f));   // died out of sight (guess)
            clear = true;
        }
        if (clear) {
            set_shared_byte(kByteState, static_cast<std::uint8_t>(st & 3));
            set_shared_byte(kByteKills, mask);
            focus_unseen = 0; focus_seen = false;
        }
    }
    const int kills = std::popcount(static_cast<unsigned>(mask));
    const bool all_dead = kills >= static_cast<int>(enemy_roster.size());
    const bool quiet = round - 2 * shared_byte(kByteSeen) >= kQuietRounds;
    if (all_dead || (kills >= 1 && state >= 1 && quiet)) end_wipe();
}

// One enemy at a time: the common focus target is the visible enemy with the lowest health (ties: nearest to the
// chooser), kept in shared byte 7. Everyone walks to it, casts every ready spell on it and attacks; nobody touches
// camps or other enemies meanwhile. Members under 30 per cent step back behind the stack.
bool Bot::wipe_turn(const TurnView& v, const std::vector<cambc::champion>& enemies,
                    const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    std::uint8_t st = shared_byte(kByteState);
    int f = static_cast<int>((st >> 2) & 15) - 1;
    if (f >= static_cast<int>(enemy_roster.size())) f = -1;
    if (f < 0) {
        int best = -1; float best_hp = 1e30f; int best_d = 99;
        for (const auto& e : enemies) {
            if (e.health() <= 0 || enemy_spawn.is_set(e.position())) continue;
            int idx = -1;
            for (std::size_t i = 0; i < enemy_roster.size(); ++i) if (enemy_roster[i] == e) { idx = static_cast<int>(i); break; }
            if (idx < 0) continue;
            const int d = tile_distance(v.pos, e.position());
            if (e.health() < best_hp || (e.health() == best_hp && d < best_d)) { best = idx; best_hp = e.health(); best_d = d; }
        }
        if (best < 0) return false;
        f = best;
        set_shared_byte(kByteState, static_cast<std::uint8_t>((st & 3) | ((f + 1) << 2)));
        focus_unseen = 0; focus_seen = false;
    }
    const cambc::champion* foe = nullptr;
    for (const auto& e : enemies) if (e == enemy_roster[f] && e.health() > 0) { foe = &e; break; }
    if (foe == nullptr) {   // out of my view: follow the stack to the last known position, touch nothing else
        std::vector<vec2> dest{focus_seen ? focus_last : v.pos};
        if (stay_with_stack(v, allies, dest)) return true;
        if (focus_seen && tile_distance(v.pos, focus_last) > 2)
            move_towards_closest(static_block, camp_static, tiles_around(focus_last, 1));
        return true;
    }
    focus_seen = true; focus_unseen = 0; focus_last = foe->position();
    const vec2 fp = foe->position();
    const bool low = me.health() < me.max_health() * kLowHealth;
    const int range = std::max(1, static_cast<int>(me.attack_range()));
    if (low) {   // step back: further from the target, within 2 tiles of the stack centre
        long sx = v.pos.x, sy = v.pos.y, n = 1;
        for (const auto& a : allies) {
            if (a == me || a.health() < a.max_health() * kLowHealth) continue;
            sx += a.position().x; sy += a.position().y; ++n;
        }
        const vec2 centre(static_cast<int>(sx / n), static_cast<int>(sy / n));
        auto blocked = static_block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles() | camp_static;
        vec2 best = v.pos; int best_foe = tile_distance(v.pos, fp), best_ctr = tile_distance(v.pos, centre);
        if (me.can_move())
            for (auto t : tiles_around(v.pos, 1).positions()) {
                if (blocked.is_set(t) || t == v.pos || tile_distance(t, centre) > 2) continue;
                const int df = tile_distance(t, fp), dc = tile_distance(t, centre);
                if (df > best_foe || (df == best_foe && dc < best_ctr)) { best = t; best_foe = df; best_ctr = dc; }
            }
        if (best != v.pos) { try { me.move_absolute(best); } catch (...) {} }
    } else if (!me.attack_tiles().is_set(fp)) {
        auto goals = tiles_around(fp, range) & ~static_block;
        goals.set(fp, false);
        if (range > 1) goals &= ~tiles_around(fp, range - 1);   // ranged: stand at full range
        if (goals.is_empty()) { goals = tiles_around(fp, 1) & ~static_block; goals.set(fp, false); }
        move_towards_closest(static_block, camp_static, goals);
    }
    for (int k = 0; k < 2; ++k) if (!cast_on_enemy(*foe)) break;
    if (attack_if_in_range(*foe)) reset_and_attack_again(*foe);
    return true;
}
