// census: prints engine values the map readers miss, as JSON lines prefixed "CENSUS ".
// Draft: each turn picks the lowest-id champion still free (deterministic), and the first turn prints the map record with team_size.
// Play: every champion prints its roster line on its first turn; the lowest-id ally then works through a fuel-budgeted queue:
// settings and passive constants, every camp group with per-camp base_stats, the 13 champion designs with abilities,
// every item, every consumable, then a "done" record.
#include <cambc.h>
#include <cstdio>
#include <string>
#include <variant>
#include <algorithm>
#include <vector>

using namespace cambc;

static std::string buf;
static void emit(const std::string& s) { buf += "CENSUS "; buf += s; buf += '\n'; }
static void flush() { if (!buf.empty()) { std::fwrite(buf.data(), 1, buf.size(), stdout); std::fflush(stdout); buf.clear(); } }
static std::string f(double v) { char t[32]; std::snprintf(t, sizeof t, "%.6g", v); return t; }
static std::string q(const std::string& s) { return "\"" + s + "\""; }
static std::string tiles(const bitboard& b) {
    std::string s = "[";
    bool first = true;
    for (vec2 p : b.positions()) { if (!first) s += ","; first = false; s += "[" + std::to_string(p.x) + "," + std::to_string(p.y) + "]"; }
    return s + "]";
}
static std::string camp_stats_json(const camp_stats& c) {
    return "{\"vision\":" + f(c.vision_radius) + ",\"move\":" + std::to_string(c.base_move_speed) + ",\"range\":" + std::to_string(c.base_attack_range) +
           ",\"hp\":" + f(c.base_health) + ",\"hp_g\":" + f(c.health_growth) + ",\"str\":" + f(c.base_strength) + ",\"str_g\":" + f(c.strength_growth) +
           ",\"foc\":" + f(c.base_focus) + ",\"foc_g\":" + f(c.focus_growth) + "}";
}
static std::string champ_stats_json(const champion_stats& c) {
    return "{\"vision\":" + std::to_string(c.vision_radius) + ",\"move\":" + std::to_string(c.base_move_speed) + ",\"range\":" + std::to_string(c.base_attack_range) +
           ",\"hp\":" + f(c.base_health) + ",\"hp_g\":" + f(c.health_growth) + ",\"str\":" + f(c.base_strength) + ",\"str_g\":" + f(c.strength_growth) +
           ",\"foc\":" + f(c.base_focus) + ",\"foc_g\":" + f(c.focus_growth) + ",\"camp_mult\":" + f(c.camp_damage_multiplier) + "}";
}
static std::string props_json(const ability_properties& p) {
    return "\"cd\":" + std::to_string(p.max_cooldown) + ",\"targets\":" + std::to_string(static_cast<std::uint32_t>(p.valid_targets.bits())) +
           ",\"excl_move\":" + (p.exclusive_with_movement ? "true" : "false");
}
static std::string ability_json(const ability& a) {
    try {
        const auto& oi = a.info();
        if (!oi) return "null";
        return std::visit([](const auto& x) -> std::string {
            using T = std::decay_t<decltype(x)>;
            std::string s = "{\"kind\":" + q(to_string(T::kind)) + "," + props_json(x);
            if constexpr (std::is_same_v<T, ability_attack_reset>) s += ",\"enhanced\":" + std::string(x.enhanced_attack ? "true" : "false");
            if constexpr (requires { x.range; }) s += ",\"range\":" + f(x.range);
            if constexpr (requires { x.modifier; }) s += ",\"mod\":" + f(x.modifier);
            if constexpr (requires { x.radius; }) s += ",\"radius\":" + f(x.radius);
            if constexpr (requires { x.duration; }) s += ",\"dur\":" + std::to_string(x.duration);
            return s + "}";
        }, *oi);
    } catch (const std::exception& e) { return "{\"error\":" + q(e.what()) + "}"; }
}

