// EXP-093 rows commander: fixed pack schedule (model policy e2, revised after game 1). Unit A (Issac+Cassi) owns the nearest own-half
// pack; unit B (Varg+Tavi+Stellar: Varg splashes the whole pack and tanks for the two ranged champions, who cannot clear a pack
// alone) alternates between the other two. Each unit waits beside its pack for the 40-round respawn, clears it, one capturer
// (highest health) stands on a spawn tile, and the bank walk happens inside the next wait. Active only on rows with the roster
// Cassi, Issac, Varg, Tavi, Stellar; otherwise the v4-14 flow runs (with the one-capturer rule).
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#include <array>
#include "util.h"
#if P_ROWS_DBG
#include <iostream>
#endif
#include <span>
#include <vector>

#if P_ROWS_DBG
std::string g_dbg;
#endif
#include "profile.h"
using CT = cambc::champion_type;
using cambc::vec2;

void Bot::init_rows_sched() {
    if (!rows) return;
    rows_sched = false;
    if (!rows || !P_ROWS_SCHED || roster.size() != 5) return;
    std::vector<int> own;
    for (std::size_t i = 0; i < groups.size(); ++i)
        if (groups[i].wolves >= 3 && !groups[i].monolith && groups[i].dist_ally <= groups[i].dist_enemy) own.push_back(static_cast<int>(i));
    if (own.size() != 3) return;
    std::sort(own.begin(), own.end(), [&](int a, int b) { return groups[a].dist_ally < groups[b].dist_ally; });
    mates.clear();
    pack_list = own;   // EXP-095: every champion is an independent collector over the three own-half packs
    my_pack = -1;
    takes = 0;
    rows_sched = true;
}

// Shared bytes 0..4 = each champion's claimed group + 1; bytes 5..7 = last capture round / 4 (mod 255) + 1 of own pack k.
static int decode_capture(std::uint8_t s, int round) {
    if (s == 0) return -1000;
    const int r4 = round / 4;
    const int delta = ((r4 - (s - 1)) % 255 + 255) % 255;
    return (r4 - delta) * 4;
}

int Bot::rows_ready(int k, int round, const std::array<std::uint8_t, 8>& mem) const {
    const int c = decode_capture(mem[5 + k], round);
    return std::max(c > -1000 ? c + 45 : 0, groups[pack_list[k]].ready_round);
}

bool Bot::rows_claimed(int k, const std::array<std::uint8_t, 8>& mem, bool lower_only) const {
    for (int j = 0; j < 5; ++j) {
        if (j == ally_index || (lower_only && j > ally_index)) continue;
        if (mem[j] == pack_list[k] + 1) return true;
    }
    return false;
}

// Earliest ready round among own packs nobody else has claimed (1 << 20 when none).
int Bot::rows_free_ready(int round) const {
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    int best = 1 << 20;
    for (std::size_t k = 0; k < pack_list.size(); ++k)
        if (!rows_claimed(static_cast<int>(k), mem, false)) best = std::min(best, std::max(rows_ready(static_cast<int>(k), round, mem), round));
    return best;
}

// EXP-096 wave relay. Every champion is an independent collector: pick the own pack with the earliest ready time (ties:
// fewest allies heading there, then distance), walk to ONE static goal set, attack the lowest-health wolf in reach, never
// leave a pack that is up, and leave at once after the clear (one champion stays to capture). Cassi follows Issac.
void Bot::resolve_rows(const TurnView& v, const std::vector<cambc::champion>& allies) {
    const int round = v.round;
    auto& me = cambc::self();
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    if (holding >= 0) {
        auto& hg = groups[holding];
        bool alive = false;
        for (const auto& c : v.camps) if (c.group == holding) alive = true;
        refresh_group(holding, round, v.visible);
        const bool paid = alive || hg.ready_round > round;
        if (paid || round - hold_since > 20) {
            if (!paid) hg.ready_round = round + 40;
            for (std::size_t k = 0; k < pack_list.size(); ++k)
                if (pack_list[k] == holding) {
                    const std::uint8_t b = static_cast<std::uint8_t>((round / 4) % 255 + 1);
                    cambc::shared_memory.write(5 + static_cast<int>(k), std::span<const std::uint8_t>{&b, 1});
                }
            holding = -1;
            my_pack = -1;
            ++takes;
        } else { target = holding; return; }
    }
    if (banking) my_pack = -1;
    int cur = -1;
    for (std::size_t i = 0; i < pack_list.size(); ++i) if (pack_list[i] == my_pack) cur = static_cast<int>(i);
    if (cur >= 0 && rows_ready(cur, round, mem) <= round && group_cleared(v, my_pack)) {   // rule 4: the pack fell
        int best_d = min_distance(v.pos, groups[my_pack].tiles), best_i = ally_index;
        for (const auto& a : allies) {
            if (!a.is_visible()) continue;
            const int idx = roster_index(a);
            if (idx < 0 || a == me || !a.is_visible() || !a.is_alive()) continue;
            const int d = min_distance(a.position(), groups[my_pack].tiles);
            if (d > 4) continue;
            if (d < best_d || (d == best_d && idx < best_i)) { best_d = d; best_i = idx; }
        }
        if (best_i == ally_index) { holding = my_pack; hold_since = round; target = holding; return; }
        groups[my_pack].ready_round = round + 40;   // the capturer is on it: leave at once
        my_pack = -1;
    }
    if (my_pack < 0) {
        int best = 0, best_key = 1 << 30;
        const int follow = (kit_of(me.champion_name()).pair_follower && ally_index == 0) ? mem[1] : 0;   // Cassi and Issac move as one unit
        for (std::size_t i = 0; i < pack_list.size(); ++i) {
            const int k = static_cast<int>(i);
            int claims = 0;
            for (int j = 0; j < 5; ++j) if (j != ally_index && mem[j] == pack_list[i] + 1) ++claims;
            const int d = min_distance(v.pos, groups[pack_list[i]].tiles);
            int key = (std::max(rows_ready(k, round, mem), round) * 16 + std::min(claims, 15)) * 128 + std::min(d, 127);
            if (follow && follow == pack_list[i] + 1) key = -1;
            if (key < best_key) { best_key = key; best = k; }
        }
        my_pack = pack_list[best];
    }
    idle_rows = false;
    target = my_pack;
}

