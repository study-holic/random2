// v4 turn flow: fuel-lean view (v2), retreat mode with a spawn walk when the spawn is within 6 tiles, the fight-back
// rule against enemy champions, banking at 36 gems (S-13), then squad or solo farming (squads.cpp, micro.cpp).
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

static constexpr int kBankBag = 36;
static constexpr int kNearSpawn = 6;

// Live camps near the champion plus every visible camp of the target or held group. vision::camps() costs about
// 11,000 fuel per camp, so camps come from the cached groups and only the relevant ones are queried. Bitboard
// operations cost about 450 fuel each on an 80x80 map, so groups are filtered by tile distance first.
TurnView Bot::build_view() const {
    TurnView v;
    v.pos = cambc::self().position();
    v.round = static_cast<int>(cambc::round_number());
    v.visible = cambc::vision::visible_tiles();
    const cambc::bitboard near = tiles_around(v.pos, 4) & v.visible;
    for (std::size_t i = 0; i < groups.size(); ++i) {
        const auto& g = groups[i];
        if (g.ready_round > v.round) continue;   // known to be respawning
        const bool focus = is_squad_group(static_cast<int>(i)) && (static_cast<int>(i) == target || static_cast<int>(i) == holding);
        const int reach = focus ? 8 : 4;
        if (min_distance(v.pos, g.tiles) > reach) continue;
        const auto& area = focus ? v.visible : near;
        for (const auto& c : g.group.camps())
            if (area.is_set(c.spawn_position()) && c.is_visible()) {
                const float h = c.health();
                if (h > 0) v.camps.push_back({c, static_cast<int>(i), h});
            }
    }
    return v;
}

void Bot::init() {
    initialised = true;
    roster = cambc::champions(team::ally);
    for (std::size_t i = 0; i < roster.size(); ++i)
        if (roster[i] == cambc::self()) ally_index = static_cast<int>(std::min<std::size_t>(i, kSquadClaimByte - 1));
    assign_squads();
    static_block = cambc::map::walls() | cambc::map::holes() | cambc::map::altars() | cambc::map::spawn_region(team::enemy);
    ally_spawn = cambc::map::spawn_region(team::ally);
    spawn_tiles = ally_spawn.positions();
    auto centre = [](const cambc::bitboard& region) {
        long sx = 0, sy = 0, n = 0;
        for (auto t : region.positions()) { sx += t.x; sy += t.y; ++n; }
        return n ? vec2(static_cast<int>(sx / n), static_cast<int>(sy / n)) : vec2(0, 0);
    };
    const vec2 ally_centre = centre(ally_spawn);
    const vec2 enemy_centre = centre(cambc::map::spawn_region(team::enemy));
    camp_static.clear();
    for (auto g : cambc::map::camp_groups()) {
        GroupInfo info{g};
        info.mask = g.spawn_tiles();
        info.tiles = info.mask.positions();
        for (auto t : info.tiles) info.ring |= tiles_around(t, 1);
        info.ring &= ~info.mask;
        bool elk = false;
        for (const auto& c : g.camps()) {
            elk |= c.is_elk();
            info.heavy |= c.is_elk() || c.is_monolith();
            info.monolith |= c.is_monolith();
            if (c.is_wolf()) ++info.wolves;
        }
        info.squad_target = !info.monolith && elk;
        info.gems = static_cast<int>(g.gem_value());
        info.dist_ally = min_distance(ally_centre, info.tiles);
        info.dist_enemy = min_distance(enemy_centre, info.tiles);
        camp_static |= info.mask;
        groups.push_back(std::move(info));
    }
    ambush_init(ally_centre);   // EXP-020
    altars = cambc::map::altars().positions();
    lock_until.assign(altars.size(), 0);
    altar_skip_until.assign(altars.size(), 0);
    altar_goals.clear();
    for (auto a : altars) altar_goals |= tiles_around(a, cambc::constants::submit_gems_range);
}

