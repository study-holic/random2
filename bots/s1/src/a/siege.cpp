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
// siege.cpp (w9): Monolith siege. Casters (a damage, area-damage or damaging stun ability with range >= 4 that
// targets camps or ground) stand at Chebyshev distance exactly 4 from a live Monolith with line of sight and cast
// on it; the Monolith attacks at range 3 only, and its health does not regenerate (v2 contest replays: monotone
// decline over 2,000 rounds). When it dies, the nearest caster stands on its spawn tile for the capture; the bag
// then goes through the default banking. Non-casters keep the default behaviour.
#include "siege.h"
#include "bot.h"
#include "util.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <type_traits>
#include <variant>

using cambc::vec2;
using cambc::team;
using CT = cambc::champion_type;

namespace Siege {

namespace {

struct Mono {
    explicit Mono(const cambc::camp& c) : camp(c) {}
    cambc::camp camp;
    int group = -1;
    vec2 pos{};
    cambc::bitboard ring;      // free tiles at distance exactly 4 with line of sight to the Monolith
    float hp = 20000.0f;
    bool dead = false;         // last seen dead (reward pending or respawning)
};

Options g_opt;
bool g_ready = false;
bool g_caster = false;
std::vector<Mono> g_monos;
std::vector<bool> g_roster_caster;
cambc::bitboard g_block;
bool g_block_any = false;
int g_cur = -1;
int g_escort_blind = 0;   // turns the carrier tile was checkable with no carrier seen
int g_carry_wait = 0;     // turns a fresh carrier waited for its escort
bool g_hold_bag = false;  // carrier near an altar with an enemy close: no tribute start this turn
int g_hold_turns = 0;     // turns held for the current bag

bool type_caster(CT t) {
    if (t == CT::cassi && g_opt.cassi_farms) return false;
    return t == CT::cassi || t == CT::stellar || t == CT::prism || t == CT::marcella || t == CT::tavi || t == CT::ren
        || t == CT::ryth;
}

// Range and damage modifier of a damage, area-damage or crowd-control ability; false for other kinds.
bool dmg_info(const cambc::ability& ab, float& range, float& mod) {
    try {
        const auto& inf = ab.info();
        if (!inf) return false;
        bool ok = false;
        std::visit([&](const auto& x) {
            using X = std::decay_t<decltype(x)>;
            if constexpr (std::is_same_v<X, cambc::ability_damage> || std::is_same_v<X, cambc::ability_aoe_damage>
                          || std::is_same_v<X, cambc::ability_cc>) { range = x.range; mod = x.modifier; ok = true; }
        }, *inf);
        return ok;
    } catch (...) { return false; }
}

bool cast_on_mono(const Mono& m) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        float range = 0, mod = 0;
        if (!dmg_info(ab, range, mod) || mod <= 0.0f || range < 4.0f) continue;
        try {
            if (!ab.is_ready()) continue;
            const auto tiles = ab.valid_tiles();
            if (ab.can_target_camps()) {
                if (tiles.is_set(m.pos) && ab.use(m.camp)) return true;
            } else if (ab.can_target_ground()) {
                if (tiles.is_set(m.pos)) { try { if (ab.use(m.pos)) return true; } catch (...) {} }
                for (auto t : (tiles_around(m.pos, 1) & tiles & ~cambc::map::walls()).positions()) {
                    if (t == m.pos) continue;
                    try { if (ab.use(t)) return true; } catch (...) {}
                }
            }
        } catch (...) {}
    }
    return false;
}

// Live camps of the Monolith's group other than the Monolith (gem clusters on the monolith map), basic attack.
bool hit_group_camp(const TurnView& v, int group) {
    for (const auto& c : v.camps)
        if (c.group == group && !c.camp.is_monolith() && attack_if_in_range(c.camp)) { reset_and_attack_again(c.camp); return true; }
    return false;
}

void rebuild_block(int round, const Bot& b) {
    g_block.clear();
    g_block_any = false;
    for (const auto& m : g_monos) {
        if (m.dead || b.groups[m.group].ready_round > round) continue;
        g_block |= tiles_around(m.pos, m.camp.attack_range());
        g_block_any = true;
    }
}

}  // namespace

void enable(const Options& o) { g_opt = o; }
bool active() { return g_opt.on; }
bool mono_tight() { return g_opt.on; }
const cambc::bitboard* block() { return (g_opt.on && g_block_any) ? &g_block : nullptr; }

