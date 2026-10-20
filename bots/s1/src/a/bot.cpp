// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
namespace va {
// v4 turn flow: fuel-lean view (v2), retreat mode with a spawn walk when the spawn is within 6 tiles, the fight-back
// rule against enemy champions, banking at 36 gems (S-13), then squad or solo farming (squads.cpp, micro.cpp).
#include <cambc.h>
#include "bot.h"
#include "plan.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#include <array>
#if P_ROWS_DBG
#include <iostream>
#include <string>
#endif
#include <cstdint>
#include <span>
#include <variant>

using cambc::vec2;
using cambc::team;
using CT = cambc::champion_type;

static constexpr int kBankBag = P_BANK_BAG;
static constexpr int kNearSpawn = P_NEAR_SPAWN;
static constexpr bool kAltarScoring = P_ALTAR_SCORING;  // EXP-023 (a): contested-altar choice and farm-on; off = v4-2 goals
static constexpr int kGiveUpContest = P_GIVE_UP_CONTEST;    // EXP-023: two or more enemies (8 each) at the best altar: farm on
static constexpr int kAltarFoeRadius = P_ALTAR_FOE_RADIUS;
static constexpr int kAltarFoeMemory = P_ALTAR_FOE_MEMORY;
static constexpr float kRetreatHurt = P_RETREAT_HURT, kRetreatCalm = P_RETREAT_CALM, kRetreatTank = P_RETREAT_TANK;
static constexpr float kRecallDoneField = P_RECALL_DONE_FIELD, kRecallDoneSpawn = P_RECALL_DONE_SPAWN, kSpawnLeaveHp = P_SPAWN_LEAVE_HP;

// EXP-023: score = distance + 8 per enemy champion seen within 5 tiles in the last 20 rounds + 3 when locked by the
// enemy (three hits clear it) + 20 when stuck-skipped. `contest` receives the enemy part of the winner's score.
int Bot::choose_altar(const TurnView& v, const std::vector<cambc::champion>& enemies, int& contest) {
    int best = -1, best_score = 1 << 20;
    contest = 0;
    for (std::size_t i = 0; i < altars.size(); ++i) {
        if (v.visible.is_set(altars[i])) {
            int n = 0;
            for (const auto& e : enemies) if (tile_distance(e.position(), altars[i]) <= kAltarFoeRadius) ++n;
            altar_foes[i] = n;
            altar_foes_round[i] = v.round;
        }
        const int foes = v.round - altar_foes_round[i] <= kAltarFoeMemory ? altar_foes[i] : 0;
        const int score = tile_distance(v.pos, altars[i]) + 8 * foes + (lock_until[i] > v.round ? 3 : 0)
            + (altar_skip_until[i] > v.round ? 20 : 0);
        if (score < best_score) { best_score = score; best = static_cast<int>(i); contest = 8 * foes; }
    }
    return best;
}

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
        // P_S1_BANK_REACH: an asleep Monolith outside the target is left out of the view, so no fight code wakes it
        // when a carrier walks past it to an altar inside its range (it cannot hurt while asleep: arena test 1).
        const bool skip_mono = P_S1_BANK_REACH && g.monolith && static_cast<int>(i) != target && static_cast<int>(i) != holding;
        for (const auto& c : g.group.camps())
            if (area.is_set(c.spawn_position()) && c.is_visible()) {
                if (skip_mono && c.is_monolith() && !awake_tiles().is_set(c.spawn_position())) continue;
                const float h = c.health();
                if (h > 0) v.camps.push_back({c, static_cast<int>(i), h});
            }
    }
    return v;
}

const cambc::bitboard& GroupInfo::ring() const {
    if (!ring_ready) {
        ring_ready = true;
        for (auto t : tiles) ring_ |= tiles_around(t, 1);
        ring_ &= ~mask;
    }
    return ring_;
}

