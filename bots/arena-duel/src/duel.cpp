// w14 duel lab. The arena map (tools/micro/mkarena.mjs) carries the duel spec as holes inside a wall block:
//   row 50 + 2*side: five 4-bit draft picks at x = 2.. (15 = none)
//   row 51 + 2*side: 16 policy bits at x = 2.., a 4-bit shard level at x = 18..
// side 0 = team A (spawn x 0..3, y 22..25), side 1 = team B (spawn x 46..49); the map is an x-mirror.
// Corridor c (0..4) = rows 6+8c .. 11+8c, x 6..43 (connectors x 4..5 / 44..45), walled from the others (2-tile walls block sight and reach).
// Champion k of a team (its k-th pick; Cassi and Issac share a pick) fights in corridor k. Each duel: both wait at full
// health on their start tiles (x 21 / x 28, gap 7) until the other is on its start tile, then fight with the team's
// policy until a death, an escape past the escape line, or a timeout; the survivor heals on the spawn; repeat.
// One line per duel per champion on stdout: "D c s r0 r1 me foe res hp mhp fhp".
#include <cambc.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

using cambc::vec2;
using cambc::team;
using D = cambc::champion_draft;
using CT = cambc::champion_type;

namespace {
constexpr int kW = 50;
constexpr int kStartA = 21, kStartB = 28;
constexpr int kEscapeA = 9, kEscapeB = 40;   // a fleeing champion past this x (towards its own spawn) has escaped
constexpr int kTimeout = 150;

enum : int {
    P_NORETREAT = 1 << 0,   // fight to the death (v2 walks home below 45% health with an enemy in view, bot.cpp:377-413)
    P_EARLYCAST = 1 << 1,   // cast damage/stun spells at spell range before moving (v2 casts only in attack range, enemies.cpp:111-114)
    P_KITE = 1 << 2,        // v2's kite_step (enemies.cpp:34-54): ranged beside a melee foe steps to distance 2-3 first
    P_DASHIN = 1 << 3,      // melee with a dash: foe 2-3 tiles away -> dash next to it, then attack (+reset)
    P_DASHOUT = 1 << 4,     // ranged with a dash (Vesper): melee foe adjacent -> dash 2 away, then attack
    P_SHIELDENG = 1 << 5,   // Varg: shield when the foe is within 4 (v2: only below 60% or near a camp, util.cpp:95-104)
    P_WEAVE = 1 << 30,      // (w14d bit 6; retired in w14t: no gain in World A)
    P_RACEK2 = 1 << 6,      // race retreat with margin 2: fight on unless the foe outlasts me by more than 2x
    P_STUNSTEP = 1 << 29,   // (w14d bit 7; retired in w14t)
    P_CIUNIT = 1 << 7,      // lone-flee rule counts Cassi + Issac as one foe (v2 counts two and flees from the pair)
    P_HOLDRESET = 1 << 8,   // control: never use the attack reset
    P_NOSPELL = 1 << 9,     // control: never cast damage/stun on champions
    P_RETREAT30 = 1 << 10,  // retreat threshold 30% instead of 45%
    P_FOCUSEHP = 1 << 11,   // focus the lowest effective health (C&I: min of the pair; shields add)
    P_HEAL = 1 << 13,       // Willow: heal the most hurt corridor ally within 4 below 70% (v2 never heals)
    P_RACE = 1 << 14,       // retreat only when we lose the damage race (my HP / foe dps < foe HP / my dps); else fight on
    P_ADAPT = 1 << 15,      // expand to the per-champion table below (w14 adaptation design)
    P_CDWAIT = 1 << 12,     // hold at distance 5+ while our damage/stun spell is on cooldown and the foe's is ready (needs enemy cooldown reads)
};

int g_side = -1;

int tile_distance(vec2 a, vec2 b) { return static_cast<int>(a.chebyshev_distance(b)); }
cambc::bitboard around(vec2 p, int r) { cambc::bitboard b; b.clear(); b.draw_chebyshev(p, r); return b; }

int bits(int y, int x0, int n) {
    const auto h = cambc::map::holes();
    int v = 0;
    for (int i = 0; i < n; ++i) if (h.is_set(vec2(x0 + i, y))) v |= 1 << i;
    return v;
}
int my_side() {
    if (g_side >= 0) return g_side;
    long sx = 0, n = 0;
    for (auto t : cambc::map::spawn_region(team::ally).positions()) { sx += t.x; ++n; }
    g_side = (n && sx / n < kW / 2) ? 0 : 1;
    return g_side;
}
int pick_of(int s, int k) { return bits(50 + 2 * s, 2 + 4 * k, 4); }
int policy_of(int s) { return bits(51 + 2 * s, 2, 16); }
int level_of(int s) { return bits(51 + 2 * s, 18, 4); }
// arena-duel: carried-gem target {0, 20, 40, 60} for Prism (2 bits at x 22..23 of row 51 + 2*side); gem clusters sit at the corridor end.
int gem_target_of(int s) { return 20 * bits(51 + 2 * s, 22, 2); }
// Team-fight mode: row 54 x 17 set -> side s's pick slot k fights in corridor bits(54 + s, 2+3k, 3).
bool group_mode() { return bits(54, 17, 1) != 0; }
int group_of(int s, int k) { return bits(54 + s, 2 + 3 * k, 3); }
int draft_of(CT t) { const int i = static_cast<int>(t); return i <= 3 ? i : i - 1; }
const char* name_of(CT t) {
    static const char* n[] = {"Ryth", "Vesper", "Vendetta", "Cassi", "Issac", "Stellar", "Prism", "Marcella", "Willow", "Ren", "Varg", "Tavi", "HoR"};
    const int i = static_cast<int>(t);
    return (i >= 0 && i < 13) ? n[i] : "?";
}

enum class St { home, go, ready, fight };

struct State {
    bool init = false;
    int corr = -1, lo = 0, policy = 0, level = 0, gem_target = 0, gems0 = 0, foc0 = 0;
    St st = St::home;
    vec2 start{0, 0};
    int r0 = 0;
    bool retreating = false;
    bool bought = false;
    bool dead_logged = true;
    int duels = 0;          // duels finished: seeds the start jitter
    int delay = 0;          // rounds to wait after the foe is ready (turn-order parity jitter)
    bool second_body = false;
    int body = 0, mates = 0, foe_bodies = 1;
    std::vector<cambc::map::camp_group> farm_groups, gem_groups;   // my index among the corridor's bodies of my team; allies expected in the corridor
    std::vector<cambc::champion> foes;   // corridor foes seen at fight start (handles stay valid)
    std::string foe_names;
    float foe_hp_last = 0;
    cambc::bitboard block, spawn;
    bool ranged = false;
};
State S;

bool in_corridor(vec2 p) { return p.y >= S.lo && p.y <= S.lo + 5; }
cambc::bitboard corridor_mask() { cambc::bitboard m; m.clear(); for (int y = S.lo; y <= S.lo + 5; ++y) for (int x = 6; x < kW - 6; ++x) m.set(vec2(x, y), true); return m; }

bool move_towards_closest(cambc::bitboard goals) {
    auto& me = cambc::self();
    const vec2 pos = me.position();
    if (!me.can_move() || goals.is_set(pos)) return false;
    auto blocked = S.block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles();   // arena-duel: camps block
    blocked.set(pos, false);
    goals &= ~blocked;
    if (goals.is_empty()) return false;
    cambc::distance_field field; cambc::bitboard visited;
    field.compute_to(blocked & ~goals, goals, around(pos, 0), visited);
    auto step = pos;
    for (auto t : blocked.reachable_tiles(step, me.base_stats().base_move_speed).positions())
        if (field.get(t) < field.get(step)) step = t;
    return step != pos && me.move_absolute(step);
}

void log_duel(const char* res) {
    auto& me = cambc::self();
    const bool alive = me.is_alive();
    std::cout << "D " << S.corr << ' ' << my_side() << ' ' << S.r0 << ' ' << cambc::round_number() << ' '
              << name_of(me.champion_name()) << ' ' << S.foe_names << ' ' << res << ' '
              << (alive ? static_cast<int>(me.health()) : 0) << ' ' << (alive ? static_cast<int>(me.max_health()) : 0) << ' '
              << static_cast<int>(S.foe_hp_last) << ' ' << S.policy << " g" << S.gems0 << " f" << S.foc0 << std::endl;
}

void init_state() {
    auto& me = cambc::self();
    S.init = true;
    const int s = my_side();
    const int mine = draft_of(me.champion_name());
    int myk = 0;
    for (int k = 0; k < 5; ++k) if (pick_of(s, k) == mine) myk = k;
    S.corr = group_mode() ? group_of(s, myk) : myk;
    {   // bodies of my team in this corridor, in pick order (C&I = Cassi then Issac)
        int n = 0;
        for (int k = 0; k < 5; ++k) {
            const int p = pick_of(s, k);
            if (p > 11 || (group_mode() ? group_of(s, k) : k) != S.corr) continue;
            if (k == myk) S.body = n + (me.champion_name() == CT::issac ? 1 : 0);
            n += (p == 3) ? 2 : 1;
        }
        S.mates = n - 1;
        int m = 0;   // foe bodies expected in this corridor
        for (int k = 0; k < 5; ++k) {
            const int p = pick_of(1 - s, k);
            if (p > 11 || (group_mode() ? group_of(1 - s, k) : k) != S.corr) continue;
            m += (p == 3) ? 2 : 1;
        }
        S.foe_bodies = std::max(1, m);
    }
    S.lo = 6 + 8 * S.corr;
    S.policy = policy_of(s);
    if (S.policy & P_ADAPT) {   // per-champion table from the World A duel gains (results/w14-duels.md)
        const auto t = me.champion_name();
        int add = P_RACEK2 | P_FOCUSEHP;   // CIUNIT dropped: -0.03 in T1_ci
        if (t != CT::marcella) add |= P_EARLYCAST;
        if (t == CT::ren || t == CT::vendetta || t == CT::issac) add |= P_DASHIN;
        if (t == CT::vesper_vale) add |= P_DASHOUT | P_KITE;
        if (t == CT::prism) add |= P_KITE;
        if (t == CT::varg) add |= P_SHIELDENG;
        if (t == CT::heart_of_ruin) add |= P_SHIELDENG | P_DASHIN;   // arena-duel: shield on engage, dash-in with the landing stun
        if (t == CT::willow) add |= P_HEAL;
        S.policy |= add;
    }
    S.level = level_of(s);
    S.gem_target = me.champion_name() == CT::prism ? gem_target_of(s) : 0;
    const bool second_body = me.champion_name() == CT::issac;
    S.second_body = second_body;
    S.start = vec2(s == 0 ? kStartA : kStartB, S.lo + (second_body ? 3 : 2));
    S.block = cambc::map::walls() | cambc::map::holes() | cambc::map::altars() | cambc::map::spawn_region(team::enemy);
    S.spawn = cambc::map::spawn_region(team::ally);
    S.ranged = me.attack_range() > 1;
    // One-time probe lines for the lab: ability target masks.
    std::string line = "I " + std::string(name_of(me.champion_name())) + " gold" + std::to_string(me.gold_count());
    for (const auto& ab : me.abilities()) {
        line += " [" + std::to_string(static_cast<int>(ab.slot()));
        if (ab.is_damage()) line += " dmg"; if (ab.is_aoe_damage()) line += " aoe"; if (ab.is_cc()) line += " cc";
        if (ab.is_attack_reset()) line += " reset"; if (ab.is_dash()) line += " dash"; if (ab.is_shield()) line += " shield";
        if (ab.is_heal()) line += " heal";
        try {
            line += std::string(ab.can_target_enemies() ? " E" : "") + (ab.can_target_camps() ? " C" : "") +
                    (ab.can_target_ground() ? " G" : "") + (ab.can_target_self() ? " S" : "") + (ab.can_target_allies() ? " A" : "");
        } catch (...) { line += " ?"; }
        line += "]";
    }
    std::cout << line << std::endl;
}

// Fixed build: `level` Power shards of the main stat and `level` of Health (alternating), bought once on the spawn.
// Item level (4-bit nibble per side): bits 0-1 = shards {0, 10, 20, 30}; bits 2-3 = build: 0 DMG (Power shards of the
// main stat: Focus for Cassi/Stellar/Prism, else Strength, as v2a), 1 V2 (v2a's order: Monster main x5, then Health and
// Power main alternating, Health first; bot.cpp:162-198), 2 HP (Power Health), 3 HALF (half main, half Health).
int shard_target() { static const int n[4] = {0, 10, 20, 30}; return n[S.level & 3]; }
void buy_build() {
    const int n = shard_target();
    if (S.bought || n == 0) { S.bought = true; return; }
    auto& me = cambc::self();
    using I = cambc::item_type;
    const auto t = me.champion_name();
    const bool focus = t == CT::cassi || t == CT::stellar || t == CT::prism;
    const I pow_main = focus ? I::power_shard_of_focus : I::power_shard_of_strength;
    const I mon_main = focus ? I::monster_shard_of_focus : I::monster_shard_of_strength;
    if (me.gold_count() < static_cast<std::uint32_t>(n * 30)) return;
    int a_main = 0, a_mon = 0, a_hp = 0;
    switch ((S.level >> 2) & 3) {
    case 0: a_main = n; break;
    case 1: a_mon = std::min(5, n); a_hp = (n - a_mon + 1) / 2; a_main = n - a_mon - a_hp; break;
    case 2: a_hp = n; break;
    default: a_main = n / 2; a_hp = n - a_main; break;
    }
    try {
        // Buy in chunks of 5 so each emblem combines (a count above 5 at once is accepted too; chunks are safer).
        auto go = [&](I it, int k) { while (k > 0) { const int c = std::min(5, k); if (!me.can_buy(it, c)) break; me.buy(it, c); k -= c; } };
        go(mon_main, a_mon);
        go(pow_main, a_main);
        go(I::power_shard_of_health, a_hp);
        S.bought = true;
        std::cout << "B " << name_of(t) << " lvl" << S.level << " str" << static_cast<int>(me.strength()) << " foc" << static_cast<int>(me.focus())
                  << " mhp" << static_cast<int>(me.max_health()) << " gold" << me.gold_count() << std::endl;
    } catch (const std::exception& e) { std::cout << "B-err " << e.what() << std::endl; S.bought = true; }
}

// Farm phase (item levels): kill and capture the two gold-cluster groups at this corridor's own end until the build is
// affordable. Returns true while farming (the turn is used).
bool farm_gold() {
    auto& me = cambc::self();
    const int n = shard_target();
    if (S.bought || n == 0) return false;
    if (me.gold_count() >= static_cast<std::uint32_t>(n * 30)) return false;   // go home and buy
    const vec2 pos = me.position();
    if (S.farm_groups.empty()) {
        for (auto g : cambc::map::camp_groups()) {
            bool mine = false;
            for (auto t : g.spawn_tiles().positions()) if (in_corridor(t) && (my_side() == 0 ? t.x < kW / 2 : t.x >= kW / 2)) mine = true;
            if (mine) S.farm_groups.push_back(g);
        }
        if (S.farm_groups.empty()) return false;
    }
    int best = -1, best_d = 1 << 20, best_kind = 9;   // kind 0 alive, 1 reward available, 2 respawning/unknown
    for (std::size_t i = 0; i < S.farm_groups.size(); ++i) {
        auto& g = S.farm_groups[i];
        int kind = 2;
        try {
            const auto st = g.state();
            if (std::holds_alternative<cambc::map::camp_group_alive>(st)) kind = 0;
            else if (std::holds_alternative<cambc::map::camp_group_reward_available>(st)) kind = 1;
        } catch (...) { kind = 0; }   // out of sight: assume alive, walk there
        int d = 1 << 20;
        for (auto t : g.spawn_tiles().positions()) d = std::min(d, tile_distance(pos, t));
        const int rank = kind == 1 ? 0 : kind == 0 ? 1 : 2;   // capture first, then kill, else wait
        if (rank < best_kind || (rank == best_kind && d < best_d)) { best_kind = rank; best_d = d; best = static_cast<int>(i); }
    }
    if (best < 0) return false;
    auto& g = S.farm_groups[best];
    const auto tiles = g.spawn_tiles();
    if (best_kind == 0) {   // reward available: stand on a spawn tile
        if (!tiles.is_set(pos)) move_towards_closest(tiles);
        return true;
    }
    if (best_kind == 1) {   // alive: attack the nearest live cluster
        const cambc::camp* prey = nullptr; int pd = 99;
        for (const auto& c : g.camps()) {
            try { if (!c.is_visible() || c.health() <= 0) continue; } catch (...) { continue; }
            const int d = tile_distance(pos, c.spawn_position());
            if (d < pd) { pd = d; prey = &c; }
        }
        if (!prey) { cambc::bitboard r = around(tiles.positions()[0], 2) & ~tiles; move_towards_closest(r); return true; }
        if (me.attack_tiles().is_set(prey->position())) {
            if (me.can_cast() && !(S.policy & P_NOSPELL))
                for (const auto& ab : me.abilities())
                    if (ab.is_ready() && (ab.is_damage() || ab.is_aoe_damage()) && ab.can_target_camps()) { try { if (ab.valid_tiles().is_set(prey->position()) && ab.use(*prey)) break; } catch (...) {} }
            try { if (prey->is_visible() && me.can_attack() && me.attack_tiles().is_set(prey->position())) { me.attack(*prey);
                    for (const auto& ab : me.abilities()) if (ab.is_attack_reset() && ab.is_ready() && me.can_cast()) { try { if (ab.use() && prey->is_visible() && prey->health() > 0 && me.can_attack()) me.attack(*prey); } catch (...) {} break; } } } catch (...) {}
            return true;
        }
        const int range = std::max(1, static_cast<int>(me.attack_range()));
        cambc::bitboard ring = around(prey->spawn_position(), range) & ~tiles;
        move_towards_closest(ring);
        return true;
    }
    // everything respawning: wait beside the nearest group
    cambc::bitboard r = around(tiles.positions()[0], 1) & ~tiles;
    if (!r.is_set(pos)) move_towards_closest(r);
    return true;
}
// arena-duel: tribute carried gems above the allowance (gem target + 6 for Prism, else 0) at the altar beside the spawn.
bool tribute_excess() {
    auto& me = cambc::self();
    if (me.is_submitting_gems()) return true;
    const std::uint32_t keep = S.gem_target ? static_cast<std::uint32_t>(S.gem_target + 6) : 0u;
    if (me.gem_count() <= keep) return false;
    for (auto altar : cambc::map::altars().positions()) {
        if ((my_side() == 0) != (altar.x < kW / 2)) continue;
        if (tile_distance(me.position(), altar) <= cambc::constants::submit_gems_range) { try { me.submit_gems(altar); } catch (...) {} return true; }
        move_towards_closest(around(altar, cambc::constants::submit_gems_range) & ~cambc::map::altars());
        return true;
    }
    return false;
}

bool farm_gems() {
    auto& me = cambc::self();
    if (S.gem_target == 0 || me.gem_count() >= static_cast<std::uint32_t>(S.gem_target)) return false;
    const vec2 pos = me.position();
    if (S.gem_groups.empty()) {
        for (auto g : cambc::map::camp_groups()) {
            bool mine = false;
            for (auto t : g.spawn_tiles().positions()) if (in_corridor(t) && (my_side() == 0 ? t.x < kW / 2 : t.x >= kW / 2)) mine = true;
            if (mine && g.camps()[0].is_gem_cluster()) S.gem_groups.push_back(g);
        }
        if (S.gem_groups.empty()) return false;
    }
    int best = -1, best_d = 1 << 20, best_kind = 9;   // kind 0 alive, 1 reward available, 2 respawning/unknown
    for (std::size_t i = 0; i < S.gem_groups.size(); ++i) {
        auto& g = S.gem_groups[i];
        int kind = 2;
        try {
            const auto st = g.state();
            if (std::holds_alternative<cambc::map::camp_group_alive>(st)) kind = 0;
            else if (std::holds_alternative<cambc::map::camp_group_reward_available>(st)) kind = 1;
        } catch (...) { kind = 0; }   // out of sight: assume alive, walk there
        int d = 1 << 20;
        for (auto t : g.spawn_tiles().positions()) d = std::min(d, tile_distance(pos, t));
        const int rank = kind == 1 ? 0 : kind == 0 ? 1 : 2;   // capture first, then kill, else wait
        if (rank < best_kind || (rank == best_kind && d < best_d)) { best_kind = rank; best_d = d; best = static_cast<int>(i); }
    }
    if (best < 0) return false;
    auto& g = S.gem_groups[best];
    const auto tiles = g.spawn_tiles();
    if (best_kind == 0) {   // reward available: stand on a spawn tile
        if (!tiles.is_set(pos)) move_towards_closest(tiles);
        return true;
    }
    if (best_kind == 1) {   // alive: attack the nearest live cluster
        const cambc::camp* prey = nullptr; int pd = 99;
        for (const auto& c : g.camps()) {
            try { if (!c.is_visible() || c.health() <= 0) continue; } catch (...) { continue; }
            const int d = tile_distance(pos, c.spawn_position());
            if (d < pd) { pd = d; prey = &c; }
        }
        if (!prey) { cambc::bitboard r = around(tiles.positions()[0], 2) & ~tiles & corridor_mask(); move_towards_closest(r); return true; }
        if (in_corridor(pos) && me.attack_tiles().is_set(prey->position())) {   // arena-duel: never from the connector (walls)
            if (me.can_cast() && !(S.policy & P_NOSPELL))
                for (const auto& ab : me.abilities())
                    if (ab.is_ready() && (ab.is_damage() || ab.is_aoe_damage()) && ab.can_target_camps()) { try { if (ab.valid_tiles().is_set(prey->position()) && ab.use(*prey)) break; } catch (...) {} }
            try { if (prey->is_visible() && me.can_attack() && me.attack_tiles().is_set(prey->position())) { me.attack(*prey);
                    for (const auto& ab : me.abilities()) if (ab.is_attack_reset() && ab.is_ready() && me.can_cast()) { try { if (ab.use() && prey->is_visible() && prey->health() > 0 && me.can_attack()) me.attack(*prey); } catch (...) {} break; } } } catch (...) {}
            return true;
        }
        const int range = std::max(1, static_cast<int>(me.attack_range()));
        cambc::bitboard ring = around(prey->spawn_position(), range) & ~tiles;
        ring &= corridor_mask();
        move_towards_closest(ring);
        return true;
    }
    // everything respawning: wait beside the nearest group
    cambc::bitboard r = around(tiles.positions()[0], 1) & ~tiles & corridor_mask();
    if (!r.is_set(pos)) move_towards_closest(r);
    return true;
}

bool is_melee(const cambc::champion& c) { return c.attack_range() <= 1; }

// arena-duel: one line per damaging cast: "S round caster slot kind damage gems focus" (damage = foe health drop at once).
void log_cast(const cambc::ability& ab, const cambc::champion& foe, float h0) {
    auto& me = cambc::self();
    float h1 = 0;
    try { if (foe.is_visible()) h1 = foe.health(); } catch (...) {}
    std::cout << "S " << cambc::round_number() << ' ' << name_of(me.champion_name()) << ' ' << static_cast<int>(ab.slot()) << ' '
              << (ab.is_cc() ? "cc" : ab.is_aoe_damage() ? "aoe" : "dmg") << ' ' << static_cast<int>(h0 - h1) << ' ' << me.gem_count()
              << ' ' << static_cast<int>(me.focus()) << '\n';
}
// v2 cast_on_enemy (enemies.cpp:57-67): first ready damage/aoe/cc ability that can target enemies and has the foe in range.
bool cast_on_enemy(const cambc::champion& foe) {
    auto& me = cambc::self();
    if ((S.policy & P_NOSPELL) || !me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_enemies()) continue;
        try {
            const float h0 = foe.health();
            if (ab.valid_tiles().is_set(foe.position()) && ab.use(foe)) { log_cast(ab, foe, h0); return true; }
        } catch (...) {}
    }
    return false;
}
// Ground-targeted area spells (Ryth stun, Stellar/Prism area) cast on the foe's tile (EARLYCAST only).
bool cast_ground_on(const cambc::champion& foe) {
    auto& me = cambc::self();
    if ((S.policy & P_NOSPELL) || !me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || ab.can_target_enemies() || !ab.can_target_ground()) continue;
        try {
            const float h0 = foe.health();
            if (ab.valid_tiles().is_set(foe.position()) && ab.use(foe.position())) { log_cast(ab, foe, h0); return true; }
        } catch (...) {}
    }
    return false;
}

