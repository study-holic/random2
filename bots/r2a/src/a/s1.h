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
#ifndef P_S1_BANK_REACH_ALTAR_ONLY
#define P_S1_BANK_REACH_ALTAR_ONLY 1   // v5r: the asleep-zone entry applies to the walk to an altar only (in bots/s1 it applied to every move; on gold_rush farmers then chased gold clusters inside the zone)
#endif
#ifndef P_S1_RETRY_FUEL
#define P_S1_RETRY_FUEL 1   // path retries (walk-out, zone) only when the fuel left covers the first attempt + 10k (starvation gate)
#endif
