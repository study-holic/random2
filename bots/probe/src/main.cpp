#include <string>
#include <cambc.h>
#include <iostream>
#include "bot.h"
#include "draft.h"
using namespace cambc;
static void probe() {
    static bool done = false;
    if (done) return;
    done = true;
    std::cout << "PROBE" << std::endl;
    auto w = map::walls();
    auto h = map::holes();
    auto a = map::altars();
    for (int y = 0; y < map::height(); ++y) {
        std::string row;
        for (int x = 0; x < map::width(); ++x) {
            vec2 q(x, y);
            char ch = '.';
            if (w.is_set(q)) ch = '#';
            else if (h.is_set(q)) ch = 'o';
            else if (a.is_set(q)) ch = 'A';
            row += ch;
        }
        std::cout << row << std::endl;
    }
    for (auto grp : map::camp_groups())
        for (const auto& c : grp.camps()) {
            std::string k = to_string(c.kind());
            std::cout << k << " " << c.spawn_position().x << "," << c.spawn_position().y << std::endl;
        }
    std::cout << "ENDMAP" << std::endl;
}
int main() {
    Bot bot;
    while (true) {
        try {
            if (cambc::is_draft())
                play_draft();
            else
                probe();
        } catch (const std::exception& error) {
            std::cout << "bot error: " << error.what() << std::endl;
        } catch (...) {
            std::cout << "bot error: unknown exception" << std::endl;
        }
        cambc::end_turn();
    }
}