static cambc::bitboard approach_goals(const GroupInfo& g, bool melee, const cambc::bitboard& block) {
    if (melee) return g.ring;
    cambc::bitboard near3, near2;
    near3.clear(); near2.clear();
    for (const auto& t : g.tiles) { near3 |= tiles_around(t, 3); near2 |= tiles_around(t, 2); }
    return near3 & ~near2 & ~g.mask & ~block;
}

void Bot::farm_rows(const TurnView& v, const std::vector<cambc::champion>&) {
    auto& me = cambc::self();
    if (holding >= 0) {
        const auto& hg = groups[holding];
        if (!hg.mask.is_set(v.pos)) move_towards_closest(static_block, camp_static, hg.mask);
        return;
    }
    if (ally_spawn.is_set(v.pos)) last_shop = v.round;
    if (me.gem_count() == 0 && me.gold_count() >= cambc::item_type::monster_shard_of_strength.cost() && v.round - last_shop > 40
        && min_distance(v.pos, spawn_tiles) <= 16 && !ally_spawn.is_set(v.pos)) {   // buy on the way: the spawn is near the altar route
        move_towards_closest(static_block, camp_static, ally_spawn);
        return;
    }
    if (my_pack < 0) return;
    const auto& g = groups[my_pack];
    target = my_pack;
    std::vector<const LiveCamp*> wolves;
    for (const auto& c : v.camps) if (c.group == my_pack) wolves.push_back(&c);
    std::sort(wolves.begin(), wolves.end(), [](const LiveCamp* a, const LiveCamp* b) { return a->health < b->health; });
    DBG(" d" + std::to_string(min_distance(v.pos, g.tiles)) + " wolves" + std::to_string(wolves.size()));
    auto strike = [&]() {   // rule 2: lowest-health wolf in reach
        for (const auto* w : wolves) {
            try { if (attack_if_in_range(w->camp)) { reset_and_attack_again(w->camp); return true; } } catch (...) {}   // a wolf killed this turn throws 97
        }
        return false;
    };
    if (!wolves.empty()) {
        for (const auto* w : wolves) { try { if (cast_on_camp(w->camp)) break; } catch (...) {} }
        if (strike()) return;
        const vec2 pp = wolves[0]->camp.spawn_position();
        const cambc::bitboard stat = approach_goals(g, is_melee, static_block);
        cambc::bitboard goals;
        if (!stat.is_set(v.pos) && min_distance(v.pos, g.tiles) > 4) goals = stat;   // far: the one static goal set
        else goals = is_melee ? (tiles_around(pp, 1) & ~g.mask & ~static_block) : (tiles_around(pp, 3) & ~tiles_around(pp, 2) & ~g.mask & ~static_block);
        cambc::bitboard live; live.clear();
        for (const auto& c : v.camps) live.set(c.camp.spawn_position(), true);
        goals &= ~live;
        if (goals.is_empty()) goals = stat;
        if (!goals.is_set(v.pos)) move_towards_closest(static_block, camp_static, goals);
        strike();
        return;
    }
    // Nothing visible: walk to (or wait on) the static goal set.
    const cambc::bitboard stat = approach_goals(g, is_melee, static_block);
    if (!stat.is_set(v.pos)) move_towards_closest(static_block, camp_static, stat);
}
