// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
namespace va {
// map_swirl.cpp: swirl plan. W2 death loop: the corner groups near (12,3) and its 180-degree mirror (32,26) killed
// v1 champions every ~100 rounds against starter-cpp (respawn, walk back, die again; camp kills destroy the bag).
// The groups are excluded from targeting for the whole game (resolve_target and choose_target skip excluded_until).
#include "plan.h"
#include "bot.h"

namespace Plan {

static bool near_corner(cambc::vec2 t) {
    const int W = cambc::map::width(), H = cambc::map::height();
    auto cheb = [](int ax, int ay, int bx, int by) { return std::max(std::abs(ax - bx), std::abs(ay - by)); };
    return cheb(t.x, t.y, 12, 3) <= 2 || cheb(t.x, t.y, W - 1 - 12, H - 1 - 3) <= 2;
}

static void swirl_init(Bot& b) {
    for (auto& g : b.groups) {
        bool hit = false;
        for (auto t : g.tiles) if (near_corner(t)) { hit = true; break; }
        if (hit) g.excluded_until = 1 << 30;
    }
}

static void swirl_target(Bot& b, const TurnView& v, int& chosen) {
    (void)v;
    if (chosen >= 0 && chosen < static_cast<int>(b.groups.size()) && b.groups[chosen].excluded_until > (1 << 29)) chosen = -1;
}

void setup_swirl(MapPlan& p) { p.init = swirl_init; p.target = swirl_target; }

}  // namespace Plan
} // namespace va