bool attack_if_in_range(const cambc::champion& t) {
    auto& me = cambc::self();
    if (!me.can_attack() || !t.is_visible() || t.health() <= 0 || !me.attack_tiles().is_set(t.position())) return false;
    me.attack(t);
    return true;
}
void reset_and_attack_again(const cambc::champion& t) {
    auto& me = cambc::self();
    if ((S.policy & P_HOLDRESET) || !me.can_cast()) return;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_attack_reset() || !ab.is_ready()) continue;
        try {
            if (ab.use() && t.is_visible() && t.health() > 0 && me.can_attack() && me.attack_tiles().is_set(t.position())) me.attack(t);
        } catch (...) {}
        return;
    }
}

// Step to a free tile at distance [lo, hi] from foe, inside the corridor and arena; returns true when moved.
bool step_away(const cambc::champion& foe, int lo_d, int hi_d) {
    auto& me = cambc::self();
    if (!me.can_move()) return false;
    cambc::bitboard goals = around(me.position(), 1) & around(foe.position(), hi_d) & ~around(foe.position(), lo_d - 1);
    goals &= ~S.block & ~cambc::vision::champion_tiles();
    goals.set(me.position(), false);
    if (goals.is_empty()) return false;
    return move_towards_closest(goals);
}

void fight_turn(std::vector<cambc::champion>& foes) {
    auto& me = cambc::self();
    const vec2 pos = me.position();
    // Focus: v2 = lowest health (enemies.cpp:21-30); P_FOCUSEHP = lowest effective health (C&I pair = min, shield adds).
    const cambc::champion* foe = nullptr;
    std::vector<cambc::champion> allies;
    for (auto& a : cambc::vision::champions(team::ally)) { try { if (!(a == me) && a.is_visible() && in_corridor(a.position())) allies.push_back(a); } catch (...) {} }
    const auto atk = me.attack_tiles();
    float best = 1e9f;
    for (const auto& e : foes) {
        const vec2 ep = e.position();
        // v2 threat filter (enemies.cpp:100-107): within 2, in attack range, or within 3 of an ally within 4.
        bool threat = tile_distance(pos, ep) <= 2 || atk.is_set(ep);
        for (const auto& a : allies) if (!threat && tile_distance(pos, a.position()) <= 4 && tile_distance(a.position(), ep) <= 3) threat = true;
        float h = e.health();
        if (S.policy & P_FOCUSEHP) {
            // In-range foes first (no step lost), then effective health: shield adds; C&I: the lower of the pair.
            try { auto b = e.buffs(); if (b.has_shield()) h += b.get<cambc::buff_shield>().value; } catch (...) {}
            const auto t = e.champion_name();
            if (t == CT::cassi || t == CT::issac)
                for (const auto& o : foes) { const auto ot = o.champion_name(); if (!(o == e) && (ot == CT::cassi || ot == CT::issac)) h = std::min(h, o.health()); }
            if (!atk.is_set(ep)) h += 100000.0f;
            threat = true;
        }
        if (!threat) continue;
        if (h < best) { best = h; foe = &e; }
    }
    if (!foe) {   // nobody threatening: approach the nearest (the lab closes every fight)
        int bd = 99;
        for (const auto& e : foes) { const int dd = tile_distance(pos, e.position()); if (dd < bd) { bd = dd; foe = &e; } }
    }
    if (!foe) return;
    static bool cd_probe = false;   // lab probe: can a visible enemy's ability cooldowns be read? (catalogue S6)
    if (!cd_probe) {
        cd_probe = true;
        std::string l = std::string("C ") + name_of(foe->champion_name());
        try { for (const auto& ab : foe->abilities()) l += " cd" + std::to_string(ab.cooldown()); } catch (const std::exception& ex) { l += std::string(" err ") + ex.what(); }
        try { l += " str" + std::to_string(static_cast<int>(foe->strength())) + " foc" + std::to_string(static_cast<int>(foe->focus())); } catch (...) { l += " stats-err"; }
        std::cout << l << std::endl;
    }
    // P_HEAL (Willow): the most hurt corridor ally within 4 below 70%.
    if ((S.policy & P_HEAL) && me.can_cast()) {
        for (const auto& ab : me.abilities()) {
            if (!ab.is_heal() || !ab.is_ready()) continue;
            const cambc::champion* tgt = nullptr; float frac = 0.7f;
            for (const auto& a : allies) { const float f = a.health() / a.max_health(); if (f < frac && tile_distance(pos, a.position()) <= 4) { frac = f; tgt = &a; } }
            if (tgt) { try { ab.use(*tgt); } catch (...) {} }
            break;
        }
    }
    // v2 lone flee (enemies.cpp:78-88): no ally within 4 and 2+ foes within 4 -> walk to the nearest ally or the spawn.
    if (!(S.policy & P_NORETREAT)) {
        int friends4 = 0, foes4 = 0;
        for (const auto& a : allies) if (tile_distance(pos, a.position()) <= 4) ++friends4;
        bool ci_counted = false;
        for (const auto& e : foes) {
            if (tile_distance(pos, e.position()) > 4) continue;
            const auto et = e.champion_name();
            if ((S.policy & P_CIUNIT) && (et == CT::cassi || et == CT::issac)) { if (ci_counted) continue; ci_counted = true; }
            ++foes4;
        }
        if (friends4 == 0 && foes4 >= 2) {
            cambc::bitboard goals = S.spawn; int bd = 1 << 20;
            for (const auto& a : allies) { const int dd = tile_distance(pos, a.position()); if (dd < bd) { bd = dd; goals = around(a.position(), 1); } }
            move_towards_closest(goals);
            return;
        }
    }
    S.foe_hp_last = foe->health();
    const int d = tile_distance(pos, foe->position());
    const int range = std::max(1, static_cast<int>(me.attack_range()));
    auto abs = me.abilities();

    // Varg shield: v2 below 60% (util.cpp:98); P_SHIELDENG as soon as the foe is within 4.
    if ((me.champion_name() == CT::varg || me.champion_name() == CT::heart_of_ruin) && me.can_cast()) {
        const bool want = me.health() < me.max_health() * 0.6f || ((S.policy & P_SHIELDENG) && d <= 4);
        if (want) for (const auto& ab : abs) if (ab.is_shield() && ab.is_ready()) { try { ab.use(); } catch (...) {} break; }
    }

    // v2 retreat: below 45% with an enemy in view, walk home and do nothing else (bot.cpp:377,395,411-412).
    const float thr = (S.policy & P_RETREAT30) ? 0.30f : 0.45f;
    if (!(S.policy & P_NORETREAT) && me.health() < me.max_health() * thr) {
        bool fight_on = false;
        if (S.policy & (P_RACE | P_RACEK2)) {
            // Damage race against the visible corridor foes: their summed strength vs my strength on the focus.
            float foe_dps = 0, foe_hp = foe->health();
            for (const auto& e : foes) if (tile_distance(pos, e.position()) <= 4) foe_dps += e.strength();
            const auto ft = foe->champion_name();
            if (ft == CT::cassi || ft == CT::issac)
                for (const auto& o : foes) { const auto ot = o.champion_name(); if (ot == CT::cassi || ot == CT::issac) foe_hp = std::min(foe_hp, o.health()); }
            const float my_dps = std::max(1.0f, me.strength());
            const float k = (S.policy & P_RACEK2) ? 2.0f : 1.0f;
            fight_on = foe_dps <= 0 || k * me.health() / foe_dps >= foe_hp / my_dps;
        }
        if (!fight_on) S.retreating = true;
    }
    if (S.retreating) { move_towards_closest(S.spawn); return; }

    // P_CDWAIT: our burst is down and theirs is up -> hold outside 4 (one step back) until ours is ready.
    if ((S.policy & P_CDWAIT) && d <= 6) {
        bool ours_ready = false, theirs_ready = false;
        for (const auto& ab : abs) if ((ab.is_damage() || ab.is_cc()) && ab.is_ready()) ours_ready = true;
        try { for (const auto& ab : foe->abilities()) if ((ab.is_damage() || ab.is_cc() || ab.is_aoe_damage()) && ab.cooldown() == 0) theirs_ready = true; } catch (...) {}
        bool ours_exist = false;
        for (const auto& ab : abs) if (ab.is_damage() || ab.is_cc()) ours_exist = true;
        if (ours_exist && !ours_ready && theirs_ready && d >= 2) { if (d <= 5) step_away(*foe, d + 1, d + 1); return; }
    }

    // P_DASHOUT (Vesper): melee foe adjacent -> dash to distance >= 3, then attack.
    if ((S.policy & P_DASHOUT) && S.ranged && is_melee(*foe) && d <= 1 && me.can_cast()) {
        for (const auto& ab : abs) {
            if (!ab.is_dash() || !ab.is_ready()) continue;
            cambc::bitboard goals = ab.valid_tiles() & ~around(foe->position(), 2) & ~S.block & ~cambc::vision::champion_tiles();
            goals &= around(foe->position(), 3);
            vec2 bestt = pos; int bd = -1;
            for (auto t : goals.positions()) if (in_corridor(t)) { const int dd = tile_distance(t, foe->position()); if (dd > bd) { bd = dd; bestt = t; } }
            if (bd >= 3) { try { if (ab.use(bestt)) { if (attack_if_in_range(*foe)) reset_and_attack_again(*foe); return; } } catch (...) {} }
            break;
        }
    }
    // P_WEAVE: ranged with a ready reset, melee foe adjacent: step to 2..3, reset, attack.
    if ((S.policy & P_WEAVE) && S.ranged && is_melee(*foe) && d <= 1 && me.can_cast()) {
        for (const auto& ab : abs) {
            if (!ab.is_attack_reset() || !ab.is_ready()) continue;
            if (step_away(*foe, 2, 3)) {
                try { if (ab.use() && attack_if_in_range(*foe)) {} } catch (...) {}
                return;
            }
            break;
        }
    }
    // P_STUNSTEP: adjacent melee foe is stunned -> step away.
    if ((S.policy & P_STUNSTEP) && S.ranged && is_melee(*foe) && d <= 1) {
        bool stunned = false;
        try { stunned = foe->buffs().has_stun(); } catch (...) {}
        if (stunned && step_away(*foe, 2, 3)) return;
    }
    // P_KITE: v2 kite_step.
    if ((S.policy & P_KITE) && S.ranged && is_melee(*foe) && d <= 1) step_away(*foe, 2, 3);

    // P_EARLYCAST: spells at spell range before any move.
    if (S.policy & P_EARLYCAST) {
        if (!cast_on_enemy(*foe)) cast_ground_on(*foe);
    }
    // P_DASHIN: melee with a dash, foe 2..3 away: dash adjacent.
    if ((S.policy & P_DASHIN) && !S.ranged && d >= 2 && d <= 3 && me.can_cast()) {
        for (const auto& ab : abs) {
            if (!ab.is_dash() || !ab.is_ready()) continue;
            cambc::bitboard goals = ab.valid_tiles() & around(foe->position(), 1) & ~S.block & ~cambc::vision::champion_tiles();
            for (auto t : goals.positions()) { try { if (ab.use(t)) break; } catch (...) {} }
            break;
        }
    }

    if (!foe->is_visible()) return;   // a spell killed it: the handle now throws 97
    const int d2 = tile_distance(me.position(), foe->position());
    const bool in_range = me.attack_tiles().is_set(foe->position());
    if (in_range) {
        cast_on_enemy(*foe);   // v2: cast when in attack range
        if (!foe->is_visible()) return;
        if (attack_if_in_range(*foe)) { reset_and_attack_again(*foe); return; }
        return;
    }
    // Approach / chase (v2 chases within 2 + range; the lab always closes so that every duel is fought).
    (void)d2;
    auto goals = around(foe->position(), range);
    goals.set(foe->position(), false);
    goals &= around(foe->position(), range) & ~around(foe->position(), 0);
    if (move_towards_closest(goals)) {
        if (attack_if_in_range(*foe)) reset_and_attack_again(*foe);
    }
}

