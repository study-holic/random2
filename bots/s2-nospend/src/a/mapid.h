// mapid.h: identifies the 17 maps of the 18 October pool (plus the six originals they replaced)
// from dimensions and camp composition. Usable in the draft instance and in play instances.
// current_map() calls camp_groups() once (12k to 20k fuel) and caches the result.
#pragma once
#include <cambc.h>
#include <vector>

namespace mapid {

enum class MapId {
    unknown, rows, monolith, contest, elkring, fragment,
    collision_old, collision_v2, corners_old, corners_v2, expanse_old, expanse_v2,
    interlock_old, interlock_v2, lanes_old, lanes_v2, packs_old, packs_v2,
    eight, ambush, pilgrimage, rushbait, stampede, swirl,
    big_world, boss_battle, broken_curve, concentrated_wealth, corridors, elk_arena, fracture, gold_rush, hooks, islands
};

inline const char* map_name(MapId id) {
    switch (id) {
        case MapId::rows: return "rows"; case MapId::monolith: return "monolith"; case MapId::contest: return "contest";
        case MapId::elkring: return "elkring"; case MapId::fragment: return "fragment";
        case MapId::collision_old: return "collision_old"; case MapId::collision_v2: return "collision";
        case MapId::corners_old: return "corners_old"; case MapId::corners_v2: return "corners";
        case MapId::expanse_old: return "expanse_old"; case MapId::expanse_v2: return "expanse";
        case MapId::interlock_old: return "interlock_old"; case MapId::interlock_v2: return "interlock";
        case MapId::lanes_old: return "lanes_old"; case MapId::lanes_v2: return "lanes";
        case MapId::packs_old: return "packs_old"; case MapId::packs_v2: return "packs_v2";
        case MapId::eight: return "eight"; case MapId::ambush: return "ambush"; case MapId::pilgrimage: return "pilgrimage";
        case MapId::rushbait: return "rushbait"; case MapId::stampede: return "stampede"; case MapId::swirl: return "swirl";
        case MapId::big_world: return "big_world"; case MapId::boss_battle: return "boss_battle";
        case MapId::broken_curve: return "broken_curve"; case MapId::concentrated_wealth: return "concentrated_wealth";
        case MapId::corridors: return "corridors"; case MapId::elk_arena: return "elk_arena"; case MapId::fracture: return "fracture";
        case MapId::gold_rush: return "gold_rush"; case MapId::hooks: return "hooks"; case MapId::islands: return "islands";
        default: return "unknown";
    }
}

struct Counts { int wolf = 0, elk = 0, gem = 0, gold = 0, mono = 0, packs3 = 0; };

inline Counts count_camps() {
    Counts c;
    for (auto g : cambc::map::camp_groups()) {
        int w = 0;
        for (const auto& camp : g.camps()) {
            if (camp.is_wolf()) { ++c.wolf; ++w; }
            else if (camp.is_elk()) ++c.elk;
            else if (camp.is_gem_cluster()) ++c.gem;
            else if (camp.is_gold_cluster()) ++c.gold;
            else if (camp.is_monolith()) ++c.mono;
        }
        if (w >= 3) ++c.packs3;
    }
    return c;
}

// identify(): exact identification of the 21 maps of the engine 1.0 pool (SDK 3.0.1 probe, 20 Oct).
// Switch ID_E1 (default 1): 1 = engine 1.0 table (dimensions; camp-group count only where two maps share a
// size). No retired-map id is returned (v3b table removed: it was wrong on engine 1.0).
// FIX_MAPID_EXACT (P_MAPID_EXACT): a map is identified only by its exact (width, height, camp_groups().size()); any other
// map, including a new pool map that shares a size with a known one, is unknown and gets the safe default plan.
// Group counts read from the engine (the key= print beside mapid; gate run 20 Oct, all 21 maps).
#ifndef P_MAPID_EXACT
#define P_MAPID_EXACT 1   // 0 = size-based rules (v4 as tested in the cloud), 1 = exact table
#endif
inline MapId identify_e1(int W, int H, int ngroups) {
#if P_MAPID_EXACT
    struct Entry { int w, h, g; MapId id; };
    static const Entry table[] = {
        {100, 100, 12, MapId::ambush},       {100, 100, 24, MapId::big_world},           {50, 41, 31, MapId::boss_battle},
        {40, 40, 22, MapId::broken_curve},   {41, 41, 20, MapId::collision_v2},         {61, 61, 11, MapId::concentrated_wealth},
        {50, 50, 14, MapId::contest},        {71, 71, 30, MapId::corners_v2},           {51, 40, 12, MapId::corridors},
        {41, 30, 8, MapId::eight},           {50, 50, 29, MapId::elk_arena},            {30, 30, 10, MapId::elkring},
        {80, 80, 24, MapId::expanse_v2},     {61, 71, 28, MapId::fracture},             {30, 30, 6, MapId::fragment},
        {31, 31, 15, MapId::gold_rush},      {31, 31, 12, MapId::hooks},                {40, 40, 14, MapId::interlock_v2},
        {60, 60, 46, MapId::islands},        {90, 30, 42, MapId::lanes_v2},             {50, 50, 20, MapId::monolith},
    };
    for (const Entry& e : table)
        if (e.w == W && e.h == H && e.g == ngroups) return e.id;
    return MapId::unknown;
#else
    // ngroups = -1 when not needed
    if (W == 50 && H == 50) return ngroups == 14 ? MapId::contest : ngroups == 20 ? MapId::monolith : MapId::elk_arena;
    if (W == 100 && H == 100) return ngroups == 12 ? MapId::ambush : MapId::big_world;
    if (W == 30 && H == 30) return ngroups == 10 ? MapId::elkring : MapId::fragment;
    if (W == 40 && H == 40) return ngroups == 14 ? MapId::interlock_v2 : MapId::broken_curve;
    if (W == 31 && H == 31) return ngroups == 15 ? MapId::gold_rush : MapId::hooks;
    if (W == 41 && H == 41) return MapId::collision_v2;
    if (W == 71 && H == 71) return MapId::corners_v2;
    if (W == 80 && H == 80) return MapId::expanse_v2;
    if (W == 90 && H == 30) return MapId::lanes_v2;
    if (W == 41 && H == 30) return MapId::eight;
    if (W == 50 && H == 41) return MapId::boss_battle;
    if (W == 61 && H == 61) return MapId::concentrated_wealth;
    if (W == 51 && H == 40) return MapId::corridors;
    if (W == 61 && H == 71) return MapId::fracture;
    if (W == 60 && H == 60) return MapId::islands;
    return MapId::unknown;
#endif
}
// One camp_groups() load per process (12k to 40k fuel on the large maps); Bot::init reuses it.
inline const std::vector<cambc::map::camp_group>& cached_groups() {
    static const std::vector<cambc::map::camp_group> g = cambc::map::camp_groups();
    return g;
}
inline bool needs_groups(int W, int H) {
    if (P_MAPID_EXACT) return true;
    return (W == 50 && H == 50) || (W == 100 && H == 100) || (W == 30 && H == 30) || (W == 40 && H == 40) || (W == 31 && H == 31);
}

inline MapId current_map() {
    static MapId cached = [] {
        try {
            const int W = cambc::map::width(), H = cambc::map::height();
            const int ng = needs_groups(W, H) ? static_cast<int>(cached_groups().size()) : -1;
            return identify_e1(W, H, ng);
        }
        catch (...) { return MapId::unknown; }
    }();
    return cached;
}

inline bool is_contact_map(MapId id) {
    switch (id) {
        case MapId::collision_old: case MapId::collision_v2: case MapId::corners_old: case MapId::corners_v2:
        case MapId::interlock_old: case MapId::interlock_v2: case MapId::packs_old: case MapId::packs_v2:
        case MapId::elkring: case MapId::monolith: case MapId::fragment: case MapId::eight: case MapId::swirl:
        case MapId::rushbait: case MapId::stampede: case MapId::ambush: case MapId::pilgrimage: case MapId::contest:
            return true;
        default: return false;
    }
}

} // namespace mapid
