#include <cambc.h>
#include <iostream>
#include <vector>
namespace bc = cambc;
using bc::vec2;
static int turn = 0;
static std::vector<vec2> elks;   // spawn tiles of the chosen multi-elk group
static vec2 stand{-1,-1};
static int cheb(vec2 a, vec2 b) { return (int)a.chebyshev_distance(b); }
#ifndef MODE
#define MODE 0   // 0: Tavi hits one elk from range 3. 1: Varg hits one elk adjacent (splash). 2: Tavi + Stellar both hit the same elk.
#endif
static void step() {
    auto& me = bc::self(); ++turn;
    const int r = (int)bc::round_number();
    if (!me.is_alive()) return;
    const auto t = me.champion_name();
    const bool actor = (MODE == 1) ? t == bc::champion_type::varg : (t == bc::champion_type::tavi || (MODE == 2 && t == bc::champion_type::stellar));
    if (!actor) return;
    if (elks.empty()) {
        int best = -1; int bd = 1 << 20;
        auto gs = bc::map::camp_groups();
        for (int i = 0; i < (int)gs.size(); ++i) { int n = 0; vec2 p; for (const auto& c : gs[i].camps()) if (c.is_elk()) { ++n; p = c.spawn_position(); } if (n >= 2) { int d = cheb(me.position(), p); if (d < bd) { bd = d; best = i; } } }
        for (const auto& c : gs[best].camps()) if (c.is_elk()) elks.push_back(c.spawn_position());
        std::cout << "ELKS"; for (auto e : elks) std::cout << " " << bc::to_string(e); std::cout << std::endl;
    }
    const vec2 tgt = elks[0];
    const int want = (MODE == 1) ? 1 : 3;
    if (cheb(me.position(), tgt) > want) {
        vec2 best{-1,-1}; int bd = 1 << 20;
        for (int dx = -want; dx <= want; ++dx) for (int dy = -want; dy <= want; ++dy) { vec2 p(tgt.x + dx, tgt.y + dy); if (!bc::map::is_in_bounds(p) || bc::map::walls().is_set(p) || bc::map::holes().is_set(p) || bc::vision::has_camp(p)) continue; bool near_other = false; for (size_t k = 1; k < elks.size(); ++k) if (cheb(p, elks[k]) <= 3) near_other = true; if (MODE != 1 && near_other) continue; int d = cheb(me.position(), p); if (d < bd) { bd = d; best = p; } }
        if (best.x < 0) { for (int dx = -want; dx <= want; ++dx) for (int dy = -want; dy <= want; ++dy) { vec2 p(tgt.x + dx, tgt.y + dy); if (bc::map::is_in_bounds(p) && !bc::map::walls().is_set(p) && !bc::map::holes().is_set(p) && !bc::vision::has_camp(p)) { int d = cheb(me.position(), p); if (d < bd) { bd = d; best = p; } } } }
        me.move_towards(best); return;
    }
    if (bc::vision::has_camp(tgt)) { bc::camp c = bc::vision::camp_at(tgt); if (c.health() > 0 && me.can_attack()) me.attack(c); }
    std::cout << "r" << r << " " << (int)t << " hp" << (int)me.health() << " pos" << bc::to_string(me.position());
    for (auto e : elks) { if (bc::vision::has_camp(e)) { bc::camp c = bc::vision::camp_at(e); std::cout << " | " << bc::to_string(e) << " hp" << (int)c.health() << (c.is_awake() ? " AWAKE" : " asleep"); } else std::cout << " | " << bc::to_string(e) << " dead"; }
    std::cout << std::endl;
}
int main() {
    while (true) {
        try {
            if (bc::is_draft()) {
                using D = bc::champion_draft;
                auto mine = bc::draft::current_draft(bc::team::ally);
                for (auto p : {D::tavi, D::varg, D::stellar, D::ryth, D::ren}) { bool have = false; for (auto m : mine) if (m == p) have = true; if (!have) { bc::draft::pick(p); break; } }
            } else step();
        } catch (const std::exception& e) { std::cout << "ERR " << e.what() << std::endl; }
        bc::end_turn();
    }
}
