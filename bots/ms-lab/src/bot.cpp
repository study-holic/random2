#include <cambc.h>
#include <iostream>
namespace bc = cambc;
using bc::vec2;
static int turn = 0, phase = 0, hold = 0, tp_round = -1;
static vec2 gc{-1,-1}, wall{-1,-1};
static int cheb(vec2 a, vec2 b) { return (int)a.chebyshev_distance(b); }

static bool try_buy_tp(bc::champion& me) { try { me.buy(bc::consumable_type::tp_crystal); return true; } catch (const bc::error& e) { std::cout << "TP buy err " << e.message() << std::endl; return false; } }
static bool try_recall(bc::champion& me) { try { me.recall(); return true; } catch (const bc::error& e) { std::cout << "recall err " << e.message() << std::endl; return false; } }
static int try_tp(bc::champion& me, vec2 t, const char* what) {
    try { bool ok = me.consumables()[bc::consumable_type::tp_crystal].use(t); std::cout << "TP use onto " << what << " " << bc::to_string(t) << " from " << bc::to_string(me.position()) << " -> " << (ok ? "STARTED" : "false") << std::endl; return ok ? 1 : 0; }
    catch (const bc::error& e) { std::cout << "TP use onto " << what << " " << bc::to_string(t) << " err: " << e.message() << std::endl; return -1; }
}
static bool try_move(bc::champion& me, vec2 d) { try { return me.move(d); } catch (const bc::error& e) { std::cout << "move err " << e.message() << std::endl; return false; } }

static void step() {
    auto& me = bc::self(); ++turn;
    const int r = (int)bc::round_number();
    if (me.champion_name() != bc::champion_type::ryth || !me.is_alive()) return;
    if (turn <= 40 && turn % 5 == 0) std::cout << "T" << turn << " r" << r << " pos " << bc::to_string(me.position()) << " phase " << phase << " fuel " << bc::fuel_remaining() << std::endl;
    if (turn == 1) {
        int bd = 1<<20;
        for (auto g : bc::map::camp_groups()) for (const auto& c : g.camps()) if (c.is_gold_cluster()) { int d = cheb(me.position(), c.spawn_position()); if (d < bd) { bd = d; gc = c.spawn_position(); } }
        bd = 1<<20; for (auto w : bc::map::walls().positions()) { int d = cheb(me.position(), w); if (d < bd) { bd = d; wall = w; } }
        std::cout << "PLAN gc " << bc::to_string(gc) << " wall " << bc::to_string(wall) << " spawn " << bc::to_string(me.position()) << std::endl;
    }
    if (phase == 0) {
        if (cheb(me.position(), gc) > 1) {
            vec2 best{-1,-1}; int bd = 1<<20;
            for (int dx=-1; dx<=1; ++dx) for (int dy=-1; dy<=1; ++dy) { vec2 t(gc.x+dx, gc.y+dy); if ((dx||dy) && bc::map::is_in_bounds(t) && !bc::map::walls().is_set(t) && !bc::map::holes().is_set(t) && !bc::vision::has_camp(t)) { int d = cheb(me.position(), t); if (d < bd) { bd = d; best = t; } } }
            bool mv = me.move_towards(best); if (!mv && turn % 5 == 0) std::cout << "move_towards " << bc::to_string(best) << " false" << std::endl; return; }
        bool alive = bc::vision::has_camp(gc);
        if (alive) { bc::camp c = bc::vision::camp_at(gc); if (c.health() > 0) { if (me.can_attack()) me.attack(c); } else alive = false; }
        if (!alive) { phase = 1; std::cout << "GC dead r" << r << std::endl; }
    } else if (phase == 1) {
        if (!(me.position() == gc)) { if (!me.move_absolute(gc)) std::cout << "cannot step on gc\n"; }
        else if (++hold > 7 || me.gold_count() >= 10) { phase = 2; std::cout << "CAPTURED r" << r << " gold " << me.gold_count() << std::endl; }
    } else if (phase == 2) {
        if (bc::map::spawn_region(bc::team::ally).is_set(me.position())) { phase = try_buy_tp(me) ? 3 : 9; std::cout << "at spawn r" << r << " gold " << me.gold_count() << " phase " << phase << std::endl; }
        else if (!me.is_recalling()) { if (try_recall(me)) std::cout << "recall r" << r << std::endl; }
    } else if (phase == 3) {
        if (cheb(me.position(), wall) > 3) {
            vec2 best{-1,-1}; int bd = 1<<20;
            for (int dx=-3; dx<=3; ++dx) for (int dy=-3; dy<=3; ++dy) { vec2 t(wall.x+dx, wall.y+dy); if (bc::map::is_in_bounds(t) && !bc::map::walls().is_set(t) && !bc::map::holes().is_set(t) && !bc::vision::has_camp(t)) { int d = cheb(me.position(), t); if (d < bd) { bd = d; best = t; } } }
            if (me.can_move()) { bool mv = me.move_towards(best); if (!mv && turn % 5 == 0) std::cout << "to-wall move_towards " << bc::to_string(best) << " false pos " << bc::to_string(me.position()) << std::endl; }
            return; }
        int ok = try_tp(me, wall, "WALL");
        if (ok != 1) {
            vec2 h{-1,-1}; int bd = 1<<20; for (auto t : bc::map::holes().positions()) { int d = cheb(me.position(), t); if (d <= 3 && d < bd) { bd = d; h = t; } }
            if (h.x >= 0) ok = try_tp(me, h, "HOLE");
        }
        if (ok == 1) { tp_round = r; phase = 4; } else phase = 9;
    } else if (phase == 4) {
        if (r <= tp_round + 5) std::cout << "TP r" << r << " pos " << bc::to_string(me.position()) << " teleporting=" << me.is_teleporting() << " onwall=" << bc::map::walls().is_set(me.position()) << " onhole=" << bc::map::holes().is_set(me.position()) << " canmove=" << me.can_move() << std::endl;
        if (r == tp_round + 5) { bool m = try_move(me, vec2(1,0)); std::cout << "move off -> " << m << " pos " << bc::to_string(me.position()) << std::endl; phase = 9; }
    }
}

int main() {
    while (true) {
        try {
            if (bc::is_draft()) {
                auto mine = bc::draft::current_draft(bc::team::ally);
                for (int i = 0; i < 11; ++i) { auto p = static_cast<bc::champion_draft>(i); bool have=false; for (auto m : mine) if (m==p) have=true; if (!have) { bc::draft::pick(p); break; } }
            } else step();
        } catch (const std::exception& e) { std::cout << "ERR " << e.what() << std::endl; }
        bc::end_turn();
    }
}