// FIX_STAGED_INIT: init runs in phases that each check the fuel left (init_fuel_ok) and return, to resume next turn,
// instead of letting the engine suspend the turn mid-computation. Total init was ~380k fuel on big_world.
#ifndef P_DEATH_LOOP
#define P_DEATH_LOOP 3   // 3 any death at the target group; 0 off, 1 second death at a group within P_DEATH_WINDOW rounds, 2 any death with no enemy champion in view
#endif
#ifndef P_DEATH_WINDOW
#define P_DEATH_WINDOW 300
#endif
#ifndef P_DEATH_LOOP_N
#define P_DEATH_LOOP_N 1
#endif
#ifndef P_DEATH_EXCLUDE
#define P_DEATH_EXCLUDE 500
#endif
#ifndef P_FIX_MONO_RING
#define P_FIX_MONO_RING 1
#endif
#ifndef TRACE_V
#define TRACE_V 0
#endif
void Bot::init() {
  if (init_phase == 0) {
    roster = cambc::champions(team::ally);
    for (std::size_t i = 0; i < roster.size(); ++i)
        if (roster[i] == cambc::self()) ally_index = static_cast<int>(std::min<std::size_t>(i, kSquadClaimByte - 1));
    assign_squads();
    remote_shop = P_FIELD_SHOP && cambc::self().passive() == cambc::passive_type::remote_shop;
    static_block = cambc::map::walls() | cambc::map::holes() | cambc::map::altars() | cambc::map::spawn_region(team::enemy);
    ally_spawn = cambc::map::spawn_region(team::ally);
    spawn_tiles = ally_spawn.positions();
    auto centre = [](const cambc::bitboard& region) {
        long sx = 0, sy = 0, n = 0;
        for (auto t : region.positions()) { sx += t.x; sy += t.y; ++n; }
        return n ? vec2(static_cast<int>(sx / n), static_cast<int>(sy / n)) : vec2(0, 0);
    };
    init_ally_centre = centre(ally_spawn);
    init_enemy_centre = centre(cambc::map::spawn_region(team::enemy));
    camp_static.clear();
    init_any_elk = false;
    init_cluster_groups = 0;   // EXP-030
    init_phase = 1;
  }
  if (init_phase == 1) {
    const vec2 ally_centre = init_ally_centre, enemy_centre = init_enemy_centre;
    bool& any_elk = init_any_elk;
    int& cluster_groups = init_cluster_groups;
    const auto& all_groups = mapid::cached_groups();
    while (groups.size() < all_groups.size()) {
        if (!init_fuel_ok(22000)) return;
        const auto& g = all_groups[groups.size()];
        GroupInfo info{g};
        info.mask = g.spawn_tiles();
        info.tiles = info.mask.positions();
        bool elk = false, damaging = false;
        for (const auto& c : g.camps()) {
            elk |= c.is_elk();
            info.has_gem_cluster |= c.is_gem_cluster();   // EXP-030
            damaging |= c.is_wolf() || c.is_elk() || c.is_monolith();
            info.heavy |= c.is_elk() || c.is_monolith();
            info.monolith |= c.is_monolith();
            if (c.is_wolf()) ++info.wolves;
        }
        any_elk |= elk;   // EXP-021
        info.squad_target = !info.monolith && elk;
        info.gems = static_cast<int>(g.gem_value());
        info.cluster = info.has_gem_cluster && !damaging;   // EXP-030
        if (info.cluster) ++cluster_groups;
        info.dist_ally = min_distance(ally_centre, info.tiles);
        info.dist_enemy = min_distance(enemy_centre, info.tiles);
        camp_static |= info.mask;
        groups.push_back(std::move(info));
    }
    if (P_THREAT_LAYER) foe_seen.assign(groups.size(), -1000);   // EXP-071 leash memory
    {   // EXP-031: square maps only (width and height within a factor of 1.5), so lanes (90x30) stays on v4-8's opening
        const int mw = cambc::map::width(), mh = cambc::map::height();
        const bool square = 2 * std::max(mw, mh) <= 3 * std::min(mw, mh);
        cluster_map = cluster_groups >= P_CLUSTER_MIN_GROUPS && square;   // EXP-030 / EXP-031
    }
    // EXP-021 (v4-3c): on maps with no Elk group, packs of three or more wolves are squad targets.
    if (!any_elk)
        for (auto& g : groups) if (!g.monolith && g.wolves >= P_PACK_WOLVES) g.squad_target = true;
    init_phase = 2;
  }
  if (init_phase == 2) {
    if (!init_fuel_ok(70000)) return;
    cambc::distance_field& field = init_field; cambc::bitboard& reach = init_reach;   // one spawn field, shared with init_opening_assign
    altars = cambc::map::altars().positions();
    // EXP-023: drop altars whose submit tiles cannot be reached on foot from our spawn (collision's (39,20) sits in
    // a walled pocket; heading there when the central altar was locked or skipped froze every carrier).
    {
        const cambc::bitboard terrain = cambc::map::walls() | cambc::map::holes() | cambc::map::altars();
        field.compute(terrain, ally_spawn, reach);
        std::vector<vec2> usable;
        for (auto a : altars)
            if (!(tiles_around(a, cambc::constants::submit_gems_range) & reach).is_empty()) usable.push_back(a);
        if (P_S1_BANK_REACH) {   // a submit tile that is a Monolith spawn tile needs the Monolith dead (boss_battle): not usable
            cambc::bitboard mono; mono.clear();
            for (const auto& g : groups) if (g.monolith) mono |= g.mask;
            std::vector<vec2> open;
            for (auto a : usable)
                if (!(tiles_around(a, cambc::constants::submit_gems_range) & reach & ~mono).is_empty()) open.push_back(a);
            usable = open;
            if (usable.empty()) altars.clear();   // no altar we can use: carry and farm (gold is the 0-0 tiebreak)
        }
        if (!usable.empty()) altars = usable;
    }
    init_phase = 3;
  }
  if (init_phase == 3) {
    if (!init_fuel_ok(60000)) return;
    const bool any_elk = init_any_elk;
    cambc::distance_field& field = init_field; cambc::bitboard& reach = init_reach;
    altar_foes.assign(altars.size(), 0);
    altar_foes_round.assign(altars.size(), -1000);
    lock_until.assign(altars.size(), 0);
    altar_skip_until.assign(altars.size(), 0);
    altar_goals.clear();
    for (auto a : altars) altar_goals |= tiles_around(a, cambc::constants::submit_gems_range);
    if (P_ROWS_CMD && !any_elk) {   // EXP-093: rows = 50x50, no Elk, six three-wolf packs
        int packs = 0;
        for (const auto& g : groups) if (g.wolves >= 3 && !g.monolith) ++packs;
        rows = cambc::map::width() == 50 && cambc::map::height() == 50 && packs == 6;   // EXP-096b: monolith is also 50x50 with six packs plus 16 other camps
        if (rows)   // highest health captures (Varg or Ryth stay on the pack anyway), so one bag collects the gold
            for (std::size_t i = 0; i < capture_key.size(); ++i) capture_key[i] = -static_cast<int>(roster[i].max_health()) * 8 + static_cast<int>(i);
    }
    if (rows) init_rows_sched();   // EXP-093
    if (P_OPENING_ASSIGN) init_opening_assign(field, reach);   // EXP-079
    init_gi = 0;
    init_phase = 4;
  }
  if (init_phase == 4) {   // FIX_REACH: a group no tile of which is attackable from a tile we can walk to is never targeted
    const int range = std::max(1, static_cast<int>(cambc::self().attack_range()));
    const int W = cambc::map::width(), H = cambc::map::height();
    if (init_gi == 0 && init_monos.empty())
        for (const auto& cg : mapid::cached_groups())
            for (const auto& c : cg.camps())
                if (c.is_monolith()) init_monos.push_back({c.spawn_position(), camp_range(c)});
    if (P_FIX_MONO_RING && !g_mono_zone_on) {   // per-Monolith zones for the awake-only path block (util.cpp)
        for (const auto& mp : init_monos) g_mono_list.push_back({mp.pos, tiles_around(mp.pos, mp.range)});
        g_mono_zone_on = !g_mono_list.empty();
    }
    while (init_gi < static_cast<int>(groups.size())) {
        if (!init_fuel_ok(30000)) return;
        auto& g = groups[init_gi++];
        bool ok = false;
        for (auto t : g.tiles) {
            for (int dx = -range; dx <= range && !ok; ++dx)
                for (int dy = -range; dy <= range && !ok; ++dy) {
                    const int x = t.x + dx, y = t.y + dy;
                    if (x < 0 || y < 0 || x >= W || y >= H) continue;
                    if (!init_reach.is_set(cambc::vec2(x, y))) continue;
                    bool safe = true;   // FIX_MONO_RING: stand outside every Monolith's range (engine 1.0: 5, was 3)
                    if (P_FIX_MONO_RING)
                        for (const auto& mp : init_monos) if (std::max(std::abs(x - mp.pos.x), std::abs(y - mp.pos.y)) <= mp.range) { safe = false; break; }
                    if (safe) ok = true;
                }
            if (ok) break;
        }
        if (!ok) { g.excluded_until = 1 << 30; ++init_unreachable; }
    }
    init_phase = 5;
  }
  if (!init_fuel_ok(40000)) return;
  Plan::init(*this);
  s1_init();
  if (P_S1_THREAT_COST) {   // reach of each camp spawn tile: wolf 3, elk 4, monolith 5, golem 4 (arena test 1)
      g_camp_w = cambc::map::width();
      g_camp_rng.assign(static_cast<std::size_t>(g_camp_w) * cambc::map::height(), 0);
      for (const auto& g : groups)
          for (const auto& c : g.group.camps()) {
              const int r = c.is_wolf() ? 3 : c.is_elk() ? 4 : c.is_monolith() ? 5 : c.is_golem() ? 4 : 0;
              g_camp_rng[c.spawn_position().y * g_camp_w + c.spawn_position().x] = static_cast<std::int8_t>(r);
          }
  }
  initialised = true;
#if TRACE_V
  if (ally_index == 0) for (std::size_t i = 0; i < groups.size(); ++i) { const auto& g = groups[i]; std::cout << "G" << i << " " << g.tiles[0].x << "," << g.tiles[0].y << " m" << g.monolith << " h" << g.heavy << " w" << g.wolves << " gem" << g.gems << " sq" << g.squad_target << " da" << g.dist_ally << " de" << g.dist_enemy << " ex" << (g.excluded_until > 1000000) << "\n"; }
#endif
}

