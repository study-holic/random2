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

// S2 per-map draft exceptions to the core (admitted only where a variant beat the core by >= 2 of 6 on the map and the
// overall head-to-head did not drop; results/v5-s2.md). nullptr = core.
static const std::vector<D>* s2_draft_exception(mapid::MapId id, int size) {
    (void)id; (void)size;
#if defined(P_S2_DRAFT_VARIANT) && P_S2_DRAFT_VARIANT == 1   // test build: Tavi instead of Ren (top-10 second set)
    static const std::vector<D> v5{D::ryth, D::cassi_and_issac, D::stellar, D::varg, D::tavi};
    static const std::vector<D> v3{D::cassi_and_issac, D::stellar, D::varg};
    return size <= 3 ? &v3 : &v5;
#elif defined(P_S2_DRAFT_VARIANT) && P_S2_DRAFT_VARIANT == 2   // test build: core order with Varg before Ren on 4 picks
    static const std::vector<D> v4{D::ryth, D::cassi_and_issac, D::stellar, D::varg, D::ren};
    return size == 4 ? &v4 : nullptr;
#endif
    return nullptr;
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
    if (P_S2_DRAFT_CORE) {   // S2 task 1: top-10 core by pick count (notes/top10-2026-10-20.md l.49-51); duplicates across
        // teams are allowed and picks within a round are simultaneous, so a fixed table is the whole policy. Overrides the
        // map files and w7 lists; s2_draft_exception() returns a per-map list only where a test admitted one.
        static const std::vector<D> core5{D::ryth, D::cassi_and_issac, D::stellar, D::ren, D::varg};
        static const std::vector<D> core3{D::cassi_and_issac, D::stellar, D::ren};
        const std::vector<D>* ex = s2_draft_exception(Plan::current().id, size);
        const auto& core = ex ? *ex : (size <= 3 ? core3 : core5);   // 4 picks = the first 4 of core5
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
