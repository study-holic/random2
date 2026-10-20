// plan.h: per-map hooks over v4.14. Each map file (map_<map>.cpp) fills one MapPlan; an empty
// MapPlan leaves v4.14's behaviour unchanged. The old and v2 layouts of a re-laid map share one setup
// (MapPlan::id tells them apart).
#pragma once
#include <cambc.h>
#include <vector>
#include "mapid.h"

class Bot;
struct TurnView;

namespace Plan {

enum class Phase { pre, mid };
using Champs = std::vector<cambc::champion>;

struct MapPlan {
    mapid::MapId id = mapid::MapId::unknown;
    std::vector<cambc::champion_draft> draft;   // empty = v4.14's lists; first pick not yet taken is drafted
    void (*init)(Bot&) = nullptr;
    bool (*pre)(Bot&, const TurnView&, Champs& enemies, Champs& allies) = nullptr;   // true ends the turn
    bool (*mid)(Bot&, const TurnView&, Champs& enemies, Champs& allies) = nullptr;   // true ends the turn
    void (*target)(Bot&, const TurnView&, int& chosen) = nullptr;
    bool (*buy)(Bot&) = nullptr;                                                   // true skips the default
};

MapPlan& current();
const std::vector<cambc::champion_draft>* draft();
void init(Bot& b);
bool turn(Bot& b, Phase phase, const TurnView& v, Champs& enemies, Champs& allies);
void target(Bot& b, const TurnView& v, int& chosen);
bool buy(Bot& b);

}  // namespace Plan
