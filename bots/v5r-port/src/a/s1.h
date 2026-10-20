#pragma once
// v5r: two switches ported from bots/s1 (v5 S1 economy controller, results/v5-s1.md); nothing else from S1.
#ifndef P_S1_BANK_REACH
#define P_S1_BANK_REACH 1   // bank task reachable: walk into an asleep Monolith's zone when every goal lies inside one (boss_battle altar); never wake one in passing (bot.cpp build_view, util.cpp move_towards_closest)
#endif
#ifndef P_S1_NO_SELF_BLOCK
#define P_S1_NO_SELF_BLOCK 1   // never hold our own respawn: walk out of spawn-tile blocks (util.cpp)
#endif
#ifndef P_S1_NSB_WALK
#define P_S1_NSB_WALK 1   // part of NO_SELF_BLOCK: walk out of a spawn-tile block over dead camps' tiles
#endif
#ifndef P_S1_NSB_STEP
#define P_S1_NSB_STEP 0   // part of NO_SELF_BLOCK in bots/s1 (turn-end step-off); off there and not ported
#endif
