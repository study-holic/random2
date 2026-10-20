// Tung clone turn flow: fuel-lean view, retreat mode, enemy avoidance, banking, then the squad farm step (farm.cpp).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <variant>

using cambc::vec2;
using cambc::team;
using CT = cambc::champion_type;

static constexpr int kMonsterRange = 3;   // rules: every monster attacks at range 3
static constexpr int kBankBag = 36;
static constexpr int kNearSpawn = 6;

// Damage per round that live camp monsters can deal to a champion standing on `pos`.
static double incoming_damage(const TurnView& v, vec2 pos) {
    double sum = 0;
    for (const auto& c : v.camps)
        if (tile_distance(pos, c.camp.spawn_position()) <= kMonsterRange + 1) sum += camp_strength(c.camp);
    return sum;
}

// Tiles within reach of a live damaging monster (range plus one tile margin).
cambc::bitboard danger_zone(const TurnView& v) {
    cambc::bitboard zone; zone.clear();
    for (const auto& c : v.camps)
        if (camp_strength(c.camp) > 0) zone |= tiles_around(c.camp.spawn_position(), kMonsterRange + 1);
    return zone;
}

int squad_of(CT type, int champions) {
    if (champions < 5) return 0;                                   // 3 picks: one squad
    return (type == CT::vendetta || type == CT::issac || type == CT::cassi) ? 0 : 1;
}

// Live camps near the champion plus every visible camp of the target or held group. vision::camps() costs about
// 11,000 fuel per camp, so camps come from the cached groups and only the relevant ones are queried.
TurnView Bot::build_view() const {
    TurnView v;
    v.pos = cambc::self().position();
    v.round = static_cast<int>(cambc::round_number());
    v.visible = cambc::vision::visible_tiles();
    const cambc::bitboard live_tiles = cambc::vision::camp_tiles();   // camps stay on their spawn tiles while alive
    for (std::size_t i = 0; i < groups.size(); ++i) {
        const auto& g = groups[i];
        if (g.ready_round > v.round) continue;                                  // known to be respawning
        const bool focus = static_cast<int>(i) == target || static_cast<int>(i) == holding;
        // Tile loops instead of bitboard intersections: the bitboards cost about 1 fuel per map tile each.
        bool relevant = false;
        for (auto t : g.tiles)
            if (v.visible.is_set(t) && (focus || tile_distance(v.pos, t) <= 4)) { relevant = true; break; }
        if (!relevant) continue;
        for (const auto& c : g.camps) {
            const vec2 sp = c.spawn_position();
            if (!v.visible.is_set(sp) || !live_tiles.is_set(sp) || (!focus && tile_distance(v.pos, sp) > 4)) continue;
            // Health (about 2,000 fuel) is only needed to rank prey inside the target group.
            const float h = static_cast<int>(i) == target ? c.health() : 1.0f;
            if (h > 0) v.camps.push_back({c, static_cast<int>(i), h});
        }
    }
    return v;
}

void Bot::init() {
    initialised = true;
    roster = cambc::champions(team::ally);
    ally_count = static_cast<int>(roster.size());
    for (std::size_t i = 0; i < roster.size(); ++i)
        if (roster[i] == cambc::self()) ally_index = static_cast<int>(i);
    const auto type = cambc::self().champion_name();
    squad = squad_of(type, ally_count);
    is_melee = type == CT::vendetta || type == CT::varg || type == CT::issac || type == CT::ryth
        || type == CT::ren || type == CT::marcella;
    static_block = cambc::map::walls() | cambc::map::holes() | cambc::map::altars() | cambc::map::spawn_region(team::enemy);
    ally_spawn = cambc::map::spawn_region(team::ally);
    spawn_tiles = ally_spawn.positions();
    const auto enemy_tiles = cambc::map::spawn_region(team::enemy).positions();
    camp_static.clear();
    for (auto g : cambc::map::camp_groups()) {
        GroupInfo info{g};
        info.mask = g.spawn_tiles();
        info.tiles = info.mask.positions();
        for (auto t : info.tiles) info.ring |= tiles_around(t, 1);
        info.ring &= ~info.mask;
        { const auto cs = g.camps(); info.camps.assign(cs.begin(), cs.end()); }
        for (const auto& c : info.camps) {
            info.heavy |= c.is_elk() || c.is_monolith();
            info.monolith |= c.is_monolith();
            ++info.camp_count;
            if (c.is_wolf()) ++info.wolves;
        }
        // Own half: the nearest tile is closer to our spawn region than to the enemy's.
        int best_a = 1 << 20, best_e = 1 << 20;
        for (auto t : info.tiles) {
            best_a = std::min(best_a, min_distance(t, spawn_tiles));
            best_e = std::min(best_e, min_distance(t, enemy_tiles));
        }
        info.own_half = best_a < best_e;
        info.tie_half = best_a == best_e;
        camp_static |= info.mask;
        groups.push_back(std::move(info));
    }
    altars = cambc::map::altars().positions();
    altar_goals.clear();
    for (auto a : altars) altar_goals |= tiles_around(a, cambc::constants::submit_gems_range);
}

