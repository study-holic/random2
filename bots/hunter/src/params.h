#pragma once
// All tunables of v5-base. Every knob is `#ifndef P_X / #define P_X <default> / #endif`; tools/sweep.py writes a
// src/params_override.h with `#define P_X value` lines (included first, so it wins). Defaults reproduce v4-11 exactly.
// Boolean knobs are 0/1. Float knobs may be given as `0.5` (they are converted to float where used).
#if __has_include("params_override.h")
#include "params_override.h"
#endif

// ---- bot.cpp: banking, altars, retreat ------------------------------------------------------------------------
#ifndef P_BANK_BAG
#define P_BANK_BAG 36                 // gems carried before banking
#endif
#ifndef P_NEAR_SPAWN
#define P_NEAR_SPAWN 6                // retreat walks in when the spawn is within this many tiles
#endif
#ifndef P_ALTAR_SCORING
#define P_ALTAR_SCORING 0             // EXP-023 contested-altar choice and farm-on
#endif
#ifndef P_GIVE_UP_CONTEST
#define P_GIVE_UP_CONTEST 16          // altar contest score at which a carrier farms on (needs P_ALTAR_SCORING)
#endif
#ifndef P_ALTAR_FOE_RADIUS
#define P_ALTAR_FOE_RADIUS 5
#endif
#ifndef P_ALTAR_FOE_MEMORY
#define P_ALTAR_FOE_MEMORY 20
#endif
#ifndef P_RETREAT_HURT
#define P_RETREAT_HURT 0.45f          // retreat health fraction when carrying gems or enemies are in view
#endif
#ifndef P_RETREAT_CALM
#define P_RETREAT_CALM 0.3f           // retreat health fraction otherwise
#endif
#ifndef P_RETREAT_TANK
#define P_RETREAT_TANK 0.35f          // tank swap: retreat fraction of a melee champion adjacent to a squad camp
#endif
#ifndef P_RETREAT_INCOMING_MULT
#define P_RETREAT_INCOMING_MULT 4.0   // retreat starts when health < incoming camp damage * this + base
#endif
#ifndef P_RETREAT_INCOMING_BASE
#define P_RETREAT_INCOMING_BASE 0.1   // ... + this fraction of max health
#endif
#ifndef P_RECALL_DONE_FIELD
#define P_RECALL_DONE_FIELD 0.9f      // retreat ends at this health fraction away from spawn
#endif
#ifndef P_RECALL_DONE_SPAWN
#define P_RECALL_DONE_SPAWN 0.95f     // retreat ends at this health fraction on the spawn
#endif
#ifndef P_SPAWN_LEAVE_HP
#define P_SPAWN_LEAVE_HP 0.9f         // a champion on the spawn stays until this health fraction
#endif
#ifndef P_CLUSTER_MIN_GROUPS
#define P_CLUSTER_MIN_GROUPS 3        // gem-cluster groups needed for the cluster-first opening
#endif

// ---- cluster.cpp: cluster-first opening -------------------------------------------------------------------------
#ifndef P_OPENING_ROUNDS
#define P_OPENING_ROUNDS 120
#endif
#ifndef P_CLUSTER_REACH
#define P_CLUSTER_REACH 25
#endif
#ifndef P_MIXED_MEMORY
#define P_MIXED_MEMORY 5
#endif
#ifndef P_MIXED_LOOK
#define P_MIXED_LOOK 10
#endif
#ifndef P_OPENING_BANK_NEAR
#define P_OPENING_BANK_NEAR 22
#endif
#ifndef P_OPENING_ALTAR_NEAR
#define P_OPENING_ALTAR_NEAR 4
#endif

// ---- micro.cpp: farming waits -----------------------------------------------------------------------------------
#ifndef P_GIVE_UP_WAIT
#define P_GIVE_UP_WAIT 12             // rounds at a respawn ring before giving up
#endif
#ifndef P_TANK_WAIT
#define P_TANK_WAIT 20                // ranged: rounds to wait for a melee ally who is coming
#endif
#ifndef P_TANK_WAIT_SOLO
#define P_TANK_WAIT_SOLO 3            // ranged: rounds to wait when no melee ally is coming
#endif
#ifndef P_COHESION_MAX
#define P_COHESION_MAX 6              // rounds a champion waits for a lagging squad member
#endif

