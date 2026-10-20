#pragma once
// All tunables of v5-base. Every knob is `#ifndef P_X / #define P_X <default> / #endif`; tools/sweep.py writes a
// src/params_override.h with `#define P_X value` lines (included first, so it wins). Defaults reproduce v4-13 exactly (v4-11 + P_OPENING_ROUNDS=2000).
// Boolean knobs are 0/1. Float knobs may be given as `0.5` (they are converted to float where used).
#if __has_include("params_override.h")
#include "params_override.h"
#endif
#ifndef P_W_WATCHDOG
#define P_W_WATCHDOG 1   // wd.cpp: progress watchdog (tools/cloud/sweep35.md task 3)
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
#define P_OPENING_ROUNDS 2000          // v4-13: cluster-first opening for the whole game (was 120 in v4-11)
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
#ifndef P_SQUAD_FALLBACK
#define P_SQUAD_FALLBACK 0            // EXP-081: 1 = a ranged squad member waiting for its tank takes a free solo group meanwhile; 2 = also a free pack
#endif
#ifndef P_SQUAD_FALLBACK_WAIT
#define P_SQUAD_FALLBACK_WAIT 12      // EXP-081: rounds in wait_tank before the fallback
#endif
#ifndef P_OPENING_ASSIGN
#define P_OPENING_ASSIGN 0            // EXP-079: 1 = on cluster maps each champion's first target is its own group (k-th nearest by walking distance)
#endif
#ifndef P_OPENING_ASSIGN_MARGIN
#define P_OPENING_ASSIGN_MARGIN 10    // EXP-080: a group is assignable only when dist_enemy - dist_ally >= this; fewer than 3 such groups: no assignment
#endif
#ifndef P_OPENING_ASSIGN_MIN_GROUPS
#define P_OPENING_ASSIGN_MIN_GROUPS 5 // EXP-083: assignable groups needed (cluster maps only): 80 has 11, 71 has 3
#endif
#ifndef P_OPENING_ASSIGN_ROUNDS
#define P_OPENING_ASSIGN_ROUNDS 120   // the assignment holds until the champion's first capture or this round
#endif
#ifndef P_ALLY_VISION_FIX
#define P_ALLY_VISION_FIX 0           // 1: drop champions that left vision (killed or out of sight) from the turn's lists
#endif

// ---- econ1 knobs (defaults off = v4-13 behaviour) ------------------------------------------------------------------
#ifndef P_SHOP_RECALL
#define P_SHOP_RECALL 0               // 1: after a tribute completes far from the spawn, recall to shop and heal
#endif                                // (top-2 teams start 15/20 and 18/31 recalls at an altar; strategy/top2-micro-2026-10-14.md rank 2)
#ifndef P_SHOP_RECALL_ALTAR_DIST
#define P_SHOP_RECALL_ALTAR_DIST 12   // ... only when the altar is at least this many tiles (Chebyshev) from our spawn
#endif
#ifndef P_FIELD_SHOP
#define P_FIELD_SHOP 0                // 1: a champion with the remote_shop passive (Tavi only; SDK) buys away from the spawn
#endif
#ifndef P_CLUSTER_WAIT
#define P_CLUSTER_WAIT 0              // rounds: >0 accepts a gem cluster that respawns up to this long after arrival and waits
#endif                                // beside it (rank-1 waits 29 to 38 per cent of champion-rounds; strategy/pace-audit-v43.md)
#ifndef P_FIELD_SHOP_HEALTH
#define P_FIELD_SHOP_HEALTH 0         // w8: 1 = Tavi's field purchases may include Health shards (buy_items_w8 only)
#endif
#ifndef P_BUILD_DMG
#define P_BUILD_DMG 0                 // w8 (A): no Health shards; Power shards of the damage stat (buy_items_w8 only)
#endif
#ifndef P_BUILD_MON_FIRST
#define P_BUILD_MON_FIRST 0           // w8 (B): with a damage build, a Monster emblem first (buy_items_w8 only)
#endif
#ifndef P_HEALTH_FIRST_TANKS
#define P_HEALTH_FIRST_TANKS 0        // 1: Varg, Ryth and Vendetta buy health shards first until 5 owned (rank-1 Varg +600 HP by round 87)
#endif

// ---- threat.cpp: survive-and-bank threat layer (EXP-071; default 0 = v4-13 behaviour) -----------------------------------
#ifndef P_THREAT_LAYER
#define P_THREAT_LAYER 0              // 1: bank-first, regroup and group leash (threat.cpp)
#endif
#ifndef P_THREAT_RADIUS
#define P_THREAT_RADIUS 8             // enemies within this many tiles make a threat picture
#endif
#ifndef P_THREAT_BAG
#define P_THREAT_BAG 5                // gems carried that trigger bank-first when an enemy is within the radius
#endif
#ifndef P_SAFE_DIST
#define P_SAFE_DIST 6                 // no tribute start, altar or regroup tile within this many tiles of an enemy (stuns reach 4)
#endif
#ifndef P_LEASH_ALLIES
#define P_LEASH_ALLIES 2              // allies within 6 (leash) or within 4 (regroup exit) needed
#endif
#ifndef P_REGROUP_ROUNDS
#define P_REGROUP_ROUNDS 5            // rounds with no enemy in the radius before a regroup (or a bag hold) ends
#endif
#ifndef P_LEASH_MEMORY
#define P_LEASH_MEMORY 60             // a group with an enemy seen this recently is entered only with the leash allies
#endif
#ifndef P_THREAT_MODE
#define P_THREAT_MODE 7               // bitmask of the layer's rules: 1 bank-first, 2 regroup, 4 group leash (diagnostic)
#endif

// ---- EXP-093 rows commander ----
#ifndef P_ROWS_CMD
#define P_ROWS_CMD 1                  // 1: on rows (50x50, six wolf packs): one capturer (highest health), Power-main-only buying, rows draft
#endif
#ifndef P_ROWS_BAG
#define P_ROWS_BAG 36                 // rows: gems carried before banking
#endif
#ifndef P_ROWS_SCHED
#define P_ROWS_SCHED 1                // EXP-093: fixed pack schedule on rows (rows.cpp)
#endif
#ifndef P_ROWS_BAG2
#define P_ROWS_BAG2 66                // EXP-094: rows scheduled units bank at two takes (2 x this) or when the trip fits the respawn window
#endif

// ---- v3b: w14 per-champion micro table (results/w14-duels.md; ported from bots/w14t/src/duel.cpp P_ADAPT) -------
// P_M3 = master switch; P_M3_OFF = bitmask of items to disable for ablation (bit i = item i+1).
#ifndef P_M3
#define P_M3 1
#endif
#ifndef P_M3_OFF
#define P_M3_OFF 0
#endif
enum : int { M3_NORETREAT = 1, M3_EARLYCAST = 2, M3_SHIELDENG = 4, M3_DASH = 8, M3_KITE = 16 };
inline bool m3_on(int item, cambc::champion_type t) {
    using CT = cambc::champion_type;
    if (!P_M3 || (P_M3_OFF & item)) return false;
    switch (item) {
        case M3_NORETREAT: return t != CT::willow;
        case M3_EARLYCAST: return t != CT::marcella;
        case M3_SHIELDENG: return t == CT::varg;
        case M3_DASH: return t == CT::ren || t == CT::vendetta || t == CT::issac || t == CT::vesper_vale;
        case M3_KITE: return t == CT::vesper_vale || t == CT::prism;
    }
    return false;
}