// Build order (spec 8): five shards of the Monster stat (an emblem), five Jungle Health shards, then Power shards
// of Strength and Health alternating. Shards cost 30 and five combine into an emblem automatically.
void Bot::buy_items() {
    auto& me = cambc::self();
    const bool focus = me.champion_name() == CT::cassi;
    using I = cambc::item_type;
    const I mon_shard = focus ? I::monster_shard_of_focus : I::monster_shard_of_strength;
    const I mon_emb = focus ? I::monster_emblem_of_focus : I::monster_emblem_of_strength;
    auto have = [&](const cambc::enum_map<I>& m, I shard, I emblem) {
        return static_cast<int>(m[shard]) + 5 * static_cast<int>(m[emblem]);
    };
    constexpr std::uint32_t price = 30;
    if (me.gold_count() < price) return;
    const auto owned = me.item_count();
    int mon = have(owned, mon_shard, mon_emb);
    int jun = have(owned, I::jungle_shard_of_health, I::jungle_emblem_of_health);
    int ps = have(owned, I::power_shard_of_strength, I::power_emblem_of_strength);
    int ph = have(owned, I::power_shard_of_health, I::power_emblem_of_health);
    std::uint32_t gold = me.gold_count();
    int buy_mon = 0, buy_jun = 0, buy_ps = 0, buy_ph = 0;
    while (gold >= price) {
        if (mon < 5) { ++mon; ++buy_mon; }
        else if (jun < 5) { ++jun; ++buy_jun; }
        else if (ps <= ph) { ++ps; ++buy_ps; }
        else { ++ph; ++buy_ph; }
        gold -= price;
    }
    auto go = [&](I item, int n) { if (n > 0 && me.can_buy(item, n)) me.buy(item, n); };
    go(mon_shard, buy_mon);
    go(I::jungle_shard_of_health, buy_jun);
    go(I::power_shard_of_strength, buy_ps);
    go(I::power_shard_of_health, buy_ph);
}

// Refresh the state of one group (3,800 to 14,000 fuel); false when the group is not in vision.
bool Bot::refresh_group(int index, int round, const cambc::bitboard& visible) {
    if (index < 0) return false;
    bool seen = false;
    for (auto t : groups[index].tiles) if (visible.is_set(t)) { seen = true; break; }
    if (!seen) return false;
    try {
        const auto st = groups[index].group.state();
        if (auto r = std::get_if<cambc::map::camp_group_respawning>(&st)) groups[index].ready_round = round + r->timer;
        else groups[index].ready_round = 0;
    } catch (...) { return false; }
    return true;
}

// At most one group is refreshed per turn: the target when visible, otherwise the next visible one in rotation.
void Bot::update_group_memory(int round, const cambc::bitboard& visible) {
    if (groups.empty() || holding >= 0 || (round + ally_index) % 3 != 0) return;
    if (refresh_group(target, round, visible)) return;
    for (std::size_t n = 0; n < groups.size(); ++n) {
        const std::size_t i = (state_cursor + n) % groups.size();
        if (static_cast<int>(i) == target) continue;
        if (refresh_group(static_cast<int>(i), round, visible)) { state_cursor = i + 1; break; }
    }
}

