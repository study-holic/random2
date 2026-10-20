// v5: v1 behaviour rebuilt for fuel (static data cached at init, one camp query per turn, staggered group
// state refresh), camp spawn tiles kept as permanent obstacles (routes no longer flip with fog), and a retreat
// that leaves camp range before recalling (damage interrupts recall; monsters do not chase).
#include <cambc.h>
#include "bot.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#include <variant>

using cambc::vec2;
using cambc::team;
using CT = cambc::champion_type;

static constexpr int kMonsterRange = 3;   // rules: every monster attacks at range 3

static int distance(vec2 a, vec2 b) { return static_cast<int>(a.chebyshev_distance(b)); }

static cambc::bitboard tiles_around(vec2 position, int radius) {
    cambc::bitboard tiles; tiles.clear(); tiles.draw_chebyshev(position, radius); return tiles;
}

// Strength by camp kind (rules table); avoids a design query per camp.
static int camp_strength(const cambc::camp& c) {
    if (c.is_elk()) return 60;
    if (c.is_wolf()) return 20;
    if (c.is_monolith()) return 200;
    return 0;
}

// Damage per round that live camp monsters can deal to a champion standing on `pos`.
static double incoming_damage(const TurnView& v, vec2 pos) {
    double sum = 0;
    for (const auto& c : v.camps)
        if (distance(pos, c.spawn_position()) <= kMonsterRange + 1) sum += camp_strength(c);
    return sum;
}

// Tiles within reach of a live damaging monster (range plus one tile margin).
static cambc::bitboard danger_zone(const TurnView& v) {
    cambc::bitboard zone; zone.clear();
    for (const auto& c : v.camps)
        if (camp_strength(c) > 0) zone |= tiles_around(c.spawn_position(), kMonsterRange + 1);
    return zone;
}

template<class Target>
static bool attack_if_in_range(const Target& target) {
    auto& me = cambc::self();
    if (!me.can_attack() || !target.is_visible() || target.health() <= 0 || !me.attack_tiles().is_set(target.position()))
        return false;
    me.attack(target);
    return true;
}

// Use an attack reset right after a basic attack so the champion attacks twice.
static void reset_and_attack_again(const cambc::unit& target) {
    auto& me = cambc::self();
    if (!me.can_cast()) return;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_attack_reset() || !ab.is_ready()) continue;
        try {
            if (ab.use() && target.is_visible() && target.health() > 0 && me.can_attack()
                && me.attack_tiles().is_set(target.position()))
                me.attack(target);
        } catch (...) {}
        return;
    }
}

// Cast a damage spell on a camp in spell range; these spells forbid walking this turn.
static bool cast_on_camp(const cambc::camp& camp) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_camps()) continue;
        if (ab.is_cc() && !ab.is_damage()) {
            // Cassi's stun does no damage; skip it on camps.
            try { if (ab.damage_amount() <= 0) continue; } catch (...) { continue; }
        }
        try {
            if (ab.valid_tiles().is_set(camp.position()) && ab.use(camp)) return true;
        } catch (...) {}
    }
    return false;
}

// Attack camps within reach (spell range is 4, so farther camps are skipped before any ability query).
static bool fight_camp(const TurnView& v) {
    for (const auto& camp : v.camps) {
        if (distance(v.pos, camp.spawn_position()) > 4) continue;
        if (cast_on_camp(camp)) { attack_if_in_range(camp); return true; }
        if (attack_if_in_range(camp)) { reset_and_attack_again(camp); return true; }
    }
    return false;
}

// Step towards the closest goal tile. Camp spawn tiles stay obstacles unless they are goals.
static bool move_towards_closest(const TurnView& v, const cambc::bitboard& static_block,
                                 const cambc::bitboard& camp_static, cambc::bitboard goals) {
    auto& me = cambc::self();
    if (!me.can_move() || goals.is_set(v.pos)) return false;
    auto blocked = static_block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles()
        | (camp_static & ~goals) | cambc::vision::teleport_tiles();
    blocked.set(v.pos, false);
    goals &= ~blocked | cambc::vision::camp_tiles();
    if (goals.is_empty()) return false;
    if (goals.count() == 1 && !blocked.is_set(goals.positions()[0])) return me.move_towards(goals.positions()[0]);
    const auto path_walls = blocked & ~goals;
    cambc::distance_field field; cambc::bitboard visited;
    field.compute_to(path_walls, goals, tiles_around(v.pos, 0), visited);
    auto step = v.pos;
    for (auto tile : blocked.reachable_tiles(step, me.base_stats().base_move_speed).positions())
        if (field.get(tile) < field.get(step)) step = tile;
    return step != v.pos && me.move_absolute(step);
}

