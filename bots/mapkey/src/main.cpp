// mapkey: prints "MAPKEY w h groups" on the first draft turn, then picks the lowest free id.
#include <cambc.h>
#include <cstdio>
#include <algorithm>
using namespace cambc;
int main() {
    bool done = false;
    while (true) {
        try {
            if (is_draft()) {
                if (!done) { done = true; std::printf("MAPKEY %d %d %d\n", map::width(), map::height(), static_cast<int>(map::camp_groups().size())); std::fflush(stdout); }
                auto mine = draft::current_draft(team::ally);
                for (int i = 0; i < 12; ++i) { auto c = static_cast<champion_draft>(i); if (std::find(mine.begin(), mine.end(), c) == mine.end()) { try { draft::pick(c); break; } catch (...) {} } }
            }
        } catch (...) {}
        end_turn();
    }
}