// Build (ladder-losses.md fix 3): Monster Emblem of the main stat first (five 30-gold shards combine), then Power
// shards alternating Health and the main stat (Focus for Cassi, Stellar and Prism; Strength otherwise).
void Bot::buy_items() {
    auto& me = cambc::self();
    using I = cambc::item_type;
    const auto type = me.champion_name();
    const bool focus = type == CT::cassi || type == CT::stellar || type == CT::prism;
    const I mon_shard = focus ? I::monster_shard_of_focus : I::monster_shard_of_strength;
    const I mon_emb = focus ? I::monster_emblem_of_focus : I::monster_emblem_of_strength;
    const I pow_main = focus ? I::power_shard_of_focus : I::power_shard_of_strength;
    const I pow_main_emb = focus ? I::power_emblem_of_focus : I::power_emblem_of_strength;
    const std::uint32_t price = mon_shard.cost();
    if (me.gold_count() < price) return;
    const auto owned = me.item_count();
    auto have = [&](I shard, I emblem) { return static_cast<int>(owned[shard]) + 5 * static_cast<int>(owned[emblem]); };
    int mon = have(mon_shard, mon_emb);
    int pm = have(pow_main, pow_main_emb);
    int ph = have(I::power_shard_of_health, I::power_emblem_of_health);
    std::uint32_t gold = me.gold_count();
    int buy_mon = 0, buy_pm = 0, buy_ph = 0;
    while (gold >= price) {
        if (mon < 5) { ++mon; ++buy_mon; }
        else if (ph <= pm) { ++ph; ++buy_ph; }
        else { ++pm; ++buy_pm; }
        gold -= price;
    }
    auto go = [&](I item, int n) { if (n > 0 && me.can_buy(item, n)) me.buy(item, n); };
    go(mon_shard, buy_mon);
    go(I::power_shard_of_health, buy_ph);
    go(pow_main, buy_pm);
}

