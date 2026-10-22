#include <cambc.h>
#include <iostream>
#include <variant>
namespace bc = cambc;
int main() {
    bool printed = false;
    while (true) {
        if (bc::is_draft()) {
            if (!printed) {
                printed = true;
                std::cout << "MAP " << bc::map::width() << "x" << bc::map::height() << " sym " << bc::to_string(bc::map::symmetry()) << " team_size " << bc::draft::team_size() << "\n";
                std::cout << "walls " << bc::map::walls().count() << " holes " << bc::map::holes().count() << " bushes " << bc::map::bushes().count() << "\n";
                for (auto a : bc::map::altars().positions()) std::cout << "ALTAR " << bc::to_string(a) << "\n";
                for (auto a : bc::map::spawn_region(bc::team::ally).positions()) std::cout << "SPAWN_A " << bc::to_string(a) << "\n";
                for (auto a : bc::map::spawn_region(bc::team::enemy).positions()) std::cout << "SPAWN_B " << bc::to_string(a) << "\n";
                bc::distance_field reach; bc::bitboard reach_vis;
                { auto walls = bc::map::walls() | bc::map::holes(); reach.compute(walls, bc::map::spawn_region(bc::team::ally), reach_vis); }
                int gi = 0;
                for (auto g : bc::map::camp_groups()) {
                    int best = 1 << 20;
                    for (const auto& c : g.camps()) { auto p = c.spawn_position(); for (int dx = -1; dx <= 1; ++dx) for (int dy = -1; dy <= 1; ++dy) { bc::vec2 q(p.x + dx, p.y + dy); if (bc::map::is_in_bounds(q) && reach_vis.is_set(q)) best = std::min(best, (int)reach.get(q)); } }
                    std::cout << "GROUP " << gi++ << " gems " << g.gem_value() << " dist " << (best == (1 << 20) ? -1 : best) << " :";
                    for (const auto& c : g.camps()) std::cout << " " << bc::to_string(c.kind()) << "@" << bc::to_string(c.spawn_position());
                    std::cout << "\n";
                }
            }
            auto mine = bc::draft::current_draft(bc::team::ally);
            for (int i = 0; i < 11; ++i) { auto p = static_cast<bc::champion_draft>(i); bool have=false; for (auto m : mine) if (m==p) have=true; if (!have) { bc::draft::pick(p); break; } }
        } else {
            bool first = true;
            for (const auto& c : bc::champions(bc::team::ally)) if (static_cast<std::uint32_t>(static_cast<bc::handle_champion_t>(c)) < static_cast<std::uint32_t>(static_cast<bc::handle_champion_t>(bc::self()))) first = false;
            if (first) {
            {   // print map rows, 8 per turn (fuel)
                static int next_row = 0;
                for (int y = next_row; y < bc::map::height() && y < next_row + 8; ++y) {
                    std::string row;
                    for (int x = 0; x < bc::map::width(); ++x) {
                        bc::vec2 p(x, y);
                        char ch = '.';
                        if (bc::map::walls().is_set(p)) ch = '#';
                        else if (bc::map::holes().is_set(p)) ch = 'o';
                        else if (bc::map::altars().is_set(p)) ch = 'A';
                        else if (bc::map::spawn_region(bc::team::ally).is_set(p)) ch = 'a';
                        else if (bc::map::spawn_region(bc::team::enemy).is_set(p)) ch = 'b';
                        else if (bc::map::bushes().is_set(p)) ch = '"';
                        row += ch;
                    }
                    std::cout << row << "\n";
                    next_row = y + 1;
                }
            }
            }
        }
        bc::end_turn();
    }
}
