// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cstdint>
#include <iostream>
#include <span>
#include <variant>
#include <vector>
namespace va {
// s3_contest.cpp (v5 S3): contest plays. Contest's gems come only from its two Monoliths (17,31) and (32,18): 200 gems,
// 300 gold, 20,000 health, strength 200, range 5, respawn 150, one spawn tile each. Both altars sit in the centre,
// (25,24) and (24,25), 7-8 tiles from either Monolith. v4 scored 0 gems in 9/9 real top-10 games and 0-0 in self-play
// (results/v5-s3-diag.md). The top teams bank whole Monoliths (822 gems from 4.1 tributes a game); each 200-gem
// tribute channels ceil(200/10) = 20 turns, and a stun, the carrier's death or a newly placed altar lock cancels it
// (GameRules l.181, l.195; damage does not).
//
// P_S3_INTERDICT (play a). Model: a carrier with bag g channels T = ceil(g/10) turns at a known altar. Any of our
// champions holding a lock and within 2 tiles of that altar during the channel cancels it: they lose up to T turns plus
// the lock-clearing attacks, and the bag stays exposed. Decision rule: if I see an enemy with is_submitting_gems() and a
// bag >= P_S3_INT_BAG beside altar a: place my lock at a when dist(me, a) <= 2; else cast a stun on the carrier when one
// is ready and in range; else walk towards a when dist(me, a) <= P_S3_INT_REACH (I arrive inside the channel when
// dist - 2 < T). The sighting is broadcast in shared byte 7 so champions out of sight converge too. Source: the
// "interdiction" pattern of MOBA objective play (contest the channel, not the fight) mapped onto the tribute channel rule.
//
// P_S3_MONO (play b). Model: the Monolith hits the nearest champion for 200 every round (s3 self-play trace, 20 Oct;
// results/arena-1020.md sections 1-2: nearest targeting, ties alternate, no chase, no regeneration). The squad's damage D per turn kills it
// in 20,000 / D turns while it deals 200 per turn to whoever is nearest. Rule: from round P_S3_MONO_START champions above
// P_S3_MONO_ENTER health stage at distance 6-7 until P_S3_MONO_SQUAD are within 6 (or it is already damaged); melee stand adjacent and ranged at 3 (nearest melee absorb the
// hits), ranged stand at distance 2..attack range; everybody attacks and casts on it. A champion below its
// leave health (200 per step it needs to drop behind the next-nearest ally, plus P_S3_MONO_MARGIN) leaves (v4's retreat: step out of range, recall, heal), which hands the aggro to the next
// nearest; progress is kept because the Monolith does not heal. On death the healthiest visible ally within 6 takes the
// spawn tile (a 200-gem carrier must survive 20 channel turns), the rest guard within 2; the carrier banks through
// v4's lock-aware Bot::bank. Source: raid "tank and spank" with tank rotation (MMO/MOBA objective play: Roshan in Dota 2,
// Baron in League of Legends), mapped onto nearest-target aggro.
//
// P_S3_ESCORT: a champion without a bag walks beside a visible ally carrying >= 100 gems until it has tributed.
#include "plan.h"
#include "bot.h"
#include "util.h"
#include "s3_contest.h"

#ifndef P_S3_MONO
#define P_S3_MONO 1
#endif
#ifndef P_S3_INTERDICT
#define P_S3_INTERDICT 1
#endif
#ifndef P_S3_ESCORT
#define P_S3_ESCORT 1
#endif
#ifndef P_S3_INT_BAG
#define P_S3_INT_BAG 40   // smallest channelling bag worth a lock (a 40-bag channels 4 turns)
#endif
#ifndef P_S3_INT_REACH
#define P_S3_INT_REACH 16   // walk to a seen channel from this far (a 200-bag channels 20 turns)
#endif
#ifndef P_S3_MONO_START
#define P_S3_MONO_START 100   // first round of the siege (before it: v4's gold farming and shopping)
#endif
#ifndef P_S3_MONO_TANK_HP
#define P_S3_MONO_TANK_HP 950   // melee below this maximum health do not take a distance-1 slot
#endif
#ifndef P_S3_MONO_LEAVE
#define P_S3_MONO_LEAVE 400   // leave below this health while a healthy ally is as near as I am
#endif
#ifndef P_S3_MONO_COVER_HP
#define P_S3_MONO_COVER_HP 600   // an ally above this health counts as cover
#endif
#ifndef P_S3_MONO_MARGIN
#define P_S3_MONO_MARGIN 200   // health kept above the hits taken while walking out of the line of fire
#endif
#ifndef P_S3_MONO_CI_MARGIN
#define P_S3_MONO_CI_MARGIN 300   // extra margin for Cassi and Issac (shared fate)
#endif
#ifndef P_S3_MONO_ENTER
#define P_S3_MONO_ENTER 700   // walk in only above this health
#endif
#ifndef P_S3_MONO_SQUAD
#define P_S3_MONO_SQUAD 3   // champions within 7 before the first hit
#endif
#ifndef P_S3_MONO_FOE
#define P_S3_MONO_FOE 5   // an enemy champion this close: v4's fight rules take the turn
#endif

using cambc::vec2;
using cambc::team;

namespace S3C {

namespace {

constexpr int kAlertByte = 7;   // (altar + 1) | ((round / 4) % 32) << 3: an enemy channel seen at that altar

struct Mono {
    cambc::camp camp;
    int group = -1;
    vec2 pos{};
    float hp = 20000.0f;
    int ready = 0;   // round it is believed alive again (0 = alive or unknown)
    cambc::bitboard stage;   // tiles at distance 6-7 in its line of sight (built on first use)
};

std::vector<Mono> g_m;
bool g_ready = false;
bool g_leaving = false;    // left the Monolith hurt: stay out until healed to P_S3_MONO_ENTER

bool own_lock() {
    try { return cambc::self().consumables()[cambc::consumable_type::altar_lock].count() > 0; } catch (...) { return false; }
}

bool use_lock(vec2 altar) {
    try { return cambc::self().consumables()[cambc::consumable_type::altar_lock].use(altar); } catch (...) { return false; }
}

// A ready stun (cc ability) cast on the champion; true on success.
bool stun(const cambc::champion& e) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        try {
            if (!ab.is_cc() || !ab.is_ready()) continue;
            if (ab.valid_tiles().is_set(e.position()) && ab.use(e)) return true;
        } catch (...) {}
    }
    return false;
}

int altar_near(const Bot& b, vec2 p) {
    for (std::size_t i = 0; i < b.altars.size(); ++i) if (tile_distance(p, b.altars[i]) <= 1) return static_cast<int>(i);
    return -1;
}

bool interdict(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies) {
    const bool lock = own_lock();
    int alt = -1;
    const cambc::champion* carrier = nullptr;
    for (const auto& e : enemies) {
        try {
            if (!e.is_visible() || !e.is_submitting_gems() || static_cast<int>(e.gem_count()) < P_S3_INT_BAG) continue;
            const int a = altar_near(b, e.position());
            if (a < 0) continue;
            alt = a; carrier = &e;
            const std::uint8_t w = static_cast<std::uint8_t>((a + 1) | (((v.round / 4) % 32) << 3));
            cambc::shared_memory.write(kAlertByte, std::span<const std::uint8_t>{&w, 1});
            break;
        } catch (...) {}
    }
    if (carrier != nullptr) {
        const vec2 ap = b.altars[alt];
        if (lock && tile_distance(v.pos, ap) <= 2 && use_lock(ap)) return true;
        if (stun(*carrier)) return true;
        if (lock && tile_distance(v.pos, ap) <= P_S3_INT_REACH && cambc::self().gem_count() == 0) {
            move_towards_closest(b.static_block, b.camp_static, tiles_around(ap, 2));
            return true;
        }
        return false;
    }
    // Out of sight: answer a fresh alert (at most 6 rounds old) when holding a lock and not carrying.
    if (!lock || cambc::self().gem_count() > 0) return false;
    std::uint8_t r = 0;
    cambc::shared_memory.read(kAlertByte, std::span<std::uint8_t>{&r, 1});
    const int a = (r & 7) - 1;
    if (a < 0 || a >= static_cast<int>(b.altars.size())) return false;
    const int age = ((v.round / 4) % 32 - (r >> 3) + 32) % 32;
    if (age > 1) return false;
    const vec2 ap = b.altars[a];
    const int d = tile_distance(v.pos, ap);
    if (d <= 2 || d > P_S3_INT_REACH) return false;   // in place and nothing seen: the channel is over
    for (const auto& e : enemies) if (e.is_visible() && tile_distance(e.position(), v.pos) <= 3) return false;   // fight first
    move_towards_closest(b.static_block, b.camp_static, tiles_around(ap, 2));
    return true;
}

bool escort(Bot& b, const TurnView& v, std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    if (me.gem_count() > 0) return false;
    const cambc::champion* best = nullptr; int bd = 99;
    for (const auto& a : allies) {
        try {
            if (a == me || !a.is_visible() || a.gem_count() < 100) continue;
            const int d = tile_distance(a.position(), v.pos);
            if (d < bd) { bd = d; best = &a; }
        } catch (...) {}
    }
    if (best == nullptr) return false;
    if (bd <= 2) return true;   // beside it: hold (fights already ran for anything in range)
    auto goals = tiles_around(best->position(), 2) & ~tiles_around(best->position(), 0) & ~b.static_block;
    move_towards_closest(b.static_block, b.camp_static, goals);
    return true;
}

// Refresh what this champion sees of each Monolith.
void observe(Bot& b, const TurnView& v) {
    for (auto& m : g_m) {
        if (!v.visible.is_set(m.pos)) continue;
        bool alive = false;
        try { alive = m.camp.is_visible() && m.camp.health() > 0; if (alive) m.hp = m.camp.health(); } catch (...) {}
        if (alive) { m.ready = 0; continue; }
        try {
            const auto s = b.groups[m.group].group.state();
            if (auto r = std::get_if<cambc::map::camp_group_respawning>(&s)) { m.ready = v.round + r->timer; m.hp = 20000.0f; }
            else if (std::holds_alternative<cambc::map::camp_group_reward_available>(s)) m.ready = -1;   // reward on the tile
        } catch (...) {}
    }
}

// Siege moves ignore v4's Monolith zone (util.cpp: an awake Monolith's range is a wall unless we stand inside): the
// zone stays on for every other move, so champions outside the siege keep avoiding it.
bool walk(Bot& b, const cambc::bitboard& goals) {
    const bool z = g_mono_zone_on;
    g_mono_zone_on = false;
    const bool ok = move_towards_closest(b.static_block, b.camp_static, goals);
    g_mono_zone_on = z;
    return ok;
}

bool mono(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    if (me.gem_count() > 0) return false;   // carriers bank through v4
    for (const auto& e : enemies) if (e.is_visible() && tile_distance(e.position(), v.pos) <= P_S3_MONO_FOE) return false;
    observe(b, v);
    // Target: a Monolith with a pending reward, else the first alive one in init order (nearest our spawn first), so
    // the whole team works on the same one.
    int k = -1;
    for (int i = 0; i < static_cast<int>(g_m.size()); ++i) if (g_m[i].ready < 0 && v.visible.is_set(g_m[i].pos)) { k = i; break; }
    if (k < 0)
        for (int i = 0; i < static_cast<int>(g_m.size()); ++i)
            if (g_m[i].ready <= v.round + tile_distance(v.pos, g_m[i].pos)) { k = i; break; }
    auto& m = g_m[k];
    auto& g = b.groups[m.group];
    const int d = tile_distance(v.pos, m.pos);
    b.target = -1;
    if (m.ready < 0 && v.visible.is_set(m.pos)) {   // reward pending: healthiest visible ally within 6 captures
        if (g.mask.is_set(v.pos)) return true;
        bool other = false;
        for (const auto& a : allies) {
            try {
                if (a == me || !a.is_visible() || tile_distance(a.position(), m.pos) > 6) continue;
                if (a.health() > me.health() || (a.health() == me.health() && b.roster_index(a) < b.ally_index)) { other = true; break; }
            } catch (...) {}
        }
        if (!other || d > 6) { walk(b, g.mask); return true; }
        auto guard = tiles_around(m.pos, 2) & ~g.mask & ~b.static_block;
        if (!guard.is_set(v.pos)) walk(b, guard);
        return true;
    }
    if (v.round < P_S3_MONO_START) return false;   // early gold farming and gear first (v4 default)
    // Aggro model (s3 self-play traces, 20 Oct: 200 damage to the nearest champion nearly every round; ties alternate).
    // While a healthy ally (health > P_S3_MONO_COVER_HP) stands as near as I do, one step back hands it the aggro: leave
    // at P_S3_MONO_LEAVE. Without such cover the aggro follows me out, one hit per step for 6 - d steps: leave at
    // 200 x (6 - d) + P_S3_MONO_MARGIN. Cassi and Issac add P_S3_MONO_CI_MARGIN (one body's death kills both). Traced
    // 20 Oct: 74 of 290 Monolith deaths were Ryth walking out alone from distance 1.
    int near7 = 0, m2 = 99, cover = 0;
    for (const auto& a : allies) {
        if (a == me || !a.is_visible()) continue;
        const int ad = tile_distance(a.position(), m.pos);
        if (ad <= 7) ++near7;   // the staging ring is 6-7
        if (ad <= 5) m2 = std::min(m2, ad);
        try { if (ad <= d && a.health() > P_S3_MONO_COVER_HP) ++cover; } catch (...) {}
    }
    const auto ty = me.champion_name();
    const bool ci = ty == cambc::champion_type::cassi || ty == cambc::champion_type::issac;
    const float leave = (cover > 0 ? static_cast<float>(P_S3_MONO_LEAVE) : 200.0f * static_cast<float>(std::max(0, 6 - d)) + P_S3_MONO_MARGIN)
        + (ci ? P_S3_MONO_CI_MARGIN : 0);
    const float hp = me.health();
    // Melee below P_S3_MONO_TANK_HP maximum health cannot take the hits a distance-1 slot draws (traced: Vendetta, 800,
    // reached distance 1, took 2 hits, and died on the way out in 39 of 290 deaths): they farm with v4's rules instead.
    const bool melee_me = b.is_melee || me.attack_range() <= 1;
    if (melee_me && me.max_health() < P_S3_MONO_TANK_HP && !g_leaving) return false;
    if (hp >= P_S3_MONO_ENTER) g_leaving = false;
    if (d <= 5 && hp < leave) g_leaving = true;   // sticky: stepping out and back in every turn was the death loop
    if (g_leaving && d <= 5) {   // walk out of its range ourselves (v4's retreat would recall at distance 5, inside it,
        b.retreating = true;     // and every hit interrupts the recall); outside, v4's retreat recalls and heals
        walk(b, ~tiles_around(m.pos, 5) & ~b.static_block);
        return true;
    }
    if (g_leaving || (d > 5 && hp < P_S3_MONO_ENTER)) {   // heal before the next turn as tank
        // Out of its range: recall here. v4's retreat would walk home (its unseen-hit rule reads the Monolith's last hit
        // as an unseen camp), and the path home crosses the range again (traced: 5-6-5-6 bounces until death).
        g_leaving = true;
        b.retreating = true;
        if (d <= 6) { walk(b, ~tiles_around(m.pos, 6) & ~b.static_block); return true; }
        try { if (!me.is_recalling()) me.recall(); } catch (...) {}
        return true;
    }
    b.retreating = false;
    // Approach while it sleeps: an asleep camp never attacks (results/arena-1020.md section 2), and the first champion
    // to walk into an awake Monolith's range is the nearest for 4-5 steps (traced: 800 -> 0 on the way in). Awake, we
    // enter only behind an ally already within 3. In place, we wake it only when P_S3_MONO_SQUAD of us are within 3.
    bool awake = true;
    try { if (v.visible.is_set(m.pos) && m.camp.is_visible()) awake = m.camp.is_awake(); } catch (...) {}
    int posted = 0;
    for (const auto& a : allies) if (!(a == me) && a.is_visible() && tile_distance(a.position(), m.pos) <= 3) ++posted;
    const bool squad = near7 + 1 >= P_S3_MONO_SQUAD || m.hp < 19500.0f;
    if (m.stage.is_empty()) {
        const cambc::bitboard lit = (cambc::map::walls() | cambc::map::bushes()).shadow_cast(m.pos, 8);
        const cambc::bitboard ring = tiles_around(m.pos, 7) & ~tiles_around(m.pos, 5) & ~b.static_block & ~b.camp_static;
        m.stage = (ring & lit).is_empty() ? ring : (ring & lit);
    }
    const int ar = std::max(1, static_cast<int>(me.attack_range()));
    const bool melee = b.is_melee || ar <= 1;
    const int want = melee ? 1 : ar;
    if (d > 5 && (!squad || (awake && m2 > 3))) {   // stage in sight of it, outside its range
        if (!m.stage.is_set(v.pos)) walk(b, m.stage);
        return true;
    }
    // Stand: melee adjacent (they share the hits by tie alternation); ranged at distance 3, their attack range, behind
    // them. Ranged champions share one allowance between moving and attacking (GameRules l.50): in range, they only hit.
    if (melee ? d != 1 : d > ar) {
        auto goals = (tiles_around(m.pos, want) & ~tiles_around(m.pos, want - 1)) & ~b.static_block;
        walk(b, goals);
        if (!awake || d > ar) return true;
    }
    if (!awake && posted + 1 < P_S3_MONO_SQUAD && m.hp >= 19500.0f) return true;   // in place: wait before waking it
    if (cast_on_camp(m.camp)) { attack_if_in_range(m.camp); return true; }
    if (attack_if_in_range(m.camp)) reset_and_attack_again(m.camp);
    return true;
}

}  // namespace

