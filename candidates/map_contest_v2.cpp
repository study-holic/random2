// contest: two monoliths (200 gems, 300 gold, 150-round respawn), 20 gold camps, no gem camps, 2 central altars.
// Measured 18 Oct (ct3 vs foundry-v16): the monolith keeps its damage between visits (20,000 to 508 over 900 rounds of
// trips), and panel bots finish contest on 0 gems, so one paid capture of 200 gems decides the game.
// Chip: healthy champions attack our-side monolith while no enemy is near and leave at 2.5 hits of health.
// Finish: below 3,000 health (shared in byte 7) champions stage 5 tiles out until 3 healthy allies are there, then
// kill it together and hold the spawn tiles until paid. While it respawns the core farms gold and fights.
#include "plan.h"
#include "params.h"
#include "bot.h"
#include "util.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <span>
#if __has_include("contest_cfg.h")
#include "contest_cfg.h"
#endif
#ifndef CONTEST_TRACE
#define CONTEST_TRACE 0
#endif
#ifndef CONTEST_DRAFT
#define CONTEST_DRAFT 0
#endif
#ifndef CONTEST_FINISH_HP
#define CONTEST_FINISH_HP 3000
#endif
#ifndef CONTEST_LEAVE_HITS
#define CONTEST_LEAVE_HITS 2.5
#endif

namespace {
using D = cambc::champion_draft;
constexpr int kHpByte = 7;   // monolith health / 100 + 1; 0 = unknown
int g_mono = -1;
int g_ready = 0;
int g_dead_seen = -1;
float g_hp = 20000;
cambc::bitboard g_stage;     // tiles 5 to 6 from the monolith spawn tiles

void init_contest(Bot& b) {
    int best = -1, best_score = 1 << 20;
    for (std::size_t i = 0; i < b.groups.size(); ++i) {
        const auto& g = b.groups[i];
        if (!g.monolith) continue;
        const int s = (g.dist_ally - g.dist_enemy) * 100 + g.dist_ally;
        if (s < best_score) { best_score = s; best = static_cast<int>(i); }
    }
    g_mono = best;
    if (g_mono < 0) return;
    cambc::bitboard near4, near6;
    for (auto t : b.groups[g_mono].tiles) { near4 |= tiles_around(t, 4); near6 |= tiles_around(t, 6); }
    g_stage = near6 & ~near4 & ~b.static_block;
}

float shared_hp() {
    std::uint8_t x = 0;
    cambc::shared_memory.read(kHpByte, std::span{&x, 1});
    return x == 0 ? -1.0f : (x - 1) * 100.0f;
}
void write_hp(float hp) {
    const std::uint8_t x = static_cast<std::uint8_t>(std::clamp(static_cast<int>(hp / 100.0f) + 1, 1, 255));
    cambc::shared_memory.write(kHpByte, std::span<const std::uint8_t>{&x, 1});
}

bool turn_contest(Bot& b, Phase ph, const TurnView& v, const Champs& enemies, const Champs& allies) {
    if (ph != Phase::pre || g_mono < 0) return false;
    auto& me = cambc::self();
    auto& g = b.groups[g_mono];
    bool seen = false, alive = false;
    for (const auto& c : g.group.camps())
        if (v.visible.is_set(c.spawn_position())) {
            seen = true;
            if (c.is_visible() && c.health() > 0) { alive = true; g_hp = c.health(); write_hp(g_hp); }
        }
    if (!seen) { const float s = shared_hp(); if (s >= 0) g_hp = std::min(g_hp, s); }
    if (CONTEST_TRACE && b.ally_index == 0 && v.round % 25 == 0)
        std::cout << "R" << v.round << " mono hp " << g_hp << " seen " << seen << " alive " << alive
                  << " ready " << g_ready << " gold " << me.gold_count() << std::endl;
    if (me.gem_count() > 0) { g_ready = std::max(g_ready, v.round + 145); g_dead_seen = -1; g_hp = 20000; write_hp(20000); return false; }
    if (v.round < g_ready || b.retreating) return false;
    const bool dead = seen && !alive;
    if (dead) {   // unpaid kill: hold a spawn tile and fight from it; assume paid after 15 rounds without gems
        if (g_dead_seen < 0) g_dead_seen = v.round;
        if (v.round - g_dead_seen > 15) { g_ready = v.round + 135; g_dead_seen = -1; g_hp = 20000; write_hp(20000); return false; }
        if (!g.mask.is_set(v.pos)) move_towards_closest(b.static_block, b.camp_static, g.mask);
        for (const auto& e : enemies) if (attack_if_in_range(e)) break;
        return true;
    }
    const double need = std::max<double>(me.max_health() * P_RETREAT_HURT, 200.0 * CONTEST_LEAVE_HITS + P_RETREAT_INCOMING_BASE * me.max_health());
    const bool in_range = incoming_damage(v, v.pos) > 0;
    if (me.health() < need) {
        if (in_range) { b.retreating = true; b.holding = -1; }
        return false;   // the core retreat or spawn heal takes over
    }
    int foes6 = 0;
    for (const auto& e : enemies) if (tile_distance(e.position(), v.pos) <= 6) ++foes6;
    if (foes6 > 0) return false;   // the core fights back or flees
    if (g_hp <= CONTEST_FINISH_HP) {   // finish together
        int ready = 0;
        for (const auto& a : allies)
            if (a.health() >= a.max_health() * 0.7f && min_distance(a.position(), g.tiles) <= 7) ++ready;
        if (ready < 3 && me.health() >= me.max_health() * 0.7f) {
            if (!g_stage.is_set(v.pos)) move_towards_closest(b.static_block, b.camp_static, g_stage);
            return true;
        }
    }
    b.target = g_mono;
    b.farm_solo(v, nullptr);
    return true;
}
}  // namespace

void setup_contest(Plan& p) {
    switch (CONTEST_DRAFT) {
        case 1: p.draft = {D::cassi_and_issac, D::ryth, D::stellar, D::tavi, D::varg}; break;          // ladder best (z +0.9)
        case 2: p.draft = {D::cassi_and_issac, D::ryth, D::stellar, D::prism, D::varg}; break;         // Prism +4.4 on contest
        case 3: p.draft = {D::ryth, D::marcella, D::willow, D::stellar, D::varg}; break;               // tanks plus heal
        default: break;                                                                                // v4.14 draft
    }
    P_RETREAT_HURT = 0.3f;
    P_RETREAT_CALM = 0.3f;
    P_RETREAT_INCOMING_MULT = static_cast<float>(CONTEST_LEAVE_HITS);
    p.init = init_contest;
    p.turn = turn_contest;
}