void init(Bot& b) {
    if (!g_opt.on) return;
    g_ready = true;
    const cambc::bitboard opaque = cambc::map::walls() | cambc::map::bushes();
    for (int i = 0; i < static_cast<int>(b.groups.size()); ++i) {
        if (!b.groups[i].monolith) continue;
        for (const auto& c : b.groups[i].group.camps()) {
            if (!c.is_monolith()) continue;
            Mono m(c);
            m.group = i;
            m.pos = c.spawn_position();
            const cambc::bitboard lit = opaque.shadow_cast(m.pos, 8);
            const cambc::bitboard shell = tiles_around(m.pos, 4) & ~tiles_around(m.pos, 3) & ~b.static_block & ~b.camp_static;
            m.ring = shell & lit;
            if (m.ring.is_empty()) m.ring = shell;
            g_monos.push_back(m);
        }
    }
    g_roster_caster.assign(b.roster.size(), false);
    for (std::size_t i = 0; i < b.roster.size(); ++i) g_roster_caster[i] = type_caster(b.roster[i].champion_name());
    auto& me = cambc::self();
    // A caster has a damaging ability of range >= 4 on camps or ground (checked on the live kit, not only the type).
    std::string kit;
    for (const auto& ab : me.abilities()) {
        float range = 0, mod = 0;
        if (!dmg_info(ab, range, mod)) continue;
        bool camps = false, ground = false;
        try { camps = ab.can_target_camps(); ground = ab.can_target_ground(); } catch (...) {}
        kit += " s" + std::to_string(ab.slot()) + ":r" + std::to_string(static_cast<int>(range)) + "m" + std::to_string(static_cast<int>(mod * 100))
            + (camps ? "C" : "") + (ground ? "G" : "");
        if (mod > 0.0f && range >= 4.0f && (camps || ground)) g_caster = true;
    }
    if (g_opt.cassi_farms && me.champion_name() == CT::cassi) g_caster = false;
    int mono_range = -1;
    if (!g_monos.empty()) { try { mono_range = g_monos[0].camp.attack_range(); } catch (...) {} }
    std::cout << "siege t" << static_cast<int>(me.champion_name()) << (g_caster ? " caster" : " -") << kit << " monos" << g_monos.size()
              << " monorange" << mono_range << " ring" << (g_monos.empty() ? 0 : static_cast<int>(g_monos[0].ring.count())) << '\n';
}

bool pre(Bot& b, const TurnView& v, std::vector<cambc::champion>&, std::vector<cambc::champion>& allies) {
    if (!g_opt.on || !g_ready) return false;
    // Refresh what this champion sees of each Monolith (alive or dead), then the blocked danger tiles.
    for (auto& m : g_monos) {
        if (!v.visible.is_set(m.pos)) continue;
        bool alive = false;
        try { alive = m.camp.is_visible() && m.camp.health() > 0; if (alive) m.hp = m.camp.health(); } catch (...) {}
        if (alive) { m.dead = false; if (b.groups[m.group].ready_round > v.round) b.groups[m.group].ready_round = 0; }
        else m.dead = true;
    }
    rebuild_block(v.round, b);
    // Safe tribute (g_opt.safe_tribute): a 200-gem bag channels 20 turns and any stun cancels it (GameRules 223), so a
    // carrier within 8 tiles of an altar does not start one while an enemy champion is within safe_radius tiles.
    g_hold_bag = false;
    if (g_opt.safe_tribute > 0) {
        auto& me = cambc::self();
        const int gems = static_cast<int>(me.gem_count());
        const int total = static_cast<int>(cambc::global_submitted_gems(team::ally));
        if (gems == 0) g_hold_turns = 0;
        else if (gems >= 50 && v.round < 1900 && total + gems < static_cast<int>(cambc::constants::winning_gems)
                 && g_hold_turns < g_opt.hold_max) {
            int near_altar = 1 << 20;
            for (auto a : b.altars) near_altar = std::min(near_altar, tile_distance(v.pos, a));
            if (near_altar <= 8)
                for (auto e : cambc::vision::champion_tiles(team::enemy).positions())
                    if (tile_distance(e, v.pos) <= g_opt.safe_tribute) { g_hold_bag = true; break; }
            if (g_hold_bag) { ++g_hold_turns; b.banking = false; }
        }
    }
    // Escort (g_opt.escort): a champion carrying >= 50 gems writes its tile to shared bytes 6-7 as x+100, y+100
    // (contest has no squad groups, so the squad-claim bytes are free; values >= 100 never match a group index).
    if (g_opt.escort) {
        const int gems = static_cast<int>(cambc::self().gem_count());
        std::array<std::uint8_t, 2> cb{};
        cambc::shared_memory.read(kSquadClaimByte, std::span{cb.data(), cb.size()});
        if (gems >= 50) {
            const std::array<std::uint8_t, 2> w{static_cast<std::uint8_t>(v.pos.x + 100), static_cast<std::uint8_t>(v.pos.y + 100)};
            cambc::shared_memory.write(kSquadClaimByte, std::span<const std::uint8_t>{w.data(), w.size()});
        } else if (cb[0] >= 100 && cb[1] >= 100) {
            // Stale (carrier dead, robbed or done): this champion stands on the tile, or for 3 turns has the tile in
            // view or within 2 tiles with no visible ally carrying >= 50 gems within 3 of it (the tile may be a bush).
            const vec2 cp(cb[0] - 100, cb[1] - 100);
            bool seen = false;
            for (const auto& a : allies) {
                try { if (a.is_visible() && !(a == cambc::self()) && tile_distance(a.position(), cp) <= 3 && a.gem_count() >= 50) { seen = true; break; } } catch (...) {}
            }
            bool clear = cp == v.pos;
            if (!clear && !seen && (v.visible.is_set(cp) || tile_distance(v.pos, cp) <= 2)) clear = ++g_escort_blind >= 3;
            else g_escort_blind = 0;
            if (clear) {
                g_escort_blind = 0;
                const std::array<std::uint8_t, 2> z{0, 0};
                cambc::shared_memory.write(kSquadClaimByte, std::span<const std::uint8_t>{z.data(), z.size()});
            }
        }
    }
    return false;
}

