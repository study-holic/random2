// s2.h: v5 session S2 quick wins (tools/cloud/v5-s2-quickwins.md). Every switch is separately ablatable.
#pragma once
#include <cambc.h>
#include <vector>
#include "mapid.h"
#if __has_include("params_override.h")
#include "params_override.h"   // variant builds set S2 switches here
#endif

// P_S2_DRAFT_CORE: draft the top-10 core instead of v4's static lists (Vendetta first on 14 of 21 maps).
// Cause: top-10 sides pick Stellar 96 %, C&I 95 %, Ren 72 %, Varg 70 %, Ryth 67 %, Vendetta 3 % (notes/top10-2026-10-20.md
// l.49-51). One list serves every team size, so the first 3 / 4 / 5 entries are the 3-, 4- and 5-pick cores.
#ifndef P_S2_DRAFT_CORE
#define P_S2_DRAFT_CORE 1
#endif

// P_S2_ALTAR_LOCK: buy a lock on every spawn visit (10 gold, carry 1) and place it on an altar (GameRules l.193-199).
// Cause: top-10 teams place 6.4 locks a game, v4 0. Weak evidence of benefit, so ablated on its own.
#ifndef P_S2_ALTAR_LOCK
#define P_S2_ALTAR_LOCK 1
#endif
#ifndef P_S2_LOCK_FROM
#define P_S2_LOCK_FROM 200              // first round a lock is bought (gold before it goes to shards: gear at round 200)
#endif
// P_S2_TRIBUTE_GATE: start a tribute in range unless an enemy that can stop it (stun in range, lock placement within
// 2 of the altar, or a kill before it completes) is within reach. Damage alone never cancels it (GameRules l.181).
#ifndef P_S2_TRIBUTE_GATE
#define P_S2_TRIBUTE_GATE 1
#endif
// P_S2_SPEND_NOW: keep 10 gold for a lock, spend the rest on shards every chance, and walk through the spawn to shop
// when the gold buys an emblem's worth of progress and the spawn is close.
#ifndef P_S2_SPEND_NOW
#define P_S2_SPEND_NOW 1
#endif
#ifndef P_S2_SHOP_GOLD
#define P_S2_SHOP_GOLD 90             // gold that triggers a shopping detour (3 shards)
#endif
#ifndef P_S2_SHOP_DIST
#define P_S2_SHOP_DIST 12             // max tiles to the spawn for a shopping detour on foot
#endif
#ifndef P_S2_BANK_VIA
#define P_S2_BANK_VIA 0               // 1: a banking trip passes the spawn first when gold >= P_S2_VIA_GOLD and the detour is short
#endif
#ifndef P_S2_VIA_GOLD
#define P_S2_VIA_GOLD 60
#endif
#ifndef P_S2_VIA_SLACK
#define P_S2_VIA_SLACK 6              // extra tiles allowed: d(me,spawn) + d(spawn,altar) <= d(me,altar) + slack
#endif
#ifndef P_S2_FUEL_STOP
#define P_S2_FUEL_STOP 35000          // end the turn before targeting, banking and farming with less fuel left (gate: lanes/B ran out in bank)
#endif
#ifndef P_S2_FAST_CONTEST
#define P_S2_FAST_CONTEST 1           // update_contested in one pass (stuck.cpp); decisions unchanged
#endif
#ifndef P_S2_LOCK_LOG
#define P_S2_LOCK_LOG 1               // print "LK" per placement (metrics_s2.py); '\n' only, no flush
#endif

// P_S2_DRAFT_EXC: 1 = keep v4's draft on the maps in keep_v4_draft (per-map exception rule of the brief: a variant must
// beat the core by >= 2 of 6 on the map and the overall head-to-head must not drop). 0 = core everywhere.
#ifndef P_S2_DRAFT_EXC
#define P_S2_DRAFT_EXC 0
#endif

#ifndef P_S2_CORE_VARG
#define P_S2_CORE_VARG 1
#endif

namespace s2 {

inline bool keep_v4_draft(mapid::MapId id) {
    if (!P_S2_DRAFT_EXC) return false;
    using M = mapid::MapId;
    switch (id) {
        case M::expanse_old: case M::expanse_v2: case M::contest: return true;   // candidates from the s2-d per-map table
        default: return false;
    }
}

inline const std::vector<cambc::champion_draft>& core_draft() {
    using D = cambc::champion_draft;
    static const std::vector<D> core{D::cassi_and_issac, D::stellar, D::ren, D::ryth, D::varg};
    // P_S2_CORE_VARG: Varg before Ren, so 3 picks = C&I + Stellar + Varg and 4 = + Ryth (top-10 second sets,
    // notes/top10 l.55-56). Cause: v4's squad code leans on Varg as tank and collector (interlock trace, results/v5-s2.md).
    static const std::vector<D> varg{D::cassi_and_issac, D::stellar, D::varg, D::ryth, D::ren};
    return P_S2_CORE_VARG ? varg : core;
}

}  // namespace s2
