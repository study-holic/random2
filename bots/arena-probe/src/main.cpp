// Arena probe for engine-1.0 rule tests (tools/arena/mkprobe.mjs writes the maps).
// Scenario = (map width - 31) / 2; maps are 31 high, rotational symmetry, team A spawn x 0..3, y 0..3.
// Each champion k (index of its type in the team's pick list) runs a step script; mirrored scenarios give team B
// team A's script with coordinates rotated 180 degrees. Logs (stdout, '\n' only):
//   L r side k x y hp gems gold      start of each own turn (position and health as seen by the camp phase)
//   A r side k what x y              actions
//   G r side gid state timer         camp-group state change (0 alive, 1 reward available, 2 respawning)
//   C r side gid kind x y hp awake   visible camp change
//   GI side gid gold gems respawn n x y   group table at the first turn
#include <cambc.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

using cambc::vec2;
using cambc::team;
using D = cambc::champion_draft;
using CT = cambc::champion_type;

namespace {
struct Scenario {
    std::vector<D> picksA, picksB;
    bool mirror;                                   // B runs A's scripts rotated
    std::vector<std::string> A, B;                 // one script per champion index; steps separated by ';'
};

// Steps: G x y (go to tile), W n (wait n turns), R n (wait until round n), H x y (one basic attack on the camp at x,y),
// K x y (kill the camp's whole group: attack an in-range living member, else move toward one), U x y s (wait until the
// group's state is s), T x y n (attack the camp from where we stand for n turns, never moving).
std::vector<Scenario> scenarios(int want) {
    const std::string approach = "G 3 2;G 7 2;W 2;G 8 2;W 2;G 9 2;W 2;G 10 2;W 2;G 11 2;W 2;G 12 2;W 2;G 13 2;W 3;";
    const std::string range = approach + "H 16 2;G 12 2;G 11 2;G 10 2;G 9 2;G 8 2;W 6;"   // wake at d3, back off 1/turn
                              "G 12 2;W 2;G 13 2;W 3;H 16 2;W 1;G 12 2;W 1;G 11 2;W 1;G 10 2;W 6";   // second cycle, 2 turns per tile
    std::vector<Scenario> s;
    if (want > 2) s.resize(3);
    else { s.push_back({{D::vesper_vale}, {D::vesper_vale}, true, {range}, {}});   // 0 wolf range
    s.push_back({{D::vesper_vale}, {D::vesper_vale}, true, {range}, {}});   // 1 elk range
    s.push_back({{D::vesper_vale}, {D::vesper_vale}, true, {range}, {}}); }   // 2 monolith range
    if (want != 3) s.emplace_back(); else {   // 3 rewards and splits; camps: gem 10,2; gold 10,6; wolf 14,2; elk 14,8; gem x3 8..10,12; gold x3 14..16,12
        const std::vector<D> p{D::vesper_vale, D::stellar, D::prism, D::willow, D::tavi};
        auto step = [](int k, int cx, int cy, std::vector<std::pair<int, std::pair<int, int>>> cap) {
            std::string r = "K " + std::to_string(cx) + " " + std::to_string(cy) + ";";
            for (auto& [who, t] : cap) if (who == k) r += "G " + std::to_string(t.first) + " " + std::to_string(t.second) + ";";
            return r + "U " + std::to_string(cx) + " " + std::to_string(cy) + " 2;W 2;";
        };
        std::vector<std::string> sc;
        for (int k = 0; k < 5; ++k) {
            std::string r = "G 6 " + std::to_string(2 * k) + ";R 20;";
            r += step(k, 10, 2, {{0, {10, 2}}});
            r += step(k, 10, 6, {{1, {10, 6}}});
            r += step(k, 14, 2, {{2, {14, 2}}});
            r += step(k, 9, 12, {{0, {8, 12}}, {1, {10, 12}}});
            r += step(k, 15, 12, {{0, {14, 12}}, {1, {15, 12}}, {2, {16, 12}}});
            r += step(k, 14, 8, {{3, {14, 8}}});
            sc.push_back(r);
        }
        s.push_back({p, p, true, sc, {}});
    }
    // 4-8 targeting: one elk at the centre (19+2k... see map), A from the west, B from the east, 1 Vesper each.
    // Centre for scenario n: x = (31 + 2n - 1) / 2 = 15 + n, y = 15.
    auto tgt = [&](int n, const std::string& a, const std::string& b) {
        const int c = 15 + n;
        auto sub = [&](std::string t) {   // $ = centre x, offsets written as $-3 etc.
            std::string out; for (size_t i = 0; i < t.size(); ++i) {
                if (t[i] == '$') { int off = 0; size_t j = i + 1;
                    if (j < t.size() && (t[j] == '+' || t[j] == '-')) { int sg = t[j] == '-' ? -1 : 1; ++j; int v = 0; while (j < t.size() && isdigit(t[j])) v = v * 10 + (t[j++] - '0'); off = sg * v; }
                    out += std::to_string(c + off); i = j - 1; }
                else out += t[i]; }
            return out; };
        s.push_back({{D::vesper_vale}, {D::vesper_vale}, false, {sub(a)}, {sub(b)}});
    };
    if (want < 4 || want > 8) { s.resize(9); } else {
    tgt(4, "G $-3 15;R 40;T $ 15 40", "G $+2 15;R 40;W 40");                   // A attacks at d3, B passive at d2
    tgt(5, "G $-2 15;R 40;T $ 15 40", "G $+3 15;R 40;W 40");                   // A attacks at d2, B passive at d3
    tgt(6, "G $-3 15;R 40;T $ 15 40", "G $+3 15;R 40;T $ 15 40");              // both attack at d3
    tgt(7, "G $-3 15;R 40;H $ 15;G $-4 15;G $-5 15;G $-6 15;W 30", "G $+2 15;R 40;W 40");   // A wakes it and leaves; B passive at d2
    tgt(8, "G $-3 15;R 40;T $ 15 40", "G $+6 15;R 46;G $+2 15;W 40"); }       // B walks in to d2 after 6 rounds
    // 9 respawn delay, own side, 1 Vesper each (mirrored): control, occupied 100 turns, adjacent 100 turns
    s.push_back({{D::vesper_vale}, {D::vesper_vale}, true,
                 {"G 5 2;K 8 2;G 8 2;U 8 2 2;G 7 2;U 8 2 0;W 2;"
                  "G 5 8;K 8 8;G 8 8;U 8 8 2;W 100;G 7 8;U 8 8 0;W 2;"
                  "G 5 14;K 8 14;G 8 14;U 8 14 2;G 7 14;W 100;G 3 14;U 8 14 0;W 2"}, {}});
    // 10 enemy occupation: centre gem cluster (25,15): A kills and captures, then leaves; B stands on it 100 turns
    s.push_back({{D::vesper_vale}, {D::vesper_vale}, false,
                 {"G 22 15;R 30;K 25 15;G 25 15;U 25 15 2;G 22 15;W 200"},
                 {"G 28 15;U 25 15 2;W 3;G 25 15;W 100;G 28 15;W 200"}});
    // 11 contest: centre group of 3 gem clusters (26,14..16): A kills, captures on 26,14; B joins 26,16 two rounds later for 10
    s.push_back({{D::vesper_vale}, {D::vesper_vale}, false,
                 {"G 23 14;R 30;K 26 14;G 26 14;U 26 14 2;G 23 14;W 200"},
                 {"G 29 16;U 26 16 1;W 2;G 27 16;G 26 16;W 10;G 29 16;W 200"}});
    // 12 enemy adjacent during capture: centre gem cluster (27,15); B stands at d1 (28,15) until round 70, then d3
    s.push_back({{D::vesper_vale}, {D::vesper_vale}, false,
                 {"G 24 15;R 30;K 27 15;G 27 15;U 27 15 2;G 24 15;W 300"},
                 {"G 28 15;R 70;G 30 15;W 300"}});
    // 13 ally adjacent at completion: centre gem cluster (28,15); A k0 captures on the tile, A k1 stands at d1 (29,15)
    s.push_back({{D::vesper_vale, D::stellar}, {D::vesper_vale, D::stellar}, false,
                 {"G 25 15;R 30;K 28 15;G 28 15;U 28 15 2;G 25 15;W 300", "G 25 13;R 30;G 29 15;U 28 15 2;G 25 13;W 300"},
                 {"W 300", "W 300"}});
    return s;
}

struct Step { char op; int a, b, c; };
std::vector<Step> parse(const std::string& src, bool rot, int W, int H) {
    std::vector<Step> out; const char* q = src.c_str();
    while (*q) {
        while (*q == ' ' || *q == ';') ++q;
        if (!*q) break;
        Step st{*q++, 0, 0, 0}; int* f[3] = {&st.a, &st.b, &st.c};
        for (int i = 0; i < 3; ++i) { while (*q == ' ') ++q; if (*q == ';' || !*q) break; bool neg = *q == '-'; if (neg) ++q; int v = 0; while (*q >= '0' && *q <= '9') v = v * 10 + (*q++ - '0'); *f[i] = neg ? -v : v; }
        while (*q && *q != ';') ++q;
        if (rot && (st.op == 'G' || st.op == 'H' || st.op == 'K' || st.op == 'U' || st.op == 'T')) { st.a = W - 1 - st.a; st.b = H - 1 - st.b; }
        out.push_back(st);
    }
    return out;
}

int scen_id_raw() { return (cambc::map::width() - 31) / 2; }
std::vector<Scenario> SCV; const Scenario& cur() { if (SCV.empty()) SCV = scenarios(scen_id_raw()); return SCV.at(scen_id_raw()); }
int scen_id() { return (cambc::map::width() - 31) / 2; }
bool side_b() { return cambc::map::spawn_region(team::ally).is_set(vec2(cambc::map::width() - 1, cambc::map::height() - 1)); }

struct State {
    bool init = false; int side = 0, k = 0; std::vector<Step> prog; size_t pc = 0; int wait = -1;
    std::vector<cambc::map::camp_group> groups;
    std::map<int, std::pair<int, int>> gstate; std::map<int, std::pair<int, int>> cstate;
    bool dead_logged = false;
} S;

std::map<std::pair<int, int>, int> GOF;
int group_of(vec2 p) { auto it = GOF.find({p.x, p.y}); return it == GOF.end() ? -1 : it->second; }
std::vector<cambc::camp> CAMPS;   // vision::camps() once per turn
int gstate_of(int gi, int* timer = nullptr) {
    auto st = S.groups[gi].state();
    if (std::holds_alternative<cambc::map::camp_group_alive>(st)) return 0;
    if (std::holds_alternative<cambc::map::camp_group_reward_available>(st)) return 1;
    if (timer) *timer = std::get<cambc::map::camp_group_respawning>(st).timer;
    return 2;
}
const cambc::camp* living_camp_at(vec2 p) {
    for (const auto& c : CAMPS) if (c.spawn_position() == p && c.is_visible() && !c.is_dead()) return &c;
    return nullptr;
}

void log_camps(int r) {
    const vec2 me = cambc::self().position();
    for (size_t gi = 0; gi < S.groups.size(); ++gi) {
        if (S.groups[gi].camps()[0].spawn_position().chebyshev_distance(me) > 8) continue;
        try {
            int t = 0; int st = gstate_of(static_cast<int>(gi), &t);
            auto key = std::make_pair(st, st == 2 ? t : 0);
            auto it = S.gstate.find(static_cast<int>(gi));
            if (it == S.gstate.end() || it->second != key) { S.gstate[static_cast<int>(gi)] = key; std::cout << "G " << r << ' ' << S.side << ' ' << gi << ' ' << st << ' ' << t << '\n'; }
        } catch (...) {}
    }
    for (const auto& c : CAMPS) {
        if (!c.is_visible()) continue;
        int hp = static_cast<int>(c.health()); int aw = c.is_dead() ? -1 : (c.is_awake() ? 1 : 0);
        int key = static_cast<int>(static_cast<std::uint32_t>(c.operator cambc::handle_camp_t()));
        auto v = std::make_pair(hp, aw);
        auto it = S.cstate.find(key);
        if (it == S.cstate.end() || it->second != v) { S.cstate[key] = v;
            std::cout << "C " << r << ' ' << S.side << ' ' << group_of(c.spawn_position()) << ' ' << static_cast<int>(c.kind()) << ' ' << c.position().x << ' ' << c.position().y << ' ' << hp << ' ' << aw << '\n'; }
    }
}

bool attack_camp(const cambc::camp& c) {
    auto& me = cambc::self();
    if (!me.can_attack() || !me.attack_tiles().is_set(c.position())) return false;
    me.attack(c);
    std::cout << "A " << cambc::round_number() << ' ' << S.side << ' ' << S.k << " atk " << c.position().x << ' ' << c.position().y << '\n';
    return true;
}

void turn() {
    auto& me = cambc::self();
    const int r = static_cast<int>(cambc::round_number());
    if (!S.init) {
        std::uint64_t f0 = cambc::fuel_remaining();
        S.init = true; S.side = side_b() ? 1 : 0;
        const auto& s = cur();
        const auto& picks = S.side ? s.picksB : s.picksA;
        CT t = me.champion_name();
        for (size_t i = 0; i < picks.size(); ++i) { int d = static_cast<int>(picks[i]); int ti = d <= 3 ? d : d + 1; if (static_cast<int>(t) == ti) S.k = static_cast<int>(i); }
        const auto& scripts = (S.side && !s.mirror) ? s.B : s.A;
        std::cout << "K " << S.side << " type " << static_cast<int>(t) << " k " << S.k << '\n';
        std::uint64_t f1 = cambc::fuel_remaining();
        if (S.k < static_cast<int>(scripts.size())) S.prog = parse(scripts[S.k], S.side && s.mirror, cambc::map::width(), cambc::map::height());
        std::uint64_t f2 = cambc::fuel_remaining();
        S.groups = cambc::map::camp_groups();
        std::cout << "F " << f0 << ' ' << f1 << ' ' << f2 << ' ' << cambc::fuel_remaining() << '\n';
        for (size_t gi = 0; gi < S.groups.size(); ++gi) for (const auto& c : S.groups[gi].camps()) GOF[{c.spawn_position().x, c.spawn_position().y}] = static_cast<int>(gi);
        return;   // init alone fills the first turn
    }
    static bool gi_done = false;
    if (!gi_done) { gi_done = true;
        if (S.k == 0) for (size_t gi = 0; gi < S.groups.size(); ++gi) { const auto& g = S.groups[gi];
            std::cout << "GI " << S.side << ' ' << gi << ' ' << g.gold_value() << ' ' << g.gem_value() << ' ' << g.max_respawn_timer() << ' ' << g.camps().size()
                      << ' ' << g.camps()[0].spawn_position().x << ' ' << g.camps()[0].spawn_position().y << '\n'; }
    }
    if (!me.is_alive()) { if (!S.dead_logged) { S.dead_logged = true; std::cout << "L " << r << ' ' << S.side << ' ' << S.k << " dead\n"; } return; }
    S.dead_logged = false;
    std::cout << "L " << r << ' ' << S.side << ' ' << S.k << ' ' << me.position().x << ' ' << me.position().y << ' ' << static_cast<int>(me.health())
              << ' ' << me.gem_count() << ' ' << me.gold_count() << '\n';
    CAMPS = cambc::vision::camps();
    if (S.k == 0) log_camps(r);
    for (int guard = 0; guard < 4 && S.pc < S.prog.size(); ++guard) {
        const Step& st = S.prog[S.pc];
        const vec2 p(st.a, st.b);
        bool done = false, acted = false;
        switch (st.op) {
        case 'G': if (me.position() == p) done = true; else { if (me.can_move()) { try { me.move_towards(p); } catch (...) {} } acted = true; if (me.position() == p) { done = true; } } break;
        case 'W': if (S.wait < 0) S.wait = st.a; if (S.wait-- <= 0) { S.wait = -1; done = true; } else acted = true; break;
        case 'R': if (r >= st.a) done = true; else acted = true; break;
        case 'H': { auto c = living_camp_at(p); if (c && attack_camp(*c)) { done = true; acted = true; } else { acted = true; std::cout << "A " << r << ' ' << S.side << ' ' << S.k << " hitfail\n"; done = true; } } break;
        case 'T': { if (S.wait < 0) S.wait = st.c; auto c = living_camp_at(p); if (c) attack_camp(*c); acted = true; if (--S.wait <= 0 || !c) { S.wait = -1; done = true; } } break;
        case 'K': {
            int gi = group_of(p); bool any = false; acted = true;
            for (const auto& c : CAMPS) if (c.is_visible() && !c.is_dead() && group_of(c.spawn_position()) == gi) { any = true; if (attack_camp(c)) { goto kdone; } }
            if (!any) { int s2 = 0; try { s2 = gstate_of(gi); } catch (...) {} if (s2 != 0) { done = true; acted = false; break; } }
            for (const auto& c : CAMPS) if (c.is_visible() && !c.is_dead() && group_of(c.spawn_position()) == gi) { if (me.can_move()) { try { me.move_towards(c.position()); } catch (...) {} } for (const auto& c2 : CAMPS) if (c2.is_visible() && !c2.is_dead() && group_of(c2.spawn_position()) == gi && attack_camp(c2)) break; goto kdone; }
            if (me.can_move()) { try { me.move_towards(p); } catch (...) {} }
        kdone: break; }
        case 'U': { int gi = group_of(p); int s2 = -1; try { s2 = gstate_of(gi); } catch (...) {} if (s2 == st.c) done = true; else acted = true; } break;
        default: done = true;
        }
        if (done) { ++S.pc; S.wait = -1; }
        if (acted) break;
    }
    if (cambc::fuel_remaining() < 30000) std::cout << "FX " << r << ' ' << S.k << ' ' << S.pc << ' ' << cambc::fuel_remaining() << '\n';
}

void draft() {
    const auto& s = cur();
    const bool b = cambc::map::spawn_region(team::ally).is_set(vec2(cambc::map::width() - 1, cambc::map::height() - 1));
    const auto& picks = b ? s.picksB : s.picksA;
    const auto chosen = cambc::draft::current_draft(team::ally);
    std::cout << "draft scen " << scen_id() << " b " << b << " chosen " << chosen.size() << '\n';
    for (auto d : picks) if (std::find(chosen.begin(), chosen.end(), d) == chosen.end()) { cambc::draft::pick(d); return; }
    std::cout << "draft: list exhausted\n";
}
}  // namespace

int main() {
    while (true) {
        try {
            if (cambc::is_draft()) draft(); else turn();
        } catch (const std::exception& e) {
            std::cout << "bot error: " << e.what() << '\n';
        } catch (...) {
            std::cout << "bot error: unknown\n";
        }
        std::cout.flush();
        cambc::end_turn();
    }
}