// Refresh the state of one group (3,800 to 14,000 fuel); false when the group is not in vision.
bool Bot::refresh_group(int index, int round, const cambc::bitboard& visible) {
    if (index < 0 || cambc::fuel_remaining() < 45000) return false;
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
void Bot::update_group_memory(const TurnView& v) {
    const int round = v.round;
    if (groups.empty() || holding >= 0 || (round + ally_index) % 3 != 0) return;
    if (refresh_group(target, round, v.visible)) return;
    for (std::size_t n = 0; n < groups.size(); ++n) {
        const std::size_t i = (state_cursor + n) % groups.size();
        if (static_cast<int>(i) == target || min_distance(v.pos, groups[i].tiles) > 8) continue;
        if (refresh_group(static_cast<int>(i), round, v.visible)) { state_cursor = i + 1; break; }
    }
}

// Bank at 36 gems, after 40 rounds of carrying, when no target lies within 20 tiles, when the bag wins the game, or
// late. S-13: no tribute start with an enemy within 4 tiles unless an ally is adjacent or the bag finishes before the
// visible enemies could kill us. Altar locks are remembered for 40 rounds (no more walking away and back); when
// every altar is locked, the nearest lock is cleared with basic attacks. Returns true when the turn is used.
bool Bot::bank(const TurnView& v, const std::vector<cambc::champion>& enemies, const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    const int gems = static_cast<int>(me.gem_count());
    if (gems <= 0) { banking = false; carry_since = -1; return false; }
    if (carry_since < 0) carry_since = v.round;
    if (holding >= 0) return false;
    if (!banking) {
        const bool far = target < 0 || min_distance(v.pos, groups[target].tiles) > 20;
        const int total = static_cast<int>(cambc::global_submitted_gems(team::ally));
        banking = gems >= kBankBag || far || v.round - carry_since >= 40
            || total + gems >= static_cast<int>(cambc::constants::winning_gems) || v.round >= 1900;
        if (!banking) return false;
    }
    target = -1;
    std::vector<cambc::altar_lock> locks;
    try { locks = cambc::vision::altar_locks(team::enemy); } catch (...) {}
    for (std::size_t i = 0; i < altars.size(); ++i)
        for (const auto& l : locks) if (l.position() == altars[i]) lock_until[i] = v.round + 40;
    bool all_locked = true;
    for (std::size_t i = 0; i < altars.size(); ++i) if (lock_until[i] <= v.round) all_locked = false;

    bool ally_adjacent = false;
    for (const auto& a : allies) if (!(a == me) && tile_distance(a.position(), v.pos) <= 1) ally_adjacent = true;
    int foes4 = 0;
    for (const auto& e : enemies) if (tile_distance(e.position(), v.pos) <= 4) ++foes4;
    const bool safe = foes4 == 0 || ally_adjacent || me.health() > 120.0f * foes4 * ((gems + 9) / 10 + 1);

    for (std::size_t i = 0; i < altars.size(); ++i) {
        if (tile_distance(v.pos, altars[i]) > cambc::constants::submit_gems_range) continue;
        bool locked_now = false;
        for (const auto& l : locks) {
            if (!(l.position() == altars[i])) continue;
            locked_now = true;
            if (all_locked && foes4 == 0 && me.can_attack()) {
                try { me.attack(l); } catch (...) {}
                return true;
            }
        }
        if (locked_now) continue;
        lock_until[i] = 0;   // in view and unlocked
        if (!safe) return false;   // no tribute start under threat: the enemy rules and farming take the turn
        try { me.submit_gems(altars[i]); } catch (...) {}
        return true;
    }
    move_towards_closest(static_block, camp_static, altar_goal_tiles(v.round));   // EXP-018: skips stuck-excluded altars
    return true;
}

void Bot::play_turn() {
    if (!initialised) init();
    auto& me = cambc::self();
    update_stuck();   // EXP-018
    could_attack = me.is_alive() && me.can_attack();
    if (ambush_active() && (shared_byte(kByteFlags) & 0x80)) ambush_over = true;   // EXP-020
    play_turn_inner();
    attacked = could_attack && me.is_alive() && !me.can_attack();
    if (ambush_active()) return;   // EXP-020: bytes 0, 1 and 5 carry ambush state until the ambush is over
    std::uint8_t mine = me.is_alive() ? static_cast<std::uint8_t>(target + 1) : 0;
    if (ambush_enabled && ally_index == kByteFlags) mine |= 0x80;   // EXP-020: keep the ambush-over flag in byte 5
    cambc::shared_memory.write(ally_index, std::span<const std::uint8_t>{&mine, 1});
}

void Bot::play_turn_inner() {
    auto& me = cambc::self();
    if (!me.is_alive()) { buy_items(); holding = -1; retreating = false; banking = false; return; }
    if (me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) return;

    const TurnView v = build_view();
    update_group_memory(v);
    const bool at_spawn = ally_spawn.is_set(v.pos);
    if (at_spawn) buy_items();
    if (!me.can_move() && !me.can_attack()) return;
    // Spawn recovery happens at the end of a turn spent on the spawn, so a hurt champion stays until healed.
    if (at_spawn && me.health() < me.max_health() * 0.9f) return;
    if (handle_stuck(v)) return;   // EXP-018

    cast_shield(v);   // S-14

    const auto enemies = cambc::vision::champions(team::enemy);
    const auto allies = cambc::vision::champions(team::ally);
    update_contested(v, enemies);   // EXP-018
    const bool enemies_near = !enemies.empty();
    ambush_update(v, enemies, allies);   // EXP-020
    float threshold = (me.gem_count() > 0 || enemies_near) ? 0.45f : 0.3f;
    if (engaged_at_squad_target(v)) threshold = std::max(threshold, 0.35f);   // tank swap at 35 per cent
    const double incoming = incoming_damage(v, v.pos);
    // Inside camp range the retreat starts early enough to walk out and finish a recall.
    const double need = std::max<double>(me.max_health() * threshold, incoming * 4.0 + 0.1 * me.max_health());
    // Retreat is a mode: without it a champion steps out of camp range, resumes at the edge, and repeats.
    // EXP-020: during the ambush a fight without camp damage is the stack's business (low members step back).
    const bool amb_fight = ambush_active() && enemies_near && incoming == 0;
    if (!at_spawn && me.health() < need && !amb_fight) { retreating = true; holding = -1; }
    if (retreating && (at_spawn ? me.health() >= me.max_health() * 0.95f : me.health() >= me.max_health() * 0.9f)) retreating = false;
    if (retreating && !at_spawn) {
        // Spawn within 6 tiles: walk in for a full heal at the turn end; else leave camp range and recall.
        if (min_distance(v.pos, spawn_tiles) <= kNearSpawn && move_towards_closest(static_block, camp_static, ally_spawn)) return;
        if (incoming > 0) {
            if (move_towards_closest(static_block, camp_static, ~danger_zone(v))) return;
        } else if (!enemies_near) { me.recall(); return; }
        else { move_towards_closest(static_block, camp_static, ally_spawn); return; }
    }

    // A bag ready to bank at an altar is submitted first: damage does not interrupt a tribute.
    if (banking && me.gem_count() > 0)
        for (auto altar : altars)
            if (tile_distance(v.pos, altar) <= cambc::constants::submit_gems_range) { if (bank(v, enemies, allies)) return; break; }
    if (ambush_active() && ambush_turn(v, enemies, allies)) return;   // EXP-020
    if (enemy_response(v, enemies, allies)) return;

    // Low and chased: walk home.
    if (!at_spawn && enemies_near && me.health() < me.max_health() * threshold) {
        move_towards_closest(static_block, camp_static, ally_spawn);
        return;
    }

    resolve_target(v, allies);
    if (bank(v, enemies, allies)) return;
    farm(v, allies);
}