// Build (ladder-losses.md fix 3): Monster Emblem of the main stat first (five 30-gold shards combine), then Power
// shards alternating Health and the main stat (Focus for Cassi, Stellar and Prism; Strength otherwise).
void Bot::buy_items() {
    if (Plan::buy(*this)) return;
    if (tavi_focus_field) { buy_items_w8(); return; }   // w8 build C (map flag); every other map runs v2a's code below
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
    if (rows) {   // EXP-093: Power shards of the main stat only, every visit
        const int n = static_cast<int>(me.gold_count() / price);
        if (n > 0 && me.can_buy(pow_main, n)) me.buy(pow_main, n);
        return;
    }
    const auto owned = me.item_count();
    auto have = [&](I shard, I emblem) { return static_cast<int>(owned[shard]) + 5 * static_cast<int>(owned[emblem]); };
    int mon = have(mon_shard, mon_emb);
    int pm = have(pow_main, pow_main_emb);
    int ph = have(I::power_shard_of_health, I::power_emblem_of_health);
    std::uint32_t gold = me.gold_count();
    int buy_mon = 0, buy_pm = 0, buy_ph = 0;
    const bool tank = type == CT::varg || type == CT::ryth || type == CT::vendetta;
    while (gold >= price) {
        if (P_HEALTH_FIRST_TANKS && tank && ph < 5) { ++ph; ++buy_ph; }
        else if (mon < 5) { ++mon; ++buy_mon; }
        else if (ph <= pm) { ++ph; ++buy_ph; }
        else { ++pm; ++buy_pm; }
        gold -= price;
    }
    auto go = [&](I item, int n) { if (n > 0 && me.can_buy(item, n)) me.buy(item, n); };
    go(mon_shard, buy_mon);
    go(I::power_shard_of_health, buy_ph);
    go(pow_main, buy_pm);
}

