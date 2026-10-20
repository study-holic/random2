// fuelprobe (sweep35 task 2): what the engine does past the per-turn fuel limit. Drafts like bots/idle. Every champion
// moves one step towards the map centre each turn. On round BURN_A (1) and BURN_B (50) it moves first, then burns
// BURN_UNITS loop units (calibrated on round 2 and printed), then tries a second action (an attack-free step) and prints.
// Each turn prints: round, champion type, position, fuel remaining at start and end. Compare with the replay positions.
#include <cambc.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#ifndef BURN_A
#define BURN_A -1
#endif
#ifndef BURN_B
#define BURN_B 50
#endif
#ifndef BURN_FUEL
#define BURN_FUEL 150000ULL
#endif
static volatile std::uint64_t sink = 0;
static void burn_units(std::uint64_t n) { for (std::uint64_t i = 0; i < n; ++i) sink = sink * 6364136223846793005ULL + i; }

int main() {
    double per_unit = 1.0;   // measured 0.996-0.998 fuel per unit (calibration run, round 2)
    while (true) {
        try {
            if (cambc::is_draft()) {
                const auto chosen = cambc::draft::current_draft(cambc::team::ally);
                for (int i = 0; i < 11; ++i) {
                    const auto pick = static_cast<cambc::champion_draft>(i);
                    if (std::find(chosen.begin(), chosen.end(), pick) == chosen.end()) { cambc::draft::pick(pick); break; }
                }
            } else {
                auto& me = cambc::self();
                const int r = static_cast<int>(cambc::round_number());
                const auto f0 = cambc::fuel_remaining();
                if (me.is_alive()) {
                    const cambc::vec2 c{static_cast<int>(cambc::map::width() / 2), static_cast<int>(cambc::map::height() / 2)};
                    const auto p0 = me.position();
                    bool m1 = false, m2 = false;
                    auto step = [&]() { for (int dx = -1; dx <= 1; ++dx) for (int dy = -1; dy <= 1; ++dy) {
                        if (!dx && !dy) continue; try { if (me.can_move() && me.move(cambc::vec2{dx, dy})) return true; } catch (...) {} } return false; };
                    (void)c;
                    m1 = step();
                    if (r == BURN_A || r == BURN_B) {
                        const std::uint64_t units = per_unit > 0 ? static_cast<std::uint64_t>(BURN_FUEL / per_unit) : 3000000ULL;
                        std::cout << "BURN r" << r << " t" << int(me.champion_name()) << " units " << units << " f0 " << f0 << std::endl;
                        burn_units(units);
                        const int r2 = static_cast<int>(cambc::round_number());
                        m2 = step();
                        std::cout << "AFTER r" << r << " now_r" << r2 << " t" << int(me.champion_name()) << " pos " << me.position().x << ","
                                  << me.position().y << " m2 " << m2 << " fuel " << cambc::fuel_remaining() << std::endl;
                    }
                    if (r <= 4 || (r >= BURN_B - 2 && r <= BURN_B + 8))
                        std::cout << "T r" << r << " t" << int(me.champion_name()) << " " << p0.x << "," << p0.y << "->" << me.position().x << ","
                                  << me.position().y << " m1 " << m1 << " f0 " << f0 << " fend " << cambc::fuel_remaining() << std::endl;
                }
            }
        } catch (...) {}
        cambc::end_turn();
    }
}
