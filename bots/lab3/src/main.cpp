// lab3: arena probes for the open B1b rows on engine 1.0.3. One behaviour per MODE (compile-time).
// Every line starts with "LAB <mode> r<round> <champion> ...". Opponent: bots/idle unless stated.
//   MODE 1 wake scope (#1) and camp growth (#4): Vesper hits one wolf of a multi-wolf group once, retreats to >= 5 tiles,
//          logs every member each round for 25 rounds, then every 100 rounds to round 1,990.
//   MODE 2 first mover (#2), self-play: all champions walk to the centre; each turn logs has_completed_turn of every
//          visible champion (allies and enemies) for 120 rounds.
//   MODE 3 ranged move + attack + reset (#5): Willow, Tavi, Vesper each run move-attack-reset-attack, then attack-reset-attack.
//   MODE 4 capture timing (#8), recall, teleport crystal (#6): Vesper clears a cluster group, captures, recalls, buys a crystal,
//          then tries out-of-range, ally-occupied, wall, hole, and valid destinations.
//   MODE 5 Marcella passive scope (#11): Ryth and Marcella capture gem groups; Ryth tributes first, Marcella after.
//   MODE 6 kill gold (#12), self-play: the attacking seat's Vesper kills idle defenders; logs gold per turn.
#include <cambc.h>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <variant>

using namespace cambc;
#ifndef MODE
#define MODE 1
#endif

static std::string OUT;
static void lg(const std::string& s) { OUT += "LAB " + std::to_string(MODE) + " r" + std::to_string(round_number()) + " " + s + '\n'; }
static void fl() { if (!OUT.empty()) { std::fwrite(OUT.data(), 1, OUT.size(), stdout); std::fflush(stdout); OUT.clear(); } }
static int cheb(vec2 a, vec2 b) { return static_cast<int>(a.chebyshev_distance(b)); }
static std::string P(vec2 p) { return std::to_string(p.x) + "," + std::to_string(p.y); }
static std::string nm(const champion& c) { return to_string(c.champion_name()); }
static std::string B(bool b) { return b ? "1" : "0"; }

// ---------- draft ----------
static std::vector<champion_draft> prefs() {
    using d = champion_draft;
    switch (MODE) {
        case 1: case 4: case 6: return { d::vesper_vale, d::ryth, d::ren, d::marcella, d::varg, d::vendetta };
        case 3: return { d::willow, d::tavi, d::vesper_vale, d::ryth, d::ren, d::varg };
        case 5: return { d::marcella, d::ryth, d::ren, d::varg, d::vendetta, d::willow };
        default: return { d::ryth, d::vesper_vale, d::vendetta, d::stellar, d::prism, d::ren };
    }
}
static void draft_turn() {
    auto mine = draft::current_draft(team::ally);
    for (auto c : prefs()) {
        if (std::find(mine.begin(), mine.end(), c) != mine.end()) continue;
        try { draft::pick(c); return; } catch (const std::exception&) {}
    }
}

