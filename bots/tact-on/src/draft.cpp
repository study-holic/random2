// Draft: fixed preference lists taken from the top-leaderboard rosters (docs/opponents/top-runs-2026-10-13.md).
#include <cambc.h>
#include "draft.h"
#include <algorithm>
#include <vector>

using D = cambc::champion_draft;

void play_draft() {
    const auto mine = cambc::draft::current_draft(cambc::team::ally);
    const auto size = cambc::draft::team_size();
    const std::vector<D> small{D::vendetta, D::cassi_and_issac, D::stellar, D::ryth, D::varg, D::tavi};
    const std::vector<D> large{D::vendetta, D::cassi_and_issac, D::varg, D::ryth, D::tavi, D::ren, D::marcella, D::stellar};
    const auto& prefs = size <= 3 ? small : large;
    for (auto pick : prefs)
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    for (int i = 0; i < 11; ++i) {
        auto pick = static_cast<D>(i);
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    }
}
