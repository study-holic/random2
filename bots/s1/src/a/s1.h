#pragma once
// v5 S1 economy controller switches (tools/cloud/v5-s1-economy.md; report results/v5-s1.md). Each is 0 = v4 behaviour.
#ifndef P_S1_DIAG
#define P_S1_DIAG 0   // diagnostics: self-block and shared-tile counters printed to stdout every 25 rounds (bot.cpp s1_diag)
#endif
#ifndef P_S1_TRACE
#define P_S1_TRACE 0   // >0: with P_S1_DIAG, one state line per champion every this many rounds (debug only)
#endif
#ifndef P_S1_BANK_REACH
#define P_S1_BANK_REACH 1   // bank task reachable: walk into an asleep Monolith's zone when every goal lies inside one (boss_battle altar); never wake one in passing (bot.cpp build_view, util.cpp move_towards_closest)
#endif
