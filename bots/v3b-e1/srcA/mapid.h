// mapid.h: identifies the 17 maps of the 18 October pool (plus the six originals they replaced)
// from dimensions and camp composition. Usable in the draft instance and in play instances.
// current_map() calls camp_groups() once (12k to 20k fuel) and caches the result.
#pragma once
#include <cambc.h>

namespace mapid {

enum class MapId {
    unknown, rows, monolith, contest, elkring, fragment,
    collision_old, collision_v2, corners_old, corners_v2, expanse_old, expanse_v2,
    interlock_old, interlock_v2, lanes_old, lanes_v2, packs_old, packs_v2,
    eight, ambush, pilgrimage, rushbait, stampede, swirl
};

inline const char* map_name(MapId id) {
    switch (id) {
        case MapId::rows: return "rows"; case MapId::monolith: return "monolith"; case MapId::contest: return "contest";
        case MapId::elkring: return "elkring"; case MapId::fragment: return "fragment";
        case MapId::collision_old: return "collision_old"; case MapId::collision_v2: return "collision_v2";
        case MapId::corners_old: return "corners_old"; case MapId::corners_v2: return "corners_v2";
        case MapId::expanse_old: return "expanse_old"; case MapId::expanse_v2: return "expanse_v2";
        case MapId::interlock_old: return "interlock_old"; case MapId::interlock_v2: return "interlock_v2";
        case MapId::lanes_old: return "lanes_old"; case MapId::lanes_v2: return "lanes_v2";
        case MapId::packs_old: return "packs_old"; case MapId::packs_v2: return "packs_v2";
        case MapId::eight: return "eight"; case MapId::ambush: return "ambush"; case MapId::pilgrimage: return "pilgrimage";
        case MapId::rushbait: return "rushbait"; case MapId::stampede: return "stampede"; case MapId::swirl: return "swirl";
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

// Fingerprints from the SDK probe of the 17 supplied files (cambc 2.2.2) and, for the
// original six, the full-map counts in evidence section 05.2.
inline MapId identify(int W, int H, const Counts& c) {
    if (W == 50 && H == 50) {
        if (c.mono > 0 && c.wolf == 0) return MapId::contest;
        if (c.elk == 0 && c.packs3 == 6) return MapId::rows;
        if (c.mono > 0) return MapId::monolith;
        return MapId::unknown;
    }
    if (W == 30 && H == 30) return (c.elk >= 10 && c.wolf == 0) ? MapId::elkring : MapId::fragment;
    if (W == 41 && H == 41) return c.gem > 0 ? MapId::collision_old : MapId::collision_v2;
    if (W == 71 && H == 71) return c.gem >= 6 ? MapId::corners_old : MapId::corners_v2;
    if (W == 80 && H == 80) return c.gem >= 20 ? MapId::expanse_old : MapId::expanse_v2;
    if (W == 40 && H == 40) return c.gem > 0 ? MapId::interlock_old : MapId::interlock_v2;
    if (W == 90 && H == 30) return c.gem > 0 ? MapId::lanes_old : MapId::lanes_v2;
    if (W == 31 && H == 31) return c.elk == 0 ? MapId::packs_old : MapId::packs_v2;
    if (W == 41 && H == 30) return MapId::eight;
    if (W == 100 && H == 100) return MapId::ambush;
    if (W == 50 && H == 65) return MapId::pilgrimage;
    if (W == 30 && H == 80) return MapId::rushbait;
    if (W == 60 && H == 50) return MapId::stampede;
    if (W == 45 && H == 30) return MapId::swirl;
    return MapId::unknown;
}

inline MapId current_map() {
    static MapId cached = [] {
        try { return identify(cambc::map::width(), cambc::map::height(), count_camps()); }
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