// w8 build C (results/w8-builds.md; code from bots/w8tf with its macros P_TAVI_DMG_FOCUS and P_FIELD_SHOP replaced by the
// map flag tavi_focus_field). Tavi alone takes the damage build with Focus (2.2 x Focus nuke and stun at range 4) and,
// with his remote_shop passive, buys where he stands. Every other champion buys exactly as in buy_items.
//   Damage stat per kit (GameRules champion sections): Focus for Cassi, Stellar, Prism and (w8) Tavi; Strength otherwise.
static bool damage_stat_is_focus(CT type, bool tavi_focus) {
    if (type == CT::cassi || type == CT::stellar || type == CT::prism) return true;
    if (type == CT::tavi) return P_BUILD_DMG || tavi_focus;
    return false;
}

void Bot::buy_items_w8() {
    auto& me = cambc::self();
    using I = cambc::item_type;
    // Tavi's remote_shop passive buys shards anywhere (GameRules, Tavi kit); `field` = alive and off the spawn.
    const bool field = remote_shop && me.is_alive() && !ally_spawn.is_set(me.position());
    const auto type = me.champion_name();
    const bool focus = damage_stat_is_focus(type, tavi_focus_field);
    const I mon_shard = focus ? I::monster_shard_of_focus : I::monster_shard_of_strength;
    const I mon_emb = focus ? I::monster_emblem_of_focus : I::monster_emblem_of_strength;
    const I pow_main = focus ? I::power_shard_of_focus : I::power_shard_of_strength;
    const I pow_main_emb = focus ? I::power_emblem_of_focus : I::power_emblem_of_strength;
    const std::uint32_t price = mon_shard.cost();
    if (me.gold_count() < price) return;
    auto go = [&](I item, int n) { if (n > 0 && me.can_buy(item, n)) me.buy(item, n); };
    // A field purchase that throws backs off for 50 rounds instead of costing every turn; spawn purchases rethrow.
    auto guarded = [&](auto&& f) {
        try { f(); }
        catch (...) { if (!field) throw; field_shop_off_until = static_cast<int>(cambc::round_number()) + 50; }
    };
    const auto owned = me.item_count();
    auto have = [&](I shard, I emblem) { return static_cast<int>(owned[shard]) + 5 * static_cast<int>(owned[emblem]); };
    const bool dmg_build = P_BUILD_DMG || (tavi_focus_field && type == CT::tavi);
    if (dmg_build) {
        const int n = static_cast<int>(me.gold_count() / price);
        const int buy_mon = P_BUILD_MON_FIRST ? std::min(n, std::max(0, 5 - have(mon_shard, mon_emb))) : 0;
        guarded([&] { go(mon_shard, buy_mon); go(pow_main, n - buy_mon); });
        return;
    }
    if (rows) {   // EXP-093: Power shards of the main stat only, every visit
        const int n = static_cast<int>(me.gold_count() / price);
        guarded([&] { go(pow_main, n); });
        return;
    }
    int mon = have(mon_shard, mon_emb);
    int pm = have(pow_main, pow_main_emb);
    int ph = have(I::power_shard_of_health, I::power_emblem_of_health);
    std::uint32_t gold = me.gold_count();
    int buy_mon = 0, buy_pm = 0, buy_ph = 0;
    const bool tank = type == CT::varg || type == CT::ryth || type == CT::vendetta;
    const bool field_main = field && !P_FIELD_SHOP_HEALTH;   // no Health in the field (it would not heal)
    while (gold >= price) {
        if (P_HEALTH_FIRST_TANKS && tank && ph < 5) { ++ph; ++buy_ph; }
        else if (mon < 5) { ++mon; ++buy_mon; }
        else if (!field_main && ph <= pm) { ++ph; ++buy_ph; }
        else { ++pm; ++buy_pm; }
        gold -= price;
    }
    guarded([&] {
        go(mon_shard, buy_mon);
        go(I::power_shard_of_health, buy_ph);
        go(pow_main, buy_pm);
    });
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
    if (gems <= 0) { banking = false; carry_since = -1; bank_spawn_done = false; return false; }
    if (altars.empty()) { banking = false; return false; }   // P_S1_BANK_REACH: no usable altar
    if (carry_since < 0) carry_since = v.round;
    if (holding >= 0) return false;
    if (!banking) {
        const bool far = target < 0 || min_distance(v.pos, groups[target].tiles) > 20;
        const int total = static_cast<int>(cambc::global_submitted_gems(team::ally));
        if (rows_sched) {   // EXP-095: bank at 2 bags, or at 1 bag when no unclaimed pack is ready within the round trip
            int nearest = 1 << 20;
            for (auto a : altars) nearest = std::min(nearest, tile_distance(v.pos, a));
            banking = gems >= P_ROWS_BAG2   // EXP-096: rule 1, bank at 66
                || total + gems >= static_cast<int>(cambc::constants::winning_gems) || v.round >= 1900;
        } else
        banking = gems >= (rows ? P_ROWS_BAG : kBankBag) || opening_bank(v, gems) || far || v.round - carry_since >= 40
            || total + gems >= static_cast<int>(cambc::constants::winning_gems) || v.round >= 1900;
        if (!banking) return false;
    }
    if (rows_sched && !bank_spawn_done) {   // EXP-096 rule 1: bank through the spawn (heal at turn end, buy), then the altar
        if (ally_spawn.is_set(v.pos)) {
            if (me.health() < me.max_health() * 0.99f) return true;
            bank_spawn_done = true;
        } else {
            if (incoming_damage(v, v.pos) == 0 && enemies.empty()) { try { me.recall(); return true; } catch (...) {} }
            move_towards_closest(static_block, camp_static, ally_spawn);
            return true;
        }
    }
    // EXP-023: the best altar by distance plus contest; when every altar is held by two or more enemies, keep
    // farming with the bag instead of walking into them or standing still.
    int contest = 0;
    const int pick = choose_altar(v, enemies, contest);
    const bool must = static_cast<int>(cambc::global_submitted_gems(team::ally)) + gems >= static_cast<int>(cambc::constants::winning_gems)
        || v.round >= 1900;
    if (kAltarScoring && (pick < 0 || (contest >= kGiveUpContest && !must))) { banking = false; return false; }
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
            ++lock_wait;
            // R: a locked altar in submit range is cleared whether or not the other altars are known to be locked.
            // Stall fix: an enemy standing within 4 tiles that does not hurt us no longer blocks the lock clearing
            // for good: after 3 turns beside the lock, attack it (the retreat rule still runs before banking).
            if ((foes4 == 0 || lock_wait >= 3) && me.can_attack()) {
                try { me.attack(l); } catch (...) {}
                return true;
            }
        }
        if (locked_now) continue;
        lock_until[i] = 0;   // in view and unlocked
        lock_wait = 0;
        // EXP-024/025 (coordinator, strategy/collision-v43-game.md): damage does not interrupt a tribute, so a
        // carrier in submit range of an unlocked altar submits at once, whatever the enemies.
        try { me.submit_gems(altars[i]); } catch (...) {}
        return true;
    }
    // The safety gate now only decides whether to walk in: not safe, more than 2 tiles from the nearest altar,
    // and an enemy within 4 tiles of that altar: the enemy rules and farming take the turn.
    if (!safe) {
        int near = -1, near_d = 1 << 20;
        for (std::size_t i = 0; i < altars.size(); ++i) {
            const int d = tile_distance(v.pos, altars[i]);
            if (d < near_d) { near_d = d; near = static_cast<int>(i); }
        }
        if (near >= 0 && near_d > 2)
            for (const auto& e : enemies) if (tile_distance(e.position(), altars[near]) <= 4) return false;
    }
    if (kAltarScoring && pick >= 0)
        move_towards_closest(static_block, camp_static, tiles_around(altars[pick], cambc::constants::submit_gems_range));
    else
        move_towards_closest(static_block, camp_static, altar_goal_tiles(v.round));   // EXP-018: skips stuck-excluded altars
    return true;
}