std::vector<cambc::champion> corridor_foes() {
    std::vector<cambc::champion> out;
    for (auto& e : cambc::vision::champions(team::enemy)) {
        try { if (e.is_visible() && e.health() > 0 && in_corridor(e.position())) out.push_back(e); } catch (...) {}
    }
    return out;
}

void end_fight(const char* res) {
    log_duel(res);
    ++S.duels;
    S.st = St::home;
    S.retreating = false;
    S.foes.clear();
}
}  // namespace

void duel_draft() {
    const int s = my_side();
    const auto mine = cambc::draft::current_draft(team::ally);
    for (int k = 0; k < 5; ++k) {
        const int p = pick_of(s, k);
        if (p > 11) continue;
        const D d = static_cast<D>(p);
        if (std::find(mine.begin(), mine.end(), d) == mine.end()) { cambc::draft::pick(d); return; }
    }
    for (int i = 0; i < 11; ++i) {
        const D d = static_cast<D>(i);
        if (std::find(mine.begin(), mine.end(), d) == mine.end()) { cambc::draft::pick(d); return; }
    }
}

void duel_turn() {
    auto& me = cambc::self();
    if (!S.init) init_state();
    const int round = static_cast<int>(cambc::round_number());
    if (!me.is_alive()) {
        if (S.st == St::fight) end_fight("L");
        S.st = St::home;
        buy_build();
        return;
    }
    if (me.is_recalling()) return;
    const vec2 pos = me.position();
    const bool at_spawn = S.spawn.is_set(pos);
    if (at_spawn) buy_build();
    auto foes = corridor_foes();

    if (S.st == St::fight) {
        // Foe dead?
        bool any_alive = false;
        for (auto& f : S.foes) { try { if (f.is_alive()) any_alive = true; } catch (...) { any_alive = true; } }
        if (!any_alive) { end_fight("W"); return; }
        const bool escaped_me = (my_side() == 0) ? pos.x <= kEscapeA : pos.x >= kEscapeB;
        if (escaped_me) { end_fight("E"); }
        else if (foes.empty()) { end_fight("X"); }   // foe out of sight (fled past the line or hidden)
        else {
            bool foe_escaped = true;
            for (auto& f : foes) { const int x = f.position().x; if (my_side() == 0 ? x < kEscapeB : x > kEscapeA) foe_escaped = false; }
            if (foe_escaped) end_fight("X");
            else if (round - S.r0 > kTimeout) end_fight("T");
            else { fight_turn(foes); return; }
        }
    }
    if (S.st == St::home) {
        if (!S.bought && farm_gold()) return;   // item-level arenas: farm the corridor's gold clusters first
        if (tribute_excess()) return;           // arena-duel: carried gems stay at the target (opponents 0)
        if (farm_gems()) return;                // arena-duel: Prism loads its carried gems before each duel
        if (!at_spawn) {
            if (foes.empty() && me.health() < me.max_health()) { try { me.recall(); return; } catch (...) {} }
            move_towards_closest(S.spawn);
            return;
        }
        if (me.health() < me.max_health() * 0.999f || !S.bought) return;   // spawn heals at turn end
        S.st = St::go;
        // Start jitter (positional variation): x in {21,22} (A) / {27,28} (B), any corridor row, 0-1 rounds of delay.
        std::uint32_t h = static_cast<std::uint32_t>(S.corr * 7919 + my_side() * 104729 + S.duels * 2654435761u + bits(57, 2, 3) * 1000003u);   // arena-duel: map salt
        h ^= h >> 13; h *= 0x5bd1e995u; h ^= h >> 15;
        const int jx = static_cast<int>(h % 2u), jy = static_cast<int>((h / 2u) % 6u);
        S.delay = static_cast<int>((h / 12u) % 2u);
        const int y = S.lo + (jy + 2 * S.body) % 6;
        S.start = vec2(my_side() == 0 ? kStartA + jx : kStartB - jx, y);
    }
    if (S.st == St::go) {
        if (!(pos == S.start)) {
            if (tile_distance(pos, S.start) <= 1 && cambc::vision::champion_tiles().is_set(S.start) && pos.x == S.start.x) S.start = pos;
            else { cambc::bitboard g; g.clear(); g.set(S.start, true); move_towards_closest(g); }
        }
        if (me.position() == S.start) S.st = St::ready;
        else return;
    }
    if (S.st == St::ready) {
        if (!(me.position() == S.start)) { S.st = St::go; return; }
        if (S.mates > 0) {
            int ready_mates = 0;
            for (auto& a : cambc::vision::champions(team::ally)) {
                if (a == me) continue;
                try {
                    const vec2 ap = a.position();
                    if (in_corridor(ap) && std::abs(ap.x - S.start.x) <= 1 && a.health() >= a.max_health() * 0.999f) ++ready_mates;
                } catch (...) {}
            }
            if (ready_mates < S.mates) return;
        }
        int foes_on_start = 0;
        for (auto& f : foes) { const int x = f.position().x; if (my_side() == 0 ? x >= kStartB - 1 : x <= kStartA + 1) ++foes_on_start; }
        for (auto& f : foes) {
            const int x = f.position().x;
            const bool on_start = (my_side() == 0 ? x >= kStartB - 1 : x <= kStartA + 1) && foes_on_start >= S.foe_bodies;   // all foes on their start tiles
            const bool engaging = tile_distance(pos, f.position()) <= 5;
            if (on_start && !engaging && S.delay > 0) { --S.delay; return; }
            if (on_start || engaging) {
                S.st = St::fight;
                S.r0 = round;
                S.gems0 = static_cast<int>(me.gem_count()); S.foc0 = static_cast<int>(me.focus());
                S.foes = foes;
                S.foe_names.clear();
                for (auto& g : foes) { if (!S.foe_names.empty()) S.foe_names += '+'; S.foe_names += name_of(g.champion_name()); }
                fight_turn(foes);
                return;
            }
        }
    }
}