// Step towards the goals, walking around the reach of live camps when a route exists.
bool Bot::go(const cambc::bitboard& goals, const TurnView& v) {
    bool strong = false;
    for (const auto& c : v.camps) if (camp_strength(c.camp) > 0) { strong = true; break; }
    if (strong) {
        const auto avoid = danger_zone(v);
        if (move_towards_closest(static_block, camp_static, goals, &avoid)) return true;
    }
    return move_towards_closest(static_block, camp_static, goals);
}

void Bot::play_turn() {
    if (!initialised) init();
    play_turn_inner();
}

void Bot::play_turn_inner() {
    auto& me = cambc::self();
    if (!me.is_alive()) { buy_items(); return; }
    if (me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) { return; }

    const TurnView v = build_view();
    update_group_memory(v.round, v.visible);
    const bool at_spawn = ally_spawn.is_set(v.pos);
    if (at_spawn) buy_items();
    if (!me.can_move() && !me.can_attack()) return;
    // Spawn recovery happens at the end of a turn spent on the spawn, so a hurt champion stays until healed.
    if (at_spawn && me.health() < me.max_health() * 0.9f) { return; }

    // Varg: shield whenever ready while a live camp is within reach.
    bool strong_camp_near = false;
    for (const auto& c : v.camps)
        if (camp_strength(c.camp) > 0 && tile_distance(v.pos, c.camp.spawn_position()) <= 4) { strong_camp_near = true; break; }
    if (strong_camp_near && me.can_cast())
        for (const auto& ab : me.abilities())
            if (ab.is_shield() && ab.is_ready()) { try { ab.use(); } catch (...) {} break; }

    // Enemy champions are never attacked; step back towards spawn when one is within 2 tiles and no ally is.
    const auto enemies = cambc::vision::champions(team::enemy);
    const auto allies = cambc::vision::champions(team::ally);
    bool enemy_close = false;
    for (const auto& e : enemies) if (tile_distance(v.pos, e.position()) <= 2) { enemy_close = true; break; }
    if (enemy_close && !at_spawn) {
        bool ally_close = false;
        for (const auto& a : allies)
            if (!(a == me) && tile_distance(v.pos, a.position()) <= 2) { ally_close = true; break; }
        if (!ally_close) { go(ally_spawn, v); return; }
    }

   
    const bool is_tank = tank_role(allies);
    const float threshold = is_tank ? 0.35f : 0.20f;
    const double incoming = incoming_damage(v, v.pos);
    // Inside camp range the retreat starts early enough to walk out (or reach spawn) before the damage adds up.
    const double need = std::max<double>(me.max_health() * threshold, incoming * 4.5 + 0.05 * me.max_health());
    // Retreat is a mode: without it a champion steps out of camp range, resumes at the edge, and repeats.
    if (!at_spawn && me.health() < need) retreating = true;
    if (retreating && (at_spawn ? me.health() >= me.max_health() * 0.95f : me.health() >= me.max_health() * 0.9f)) retreating = false;
    if (retreating && !at_spawn) {
        if (min_distance(v.pos, spawn_tiles) <= kNearSpawn) { go(ally_spawn, v); return; }
        if (incoming > 0) {
            if (move_towards_closest(static_block, camp_static, ~danger_zone(v))) return;
        } else if (enemies.empty()) { me.recall(); return; }
        else { go(ally_spawn, v); return; }
    }

   
    resolve_target(v, allies);

    // Bank at 36 gems, or any bag when no claimed group lies within 20 tiles. A capturer finishes the capture first.
    if (me.gem_count() > 0 && holding < 0) {
        const bool far_target = target < 0 || min_distance(v.pos, groups[target].tiles) > 20;
        if (static_cast<int>(me.gem_count()) >= kBankBag || far_target) {
            for (auto altar : altars) {
                if (tile_distance(v.pos, altar) > cambc::constants::submit_gems_range) continue;
                if (cambc::vision::has_altar_lock(altar)) continue;
                me.submit_gems(altar);
                return;
            }
            auto goals = altar_goals;
            for (auto altar : altars)
                if (tile_distance(v.pos, altar) <= 8 && cambc::vision::has_altar_lock(altar))
                    goals &= ~tiles_around(altar, cambc::constants::submit_gems_range);
            move_towards_closest(static_block, camp_static, goals);
            return;
        }
    }

    farm(v, is_tank, allies);
}