static void settings_rec() {
    namespace k = constants;
    emit("{\"rec\":\"settings\",\"capture_time\":" + std::to_string(k::capture_time) + ",\"camp_sleep_time\":" + std::to_string(k::camp_sleep_time) +
         ",\"respawn_time\":" + std::to_string(k::respawn_time) + ",\"recall_time\":" + std::to_string(k::recall_time) +
         ",\"submit_mult\":" + f(k::submit_gem_duration_multiplier) + ",\"submit_base\":" + std::to_string(k::submit_gem_base_duration) +
         ",\"submit_min\":" + std::to_string(k::submit_gem_minimum_duration) + ",\"submit_range\":" + std::to_string(k::submit_gems_range) +
         ",\"max_rounds\":" + std::to_string(k::max_rounds) + ",\"winning_gems\":" + std::to_string(k::winning_gems) +
         ",\"kill_gold\":" + std::to_string(k::kill_gold) + ",\"assist_gold\":" + std::to_string(k::assist_gold) + ",\"passive_gold\":" + std::to_string(k::passive_gold) +
         ",\"spawn_healing\":" + f(k::spawn_healing) + ",\"active_wards\":" + std::to_string(k::active_wards) +
         ",\"shared_memory_bytes\":" + std::to_string(k::shared_memory_bytes) + ",\"draft_memory_bytes\":" + std::to_string(k::draft_memory_bytes) +
         ",\"tile_reveal_rounds\":" + std::to_string(k::tile_reveal_rounds) + ",\"ward_cooldown\":" + std::to_string(k::ward_cooldown) +
         ",\"ward_lifetime\":" + std::to_string(k::ward_lifetime) + ",\"ward_vision_radius\":" + f(k::ward_vision_radius) +
         ",\"ward_placement_radius\":" + f(k::ward_placement_radius) + ",\"altar_lock_hits\":" + std::to_string(k::altar_lock_hits) +
         ",\"tp_crystal_time\":" + std::to_string(k::tp_crystal_time) +
         ",\"ward_charges\":" + std::to_string(detail::cached_game_settings.ward_charges) +
         ",\"control_ward_vision_radius\":" + f(detail::cached_game_settings.control_ward_vision_radius) +
         ",\"control_ward_placement_radius\":" + f(detail::cached_game_settings.control_ward_placement_radius) + "}");
    namespace pa = constants::passive_abilities;
    emit("{\"rec\":\"passive_constants\",\"prism_focus_per_gem\":" + f(pa::prism_focus_per_gem) + ",\"varg_splash_radius\":" + f(pa::varg_splash_radius) +
         ",\"vesper_bonus_gold_percent\":" + std::to_string(pa::vesper_vale_bonus_gold_percent) + ",\"marcella_gold_per_gem\":" + std::to_string(pa::marcella_tribute_gold_per_gem) +
         ",\"ryth_health_strength_ratio\":" + f(pa::ryth_health_strength_ratio) + ",\"ren_retaliation_focus_ratio\":" + f(pa::ren_retaliation_focus_ratio) +
         ",\"heart_of_ruin_missing_health_damage_max_bonus\":" + f(pa::heart_of_ruin_missing_health_damage_max_bonus) + "}");
}
static void map_rec() {
    emit("{\"rec\":\"map\",\"width\":" + std::to_string(map::width()) + ",\"height\":" + std::to_string(map::height()) +
         ",\"symmetry\":" + q(to_string(map::symmetry())) + ",\"team_size\":" + std::to_string(draft::team_size()) +
         ",\"altars\":" + tiles(map::altars()) + ",\"walls\":" + std::to_string(map::walls().count()) + ",\"holes\":" + std::to_string(map::holes().count()) +
         ",\"bushes\":" + std::to_string(map::bushes().count()) + "}");
}
static void group_rec(int gi, const map::camp_group& g) {
    std::string camps = "[";
    bool first = true;
    for (const auto& c : g) {
        if (!first) camps += ",";
        first = false;
        vec2 p = c.spawn_position();
        std::string bs;
        try { bs = camp_stats_json(c.base_stats()); } catch (const std::exception& e) { bs = "{\"error\":" + q(e.what()) + "}"; }
        camps += "{\"kind\":" + q(to_string(c.kind())) + ",\"pos\":[" + std::to_string(p.x) + "," + std::to_string(p.y) + "],\"base\":" + bs + "}";
    }
    camps += "]";
    emit("{\"rec\":\"group\",\"i\":" + std::to_string(gi) + ",\"gems\":" + std::to_string(g.gem_value()) + ",\"gold\":" + std::to_string(g.gold_value()) +
         ",\"respawn\":" + std::to_string(g.max_respawn_timer()) + ",\"spawn_tiles\":" + tiles(g.spawn_tiles()) + ",\"camps\":" + camps + "}");
}
static void champion_rec(int t) {
    const auto& ch = champion_designs[t];
    auto ab = ch.abilities();
    emit("{\"rec\":\"champion\",\"type\":" + q(to_string(ch.champion_name())) + ",\"passive\":" + q(to_string(ch.passive())) +
         ",\"base\":" + champ_stats_json(ch.base_stats()) + ",\"a1\":" + ability_json(ab[0]) + ",\"a2\":" + ability_json(ab[1]) + "}");
}
static void item_rec(std::uint32_t i) {
    item_type t{ i };
    try {
        auto in = t.info();
        emit("{\"rec\":\"item\",\"type\":" + q(to_string(t)) + ",\"cost\":" + std::to_string(in.cost) + ",\"combine\":" + std::to_string(in.combine_count) +
             ",\"result\":" + (in.result ? q(to_string(*in.result)) : std::string("null")) + ",\"hp\":" + std::to_string(in.health_points) +
             ",\"str\":" + std::to_string(in.strength_points) + ",\"foc\":" + std::to_string(in.focus_points) +
             ",\"passive\":" + (in.passive ? q(to_string(*in.passive)) : std::string("null")) + ",\"mag\":" + f(in.passive_magnitude) +
             ",\"buyable\":" + (in.purchasable ? "true" : "false") + "}");
    } catch (const std::exception& e) { emit("{\"rec\":\"item\",\"i\":" + std::to_string(i) + ",\"error\":" + q(e.what()) + "}"); }
}
static void consumable_rec(std::uint32_t i) {
    consumable_type t{ i };
    auto in = t.info();
    emit("{\"rec\":\"consumable\",\"type\":" + q(to_string(t)) + ",\"cost\":" + std::to_string(in.cost) + ",\"carry\":" + std::to_string(in.carry_limit) + "}");
}