void init(Bot& b) {
    if (!(P_S3_MONO || P_S3_INTERDICT || P_S3_ESCORT)) return;
    g_ready = true;
    for (int i = 0; i < static_cast<int>(b.groups.size()); ++i) {
        if (!b.groups[i].monolith) continue;
        for (const auto& c : b.groups[i].group.camps()) {
            if (!c.is_monolith()) continue;
            Mono m{c, i, c.spawn_position()};
            g_m.push_back(m);
        }
    }
    if (!b.spawn_tiles.empty())
        std::stable_sort(g_m.begin(), g_m.end(), [&](const Mono& x, const Mono& y) { return min_distance(x.pos, b.spawn_tiles) < min_distance(y.pos, b.spawn_tiles); });
    // v4's Monolith zone (util.cpp: an awake Monolith's range is a wall unless we stand inside) stays on: the siege walks in
    // while it sleeps, and champions outside the siege keep avoiding it.
}

bool pre(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies) {
    if (!g_ready) return false;
    if (P_S3_INTERDICT && interdict(b, v, enemies)) return true;
    if (P_S3_ESCORT && escort(b, v, allies)) return true;
    if (P_S3_MONO && !g_m.empty() && mono(b, v, enemies, allies)) return true;
    return false;
}

bool buy(Bot& b) {
    (void)b;
    if (!P_S3_INTERDICT) return false;
    auto& me = cambc::self();
    try { if (me.can_buy(cambc::consumable_type::altar_lock)) me.buy(cambc::consumable_type::altar_lock); } catch (...) {}
    return false;   // the default item purchase follows
}

}  // namespace S3C
} // namespace va
