// v12 Monolith play. Every monster attacks at range 3; the damage spells of Tavi, Stellar, Prism, Cassi, Ren (unit) and
// Ryth, Stellar (ground) reach 4. A poker stands at Chebyshev distance exactly 4 from the Monolith and casts on it: no
// damage taken, 20,000 health chipped down. Normal maps: idle pokers only. mono_mode (the Monoliths are the only gems on
// the map, e.g. contest): every poker works the Monolith on our side full time, shops Focus, and the rest farm gold.
// Once it is seen below P_MONO_FINISH everyone within P_MONO_FINISH_REACH joins (melee only when healthy); whoever is
// beside the cleared tile captures the 300 gold + 200 gems.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <array>
#include <cstdint>
#include <span>

using cambc::vec2;

void Bot::init_monolith() {
    mono_group = -1;
    int other_gems = 0;
    bool any = false;
    for (std::size_t i = 0; i < groups.size(); ++i) {
        if (groups[i].monolith) any = true; else other_gems += groups[i].gems;
    }
    mono_mode = any && (other_gems == 0 || !altar_walkable);
    gate_mode = any && !altar_walkable;   // v21: boss_battle: kill the Monoliths, then tribute from the cleared tile beside the altar
    poker = false;
    for (const auto& ab : cambc::self().abilities())
        if ((ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) && (ab.can_target_camps() || ab.can_target_ground())) poker = true;
}

// Alive Monolith group on our side of the map (then nearest); -1 when none.
int Bot::pick_monolith(const TurnView& v) const {
    int best = -1; long long best_key = 1LL << 40;   // v21: long is 32-bit in wasm32 (1L << 40 was 0: no Monolith was ever picked)
    for (std::size_t i = 0; i < groups.size(); ++i) {
        const auto& g = groups[i];
        if (!g.monolith || g.ready_round > v.round || g.tiles.empty()) continue;
        const long long key = static_cast<long long>(g.dist_ally - g.dist_enemy) * 1000 + min_distance(v.pos, g.tiles);
        if (key < best_key) { best_key = key; best = static_cast<int>(i); }
    }
    return best;
}

// Ground-targeted spells (Ryth's area stun, Stellar's area damage) land on the Monolith's tile.
static bool cast_at_tile(vec2 tile) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_ground()) continue;
        try { if (ab.valid_tiles().is_set(tile) && ab.use(tile)) return true; } catch (...) {}
    }
    return false;
}

bool Bot::monolith_play(const TurnView& v) {
    if (!P_MONO || holding >= 0 || rows_sched) return false;   // rows keeps its pack schedule (EXP-093)
    if (!mono_mode && foes_visible) return false;
    mono_group = pick_monolith(v);
    if (mono_group < 0) return false;
    auto& me = cambc::self();
    const auto& g = groups[mono_group];
    const vec2 mt = g.tiles[0];
    const int d = tile_distance(v.pos, mt);
    const LiveCamp* mono = nullptr;
    for (const auto& c : v.camps) if (c.group == mono_group) { mono = &c; break; }
    if (mono) { mono_hp = mono->health; mono_hp_round = v.round; }
    // Shared byte 7 (unused squad claim on Monolith maps): 2 = cleared, 1 = low, 0 = unknown; written by whoever sees it.
    std::uint8_t status = 0;
    if (mono_mode) cambc::shared_memory.read(7, std::span{&status, 1});
    const bool cleared_here = d <= 4 && group_cleared(v, mono_group);
    if (mono_mode && (mono != nullptr || cleared_here)) {
        status = cleared_here ? 2 : (mono_hp < static_cast<float>(P_MONO_FINISH) ? 1 : 0);
        cambc::shared_memory.write(7, std::span<const std::uint8_t>{&status, 1});
    }

    // Cleared: capture (resolve_target's holding branch pays out / times out). In mono_mode everyone comes, so the
    // 200-gem reward is split and no single carrier holds a 20-turn tribute.
    if (cleared_here || (mono_mode && status == 2 && d <= 20)) {
        holding = mono_group; target = mono_group; hold_since = v.round;
        if (!g.mask.is_set(v.pos)) move_towards_closest(static_block, camp_static, g.mask);
        still_rounds = 0;
        return true;
    }

    const bool low = (v.round - mono_hp_round <= 80 && mono_hp < static_cast<float>(P_MONO_FINISH)) || (mono_mode && status == 1);
    const bool waiting = target >= 0 && groups[target].ready_round > 0 && groups[target].ready_round > v.round - 3;
    const bool idle = target < 0 || (waiting && d <= 6);
    if (!mono_mode) {
        if (!low && !idle) return false;
        if (!low && d > P_MONO_REACH) return false;
        if (low && d > P_MONO_FINISH_REACH) return false;
    } else if (poker && !low) {
        // Between casts, farm the gold clusters beside the Monolith (choose_target is leashed to them in mono_mode).
        bool ready = false;
        for (const auto& ab : me.abilities())
            if (ab.is_ready() && (ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) && (ab.can_target_camps() || ab.can_target_ground())) ready = true;
        if (!ready && target >= 0) return false;
    }

    cambc::bitboard goals;
    if (poker) {
        goals = tiles_around(mt, 4) & ~tiles_around(mt, 3);          // exactly 4: out of its reach, inside spell range
    } else {
        if (!low || me.health() < 600.0f) return false;              // melee joins the finish only, and only healthy
        goals = tiles_around(mt, 1);
    }
    goals &= ~static_block & ~camp_static;
    {   // only tiles with line of sight to the Monolith (contest keeps it in a walled pocket)
        const cambc::bitboard opaque = cambc::map::walls() | cambc::map::bushes();
        goals &= opaque.shadow_cast(mt, 5);
    }
    if (goals.is_empty()) return false;
    if (!goals.is_set(v.pos)) {
        const cambc::bitboard block = poker ? (static_block | tiles_around(mt, 3)) : static_block;   // never path through its reach
        move_towards_closest(block, camp_static, goals);
    }
    if (mono != nullptr && tile_distance(me.position(), mt) <= 4) {
        if (cast_on_camp(mono->camp) || cast_at_tile(mono->camp.position())) attack_if_in_range(mono->camp);
        else if (attack_if_in_range(mono->camp)) reset_and_attack_again(mono->camp);
    }
    target = -1;        // no camp target while working the Monolith (bank() still runs on a full bag)
    still_rounds = 0;   // standing still here is intended, not stuck
    return true;
}

