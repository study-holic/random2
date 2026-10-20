// siege.h (w9): Monolith siege from outside monster range. Off unless a map plan calls Siege::enable().
// Rules relied on (docs/rules/GameRules.md): Monolith 20,000 health, strength 200, range 3, 300 gold, 200 gems,
// respawn 150 rounds (line 185); "All monsters have attack range 3 and vision 8 ... do not chase" (206); casts at
// range 4 on "Enemy or camp" (Cassi 438, Prism 556, Stellar 586, Marcella 656, Tavi 688, Ren 744) or "Ground"
// (Ryth 492, Prism 562, Stellar 592); capture = occupy a spawn tile uncontested for five capture phases (193).
// Casting range is Chebyshev (cambc.h ability::mask_into draws draw_chebyshev(owner, range)).
#pragma once
#include <cambc.h>
#include <vector>

class Bot;
struct TurnView;

namespace Siege {

struct Options {
    bool on = false;          // siege mode active on this map
    bool focus_build = true;  // casters buy Focus as their main stat
    bool escort = false;      // champions without a bag walk beside an ally carrying >= 50 gems (shared bytes 6-7)
    int escort_reach = 30;    // ... when the carrier is within this many tiles
    bool tank_capture = false;  // the highest max-health caster near the dead Monolith captures
    int safe_tribute = 0;     // >0: no tribute start near an altar with an enemy champion within this many tiles
    int hold_max = 80;        // ... for at most this many turns per bag
    bool cassi_farms = false; // Cassi is not a siege caster (C&I farm gold clusters by default)
    int carry_wait = 6;       // escort: a fresh carrier waits up to this many turns for two allies within 3
};

void enable(const Options& o);
bool active();
// util.cpp: with the siege on, a live Monolith counts as dangerous at range 3 (its attack range) instead of 4.
bool mono_tight();
// util.cpp move_towards_closest: tiles within 3 of a Monolith believed alive, blocked for every step while the
// siege is on (null when off or none is alive).
const cambc::bitboard* block();

void init(Bot& b);
bool pre(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies);
bool mid(Bot& b, const TurnView& v, std::vector<cambc::champion>& enemies, std::vector<cambc::champion>& allies);
bool buy(Bot& b);   // true when the siege build handled the purchase

}  // namespace Siege
