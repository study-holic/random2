#pragma once
// v5 S1 economy controller switches (tools/cloud/v5-s1-economy.md; report results/v5-s1.md). Each is 0 = v4 behaviour.
#if __has_include("../salt.h")
#include "../salt.h"   // mapgate --salts: P_SALT of this copy (measurement only)
#endif
#ifndef P_SALT
#define P_SALT 0
#endif
// Measurement salt: with P_SALT > 0, target scores get a fixed pseudo-random tie-break of at most +-0.5 x P_S1_NOISE tile
// per (salt, champion, group), so salted copies play distinct games (the first-turn fuel burn alone repeated 28 of 42
// map-sides). Salt 0 is unchanged.
#ifndef P_S1_NOISE
#define P_S1_NOISE 1.0
#endif
inline double s1_salt_noise(int who, int group) {
    if (P_SALT <= 0) return 0.0;
    const unsigned h = (static_cast<unsigned>(P_SALT) * 2654435761u) ^ (static_cast<unsigned>(who + 1) * 40503u) ^ (static_cast<unsigned>(group + 7) * 2246822519u);
    return P_S1_NOISE * (static_cast<double>((h >> 8) % 1000) / 1000.0 - 0.5);
}
#ifndef P_S1_DIAG
#define P_S1_DIAG 0   // diagnostics: self-block and shared-tile counters printed to stdout every 25 rounds (bot.cpp s1_diag)
#endif
#ifndef P_S1_TRACE
#define P_S1_TRACE 0   // >0: with P_S1_DIAG, one state line per champion every this many rounds (debug only)
#endif
#ifndef P_S1_BANK_REACH
#define P_S1_BANK_REACH 1   // bank task reachable: walk into an asleep Monolith's zone when every goal lies inside one (boss_battle altar); never wake one in passing (bot.cpp build_view, util.cpp move_towards_closest)
#endif
#ifndef P_S1_NO_SELF_BLOCK
#define P_S1_NO_SELF_BLOCK 1   // never hold our own respawn: walk out of spawn-tile blocks (util.cpp), step off a respawning group's tile at turn end (bot.cpp step_off_respawn)
#endif
#ifndef P_S1_NSB_WALK
#define P_S1_NSB_WALK 1   // part of NO_SELF_BLOCK: walk out of a spawn-tile block over dead camps' tiles
#endif
#ifndef P_S1_NSB_STEP
#define P_S1_NSB_STEP 0   // part of NO_SELF_BLOCK: step off a respawning group's tile at turn end (off: no self-block left after NSB_WALK, and it cost broken_curve 930->434, gold_rush 719->484 gems unopposed)
#endif
#ifndef P_S1_ONE_CAPTURER
#define P_S1_ONE_CAPTURER 0   // one capturer per group: no second pack member on the tiles; non-capturers step off a reward-available group (bot.cpp step_off_respawn, squads.cpp)
#endif
#ifndef P_S1_THREAT_COST
#define P_S1_THREAT_COST 0    // paths: detour around the reach of awake wolves/elks not being fought, up to P_S1_DETOUR extra tiles (util.cpp)
#endif
#ifndef P_S1_DETOUR
#define P_S1_DETOUR 6
#endif
#ifndef P_S1_AUCTION
#define P_S1_AUCTION 0        // 1: sequential single-item auction on u_ij replaces choose_target and the squad claim (s1econ.cpp); 2: hybrid, v4's squads and passes ranked by u_ij, light claims with ETA outbid
#endif
#ifndef P_S1_ASSEMBLE
#define P_S1_ASSEMBLE 1       // with P_S1_AUCTION: wait outside a heavy group until its coalition has arrived
#endif
#ifndef P_S1_TANK_SWAP
#define P_S1_TANK_SWAP 0      // camp fights: 1 = a melee champion below 35 % yields the adjacent ring to a fresher melee mate within 2; 2 = also ranged engage only once an ally is adjacent to the prey (micro.cpp)
#endif
#ifndef P_S1_LAMBDA
#define P_S1_LAMBDA 0.1       // gems per gold in u_ij (wolf: 4 gems + 40 gold -> 8)
#endif
#ifndef P_S1_DPS_MULT
#define P_S1_DPS_MULT 1.5     // dps = strength x this (basic attack plus resets and spells)
#endif
#ifndef P_S1_RISK_RATIO
#define P_S1_RISK_RATIO 0.6   // damage taken while clearing / pooled health at which the clear starts to fail
#endif
#ifndef P_S1_FOE_MEMORY
#define P_S1_FOE_MEMORY 40    // rounds an enemy sighting near a group raises its risk
#endif
#ifndef P_S1_CANDIDATES
#define P_S1_CANDIDATES 8     // groups evaluated per turn (fuel)
#endif
#ifndef P_S1_HYST
#define P_S1_HYST 1.2         // switch only when u_new > this x u_cur
#endif
#ifndef P_S1_ASSEMBLE_WAIT
#define P_S1_ASSEMBLE_WAIT 25 // rounds waiting for the coalition before engaging anyway
#endif
#ifndef P_S1_TABLE_FUEL
#define P_S1_TABLE_FUEL 30000 // distance tables are built while more fuel than this remains at the end of a turn
#endif
#ifndef P_S1_U_SCALE
#define P_S1_U_SCALE 100.0   // hybrid: score = -u x this (v4's pass logic compares scores; the claim bonus is 8 tiles)
#endif
#ifndef P_S1_OUTBID
#define P_S1_OUTBID 1   // hybrid: a claimed light group may be taken with an ETA bucket 2 better (0: never, as v4)
#endif
#ifndef P_S1_TRAVEL_W
#define P_S1_TRAVEL_W 1.0   // weight of walking rounds in the u_ij denominator
#endif
#ifndef P_S1_RISK_MAXHP
#define P_S1_RISK_MAXHP 0   // camp risk against max health (1) or current health (0)
#endif
#ifndef P_S1_WAIT_W
#define P_S1_WAIT_W 1.0   // weight of respawn-wait rounds in the u_ij denominator (v4's score used 2)
#endif
#ifndef P_S1_SQUAD_LIVE
#define P_S1_SQUAD_LIVE 1   // hybrid: coalition k of a squad group = squad mates alive with a claim (1) or the squad size (0)
#endif
#ifndef P_S1_BANK_RATE
#define P_S1_BANK_RATE 0   // bank when u_bank > P_S1_BANK_MULT x u(current target) and the bag >= P_S1_BANK_MIN (replaces the 36-gem and 40-round triggers)
#endif
#ifndef P_S1_BANK_MIN
#define P_S1_BANK_MIN 0
#endif
#ifndef P_S1_BANK_MULT
#define P_S1_BANK_MULT 1.0
#endif
#ifndef P_S1_FUELDBG
#define P_S1_FUELDBG 0   // debug: fuel prints (distance tables, turns over 60k)
#endif
#ifndef P_S1_CARRY_ROUNDS
#define P_S1_CARRY_ROUNDS 40   // v4: bank after carrying this many rounds
#endif
