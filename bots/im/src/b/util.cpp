// w3 dispatcher copy: system headers at global scope, bot code in namespace vb
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <variant>
#include <vector>
namespace vb {
#include "util.h"
#include "bot.h"
#include "params.h"
#include <algorithm>
#include <iostream>

using cambc::vec2;

int tile_distance(vec2 a, vec2 b) { return static_cast<int>(a.chebyshev_distance(b)); }

cambc::bitboard tiles_around(vec2 position, int radius) {
    cambc::bitboard tiles; tiles.clear(); tiles.draw_chebyshev(position, radius); return tiles;
}

// Strength by camp kind (rules table); avoids a design query per camp.
int camp_strength(const cambc::camp& c) {
    if (c.is_elk()) return 60;
    if (c.is_wolf()) return 20;
    if (c.is_monolith()) return 200;
    return 0;
}

int min_distance(vec2 pos, const std::vector<vec2>& tiles) {
    int d = 1 << 20;
    for (auto t : tiles) d = std::min(d, tile_distance(pos, t));
    return d;
}

double incoming_damage(const TurnView& v, vec2 pos) {
    double sum = 0;
    for (const auto& c : v.camps)
        if (tile_distance(pos, c.camp.spawn_position()) <= kMonsterRange + 1) sum += camp_strength(c.camp);
    return sum;
}

cambc::bitboard danger_zone(const TurnView& v) {
    cambc::bitboard zone; zone.clear();
    for (const auto& c : v.camps)
        if (camp_strength(c.camp) > 0) zone |= tiles_around(c.camp.spawn_position(), kMonsterRange + 1);
    return zone;
}

void keep_visible(std::vector<cambc::champion>& list) {
    std::vector<cambc::champion> kept;
    kept.reserve(list.size());
    for (const auto& c : list) if (c.is_visible()) kept.push_back(c);
    list = std::move(kept);
}

bool attack_if_in_range(const cambc::unit& target) {
    auto& me = cambc::self();
    if (!me.can_attack() || !target.is_visible() || target.health() <= 0 || !me.attack_tiles().is_set(target.position()))
        return false;
    me.attack(target);
    DBG(" ATK(hp" + std::to_string(int(target.health())) + ")");
    return true;
}

// Use an attack reset right after a basic attack so the champion attacks twice.
void reset_and_attack_again(const cambc::unit& target) {
    auto& me = cambc::self();
    if (!me.can_cast()) return;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_attack_reset() || !ab.is_ready()) continue;
        try {
            DBG(" RESET");
            if (ab.use() && target.is_visible() && target.health() > 0 && me.can_attack()
                && me.attack_tiles().is_set(target.position()))
                { me.attack(target); DBG(" ATK2"); }
        } catch (...) {}
        return;
    }
}

// P_SAVE_CC: an enemy champion within 8 tiles of this champion.
static bool enemy_champion_near() {
    const cambc::vec2 p = cambc::self().position();
    for (auto e : cambc::vision::champion_tiles(cambc::team::enemy).positions())
        if (tile_distance(p, e) <= 8) return true;
    return false;
}

bool cast_on_camp(const cambc::camp& camp) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_camps()) continue;
        if (P_SAVE_CC && ab.is_cc() && enemy_champion_near()) continue;   // keep the stun for the champion fight
        try {
            if (ab.valid_tiles().is_set(camp.position()) && ab.use(camp)) { DBG(" CAST"); return true; }
        } catch (...) {}
    }
    return false;
}

void cast_shield(const TurnView& v) {
    auto& me = cambc::self();
    if (!me.can_cast() || me.champion_name() != cambc::champion_type::varg) return;
    bool camp_near = me.health() < me.max_health() * static_cast<float>(P_VARG_SHIELD_HP);
    for (const auto& c : v.camps)
        if (camp_strength(c.camp) > 0 && tile_distance(v.pos, c.camp.spawn_position()) <= 4) { camp_near = true; break; }
    if (!camp_near && m3_on(M3_SHIELDENG, me.champion_name()))   // v3b item 3: shield at engage (enemy champion within 4)
        for (auto e : cambc::vision::champion_tiles(cambc::team::enemy).positions())
            if (tile_distance(v.pos, e) <= 4) { camp_near = true; break; }
    if (!camp_near) return;
    for (const auto& ab : me.abilities())
        if (ab.is_shield() && ab.is_ready()) { try { ab.use(); } catch (...) {} break; }
}

