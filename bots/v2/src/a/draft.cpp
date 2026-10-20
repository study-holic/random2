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
#include <variant>
#include <vector>
namespace va {
// Draft: fixed preference lists taken from the top-leaderboard rosters (docs/opponents/top-runs-2026-10-13.md).
#include <cambc.h>
#include "draft.h"
#include "params.h"
#include "plan.h"
#include <algorithm>
#include <vector>

using D = cambc::champion_draft;

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
    const auto* plan = Plan::draft();   // a map file's draft overrides the lists
    const auto& prefs = plan ? *plan : (rows ? rows_prefs : (size <= 3 ? small : large));
    for (auto pick : prefs)
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    for (int i = 0; i < 11; ++i) {
        auto pick = static_cast<D>(i);
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    }
}
} // namespace va
