// w7_drafts.h: per-map static drafts (builder w7, 19 Oct). Plan::current() applies draft_for(id) after the map's
// setup, so a list here overrides the map file's p.draft. nullptr = keep v2a's behaviour (map file or v4.14 lists).
// W7_SET (w7_set.h): 0 = v2a lists, 1 = R (former teammate's ladder report), 2 = D (intel data), 3 = P1 (per-map pick
// after the panel test). Source of every D list: tools/v3/w7_drafts.py over intel/matches-2026-10-18.csv: the
// highest shrunk rating-adjusted excess (excess * n / (n + 30)) among drafts with n >= 30 on that map, Willow
// excluded (heal never cast by our code), ties within 2 pp broken toward full kit use (Ren/Vendetta/Issac dashes are
// never cast), then more distinct teams. List order = pick order; the last entry is the one part 2 may replace.
#pragma once
#include <cambc.h>
#include <vector>
#include "mapid.h"
#if __has_include("w7_set.h")
#include "w7_set.h"
#endif
#ifndef W7_SET
#define W7_SET 3
#endif

namespace w7 {

using D = cambc::champion_draft;
using L = std::vector<D>;

// R: "Tavi/Stellar/Ryth/Varg/Ren; Cassi & Issac first on small maps; Fish's roster on swirl".
inline const L* draft_r(mapid::MapId id) {
    using M = mapid::MapId;
    static const L five{D::tavi, D::stellar, D::ryth, D::varg, D::ren};
    static const L three{D::cassi_and_issac, D::tavi, D::stellar};
    static const L four{D::cassi_and_issac, D::tavi, D::stellar, D::ryth};
    static const L fish{D::cassi_and_issac, D::stellar, D::ryth, D::ren};   // Intercontinental Fish, swirl 2-0 (CSV)
    switch (id) {
        case M::ambush: case M::contest: case M::corners_old: case M::corners_v2: case M::eight: case M::elkring:
        case M::lanes_old: case M::lanes_v2: case M::monolith: case M::pilgrimage: case M::stampede: return &five;
        case M::collision_old: case M::collision_v2: case M::expanse_old: case M::expanse_v2: case M::fragment:
        case M::packs_old: case M::packs_v2: return &three;
        case M::interlock_old: case M::interlock_v2: case M::rows: case M::rushbait: return &four;
        case M::swirl: return &fish;
        default: return nullptr;
    }
}

// D: data lists (n, rating-adjusted excess in pp, shrunk, distinct teams).
inline const L* draft_d(mapid::MapId id) {
    using M = mapid::MapId;
    static const L ambush{D::cassi_and_issac, D::stellar, D::varg, D::ryth, D::tavi};          // n81 +11.8 sh8.6 t27
    static const L collision{D::stellar, D::prism, D::ren};                                      // n31 +17.7 sh9.0 t5
    static const L contest{D::cassi_and_issac, D::stellar, D::tavi, D::ryth, D::prism};         // n40 +18.5 sh10.6 t13
    static const L corners{D::stellar, D::tavi, D::ryth, D::prism, D::ren};                     // n56 +13.4 sh8.7 t11
    static const L eight{D::cassi_and_issac, D::stellar, D::tavi, D::ryth, D::ren};             // n86 +7.4 sh5.5 t17
    static const L elkring{D::cassi_and_issac, D::stellar, D::varg, D::ryth, D::tavi};          // n99 +1.1 sh0.9 t27
    static const L expanse{D::cassi_and_issac, D::varg, D::ryth};                                // n32 +37.5 sh19.3 t12
    static const L fragpacks{D::cassi_and_issac, D::stellar, D::tavi};                           // fragment n70 +10.1; packs n80 +12.5
    static const L interlock{D::cassi_and_issac, D::stellar, D::varg, D::ren};                  // n154 +6.5 sh5.4 t47
    static const L lanes{D::cassi_and_issac, D::stellar, D::varg, D::ryth, D::tavi};            // n67 +9.5 sh6.5 t23
    static const L monolith{D::cassi_and_issac, D::varg, D::ryth, D::ren, D::vendetta};         // n53 +21.3 sh13.6 t10
    static const L pilgrimage{D::cassi_and_issac, D::stellar, D::varg, D::ryth, D::ren};        // n329 +8.0 sh7.3 t102
    static const L rows{D::cassi_and_issac, D::varg, D::stellar, D::vendetta};                  // n79 +12.5 sh9.0 t21
    static const L rushbait{D::cassi_and_issac, D::stellar, D::ryth, D::ren};                   // n185 +6.3 sh5.4 t54
    static const L stampede{D::cassi_and_issac, D::stellar, D::prism, D::tavi, D::ryth};        // n116 +16.9 sh13.4 t32
    static const L swirl{D::cassi_and_issac, D::stellar, D::varg, D::ryth};                     // n138 +6.1 sh5.0 t44
    switch (id) {
        case M::ambush: return &ambush;
        case M::collision_old: case M::collision_v2: return &collision;
        case M::contest: return &contest;
        case M::corners_old: case M::corners_v2: return &corners;
        case M::eight: return &eight;
        case M::elkring: return &elkring;
        case M::expanse_old: case M::expanse_v2: return &expanse;
        case M::fragment: case M::packs_old: case M::packs_v2: return &fragpacks;
        case M::interlock_old: case M::interlock_v2: return &interlock;
        case M::lanes_old: case M::lanes_v2: return &lanes;
        case M::monolith: return &monolith;
        case M::pilgrimage: return &pilgrimage;
        case M::rows: return &rows;
        case M::rushbait: return &rushbait;
        case M::stampede: return &stampede;
        case M::swirl: return &swirl;
        default: return nullptr;
    }
}

// P1: per-map choice after the panel test (nullptr = v2a's list). Admitted by cmpcells over results/v2-panel.json
// (3 salts): eight D (dW +2.00, dStrong +1.00, dGem +2,684; results/w7d-panel.json; R also +2.00 but dGem +1,866);
// stampede R = Tavi, Stellar, Ryth, Varg, Ren (dW +2.00, dStrong +1.33, dGem +2,760; results/w7r-panel.json).
inline const L* draft_p1(mapid::MapId id) {
    using M = mapid::MapId;
    switch (id) {
        case M::eight: return draft_d(id);
        case M::stampede: return draft_r(id);
        default: return nullptr;
    }
}

#ifndef W7_CD_ALL
#define W7_CD_ALL 0
#endif
// Part 2 flag per map: on everywhere only in a test build (W7_CD_ALL 1); otherwise only maps listed here.
inline bool counter_on(mapid::MapId id) {
    (void)id;
    return W7_CD_ALL != 0;
}

inline const L* draft_for(mapid::MapId id) {
#if W7_SET == 1
    return draft_r(id);
#elif W7_SET == 2
    return draft_d(id);
#elif W7_SET == 3
    return draft_p1(id);
#else
    (void)id;
    return nullptr;
#endif
}

}  // namespace w7
