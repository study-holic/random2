// Draft: fixed preference lists taken from the top-leaderboard rosters (docs/opponents/top-runs-2026-10-13.md).
#include <cambc.h>
#include "draft.h"
#include "params.h"
#include <algorithm>
#include <vector>

using D = cambc::champion_draft;

void play_draft() {
    const auto mine = cambc::draft::current_draft(cambc::team::ally);
    const auto size = cambc::draft::team_size();
    const std::vector<D> small{D::cassi_and_issac, D::tavi, D::stellar, D::ryth, D::varg, D::vendetta};   // v13: two bodies for one pick on small rosters, then the ladder core
    const std::vector<D> large{D::tavi, D::stellar, D::ryth, D::varg, D::ren, D::marcella, D::vendetta, D::cassi_and_issac};   // v12
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
    // EXP-105: interlock (40x40, four picks) drops Cassi and Issac (death share per contact 48% and 43% against 14% and 17% for top teams);
    // Varg and Ryth move up one slot, the next existing preference (Tavi) fills the fourth pick. Picks are not exclusive across teams (GameRules.md: selected by the opposing team is allowed).
    const std::vector<D> il_prefs{D::tavi, D::stellar, D::ryth, D::varg, D::ren, D::marcella, D::vendetta};   // v12
    bool il = false;
    try { il = P_IL_DRAFT && size == 4 && cambc::map::width() == 40 && cambc::map::height() == 40; } catch (...) { il = false; }
    // EXP-115 screen (scr-packs): map-gated list, everything else v9.
    const std::vector<D> scr_prefs{D::tavi, D::stellar, D::ryth, D::varg, D::vendetta, D::ren};   // v12
    bool scr = false;
    try { scr = !rows && size == 3 && cambc::map::width() == 31 && cambc::map::height() == 31; } catch (...) { scr = false; }
    // EXP-121: expanse (80x80, three picks) drafts Vendetta, Cassi & Issac, Varg; the Varg is the stack runner (runner.cpp).
    const std::vector<D> runner_prefs{D::vendetta, D::cassi_and_issac, D::varg};
    bool run = false;
    try { run = P_RUNNER && !rows && size == 3 && cambc::map::width() == 80 && cambc::map::height() == 80; } catch (...) { run = false; }
    // v13: rushbait_v2 (30x80, four picks) is a centre brawl with the altars at the far ends; Vendetta's kill resets decide it, so keep v11's list there.
    const std::vector<D> brawl_prefs{D::vendetta, D::cassi_and_issac, D::varg, D::ryth, D::tavi, D::stellar};
    bool brawl = false;
    try { brawl = size == 4 && cambc::map::width() == 30 && cambc::map::height() == 80; } catch (...) { brawl = false; }
    // v18: on the brawl maps (swirl 45x30 four picks; contest 50x50 where the Monoliths are the only gems) the #1 team's
    // roster (Stellar, Ren, Cassi & Issac, Ryth, Varg) won 2-0 both; on pure farming maps it lost 0-2, so it is map-gated.
    const std::vector<D> fish_prefs{D::stellar, D::ren, D::cassi_and_issac, D::ryth, D::varg, D::tavi};
    bool fish = false;
    try {
        const int w = cambc::map::width(), h = cambc::map::height();
        bool mono_only = false;
        if (w == 50 && h == 50) { int other = 0; bool mono = false; for (auto g : cambc::map::camp_groups()) { bool m = false; for (const auto& c : g.camps()) if (c.is_monolith()) m = true; if (m) mono = true; else other += static_cast<int>(g.gem_value()); } mono_only = mono && other == 0; }
        fish = (w == 45 && h == 30 && size == 4); (void)mono_only;   // v19: swirl only (contest stays on the v16 roster)
    } catch (...) { fish = false; }
    const auto& prefs = fish ? fish_prefs : brawl ? brawl_prefs : run ? runner_prefs : rows ? rows_prefs : (scr ? scr_prefs : (il ? il_prefs : (size <= 4 ? small : large)));   // v13: small list up to 4 picks
    for (auto pick : prefs)
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    for (int i = 0; i < 11; ++i) {
        auto pick = static_cast<D>(i);
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    }
}