// P_S1_NO_SELF_BLOCK: a champion ending its turn on a spawn tile of a group that is respawning (its capture is paid)
// steps off it: at timer 0 an occupied tile holds the whole group (arena test 3). Rule: on tile of group g, g not held
// for a capture, state(g) = respawning, can move -> step to the nearest tile that is no spawn tile.
void Bot::step_off_respawn() {
    auto& me = cambc::self();
    if (!initialised || !me.is_alive() || !me.can_move() || me.is_submitting_gems() || me.is_recalling() || me.is_teleporting()) return;
    const vec2 p = me.position();
    if (!camp_static.is_set(p)) return;
    for (std::size_t i = 0; i < groups.size(); ++i) {
        if (!groups[i].mask.is_set(p)) continue;
        if (static_cast<int>(i) == holding && groups[i].ready_round <= static_cast<int>(cambc::round_number())) return;
        try {
            const auto st = groups[i].group.state();
            const bool paid = std::holds_alternative<cambc::map::camp_group_respawning>(st);
            // P_S1_ONE_CAPTURER: a capture in progress belongs to the holder (or the solo farmer of the group) alone
            const bool other = P_S1_ONE_CAPTURER && std::holds_alternative<cambc::map::camp_group_reward_available>(st)
                && static_cast<int>(i) != holding && static_cast<int>(i) != target;
            if (!paid && !other) return;
        } catch (...) { return; }
        cambc::bitboard out = tiles_around(p, 3) & ~camp_static & ~static_block;
        move_towards_closest(static_block, camp_static, out);
        return;
    }
}