// mono_mode: a champion with an empty bag walks beside the nearest visible ally carrying a bag (the Monolith reward) and
// fights whatever comes near, so the carrier is never alone on the 20-turn tribute. Returns true when the turn is used.
bool Bot::escort_carrier(const TurnView& v, const std::vector<cambc::champion>& allies, const std::vector<cambc::champion>& enemies) {
    auto& me = cambc::self();
    if (me.gem_count() > 0 || holding >= 0) return false;
    const cambc::champion* carrier = nullptr; int best = 1 << 20; int carried = 0;
    for (const auto& a : allies) {
        if (a == me || !a.is_visible()) continue;
        int gems = 0; try { gems = static_cast<int>(a.gem_count()); } catch (...) { continue; }
        if (gems < 50) continue;
        const int dd = tile_distance(v.pos, a.position());
        if (dd < best) { best = dd; carrier = &a; carried = gems; }
    }
    if (carrier == nullptr) return false;
    const vec2 cp = carrier->position();
    // Hit the nearest enemy in range first (basic attack and spell), then keep adjacent to the carrier.
    const cambc::champion* foe = nullptr; int fd = 1 << 20;
    for (const auto& e : enemies) { if (!e.is_visible()) continue; const int dd = tile_distance(cp, e.position()); if (dd < fd) { fd = dd; foe = &e; } }
    if (foe != nullptr && fd <= 6) {
        if (me.can_cast()) for (const auto& ab : me.abilities()) {
            if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_enemies()) continue;
            try { if (ab.valid_tiles().is_set(foe->position()) && ab.use(*foe)) break; } catch (...) {}
        }
        if (attack_if_in_range(*foe)) { reset_and_attack_again(*foe); }
    }
    if (best > 1) {
        cambc::bitboard goals = tiles_around(cp, 1); goals.set(cp, false);
        move_towards_closest(static_block, camp_static, goals);
        if (foe != nullptr && attack_if_in_range(*foe)) reset_and_attack_again(*foe);
    }
    target = -1; still_rounds = 0;
    (void)carried;
    return true;
}

// Ward: when near an altar, put it on the tile within 2 nearest the altar (bushes around altars hide ambushes); otherwise,
// when poking or escorting, drop it where we stand. Placing a ward costs no action.
void Bot::place_ward_smart(const TurnView& v) {
    auto& me = cambc::self();
    try {
        if (me.ward_cooldown() > 0) return;
        int near = -1, nd = 1 << 20;
        for (std::size_t i = 0; i < altars.size(); ++i) { const int dd = tile_distance(v.pos, altars[i]); if (dd < nd) { nd = dd; near = static_cast<int>(i); } }
        if (near < 0) return;
        vec2 dest = v.pos; int bd = nd;
        if (nd <= 6) {
            for (auto t : me.ward_placement_tiles().positions()) {
                if (cambc::map::walls().is_set(t)) continue;
                const int dd = tile_distance(t, altars[near]);
                if (dd < bd) { bd = dd; dest = t; }
            }
            if (cambc::vision::has_ward(dest, cambc::team::ally)) return;
            me.place_ward(dest);
        } else if (mono_mode) {
            if (cambc::vision::has_ward(dest, cambc::team::ally)) return;
            me.place_ward(dest);
        }
    } catch (...) {}
}