// ---- squads.cpp: squads, targets -----------------------------------------------------------------------------------
#ifndef P_RESPAWN_TIME
#define P_RESPAWN_TIME 65
#endif
#ifndef P_CLAIM_SLACK
#define P_CLAIM_SLACK 25
#endif
#ifndef P_EXCLUDE_ROUNDS
#define P_EXCLUDE_ROUNDS 30
#endif
#ifndef P_CLAIM_BONUS
#define P_CLAIM_BONUS 8
#endif
#ifndef P_CAPTURE_BY_HEALTH
#define P_CAPTURE_BY_HEALTH 1
#endif
#ifndef P_SQUAD_MIN_ROSTER
#define P_SQUAD_MIN_ROSTER 5          // champions needed for the two-squad split by type
#endif
#ifndef P_SQUAD_MIN_SIZE
#define P_SQUAD_MIN_SIZE 2            // a squad smaller than this triggers the melee-first redeal
#endif
#ifndef P_ELK_MIN_SQUAD
#define P_ELK_MIN_SQUAD 1             // squad members needed (with a melee champion) before Elk groups are squad targets
#endif
#ifndef P_PACK_WOLVES
#define P_PACK_WOLVES 3               // wolves in a pack that becomes a squad target on Elk-free maps
#endif
#ifndef P_OWN_HALF_ROUNDS
#define P_OWN_HALF_ROUNDS 400         // before this round only own-half groups are chosen
#endif
#ifndef P_WAIT_WEIGHT
#define P_WAIT_WEIGHT 2.0             // target score: tiles per round of respawn wait
#endif

// ---- stuck.cpp: contested groups and the stuck detector ----------------------------------------------------------
#ifndef P_CONTEST_RADIUS
#define P_CONTEST_RADIUS 4
#endif
#ifndef P_CONTEST_ROUNDS
#define P_CONTEST_ROUNDS 30
#endif
#ifndef P_FINISH_HEALTH
#define P_FINISH_HEALTH 0.3f
#endif
#ifndef P_STUCK_EXCLUDE
#define P_STUCK_EXCLUDE 15
#endif
#ifndef P_STUCK_WALK
#define P_STUCK_WALK 30
#endif
#ifndef P_STUCK_EXCLUDE_ROUNDS
#define P_STUCK_EXCLUDE_ROUNDS 60
#endif
#ifndef P_STUCK_WALK_TILES
#define P_STUCK_WALK_TILES 5
#endif

// ---- util.cpp --------------------------------------------------------------------------------------------------------
#ifndef P_VARG_SHIELD_HP
#define P_VARG_SHIELD_HP 0.6f         // Varg shields when below this health fraction (or a damaging camp is within 4)
#endif

// ---- enemies.cpp: champion micro (v5 knobs) ------------------------------------------------------------------------
#ifndef P_FLEE_FOES
#define P_FLEE_FOES 2                 // alone with this many enemies within P_FLEE_RADIUS: walk to an ally or the spawn
#endif
#ifndef P_FLEE_RADIUS
#define P_FLEE_RADIUS 4
#endif
#ifndef P_FOCUS_RULE
#define P_FOCUS_RULE 0                // 0 lowest health, 1 most carried gems (tie lowest health), 2 nearest
#endif
#ifndef P_CHASE_TILES
#define P_CHASE_TILES 2               // chase a foe at most this many tiles beyond attack range
#endif
#ifndef P_CHASE_FROM_CAMP
#define P_CHASE_FROM_CAMP 3           // ... and only while the foe is within this many tiles of the farmed camp
#endif
#ifndef P_ENGAGE_MIN_HP
#define P_ENGAGE_MIN_HP 0.0           // own health fraction below which a champion retreats instead of fighting back
#endif
#ifndef P_RANGED_KITE
#define P_RANGED_KITE 0               // 1: a ranged champion beside an enemy melee champion steps back to distance 2..3
#endif
#ifndef P_TANK_ROTATE_HP
#define P_TANK_ROTATE_HP 0.0          // >0: a melee champion below this fraction beside a camp yields to a fresher mate
#endif
#ifndef P_SAVE_CC
#define P_SAVE_CC 0                   // 1: no stun/cc on camps while an enemy champion is within 8 tiles
#endif
#ifndef P_ALLY_VISION_FIX
#define P_ALLY_VISION_FIX 0           // 1: drop champions that left vision (killed or out of sight) from the turn's lists
#endif
