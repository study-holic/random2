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
    P_WEAVE = 1 << 6,       // ranged with a reset: melee adjacent -> step away, reset, attack (move and shoot)
    P_STUNSTEP = 1 << 7,    // ranged: adjacent melee foe is stunned -> step away rather than attack
    P_HOLDRESET = 1 << 8,   // control: never use the attack reset
    P_NOSPELL = 1 << 9,     // control: never cast damage/stun on champions
    P_RETREAT30 = 1 << 10,  // retreat threshold 30% instead of 45%
    P_FOCUSEHP = 1 << 11,   // focus the lowest effective health (C&I: min of the pair; shields add)
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
int draft_of(CT t) { const int i = static_cast<int>(t); return i <= 3 ? i : i - 1; }
const char* name_of(CT t) {
    static const char* n[] = {"Ryth", "Vesper", "Vendetta", "Cassi", "Issac", "Stellar", "Prism", "Marcella", "Willow", "Ren", "Varg", "Tavi"};
    const int i = static_cast<int>(t);
    return (i >= 0 && i < 12) ? n[i] : "?";
}

enum class St { home, go, ready, fight };

struct State {
    bool init = false;
    int corr = -1, lo = 0, policy = 0, level = 0;
    St st = St::home;
    vec2 start{0, 0};
    int r0 = 0;
    bool retreating = false;
    bool bought = false;
    bool dead_logged = true;
    int duels = 0;          // duels finished: seeds the start jitter
    int delay = 0;          // rounds to wait after the foe is ready (turn-order parity jitter)
    bool second_body = false;
    std::vector<cambc::champion> foes;   // corridor foes seen at fight start (handles stay valid)
    std::string foe_names;
    float foe_hp_last = 0;
    cambc::bitboard block, spawn;
    bool ranged = false;
};
State S;

bool in_corridor(vec2 p) { return p.y >= S.lo && p.y <= S.lo + 5; }

bool move_towards_closest(cambc::bitboard goals) {
    auto& me = cambc::self();
    const vec2 pos = me.position();
    if (!me.can_move() || goals.is_set(pos)) return false;
    auto blocked = S.block | cambc::vision::champion_tiles();
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
              << static_cast<int>(S.foe_hp_last) << ' ' << S.policy << std::endl;
}

void init_state() {
    auto& me = cambc::self();
    S.init = true;
    const int s = my_side();
    const int mine = draft_of(me.champion_name());
    for (int k = 0; k < 5; ++k) if (pick_of(s, k) == mine) S.corr = k;
    if (S.corr < 0) S.corr = 0;
    S.lo = 6 + 8 * S.corr;
    S.policy = policy_of(s);
    S.level = level_of(s);
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
void buy_build() {
    if (S.bought || S.level == 0) { S.bought = true; return; }
    auto& me = cambc::self();
    using I = cambc::item_type;
    const auto t = me.champion_name();
    const bool focus = t == CT::cassi || t == CT::stellar || t == CT::prism;
    const I main_shard = focus ? I::power_shard_of_focus : I::power_shard_of_strength;
    const int n = S.level;
    if (me.gold_count() < static_cast<std::uint32_t>(2 * n * 30)) return;
    try {
        if (me.can_buy(main_shard, n)) me.buy(main_shard, n);
        if (me.can_buy(I::power_shard_of_health, n)) me.buy(I::power_shard_of_health, n);
        S.bought = true;
    } catch (...) {}
}

bool is_melee(const cambc::champion& c) { return c.attack_range() <= 1; }

// v2 cast_on_enemy (enemies.cpp:57-67): first ready damage/aoe/cc ability that can target enemies and has the foe in range.
bool cast_on_enemy(const cambc::champion& foe) {
    auto& me = cambc::self();
    if ((S.policy & P_NOSPELL) || !me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_enemies()) continue;
        try {
            if (ab.valid_tiles().is_set(foe.position()) && ab.use(foe)) return true;
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
            if (ab.valid_tiles().is_set(foe.position()) && ab.use(foe.position())) return true;
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
    float best = 1e9f;
    for (const auto& e : foes) {
        float h = e.health();
        if (S.policy & P_FOCUSEHP) {
            try { auto b = e.buffs(); if (b.has_shield()) h += b.get<cambc::buff_shield>().value; } catch (...) {}
        }
        if (h < best) { best = h; foe = &e; }
    }
    if (!foe) return;
    S.foe_hp_last = foe->health();
    const int d = tile_distance(pos, foe->position());
    const int range = std::max(1, static_cast<int>(me.attack_range()));
    auto abs = me.abilities();

    // Varg shield: v2 below 60% (util.cpp:98); P_SHIELDENG as soon as the foe is within 4.
    if (me.champion_name() == CT::varg && me.can_cast()) {
        const bool want = me.health() < me.max_health() * 0.6f || ((S.policy & P_SHIELDENG) && d <= 4);
        if (want) for (const auto& ab : abs) if (ab.is_shield() && ab.is_ready()) { try { ab.use(); } catch (...) {} break; }
    }

    // v2 retreat: below 45% with an enemy in view, walk home and do nothing else (bot.cpp:377,395,411-412).
    const float thr = (S.policy & P_RETREAT30) ? 0.30f : 0.45f;
    if (!(S.policy & P_NORETREAT) && me.health() < me.max_health() * thr) S.retreating = true;
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
        if (p > 10) continue;
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
        if (!at_spawn) {
            if (foes.empty() && me.health() < me.max_health()) { try { me.recall(); return; } catch (...) {} }
            move_towards_closest(S.spawn);
            return;
        }
        if (me.health() < me.max_health() * 0.999f || !S.bought) return;   // spawn heals at turn end
        S.st = St::go;
        // Start jitter (positional variation): x in {21,22} (A) / {27,28} (B), any corridor row, 0-1 rounds of delay.
        std::uint32_t h = static_cast<std::uint32_t>(S.corr * 7919 + my_side() * 104729 + S.duels * 2654435761u + static_cast<int>(me.champion_name()) * 97);
        h ^= h >> 13; h *= 0x5bd1e995u; h ^= h >> 15;
        const int jx = static_cast<int>(h % 2u), jy = static_cast<int>((h / 2u) % 6u);
        S.delay = static_cast<int>((h / 12u) % 2u);
        int y = S.lo + jy;
        if (S.second_body) y = S.lo + (jy + 3) % 6;
        S.start = vec2(my_side() == 0 ? kStartA + jx : kStartB - jx, y);
    }
    if (S.st == St::go) {
        if (!(pos == S.start)) { cambc::bitboard g; g.clear(); g.set(S.start, true); move_towards_closest(g); }
        if (me.position() == S.start) S.st = St::ready;
        else return;
    }
    if (S.st == St::ready) {
        if (!(me.position() == S.start)) { S.st = St::go; return; }
        for (auto& f : foes) {
            const int x = f.position().x;
            const bool on_start = my_side() == 0 ? x >= kStartB - 1 : x <= kStartA + 1;   // foe on its start tiles
            const bool engaging = tile_distance(pos, f.position()) <= 5;
            if (on_start && !engaging && S.delay > 0) { --S.delay; return; }
            if (on_start || engaging) {
                S.st = St::fight;
                S.r0 = round;
                S.foes = foes;
                S.foe_names.clear();
                for (auto& g : foes) { if (!S.foe_names.empty()) S.foe_names += '+'; S.foe_names += name_of(g.champion_name()); }
                fight_turn(foes);
                return;
            }
        }
    }
}