// v15: drop the carried altar lock on an altar within 2 tiles that has no allied lock, unless an ally is tributing there
// (placing a lock cancels channels at that altar). Costs no movement; the engine validates everything else.
void Bot::altar_lock_pass(const TurnView& v, const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    try {
        const auto& lock = me.consumables()[cambc::consumable_type::altar_lock];
        if (lock.count() == 0 || me.is_submitting_gems()) return;
        for (auto a : altars) {
            if (tile_distance(v.pos, a) > 2) continue;
            bool ours = false;
            if (cambc::vision::has_altar_lock(a)) { try { ours = cambc::vision::altar_lock_at(a).is_ally(); } catch (...) { ours = true; } }
            if (ours) continue;
            bool ally_channel = false;
            for (const auto& al : allies) if (!(al == me) && al.is_visible() && tile_distance(al.position(), a) <= 1 && al.is_submitting_gems()) ally_channel = true;
            if (ally_channel) continue;
            lock.use(a);
            return;
        }
    } catch (...) {}
}

// v16: an enemy channeling a tribute (or a recall) is a stationary target whose whole channel is wiped by an altar lock
// (placed on that altar within 2 tiles) or a one-round stun (range 4). Otherwise attack it or walk towards it.
bool Bot::interrupt_channel(const TurnView& v, const std::vector<cambc::champion>& enemies) {
    auto& me = cambc::self();
    if (me.is_submitting_gems() || me.is_recalling()) return false;
    const cambc::champion* target_c = nullptr; int best = 1 << 20; bool tribute = false;
    for (const auto& e : enemies) {
        if (!e.is_visible()) continue;
        bool sub = false, rec = false;
        try { sub = e.is_submitting_gems(); rec = e.is_recalling(); } catch (...) { continue; }
        if (!sub && !rec) continue;
        const int d = tile_distance(v.pos, e.position());
        if (d < best) { best = d; target_c = &e; tribute = sub; }
    }
    if (target_c == nullptr || best > 8) return false;
    const vec2 ep = target_c->position();
    // 1. Lock the altar the tribute is channeling at.
    if (tribute) {
        try {
            const auto& lock = me.consumables()[cambc::consumable_type::altar_lock];
            if (lock.count() > 0)
                for (auto a : altars)
                    if (tile_distance(ep, a) <= cambc::constants::submit_gems_range && tile_distance(v.pos, a) <= 2 && !cambc::vision::has_altar_lock(a)) {
                        if (lock.use(a)) { DBG(" LOCKCANCEL"); return true; }
                    }
        } catch (...) {}
    }
    // 2. Stun (cancels tribute and recall) or any damage spell (cancels recall).
    if (me.can_cast()) {
        for (const auto& ab : me.abilities()) {
            if (!ab.is_ready() || !ab.can_target_enemies()) continue;
            if (!(ab.is_cc() || (!tribute && (ab.is_damage() || ab.is_aoe_damage())))) continue;
            try { if (ab.valid_tiles().is_set(ep) && ab.use(*target_c)) { attack_if_in_range(*target_c); return true; } } catch (...) {}
        }
    }
    // 3. Hit it; damage breaks a recall and chips a tribute carrier that cannot move.
    if (attack_if_in_range(*target_c)) { reset_and_attack_again(*target_c); return true; }
    // 4. Close in (it is not going anywhere for several turns).
    const int range = std::max(1, static_cast<int>(me.attack_range()));
    cambc::bitboard goals = tiles_around(ep, range); goals.set(ep, false);
    if (move_towards_closest(static_block, camp_static, goals)) { if (attack_if_in_range(*target_c)) reset_and_attack_again(*target_c); return true; }
    return false;
}

// v21: in gate_mode the altar is usable only while the gate Monolith group is dead (its spawn tiles are the only
// tiles within tribute range). The group's state is refreshed by update_group_memory and the capture branch.
bool Bot::gate_open(int round) const {
    for (const auto& g : groups)
        if (g.monolith && g.ready_round > round) return true;
    return false;
}