#if P_S1_DIAG
// S1 diagnostics (v5-s1-economy.md step 1): champion-rounds on a spawn tile of a group whose respawn timer is 0 (the
// occupant alone holds the respawn: arena test 3), and champion-rounds sharing a group's tiles with another ally.
// Cumulative counts printed every 25 rounds (std::endl ~3,500 fuel; 4 lines per 100 rounds).
void Bot::s1_diag() {
    auto& me = cambc::self();
    const int rnd = static_cast<int>(cambc::round_number());
    diag_sb_grp = -1;
    if (me.is_alive() && initialised) {
        const vec2 p = me.position();
        for (std::size_t i = 0; i < groups.size(); ++i) {
            if (!groups[i].mask.is_set(p)) continue;
            ++diag_on;
            try {
                const auto st = groups[i].group.state();
                if (auto r = std::get_if<cambc::map::camp_group_respawning>(&st)) { if (r->timer <= 0) { ++diag_selfblock; diag_sb_grp = static_cast<int>(i); } else ++diag_resp; }
                else if (std::holds_alternative<cambc::map::camp_group_reward_available>(st)) ++diag_capt;
            } catch (...) {}
            try { if ((cambc::vision::champion_tiles(team::ally) & groups[i].mask).count() >= 2) ++diag_dup; } catch (...) {}
            break;
        }
    }
#if P_S1_TRACE
    if (rnd % P_S1_TRACE == 0) {
        const vec2 p = me.position();
        std::cout << "S1T r" << rnd << " a" << ally_index << " ch" << int(me.champion_name()) << " p" << p.x << "," << p.y << " hp" << int(me.is_alive() ? me.health() : -1)
                  << " g" << me.gem_count() << " au" << me.gold_count() << " tg" << target << " hd" << holding << " rt" << retreating << " bk" << banking << " sub" << me.is_submitting_gems() << " rc" << me.is_recalling()
                  << " rdy" << (target >= 0 ? groups[target].ready_round : -1) << " ex" << (target >= 0 ? groups[target].excluded_until : -1) << " nc" << last_nc << " sbg" << diag_sb_grp << " wt" << wait_rounds;
        if (diag_sb_grp >= 0) for (auto t : groups[diag_sb_grp].tiles) std::cout << " " << t.x << "," << t.y;
        std::cout << std::endl;
    }
#endif
    if (rnd % 25 == 0) std::cout << "S1D a" << ally_index << " r" << rnd << " on" << diag_on << " sb" << diag_selfblock << " rs" << diag_resp << " cp" << diag_capt << " dup" << diag_dup << std::endl;
}
#endif

void Bot::play_turn() {
#if TRACE_V
    struct FG { int r, u; ~FG() { if (r <= 14) std::cout << "F r" << r << " u" << u << " left" << cambc::fuel_remaining() << "\n"; } } fg{static_cast<int>(cambc::round_number()), ally_index};
#endif
    if (!initialised) { init(); if (!initialised || cambc::fuel_remaining() < 80000) return; }   // staged init: an unfinished init (or one that left < 80k) yields the turn
#if TRACE_V
    if (ally_index == 0 && cambc::round_number() >= 18 && cambc::round_number() <= 45) std::cout << "S r" << cambc::round_number() << " p" << cambc::self().position().x << "," << cambc::self().position().y << " hp" << (int)cambc::self().health() << " alive" << cambc::self().is_alive() << "\n";
#endif
    if (P_DEATH_LOOP) {   // FIX_DEATH_LOOP: dead champions still get turns; count a death once at the group we were working; twice = leave it for a long time
        const bool alive_now = cambc::self().is_alive();
        if (!alive_now && was_alive) {
            was_alive = false;
            if (last_tgt >= 0 && last_tgt < static_cast<int>(groups.size())) {
                auto& lg = groups[last_tgt];
            const int rnd = static_cast<int>(cambc::round_number());
            // a loop = a second death at the same group within P_DEATH_WINDOW rounds (period ~90 in the corridors/broken_curve loops)
            if (((P_DEATH_LOOP == 3) ? true : (P_DEATH_LOOP == 2) ? !last_had_foe : (lg.last_death >= 0 && rnd - lg.last_death <= P_DEATH_WINDOW)) && ++lg.deaths >= P_DEATH_LOOP_N)
                lg.excluded_until = std::max(lg.excluded_until, rnd + P_DEATH_EXCLUDE);
            lg.last_death = rnd;
        }
            last_tgt = -1;
        } else if (alive_now) was_alive = true;
    }
    auto& me = cambc::self();
    update_stuck();   // EXP-018
    could_attack = me.is_alive() && me.can_attack();
#if P_ROWS_DBG
    g_dbg.clear();
#endif
#if P_ROWS_DBG
    const cambc::vec2 pos0 = me.position();
    const int hp0 = me.is_alive() ? int(me.health()) : -1;
    const int rnd = static_cast<int>(cambc::round_number());
    try { play_turn_inner(); } catch (...) { if (P_ROWS_DBG) std::cout << "EXC r" << rnd << " a" << ally_index << g_dbg << " pk" << my_pack << " retr" << retreating << " bank" << banking << std::endl; throw; }
    if (P_ROWS_DBG && rows_sched && rnd <= 100 && me.is_alive()) {
        std::array<std::uint8_t, 8> mm{};
        cambc::shared_memory.read(0, std::span{mm.data(), mm.size()});
        const auto p1 = me.position();
        std::cout << "r" << rnd << " a" << ally_index << " " << int(me.champion_name()) << " " << pos0.x << "," << pos0.y << ">" << p1.x << "," << p1.y
                  << " pk" << my_pack << " tg" << target << " idle" << idle_rows << " hold" << holding << " bank" << banking << " retr" << retreating
                  << " hp" << hp0 << ">" << int(me.health()) << " cm" << me.can_move() << " ca" << me.can_attack() << " fuel" << cambc::fuel_remaining()
                  << " mem" << int(mm[0]) << int(mm[1]) << int(mm[2]) << int(mm[3]) << int(mm[4]) << g_dbg << std::endl;
    }
#else
    play_turn_inner();
#endif
    attacked = could_attack && me.is_alive() && !me.can_attack();
    if (P_S1_NO_SELF_BLOCK && P_S1_NSB_STEP) step_off_respawn();
#if P_S1_DIAG
    s1_diag();
#endif
    std::uint8_t mine = me.is_alive() ? static_cast<std::uint8_t>(target + 1) : 0;
    if (P_S1_AUCTION == 2 && mine && initialised) {   // hybrid: ETA of the current target for the claim byte
        const int rnd = static_cast<int>(cambc::round_number()), tau = s1_tau(target, me.position());
        s1_eta = tau + std::max(0, groups[target].ready_round - rnd - tau);
    }
    if (P_S1_AUCTION && mine) mine |= static_cast<std::uint8_t>(eta_bucket(s1_eta) << 6);   // S1 claim: task | ETA bucket
    if (P_S1_AUCTION) s1_tables_step();
    cambc::shared_memory.write(ally_index, std::span<const std::uint8_t>{&mine, 1});
}