// ---------- helpers ----------
static bool step_to(champion& me, vec2 t) {
    try { return me.move_towards(t); } catch (const std::exception& e) { lg(nm(me) + " move_err " + e.what()); return false; }
}
static std::string try_attack(champion& me, const unit& u) {
    try { me.attack(u); return "ok"; } catch (const std::exception& e) { return std::string("err:") + e.what(); }
}
static int reset_slot(const champion& me) {
    auto ab = me.abilities();
    for (int i = 0; i < 2; ++i) { try { if (ab[i].is_attack_reset()) return i; } catch (...) {} }
    return -1;
}
static std::string use_reset(champion& me) {
    int s = reset_slot(me);
    if (s < 0) return "noreset";
    auto ab = me.abilities();
    try { return std::string("ready=") + B(ab[s].is_ready()) + " use=" + B(ab[s].use()); } catch (const std::exception& e) { return std::string("err:") + e.what(); }
}
static std::string camp_line(const camp& c) {
    std::string s = to_string(c.kind()) + "@" + P(c.spawn_position());
    try {
        if (!c.is_visible()) return s + " hidden";
        if (c.is_dead()) return s + " dead";
        s += " awake=" + B(c.is_awake()) + " hp=" + std::to_string(static_cast<int>(c.health())) + "/" + std::to_string(static_cast<int>(c.max_health())) +
             " str=" + std::to_string(static_cast<int>(c.strength()));
    } catch (const std::exception& e) { s += std::string(" err:") + e.what(); }
    return s;
}
static int nearest_group(vec2 from, bool (*ok)(const map::camp_group&), int skip = 0) {
    auto gs = map::camp_groups();
    std::vector<std::pair<int, int>> cand;
    for (int i = 0; i < static_cast<int>(gs.size()); ++i) {
        if (!ok(gs[i])) continue;
        int best = 1 << 20;
        for (const auto& c : gs[i]) best = std::min(best, cheb(from, c.spawn_position()));
        cand.push_back({ best, i });
    }
    std::sort(cand.begin(), cand.end());
    if (static_cast<int>(cand.size()) <= skip) return -1;
    return cand[skip].second;
}
static bool kinds_only(const map::camp_group& g, std::initializer_list<camp_type> allowed) {
    for (const auto& c : g) if (std::find(allowed.begin(), allowed.end(), c.kind()) == allowed.end()) return false;
    return true;
}
static int count_kind(const map::camp_group& g, camp_type k) { int n = 0; for (const auto& c : g) n += c.kind() == k; return n; }
static std::string group_state(const map::camp_group& g) {
    try {
        auto st = g.state();
        if (std::holds_alternative<map::camp_group_alive>(st)) return "alive";
        if (std::holds_alternative<map::camp_group_reward_available>(st)) return "reward";
        return "respawn" + std::to_string(std::get<map::camp_group_respawning>(st).timer);
    } catch (const std::exception&) { return "unseen"; }
}
// Kill every living member of group g: attack one in range, else walk to the nearest one. Returns true once all are dead.
static bool clear_group(champion& me, const map::camp_group& g) {
    const std::string gst = group_state(g);
    if (gst == "reward" || gst.rfind("respawn", 0) == 0) return true;
    bool any = false;
    vec2 target{ -1, -1 }; int bd = 1 << 20;
    for (const auto& c : g) {
        bool dead = false;
        try { dead = c.is_dead(); } catch (...) {}
        if (dead) continue;
        any = true;
        int d = cheb(me.position(), c.spawn_position());
        if (d < bd) { bd = d; target = c.spawn_position(); }
    }
    if (!any) return true;
    try {
        auto c = map::camp(target);
        if (c.is_visible() && !c.is_dead() && me.attack_tiles().is_set(c.position())) { me.attack(c); return false; }
    } catch (...) {}
    step_to(me, target);
    return false;
}
static vec2 first_spawn_tile(const map::camp_group& g, vec2 from) {
    vec2 best{ -1, -1 }; int bd = 1 << 20;
    for (vec2 t : g.spawn_tiles().positions()) { int d = cheb(from, t); if (d < bd) { bd = d; best = t; } }
    return best;
}
static bool on_group(const map::camp_group& g, vec2 p) { return g.spawn_tiles().is_set(p); }
static vec2 centre() {   // nearest walkable tile to the geometric centre
    static vec2 c{ -1, -1 };
    if (c.x >= 0) return c;
    vec2 g(map::width() / 2, map::height() / 2); int bd = 1 << 20;
    for (int x = 0; x < map::width(); ++x) for (int y = 0; y < map::height(); ++y) {
        vec2 p(x, y);
        if (map::walls().is_set(p) || map::holes().is_set(p) || map::altars().is_set(p)) continue;
        int d = cheb(p, g); if (d < bd) { bd = d; c = p; }
    }
    return c;
}
static bool attacker_seat() {
    auto a = map::spawn_region(team::ally).positions(), e = map::spawn_region(team::enemy).positions();
    if (a.empty() || e.empty()) return true;
    return std::make_pair(a[0].y, a[0].x) < std::make_pair(e[0].y, e[0].x);
}