// Live camps near the champion. vision::camps() costs about 11,000 fuel per visible camp on the first call of
// a turn and health() about 2,000, so camps come from the cached groups and only nearby ones are queried.
TurnView Bot::build_view() const {
    TurnView v;
    v.pos = cambc::self().position();
    v.round = static_cast<int>(cambc::round_number());
    cambc::bitboard near = tiles_around(v.pos, 4);
    near &= cambc::vision::visible_tiles();
    for (const auto& g : groups) {
        if ((g.mask & near).is_empty() || g.ready_round > v.round) continue;   // none near, or known to be respawning
        for (const auto& c : g.group.camps())
            if (near.is_set(c.spawn_position()) && c.is_visible() && c.health() > 0) v.camps.push_back(c);
    }
    return v;
}

void Bot::init() {
    initialised = true;
    const auto allies = cambc::champions(team::ally);
    for (std::size_t i = 0; i < allies.size(); ++i)
        if (allies[i] == cambc::self()) ally_index = static_cast<int>(std::min<std::size_t>(i, 7));
    static_block = cambc::map::walls() | cambc::map::holes() | cambc::map::altars() | cambc::map::spawn_region(team::enemy);
    ally_spawn = cambc::map::spawn_region(team::ally);
    camp_static.clear();
    for (auto g : cambc::map::camp_groups()) {
        GroupInfo info{g};
        info.mask = g.spawn_tiles();
        info.tiles = info.mask.positions();
        for (auto t : info.tiles) info.ring |= tiles_around(t, 1);
        info.ring &= ~info.mask;
        for (const auto& c : g.camps()) { info.heavy |= c.is_elk() || c.is_monolith(); info.monolith |= c.is_monolith(); }
        camp_static |= info.mask;
        groups.push_back(std::move(info));
    }
    altars = cambc::map::altars().positions();
    altar_goals.clear();
    for (auto a : altars) altar_goals |= tiles_around(a, cambc::constants::submit_gems_range);
}

void Bot::buy_items() {
    auto& me = cambc::self();
    const auto type = me.champion_name();
    cambc::item_type item = cambc::item_type::power_shard_of_strength;
    if (type == CT::cassi || type == CT::stellar || type == CT::prism) item = cambc::item_type::power_shard_of_focus;
    else if (type == CT::ryth) item = cambc::item_type::power_shard_of_health;
    // Test opponent (EXP-023): keep one altar lock in stock.
    try {
        if (me.consumables()[cambc::consumable_type::altar_lock].count() == 0 && me.can_buy(cambc::consumable_type::altar_lock))
            me.buy(cambc::consumable_type::altar_lock);
    } catch (...) {}
    const unsigned count = me.gold_count() / item.cost();
    if (count && me.can_buy(item, count)) me.buy(item, count);
}

// Group state costs 4,000 to 14,000 fuel per call, so at most one group is refreshed per turn: the target group
// when it is close or every few rounds while waiting, otherwise the next visible group in rotation.
void Bot::update_group_memory(int round) {
    if (groups.empty() || (round + ally_index) % 3 != 0) return;
    const auto visible = cambc::vision::visible_tiles();
    auto refresh = [&](std::size_t i) {
        if ((groups[i].mask & visible).is_empty()) return false;
        try {
            const auto st = groups[i].group.state();
            if (auto r = std::get_if<cambc::map::camp_group_respawning>(&st)) groups[i].ready_round = round + r->timer;
            else groups[i].ready_round = 0;
        } catch (...) {}
        return true;
    };
    if (target >= 0 && refresh(static_cast<std::size_t>(target))) return;
    for (std::size_t n = 0; n < groups.size(); ++n) {
        const std::size_t i = (state_cursor + n) % groups.size();
        if (static_cast<int>(i) == target) continue;
        if (refresh(i)) { state_cursor = i + 1; break; }
    }
}

int Bot::choose_target() {
    const auto pos = cambc::self().position();
    const int round = static_cast<int>(cambc::round_number());
    std::array<std::uint8_t, 8> claims{};
    cambc::shared_memory.read(0, std::span{claims.data(), claims.size()});
    int best = -1; double best_score = 1e18;
    for (std::size_t i = 0; i < groups.size(); ++i) {
        const auto& g = groups[i];
        if (g.monolith) continue;
        int claimed = 0;
        for (int k = 0; k < 8; ++k) if (k != ally_index && claims[k] == i + 1) ++claimed;
        if (claimed >= (g.heavy ? 2 : 1)) continue;
        int d = 1 << 20;
        for (auto t : g.tiles) d = std::min(d, distance(pos, t));
        const int wait = std::max(0, g.ready_round - round - d);
        const double score = d + 2.0 * wait - (g.heavy && claimed ? 6 : 0) - 0.05 * g.group.gem_value();
        if (score < best_score) { best_score = score; best = static_cast<int>(i); }
    }
    return best;
}

void Bot::play_turn() {
    if (!initialised) init();
    play_turn_inner();
    std::uint8_t mine = cambc::self().is_alive() ? static_cast<std::uint8_t>(target + 1) : 0;
    cambc::shared_memory.write(ally_index, std::span<const std::uint8_t>{&mine, 1});
}