void Bot::play_turn_inner() {
    auto& me = cambc::self();
    if (!me.is_alive()) { unseen_hp_prev = -1; buy_items(); holding = -1; retreating = false; banking = false; bank_spawn_done = false; regrouping = false; bag_fled = false; return; }
    if (me.is_submitting_gems()) was_submitting = true;
    if (me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) return;
    const bool tribute_done = was_submitting && me.gem_count() == 0;   // P_SHOP_RECALL: first free turn after a tribute
    was_submitting = false;

    if (P_OPENING_ASSIGN && me.gem_count() > 0) open_done = true;   // EXP-079: first gems collected
    const TurnView v = build_view();
    last_nc = static_cast<int>(v.camps.size());
    g_threat_exempt.clear();   // P_S1_THREAT_COST: retreats avoid every camp; set to the target after resolve_target
    update_group_memory(v);
    note_mixed_clusters(v);   // EXP-030
    const bool at_spawn = ally_spawn.is_set(v.pos);
    if (at_spawn || (P_FIELD_SHOP && remote_shop) || (tavi_focus_field && remote_shop && v.round >= field_shop_off_until)) buy_items();   // w8 C
    if (!me.can_move() && !me.can_attack()) return;
    // Spawn recovery happens at the end of a turn spent on the spawn, so a hurt champion stays until healed.
    if (at_spawn && me.health() < me.max_health() * kSpawnLeaveHp) { DBG(" HEALWAIT"); return; }
    if (handle_stuck(v)) return;   // EXP-018

    cast_shield(v);   // S-14

    auto enemies = cambc::vision::champions(team::enemy);
    last_had_foe = !enemies.empty();
    if (P_S1_AUCTION) s1_note_foes(v, enemies);   // FIX_DEATH_LOOP: a death with no enemy champion in view was the camps
    auto allies = cambc::vision::champions(team::ally);
    update_contested(v, enemies);   // EXP-018
    if (Plan::turn(*this, Plan::Phase::pre, v, enemies, allies)) return;
    const bool enemies_near = !enemies.empty();
    float threshold = (me.gem_count() > 0 || enemies_near) ? kRetreatHurt : kRetreatCalm;
    if (engaged_at_squad_target(v)) threshold = std::max(threshold, kRetreatTank);   // tank swap at 35 per cent
    const double incoming = incoming_damage(v, v.pos);
    if (P_UNSEEN_HIT) {   // FIX_UNSEEN_HIT
        const float hp = me.health();
        if (unseen_hp_prev >= 0 && hp < unseen_hp_prev && incoming == 0 && enemies.empty() && !at_spawn) unseen_hit_until = v.round + P_UNSEEN_MEMORY;
        unseen_hp_prev = hp;
    }
    const bool unseen_hit = P_UNSEEN_HIT && v.round <= unseen_hit_until;
    if (P_SHOP_RECALL && tribute_done && !at_spawn && incoming == 0 && min_distance(v.pos, spawn_tiles) >= P_SHOP_RECALL_ALTAR_DIST) {
        // P_SHOP_RECALL: recall to shop and heal after a far tribute (no champion within 8; damage would interrupt it).
        bool foe8 = false;
        for (const auto& e : enemies) if (tile_distance(e.position(), v.pos) <= 8) foe8 = true;
        const bool can_shop = !((P_FIELD_SHOP || tavi_focus_field) && remote_shop) && me.gold_count() >= cambc::item_type::monster_shard_of_strength.cost();   // w8: Tavi shops where he stands
        if (!foe8 && (can_shop || me.health() < me.max_health() * 0.9f)) { try { me.recall(); } catch (...) {} return; }
    }
    // Inside camp range the retreat starts early enough to walk out and finish a recall.
    double need = std::max<double>(me.max_health() * threshold, incoming * P_RETREAT_INCOMING_MULT + P_RETREAT_INCOMING_BASE * me.max_health());
    if (rows_sched && !enemies_near) {   // EXP-096 rule 1: leave below 35 per cent unless an ally is within 3 tiles of my pack (floor 15 per cent)
        need = me.max_health() * 0.35;
        if (my_pack >= 0)
            for (const auto& a : allies) if (a.is_visible() && !(a == me) && min_distance(a.position(), groups[my_pack].tiles) <= 3) { need = me.max_health() * 0.15; break; }
    }
    // Retreat is a mode: without it a champion steps out of camp range, resumes at the edge, and repeats.
    if (!at_spawn && me.health() < need) { retreating = true; holding = -1; DBG(" RETREAT-FIRE need" + std::to_string(int(need))); }
    if (retreating && m3_on(M3_NORETREAT, me.champion_name()) && me.gem_count() < 40) {   // v3b item 1: no walk-home mid-fight
        int f4 = 0, a4 = 0;
        for (const auto& e : enemies) if (e.health() > 0 && tile_distance(v.pos, e.position()) <= 4) ++f4;
        for (const auto& a : allies) if (!(a == me) && tile_distance(v.pos, a.position()) <= 4) ++a4;
        if (f4 >= 1 && !(f4 >= 2 && a4 == 0)) { retreating = false; DBG(" M3-NORETREAT"); }
    }
    DBG(" need" + std::to_string(int(need)) + (retreating ? " retr" : ""));
    if (rows_sched && retreating) { target = -1; my_pack = -1; }   // EXP-095: release the claim while healing
    if (retreating && (rows_sched && !at_spawn ? me.health() >= me.max_health() * 0.6f : at_spawn ? me.health() >= me.max_health() * kRecallDoneSpawn : me.health() >= me.max_health() * kRecallDoneField)) retreating = false;
    // Stall fix: a retreat that steps A, B, A, B between two tiles of overlapping camp danger zones never heals (the
    // recall needs no live camp within 4 tiles). After 4 such turns walk home through the zone for 40 rounds.
    if (retreating && v.pos == pos_prev2 && !(v.pos == pos_prev)) ++retreat_osc;
    else if (!(v.pos == pos_prev2)) retreat_osc = 0;
    if (retreat_osc >= 4) { retreat_walk_until = v.round + 40; retreat_osc = 0; }
    pos_prev2 = pos_prev; pos_prev = v.pos;
    if (retreating && !at_spawn) {
        // Spawn within 6 tiles: walk in for a full heal at the turn end; else leave camp range and recall.
        if (min_distance(v.pos, spawn_tiles) <= kNearSpawn && move_towards_closest(static_block, camp_static, ally_spawn)) return;
        if (incoming > 0) {
            if (v.round < retreat_walk_until && move_towards_closest(static_block, camp_static, ally_spawn)) return;
            if (move_towards_closest(static_block, camp_static, ~danger_zone(v))) return;
        } else if (unseen_hit) { move_towards_closest(static_block, camp_static, ally_spawn); return; }   // FIX_UNSEEN_HIT
        else if (!enemies_near) { me.recall(); return; }
        else { move_towards_closest(static_block, camp_static, ally_spawn); return; }
    }

    if (P_THREAT_LAYER) {   // EXP-071: survive and bank (after the health retreat, which stays)
        allies6 = 0;
        for (const auto& a : allies) if (!(a == me) && tile_distance(v.pos, a.position()) <= 6) ++allies6;
        if (threat_layer(v, enemies, allies)) { DBG(" THREAT"); return; }
    }
    // A bag ready to bank at an altar is submitted first: damage does not interrupt a tribute.
    if (banking && me.gem_count() > 0)
        for (auto altar : altars)
            if (tile_distance(v.pos, altar) <= cambc::constants::submit_gems_range) { if (bank(v, enemies, allies)) return; break; }
    if (enemy_response(v, enemies, allies)) { DBG(" ENEMYRESP"); return; }
    if (P_ALLY_VISION_FIX) { keep_visible(enemies); keep_visible(allies); }   // killed or out-of-sight handles throw 97
    if (Plan::turn(*this, Plan::Phase::mid, v, enemies, allies)) return;

    // Low and chased: walk home.
    if (!at_spawn && enemies_near && me.health() < me.max_health() * threshold) {
        move_towards_closest(static_block, camp_static, ally_spawn);
        return;
    }

    resolve_target(v, allies);
    Plan::target(*this, v, target);
    if (P_S1_THREAT_COST && target >= 0) g_threat_exempt = groups[target].mask;
    if (target >= 0) last_tgt = target;
#if TRACE_V
    if ((v.round >= 18 && v.round <= 45) || v.round % 200 == 5) std::cout << "T r" << v.round << " u" << ally_index << " p" << v.pos.x << "," << v.pos.y << " tg" << target << "x" << (target >= 0 ? groups[target].excluded_until : -1) << " hp" << (int)cambc::self().health() << " g" << (int)cambc::self().gem_count() << "\n";
#endif
    if (bank(v, enemies, allies)) { DBG(" BANK"); return; }
    DBG(" FARM");
    farm(v, allies);
}
} // namespace va
