// Draft: fixed list from strategy/tung-clone.md (Vendetta, Varg, Cassi and Issac, Ryth, Tavi); no map logic.
#include <cambc.h>
#include "draft.h"
#include <algorithm>
#include <vector>

using D = cambc::champion_draft;

void play_draft() {
    const auto mine = cambc::draft::current_draft(cambc::team::ally);
    const std::vector<D> prefs{D::vendetta, D::varg, D::cassi_and_issac, D::ryth, D::tavi,
                               D::stellar, D::marcella, D::ren, D::prism, D::willow, D::vesper_vale};
    for (auto pick : prefs) {
        if (std::find(mine.begin(), mine.end(), pick) != mine.end()) continue;
        try { cambc::draft::pick(pick); return; } catch (...) {}   // taken by the opponent: try the next one
    }
}