bool mid(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies) {
    (void)enemies;
    if (!g_opt.on || !g_ready || g_monos.empty()) return false;
    auto& me = cambc::self();
    auto release = [&]() {
        if (b.target >= 0 && b.target < static_cast<int>(b.groups.size()) && b.groups[b.target].monolith) b.target = -1;
        return false;
    };
    if (g_opt.escort && me.gem_count() >= 50 && !b.retreating && !b.banking) {
        // Fresh carrier: hold (fights already ran) until two allies are within 3 tiles, at most carry_wait turns.
        int near = 0;
        for (const auto& a : allies) if (!(a == me) && a.is_visible() && tile_distance(a.position(), v.pos) <= 3) ++near;
        if (near < 2 && g_carry_wait < g_opt.carry_wait) { ++g_carry_wait; release(); return true; }
    }
    if (me.gem_count() == 0) g_carry_wait = 0;
    if (g_hold_bag && !b.retreating) {   // hold beside the escort, out of the channel
        release();
        return true;
    }
    if (me.gem_count() > 0 || b.retreating || b.banking) return release();
    if (g_opt.escort) {   // walk beside the bag carrier (any champion type) until it has tributed
        std::array<std::uint8_t, 2> cb{};
        cambc::shared_memory.read(kSquadClaimByte, std::span{cb.data(), cb.size()});
        if (cb[0] >= 100 && cb[1] >= 100) {
            const vec2 cp(cb[0] - 100, cb[1] - 100);
            const int dc = tile_distance(v.pos, cp);
            if (dc <= g_opt.escort_reach) {
                release();
                if (dc > 2) {
                    auto goals = tiles_around(cp, 2) & ~tiles_around(cp, 0) & ~b.static_block;
                    move_towards_closest(b.static_block, b.camp_static, goals);
                }
                return true;
            }
        }
    }
    if (!g_caster) return false;

    // Choose the Monolith: allies already on one (shared target bytes), then lower remembered health, then distance.
    std::array<std::uint8_t, 8> mem{};
    cambc::shared_memory.read(0, std::span{mem.data(), mem.size()});
    int best = -1; double best_score = 1e18;
    for (int k = 0; k < static_cast<int>(g_monos.size()); ++k) {
        const auto& m = g_monos[k];
        const auto& g = b.groups[m.group];
        const int d = tile_distance(v.pos, m.pos);
        if (g.ready_round > v.round + d) continue;   // respawning beyond our arrival
        int mates = 0;
        for (int s = 0; s < kSquadClaimByte; ++s) if (s != b.ally_index && (mem[s] & 0x3F) == m.group + 1) ++mates;
        double score = d + 20.0 * (m.hp / 20000.0) - 25.0 * mates + (k == g_cur ? -8.0 : 0.0);
        if (score < best_score) { best_score = score; best = k; }
    }
    g_cur = best;
    if (best < 0) return release();
    auto& m = g_monos[best];
    auto& g = b.groups[m.group];
    const int d = tile_distance(v.pos, m.pos);
    b.target = m.group;

    if (v.visible.is_set(m.pos) && m.dead) {
        // Dead in view: reward pending (capture), group still alive (other camps), or respawning.
        int st = -1, timer = 0;
        try {
            const auto s = g.group.state();
            if (std::holds_alternative<cambc::map::camp_group_reward_available>(s)) st = 1;
            else if (auto r = std::get_if<cambc::map::camp_group_respawning>(&s)) { st = 2; timer = r->timer; }
            else st = 0;
        } catch (...) {}
        if (st == 2) { g.ready_round = v.round + timer; g_cur = -1; return release(); }
        if (st == 1) {
            if (g.mask.is_set(v.pos)) return true;   // capturing: hold the tile
            // Capturer: the visible caster within 6 tiles with the highest max health (it carries 200 gems
            // through a 20-turn tribute); ties by distance, then roster index. Others guard within 2.
            bool other = false;
            const float my_hp = me.max_health();
            for (const auto& a : allies) {
                if (a == me || !a.is_visible()) continue;
                const int idx = b.roster_index(a);
                if (idx < 0 || !g_roster_caster[idx]) continue;
                const int ad = tile_distance(a.position(), m.pos);
                if (ad > 6 && d <= 6) continue;
                const float ah = a.max_health();
                const bool better = g_opt.tank_capture
                    ? (ah > my_hp || (ah == my_hp && (ad < d || (ad == d && idx < b.ally_index))))
                    : (ad < d || (ad == d && idx < b.ally_index));
                if (better) { other = true; break; }
            }
            if (!other) { move_towards_closest(b.static_block, b.camp_static, g.mask); return true; }
            auto guard = tiles_around(m.pos, 2) & ~g.mask & ~b.static_block;
            if (!guard.is_set(v.pos)) move_towards_closest(b.static_block, b.camp_static, guard);
            return true;
        }
        if (st == 0) {   // the Monolith is dead, other camps of the group live: clear them (they do not hurt)
            if (hit_group_camp(v, m.group)) return true;
            const LiveCamp* prey = nullptr;
            for (const auto& c : v.camps) if (c.group == m.group && (prey == nullptr || c.health < prey->health)) prey = &c;
            if (prey != nullptr) {
                auto reach = tiles_around(prey->camp.spawn_position(), std::max(1, static_cast<int>(me.attack_range())));
                reach.set(prey->camp.spawn_position(), false);
                if (!reach.is_set(v.pos)) move_towards_closest(b.static_block, b.camp_static, reach);
                hit_group_camp(v, m.group);
                return true;
            }
            return release();
        }
        return release();
    }

    // Alive (or not yet seen): cast from where we stand when it is in range and in view, else take a ring tile.
    if (d <= 4 && v.visible.is_set(m.pos) && !m.dead) {
        if (d >= 4 && cast_on_mono(m)) { hit_group_camp(v, m.group); return true; }
        if (d <= kMonsterRange) {   // inside its range: step out to the ring first
            auto goals = m.ring & ~cambc::vision::champion_tiles();
            if (goals.is_empty()) goals = tiles_around(m.pos, 5) & ~tiles_around(m.pos, 3) & ~b.static_block;
            move_towards_closest(b.static_block, b.camp_static, goals);
            return true;
        }
        hit_group_camp(v, m.group);
        return true;   // on the ring with cooldowns running: hold
    }
    auto goals = m.ring & ~cambc::vision::champion_tiles();
    if (goals.is_empty()) goals = tiles_around(m.pos, 6) & ~tiles_around(m.pos, 3) & ~b.static_block;
    move_towards_closest(b.static_block, b.camp_static, goals);
    return true;
}