// ---------- modes ----------
#if MODE == 1
static void play() {
    auto& me = self();
    if (me.champion_name() != champion_type::vesper_vale) return;
    static int phase = 0, gi = -1, hit = -1; static vec2 wolf{ -1, -1 };
    auto gs = map::camp_groups();
    if (gi < 0) {
        gi = nearest_group(me.position(), [](const map::camp_group& g) { return count_kind(g, camp_type::wolf) >= 2 && kinds_only(g, { camp_type::wolf, camp_type::gold_cluster, camp_type::gem_cluster }); });
        int bd = 1 << 20;
        for (const auto& c : gs[gi]) if (c.kind() == camp_type::wolf && cheb(me.position(), c.spawn_position()) < bd) { bd = cheb(me.position(), c.spawn_position()); wolf = c.spawn_position(); }
        lg("vesper group " + std::to_string(gi) + " wolf " + P(wolf));
    }
    const auto& g = gs[gi];
    auto members = [&]() { std::string s; for (const auto& c : g) s += " | " + camp_line(c); return s; };
    const int r = static_cast<int>(round_number());
    if (phase == 0) {
        if (cheb(me.position(), wolf) > 3) { step_to(me, wolf); if (r % 5 == 0) lg("approach pos " + P(me.position()) + members()); }
        else phase = 1;
    }
    if (phase == 1) {
        lg("prehit pos " + P(me.position()) + members());
        try { auto c = map::camp(wolf); lg(std::string("hit ") + try_attack(me, c)); } catch (const std::exception& e) { lg(std::string("hit err ") + e.what()); }
        hit = r; phase = 2;
        return;
    }
    if (phase == 2) {
        int mind = 1 << 20; for (const auto& c : g) mind = std::min(mind, cheb(me.position(), c.spawn_position()));
        if (mind < 5) {
            vec2 best = me.position(); int bs = -1;
            for (int dx = -1; dx <= 1; ++dx) for (int dy = -1; dy <= 1; ++dy) {
                vec2 t(me.position().x + dx, me.position().y + dy);
                if (!map::is_in_bounds(t) || map::walls().is_set(t) || map::holes().is_set(t) || map::altars().is_set(t) || cheb(t, wolf) > 7) continue;
                int md = 1 << 20; for (const auto& c : g) md = std::min(md, cheb(t, c.spawn_position()));
                if (md > bs) { bs = md; best = t; }
            }
            if (!(best == me.position())) { try { me.move_absolute(best); } catch (const std::exception& e) { lg(std::string("retreat err ") + e.what()); } }
        }
        if (r <= hit + 25 || r % 100 == 0 || r >= 1990) lg("watch hp=" + std::to_string(static_cast<int>(me.health())) + " pos " + P(me.position()) + members());
    }
}
#elif MODE == 2
static void play() {
    auto& me = self();
    if (round_number() > 120) return;
    auto en = vision::champions(team::enemy);
    auto al = vision::champions(team::ally);
    std::string s = nm(me) + " prio=" + std::to_string(me.priority()) + " pos " + P(me.position());
    for (const auto& c : al) if (!(c == me)) s += " A:" + nm(c) + "=" + B(c.has_completed_turn());
    for (const auto& c : en) s += " E:" + nm(c) + "=" + B(c.has_completed_turn());
    lg(s);
    if (en.empty()) step_to(me, centre());
}
#elif MODE == 3
static void play() {
    auto& me = self();
    auto t = me.champion_name();
    if (t != champion_type::willow && t != champion_type::tavi && t != champion_type::vesper_vale) return;
    static int phase = 0, waited = 0; static vec2 wolf{ -1, -1 };
    if (wolf.x < 0) {
        int bd = 1 << 20;
        for (const auto& g : map::camp_groups()) for (const auto& c : g)
            if (c.kind() == camp_type::wolf && cheb(me.position(), c.spawn_position()) < bd) { bd = cheb(me.position(), c.spawn_position()); wolf = c.spawn_position(); }
    }
    if (phase > 3) return;
    camp c = map::camp(wolf);
    bool ready = false;
    try { ready = c.is_visible() && !c.is_dead() && cheb(me.position(), wolf) <= 2; } catch (...) {}
    if (phase == 0) {
        if (!ready) { step_to(me, wolf); return; }
        std::string s = nm(me) + " A: can_attack0=" + B(me.can_attack());
        bool moved = false;
        for (int dx = -1; dx <= 1 && !moved; ++dx) for (int dy = -1; dy <= 1 && !moved; ++dy) {
            vec2 n(me.position().x + dx, me.position().y + dy);
            if ((dx || dy) && cheb(n, wolf) <= 3 && cheb(n, wolf) >= 1) { try { moved = me.move_absolute(n); } catch (...) {} }
        }
        s += " moved=" + B(moved) + " can_attack1=" + B(me.can_attack()) + " atk1=" + try_attack(me, c) + " reset=" + use_reset(me) +
             " can_attack2=" + B(me.can_attack()) + " atk2=" + try_attack(me, c);
        lg(s); phase = 1; return;
    }
    if (phase == 1) {
        std::string s = nm(me) + " B: can_attack0=" + B(me.can_attack()) + " atk1=" + try_attack(me, c) + " reset=" + use_reset(me) + " atk2=" + try_attack(me, c);
        lg(s); phase = 2; return;
    }
    if (phase == 2) {
        int s = reset_slot(me);
        bool rd = false; try { rd = s >= 0 && me.abilities()[s].is_ready(); } catch (...) {}
        if (!rd && ++waited < 30) return;
        std::string o = nm(me) + " C(after " + std::to_string(waited) + "): atk1=" + try_attack(me, c) + " reset=" + use_reset(me) + " can_attack=" + B(me.can_attack()) + " atk2=" + try_attack(me, c);
        lg(o); phase = 4;
    }
}
#elif MODE == 4
static void play() {
    auto& me = self();
    if (me.champion_name() != champion_type::vesper_vale) return;
    static int phase = 0, gi = -1, arrive = -1, gold0 = 0, rec = -1, tests = 0, tp_round = -1;
    static vec2 tile{ -1, -1 }, lab{ -1, -1 };
    auto gs = map::camp_groups();
    const int r = static_cast<int>(round_number());
    if (gi < 0) {
        gi = nearest_group(me.position(), [](const map::camp_group& g) { return g.gold_value() > 0 && kinds_only(g, { camp_type::gold_cluster, camp_type::gem_cluster }); });
        lg("group " + std::to_string(gi) + " gold " + std::to_string(gs[gi].gold_value()) + " gems " + std::to_string(gs[gi].gem_value()));
    }
    const auto& g = gs[gi];
    auto st = [&]() { return "pos " + P(me.position()) + " gold=" + std::to_string(me.gold_count()) + " gems=" + std::to_string(me.gem_count()) + " grp=" + group_state(g); };
    if (phase == 0) { if (clear_group(me, g)) { phase = 1; lg("cleared " + st()); } return; }
    if (phase == 1) {
        if (tile.x < 0) tile = first_spawn_tile(g, me.position());
        if (!(me.position() == tile)) { step_to(me, tile); if (me.position() == tile) { arrive = r; gold0 = static_cast<int>(me.gold_count()); lg("arrived " + st()); } return; }
        lg("on_tile " + st());
        if (static_cast<int>(me.gold_count()) > gold0) { lg("paid arrival_round=" + std::to_string(arrive) + " paid_round=" + std::to_string(r) + " " + st()); phase = 2; }
        return;
    }
    if (phase == 2) {
        try { me.recall(); rec = r; lg("recall_start " + st()); } catch (const std::exception& e) { lg(std::string("recall err ") + e.what()); }
        phase = 3; return;
    }
    if (phase == 3) {
        bool home = map::spawn_region(team::ally).is_set(me.position());
        lg("recalling=" + B(me.is_recalling()) + " home=" + B(home) + " " + st());
        if (home) {
            lg("recall_done start=" + std::to_string(rec) + " home_round=" + std::to_string(r));
            try { me.buy(consumable_type::tp_crystal); lg("bought " + st() + " crystals=" + std::to_string(me.consumables()[consumable_type::tp_crystal].count())); }
            catch (const std::exception& e) { lg(std::string("buy err ") + e.what()); }
            phase = 4;
        } else if (!me.is_recalling() && r > rec + 8) { step_to(me, map::spawn_region(team::ally).positions()[0]); }
        return;
    }
    auto tp = [&](vec2 t, const char* what) {
        std::string res;
        try { res = B(me.consumables()[consumable_type::tp_crystal].use(t)); } catch (const std::exception& e) { res = std::string("err:") + e.what(); }
        lg(std::string("tp ") + what + " " + P(t) + " from " + P(me.position()) + " d=" + std::to_string(cheb(me.position(), t)) + " -> " + res);
        return res == "1";
    };
    if (phase == 4) {   // at spawn: out of range and ally-occupied
        vec2 far(me.position().x + 4, me.position().y); if (!map::is_in_bounds(far)) far = vec2(me.position().x - 4, me.position().y);
        tp(far, "range4");
        for (const auto& a : vision::champions(team::ally)) if (!(a == me) && cheb(a.position(), me.position()) <= 3) { tp(a.position(), "ally"); break; }
        // lab spot: nearest passable tile with a wall and a hole within 3
        int bd = 1 << 20;
        for (int x = 0; x < map::width(); ++x) for (int y = 0; y < map::height(); ++y) {
            vec2 p(x, y);
            if (map::walls().is_set(p) || map::holes().is_set(p) || map::altars().is_set(p)) continue;
            bool w = false, h = false;
            for (int dx = -3; dx <= 3; ++dx) for (int dy = -3; dy <= 3; ++dy) { vec2 q(x + dx, y + dy); if (!map::is_in_bounds(q)) continue; w |= map::walls().is_set(q); h |= map::holes().is_set(q); }
            if (w && h && cheb(p, me.position()) < bd) { bd = cheb(p, me.position()); lab = p; }
        }
        lg("lab_spot " + P(lab));
        phase = 5; return;
    }
    if (phase == 5) {
        if (!(me.position() == lab)) { step_to(me, lab); return; }
        vec2 w{ -1, -1 }, h{ -1, -1 }, ok{ -1, -1 };
        for (int dx = -3; dx <= 3; ++dx) for (int dy = -3; dy <= 3; ++dy) {
            vec2 q(lab.x + dx, lab.y + dy); if (!map::is_in_bounds(q)) continue;
            if (map::walls().is_set(q) && w.x < 0) w = q;
            if (map::holes().is_set(q) && h.x < 0) h = q;
            if (cheb(q, lab) == 3 && !map::walls().is_set(q) && !map::holes().is_set(q) && !map::altars().is_set(q) && ok.x < 0) ok = q;
        }
        if (w.x >= 0) tp(w, "wall");
        if (h.x >= 0) tp(h, "hole");
        if (ok.x >= 0 && tp(ok, "valid")) { tp_round = r; phase = 6; }
        else phase = 7;
        return;
    }
    if (phase == 6) {
        lg("channel teleporting=" + B(me.is_teleporting()) + " left=" + std::to_string(me.remaining_channel_time()) + " " + st() +
           " crystals=" + std::to_string(me.consumables()[consumable_type::tp_crystal].count()));
        if (++tests > 6) phase = 7;
    }
}
#elif MODE == 5
static void play() {
    auto& me = self();
    auto t = me.champion_name();
    if (t != champion_type::marcella && t != champion_type::ryth) return;
    if (round_number() > 700) return;
    static int phase = 0, gi = -1, seen_score = -1; static vec2 tile{ -1, -1 }, altar{ -1, -1 }, stand{ -1, -1 };
    auto gs = map::camp_groups();
    const bool is_m = t == champion_type::marcella;
    if (gi < 0) gi = nearest_group(me.position(), [](const map::camp_group& g) { return kinds_only(g, { camp_type::gem_cluster }); }, is_m ? 0 : 1);
    const auto& g = gs[gi];
    const int score = static_cast<int>(global_submitted_gems(team::ally));
    lg(nm(me) + " ph=" + std::to_string(phase) + " pos " + P(me.position()) + " gems=" + std::to_string(me.gem_count()) + " gold=" + std::to_string(me.gold_count()) +
       " team_score=" + std::to_string(score) + " submitting=" + B(me.is_submitting_gems()));
    if (phase == 0) { if (clear_group(me, g)) phase = 1; return; }
    if (phase == 1) {
        if (tile.x < 0) tile = first_spawn_tile(g, me.position());
        if (!(me.position() == tile)) { step_to(me, tile); return; }
        if (me.gem_count() > 0) phase = 2;
        return;
    }
    if (phase == 2) {
        if (altar.x < 0) {
            int bd = 1 << 20;
            for (vec2 a : map::altars().positions()) if (cheb(me.position(), a) < bd) { bd = cheb(me.position(), a); altar = a; }
            std::vector<std::pair<int, vec2>> ring;
            for (int dx = -1; dx <= 1; ++dx) for (int dy = -1; dy <= 1; ++dy) {
                vec2 q(altar.x + dx, altar.y + dy);
                if ((dx || dy) && map::is_in_bounds(q) && !map::walls().is_set(q) && !map::holes().is_set(q) && !map::altars().is_set(q))
                    ring.push_back({ dx * 3 + dy, q });
            }
            std::sort(ring.begin(), ring.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
            stand = ring.empty() ? altar : ring[(is_m && ring.size() > 1) ? 1 : 0].second;   // fixed, distinct tiles for the two champions
        }
        if (cheb(me.position(), altar) > 1) { step_to(me, stand); return; }
        phase = 3;
    }
    if (phase == 3) {
        if (is_m && score == 0) return;                         // Marcella waits for Ryth's tribute to land
        if (is_m && seen_score < 0) { seen_score = static_cast<int>(round_number()); return; }
        if (is_m && static_cast<int>(round_number()) < seen_score + 3) return;
        try { me.submit_gems(altar); lg(nm(me) + " submit_start gems=" + std::to_string(me.gem_count())); } catch (const std::exception& e) { lg(nm(me) + " submit err " + e.what()); }
        phase = 4; return;
    }
    if (phase >= 4 && ++phase > 14) phase = 99;
}
#elif MODE == 6
static void play() {
    auto& me = self();
    const bool att = attacker_seat();
    const int r = static_cast<int>(round_number());
    if (r > 400) return;
    if (!att) {   // defenders walk to the centre and wait
        if (cheb(me.position(), centre()) > 1) step_to(me, centre());
        if (r % 20 == 0) lg("def " + nm(me) + " hp=" + std::to_string(static_cast<int>(me.health())) + " pos " + P(me.position()));
        return;
    }
    if (me.champion_name() != champion_type::vesper_vale) {   // other attackers walk out of the spawn and never attack
        if (cheb(me.position(), centre()) > 4) step_to(me, centre());
        return;
    }
    auto en = vision::champions(team::enemy);
    const champion* tgt = nullptr; int bd = 1 << 20;
    for (const auto& c : en) { int d = cheb(me.position(), c.position()); if (d < bd) { bd = d; tgt = &c; } }
    std::string s = "att gold=" + std::to_string(me.gold_count()) + " pos " + P(me.position());
    if (tgt) {
        s += " tgt=" + nm(*tgt) + " hp=" + std::to_string(static_cast<int>(tgt->health())) + " d=" + std::to_string(bd);
        if (me.attack_tiles().is_set(tgt->position())) s += " atk=" + try_attack(me, *tgt); else step_to(me, tgt->position());
    } else step_to(me, centre());
    lg(s);
}
#endif

int main() {
    while (true) {
        try { if (is_draft()) draft_turn(); else play(); }
        catch (const std::exception& e) { lg(std::string("error ") + e.what()); }
        fl();
        end_turn();
    }
}
