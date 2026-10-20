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
// Draft: fixed preference lists taken from the top-leaderboard rosters (docs/opponents/top-runs-2026-10-13.md).
#include <cambc.h>
#include "draft.h"
#include "params.h"
#include "plan.h"
#include "counter.h"
#include <algorithm>
#include <vector>

using D = cambc::champion_draft;

// P_S2_DRAFT_KEEPV4: maps where v4's own draft beat the core by >= 2 of 6 in the forced-draft head-to-head with play code
// unchanged (results/s2d-vs-v4.json, core <= 1/6 on boss_battle, corners, interlock, elk_arena, gold_rush, islands), then
// re-tested with locks, gate and spending on (results/s2-keep6-vs-v4.json vs s2-allcore-vs-v4.json, wins of 6, v4 draft vs
// core): boss_battle 3-0, gold_rush 3-1 kept; interlock 4-3, elk_arena 6-5, corners 6-6 below the +2 bar; islands 2-6 reverted.
static bool s2_keep_v4_draft(mapid::MapId id) {
    if (!P_S2_DRAFT_KEEPV4) return false;
    using M = mapid::MapId;
    switch (id) {
        case M::boss_battle: case M::gold_rush: return true;
        default: return false;
    }
}

void play_draft() {
    const auto mine = cambc::draft::current_draft(cambc::team::ally);
    const auto size = cambc::draft::team_size();
    const std::vector<D> small{D::vendetta, D::cassi_and_issac, D::stellar, D::ryth, D::varg, D::tavi};
    const std::vector<D> large{D::vendetta, D::cassi_and_issac, D::varg, D::ryth, D::tavi, D::ren, D::marcella, D::stellar};
    std::vector<D> rows_prefs{D::cassi_and_issac, D::varg, D::tavi, D::stellar, D::ryth, D::vendetta};
    bool rows = false;
    try { if (P_ROWS_CMD && cambc::map::width() == 50 && cambc::map::height() == 50) {   // EXP-093: six wolf packs, no Elk
        int packs = 0, total = 0; bool elk = false;
        for (auto g : cambc::map::camp_groups()) {
            int w = 0; ++total;
            for (const auto& c : g.camps()) { if (c.is_elk()) elk = true; if (c.is_wolf()) ++w; }
            if (w >= 3) ++packs;
        }
        rows = !elk && packs == 6;   // EXP-096b: monolith has six packs among 22 groups
    } } catch (...) { rows = false; }
    if (P_S2_DRAFT_CORE && !s2_keep_v4_draft(Plan::current().id)) {   // S2 task 1: top-10 core by pick count (notes/top10-2026-10-20.md l.49-51); duplicates across
        // teams are allowed and picks within a round are simultaneous, so a fixed table is the whole policy. Overrides the
        // map files and w7 lists, except where s2_keep_v4_draft() keeps v4's draft.
        static const std::vector<D> core5{D::ryth, D::cassi_and_issac, D::stellar, D::ren, D::varg};
        static const std::vector<D> core3{D::cassi_and_issac, D::stellar, D::ren};
        const auto& core = size <= 3 ? core3 : core5;   // 4 picks = the first 4 of core5
        for (auto pick : core)
            if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    }
    const auto* plan = Plan::draft();   // a map file's draft overrides the lists
    const auto& prefs = plan ? *plan : (rows ? rows_prefs : (size <= 3 ? small : large));
    D counter_choice{};   // w7 part 2: data-driven counter at our last pick (counter.h), per-map flag
    if (Plan::current().counter && w7cd::pick(Plan::current().id, mine, prefs, size, counter_choice)) {
        cambc::draft::pick(counter_choice); return;
    }
    for (auto pick : prefs)
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    for (int i = 0; i < 11; ++i) {
        auto pick = static_cast<D>(i);
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    }
}
} // namespace va