bool move_towards_closest(const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                          cambc::bitboard goals) {
    auto& me = cambc::self();
    const vec2 pos = me.position();
    if (!me.can_move() || goals.is_set(pos)) return false;
    auto blocked = static_block | cambc::vision::champion_tiles() | cambc::vision::camp_tiles()
        | (camp_static & ~goals) | cambc::vision::teleport_tiles();
    blocked.set(pos, false);
    goals &= ~blocked | cambc::vision::camp_tiles();
    if (goals.is_empty()) return false;
    if (goals.count() == 1 && !blocked.is_set(goals.positions()[0])) { const bool ok = me.move_towards(goals.positions()[0]); DBG(" MT1" + std::string(ok ? "ok" : "NO")); return ok; }
    const auto path_walls = blocked & ~goals;
    cambc::distance_field field; cambc::bitboard visited;
    field.compute_to(path_walls, goals, tiles_around(pos, 0), visited);
    auto step = pos;
    for (auto tile : blocked.reachable_tiles(step, me.base_stats().base_move_speed).positions())
        if (field.get(tile) < field.get(step)) step = tile;
    const bool ok = step != pos && me.move_absolute(step);
    DBG(" MV(" + std::to_string(step.x) + "," + std::to_string(step.y) + (ok ? ")ok" : ")NO") + " g" + std::to_string(goals.count()));
    return ok;
}

bool dash_escape(const TurnView& v, double need, const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                 const std::vector<cambc::champion>& enemies, const std::vector<cambc::champion>& allies,
                 const std::vector<vec2>& spawn_tiles) {
    auto& me = cambc::self();
    if (!me.can_cast() || !(me.health() < need) || enemies.empty()) return false;
    const cambc::ability* dash = nullptr;
    const auto slots = me.abilities();
    for (const auto& ab : slots) if (ab.is_dash()) { dash = &ab; break; }
    if (!dash || !dash->is_ready()) return false;
    std::vector<vec2> foes, friends;
    int d0 = 1 << 20;
    for (const auto& e : enemies) {
        if (!e.is_visible()) continue;
        const vec2 p = e.position();
        foes.push_back(p);
        d0 = std::min(d0, tile_distance(v.pos, p));
    }
    if (foes.empty() || d0 > 2) return false;
    if (P_RUNNER && cambc::map::width() == 80 && cambc::map::height() == 80) return false;   // EXP-123b: no dash on the runner map (expanse), so each validated part acts as validated
    for (const auto& a : allies) if (a.is_visible() && !(a == me)) friends.push_back(a.position());
    auto blocked = static_block | camp_static | cambc::vision::champion_tiles() | cambc::vision::camp_tiles() | cambc::vision::teleport_tiles();
    struct Cand { int md, ad, sd; vec2 p; };
    std::vector<Cand> cands;
    for (const vec2 p : dash->valid_tiles().positions()) {
        if (blocked.is_set(p) || p == v.pos) continue;
        int md = 1 << 20;
        for (const vec2 f : foes) md = std::min(md, tile_distance(p, f));
        if (md <= d0) continue;
        int ad = 1 << 20;
        for (const vec2 f : friends) ad = std::min(ad, tile_distance(p, f));
        cands.push_back({md, ad, min_distance(p, spawn_tiles), p});
    }
    std::sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b) {
        if (a.md != b.md) return a.md > b.md;
        if (a.ad != b.ad) return a.ad < b.ad;
        return a.sd < b.sd;
    });
    // The host rejects a destination with no legal dash path; fall back through the next best few.
    for (std::size_t i = 0; i < cands.size() && i < 4; ++i) {
        bool ok = false;
        try { ok = dash->use(cands[i].p); } catch (...) { ok = false; }
        if (ok) {
#if P_DASH_DBG
            std::cout << "DASH r" << cambc::round_number() << " " << v.pos.x << "," << v.pos.y << ">" << cands[i].p.x << "," << cands[i].p.y
                      << " hp" << int(me.health()) << " d" << d0 << ">" << cands[i].md << "\n";
#endif
            return true;
        }
#if P_DASH_DBG
        std::cout << "DASHFAIL r" << cambc::round_number() << " " << cands[i].p.x << "," << cands[i].p.y << "\n";
#endif
    }
    return false;
}
} // namespace vb