// Default v2 build with Focus as the main stat for casters: Monster emblem first, then Power shards alternating
// Health and Focus.
bool buy(Bot& b) {
    (void)b;
    if (!g_opt.on || !g_opt.focus_build || !g_caster) return false;
    auto& me = cambc::self();
    using I = cambc::item_type;
    const std::uint32_t price = I::monster_shard_of_focus.cost();
    if (me.gold_count() < price) return true;
    const auto owned = me.item_count();
    auto have = [&](I shard, I emblem) { return static_cast<int>(owned[shard]) + 5 * static_cast<int>(owned[emblem]); };
    int mon = have(I::monster_shard_of_focus, I::monster_emblem_of_focus);
    int pm = have(I::power_shard_of_focus, I::power_emblem_of_focus);
    int ph = have(I::power_shard_of_health, I::power_emblem_of_health);
    std::uint32_t gold = me.gold_count();
    int buy_mon = 0, buy_pm = 0, buy_ph = 0;
    while (gold >= price) {
        if (mon < 5) { ++mon; ++buy_mon; }
        else if (ph < pm) { ++ph; ++buy_ph; }
        else { ++pm; ++buy_pm; }
        gold -= price;
    }
    auto go = [&](I item, int n) { if (n > 0 && me.can_buy(item, n)) me.buy(item, n); };
    go(I::monster_shard_of_focus, buy_mon);
    go(I::power_shard_of_focus, buy_pm);
    go(I::power_shard_of_health, buy_ph);
    return true;
}

}  // namespace Siege
} // namespace va