void Bot::play_turn_inner() {
    auto& me = cambc::self();
    if (!me.is_alive()) { buy_items(); return; }
    if (me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) return;

    const TurnView v = build_view();
    update_group_memory(v.round);
    const bool at_spawn = ally_spawn.is_set(v.pos);
    if (at_spawn) buy_items();
    if (!me.can_move() && !me.can_attack()) return;
    // Spawn recovery happens at the end of a turn spent on the spawn, so a hurt champion stays until healed.
    if (at_spawn && me.health() < me.max_health() * 0.9f) return;

    // Varg: shield when hurt.
    if (me.health() < me.max_health() * 0.6f && me.can_cast())
        for (const auto& ab : me.abilities())
            if (ab.is_shield() && ab.is_ready()) { try { ab.use(); } catch (...) {} break; }

    const auto enemies = cambc::vision::champions(team::enemy);
    const bool enemies_near = !enemies.empty();
    const float threshold = (me.gem_count() > 0 || enemies_near) ? 0.45f : 0.3f;
    const double incoming = incoming_damage(v, v.pos);
    // Inside camp range the retreat starts early enough to walk out and finish a recall.
    const double need = std::max<double>(me.max_health() * threshold, incoming * 4.0 + 0.1 * me.max_health());
    // Retreat is a mode: without it a champion steps out of camp range, resumes at the edge, and repeats.
    if (!at_spawn && me.health() < need) retreating = true;
    if (retreating && (at_spawn ? me.health() >= me.max_health() * 0.95f : me.health() >= me.max_health() * 0.9f)) retreating = false;
    if (retreating && !at_spawn) {
        if (incoming > 0) {
            if (move_towards_closest(v, static_block, camp_static, ~danger_zone(v))) return;
        } else if (!enemies_near) { me.recall(); return; }
        else { move_towards_closest(v, static_block, camp_static, ally_spawn); return; }
    }

    // Fight enemies in range (kills pay their carried gems).
    for (const auto& enemy : enemies)
        if (attack_if_in_range(enemy)) { reset_and_attack_again(enemy); return; }

    // Low and chased: walk home.
    if (!at_spawn && me.health() < me.max_health() * threshold) {
        move_towards_closest(v, static_block, camp_static, ally_spawn);
        return;
    }

    // Test opponent (EXP-023): lock any altar within 2 tiles that has no lock.
    try {
        const auto& lock = me.consumables()[cambc::consumable_type::altar_lock];
        if (lock.count() > 0)
            for (auto altar : altars)
                if (distance(v.pos, altar) <= 2 && !cambc::vision::has_altar_lock(altar)) { lock.use(altar); return; }
    } catch (...) {}
    // Bank carried gems.
    if (me.gem_count() > 0) {
        target = -1;
        for (auto altar : altars) {
            if (distance(v.pos, altar) > cambc::constants::submit_gems_range) continue;
            if (cambc::vision::has_altar_lock(altar)) continue;
            me.submit_gems(altar);
            return;
        }
        auto goals = altar_goals;
        for (auto altar : altars)
            if (distance(v.pos, altar) <= 8 && cambc::vision::has_altar_lock(altar))
                goals &= ~tiles_around(altar, cambc::constants::submit_gems_range);
        move_towards_closest(v, static_block, camp_static, goals);
        return;
    }

    // Farm: fight what is in reach, otherwise go to the claimed camp group.
    if (fight_camp(v)) return;
    if (target < 0 || (groups[target].ready_round && groups[target].ready_round > v.round + 10))
        target = choose_target();
    if (target >= 0) {
        const auto& g = groups[target];
        if (g.ready_round > v.round - 3 && g.ready_round > 0) {
            // Respawning: an occupied spawn tile delays the whole group's respawn, so wait beside the tiles.
            if (!g.ring.is_set(v.pos)) move_towards_closest(v, static_block, camp_static, g.ring);
        } else {
            // A live camp of this group within reach but outside attack range: close in on it. Standing on a
            // spawn tile and waiting leaves the champion idle, because monsters do not chase.
            const cambc::camp* prey = nullptr; int best_d = 99;
            for (const auto& c : v.camps) {
                if (!g.mask.is_set(c.spawn_position())) continue;
                const int d = distance(v.pos, c.spawn_position());
                if (d < best_d) { best_d = d; prey = &c; }
            }
            if (prey != nullptr) {
                auto reach = tiles_around(prey->spawn_position(), std::max(1, static_cast<int>(me.attack_range())));
                reach.set(prey->spawn_position(), false);
                if (!reach.is_set(v.pos)) move_towards_closest(v, static_block, camp_static, reach);
            } else {
                move_towards_closest(v, static_block, camp_static, g.mask);   // else already on the group: stay to capture
            }
        }
        fight_camp(v);
    }
}
