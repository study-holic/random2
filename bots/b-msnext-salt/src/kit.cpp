// Champion-kit layer: the static kit table (role, default main stat, flags) and kit actions that the profile switches on.
// Willow heal is reachable only through prof().willow_heal; kit_dash_escape is a primitive no decision code calls yet.
#include <cambc.h>
#include "profile.h"
#include "util.h"
#include "params.h"
#include <algorithm>
#include <utility>
#include <vector>
#if P_KIT_DBG
#include <iostream>
#endif

using CT = cambc::champion_type;
using cambc::vec2;

bool kit_willow_heal() {
    auto& me = cambc::self();
    if (!me.is_alive() || me.champion_name() != CT::willow || !me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_heal() || !ab.is_ready() || !ab.can_target_allies()) continue;
        const auto allies = cambc::vision::champions(cambc::team::ally);
        const vec2 pos = me.position();
        const auto reach = ab.valid_tiles();
        const cambc::champion* best = nullptr;
        float best_frac = prof().willow_heal_below;
        for (const auto& a : allies) {
            if (a == me || !a.is_visible() || !a.is_alive()) continue;
            if (tile_distance(pos, a.position()) > prof().willow_heal_range || !reach.is_set(a.position())) continue;
            const float frac = a.health() / a.max_health();
            if (frac < best_frac) { best_frac = frac; best = &a; }
        }
        if (best == nullptr) return false;
        bool ok = false;
        try { ok = ab.use(*best); } catch (...) { ok = false; }
#if P_KIT_DBG
        std::cout << "KIT heal r" << cambc::round_number() << " frac" << best_frac << " ok" << ok << std::endl;
#endif
        return ok;
    }
    return false;
}

bool kit_dash_escape(vec2 threat, const cambc::bitboard& blocked_static, const std::vector<vec2>& home) {
    auto& me = cambc::self();
    if (!me.is_alive() || !me.can_move() || !me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_dash() || !ab.is_ready()) continue;
        const vec2 pos = me.position();
        const auto blocked = blocked_static | cambc::vision::champion_tiles() | cambc::vision::camp_tiles();
        const int here = tile_distance(pos, threat);
        std::vector<std::pair<int, vec2>> options;   // (score, tile): far from the threat, near home
        for (auto t : ab.valid_tiles().positions()) {
            if (t == pos || blocked.is_set(t)) continue;
            const int d = tile_distance(t, threat);
            if (d <= here) continue;
            options.push_back({4 * d - (home.empty() ? 0 : min_distance(t, home)), t});
        }
        std::stable_sort(options.begin(), options.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
        for (std::size_t i = 0; i < options.size() && i < 3; ++i) {
            bool ok = false;
            try { ok = ab.use(options[i].second); } catch (...) { ok = false; }
#if P_KIT_DBG
            std::cout << "KIT dash r" << cambc::round_number() << " to " << options[i].second.x << "," << options[i].second.y << " ok" << ok << std::endl;
#endif
            if (ok) return true;
        }
        return false;
    }
    return false;
}
