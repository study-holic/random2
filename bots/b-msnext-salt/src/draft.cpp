// Draft: the preference list comes from the active map profile (profile.cpp); lists are the v10 lists.
#include <cambc.h>
#include "draft.h"
#include "params.h"
#include "profile.h"
#include <algorithm>
#include <vector>

using D = cambc::champion_draft;

void play_draft() {
    const auto mine = cambc::draft::current_draft(cambc::team::ally);
    const MapProfile& p = identify_in_draft();
    for (int i = 0; i < p.draft_n; ++i) {
        const auto pick = p.draft[i];
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    }
    for (int i = 0; i < 11; ++i) {
        auto pick = static_cast<D>(i);
        if (std::find(mine.begin(), mine.end(), pick) == mine.end()) { cambc::draft::pick(pick); return; }
    }
}