static constexpr std::uint64_t RESERVE = 60000;
static void run_queue(int& next) {
    static std::vector<map::camp_group> groups;
    static bool loaded = false;
    if (!loaded) { groups = map::camp_groups(); loaded = true; }
    const int G = static_cast<int>(groups.size());
    const int NI = static_cast<int>(item_type::enum_type::count), NC = static_cast<int>(consumable_type::enum_type::count);
    const int total = 1 + G + 13 + NI + NC;
    while (next < total && fuel_remaining() > RESERVE) {
        const int i = next++;
        if (i == 0) settings_rec();
        else if (i <= G) group_rec(i - 1, groups[i - 1]);
        else if (i <= G + 13) champion_rec(i - 1 - G);
        else if (i <= G + 13 + NI) item_rec(static_cast<std::uint32_t>(i - 1 - G - 13));
        else consumable_rec(static_cast<std::uint32_t>(i - 1 - G - 13 - NI));
        flush();
    }
    if (next == total) {
        emit("{\"rec\":\"done\",\"groups\":" + std::to_string(G) + ",\"round\":" + std::to_string(round_number()) + "}");
        flush();
        next = total + 1;
    }
}

static void draft_turn() {
    static bool map_done = false;
    auto mine = draft::current_draft(team::ally);
    auto theirs = draft::current_draft(team::enemy);
    for (int i = 0; i < 12; ++i) {
        auto c = static_cast<champion_draft>(i);
        if (std::find(mine.begin(), mine.end(), c) != mine.end()) continue;
        if (std::find(theirs.begin(), theirs.end(), c) != theirs.end()) continue;
        try {
            draft::pick(c);
            emit("{\"rec\":\"pick\",\"champ\":" + q(to_string(c)) + ",\"ally_so_far\":" + std::to_string(mine.size()) + ",\"fuel\":" + std::to_string(fuel_remaining()) + "}");
            break;
        } catch (const std::exception& e) { emit("{\"rec\":\"pick_error\",\"what\":" + q(e.what()) + "}"); }
    }
    if (!map_done) { map_done = true; map_rec(); }
    flush();
}

static void play_turn() {
    static bool intro = false;
    static int next = 0;
    auto& me = self();
    if (!intro) {
        intro = true;
        auto allies = vision::champions(team::ally);
        std::string names = "[";
        int lowest = 1 << 30;
        for (std::size_t i = 0; i < allies.size(); ++i) {
            if (i) names += ",";
            names += q(to_string(allies[i].champion_name()));
            lowest = std::min(lowest, static_cast<int>(allies[i].champion_name()));
        }
        names += "]";
        emit("{\"rec\":\"play\",\"round\":" + std::to_string(round_number()) + ",\"self\":" + q(to_string(me.champion_name())) +
             ",\"allies_visible\":" + std::to_string(allies.size()) + ",\"ally_names\":" + names + ",\"fuel\":" + std::to_string(fuel_remaining()) +
             ",\"spawn_ally\":" + tiles(map::spawn_region(team::ally)) + ",\"spawn_enemy\":" + tiles(map::spawn_region(team::enemy)) + "}");
        flush();
        if (static_cast<int>(me.champion_name()) != lowest) next = 1 << 30;   // the lowest-id ally alone runs the queue
    }
    run_queue(next);
}

int main() {
    while (true) {
        try {
            if (is_draft()) draft_turn(); else play_turn();
        } catch (const std::exception& e) {
            emit(std::string("{\"rec\":\"error\",\"what\":") + q(e.what()) + "}");
            flush();
        }
        end_turn();
    }
}
