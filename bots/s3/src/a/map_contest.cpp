// w3 dispatcher copy: system headers at global scope, bot code in namespace va
#include <algorithm>
#include <array>
#include <cambc.h>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
namespace va {
// map_contest.cpp (w9): Monolith siege. Contest's only gem source is its two Monoliths (17,31) and (32,18) (200 gems
// each, 20,000 health; every other camp is a gold cluster), so v2 ends 0-0 on gems. Casters poke them from range 4
// (siege.cpp). P_SIEGE_DRAFT 1 drafts the caster-heavy C&I + Prism + Ryth + Stellar + Tavi (top-50 contest draft,
// 72% over 25 games, intel/meta-2026-10-18.md section 3a); 0 keeps v2's list.
#include "plan.h"
#include "bot.h"
#include "siege.h"
#include "params.h"
#include "s3_contest.h"

#ifndef P_S3_CONTEST
#define P_S3_CONTEST 1   // v5 S3: contest plays (s3_contest.cpp); v4 scored 0 gems on contest against every real team
#endif
#ifndef P_S3_DRAFT
#define P_S3_DRAFT 1     // v5 S3: 1 = the top-10 core (Stellar, C&I, Ren, Varg, Ryth); 0 = v4 lists
#endif

#ifndef P_SIEGE_CONTEST
#define P_SIEGE_CONTEST 0   // FIX_SIEGE_OFF: engine 1.0 monolith range 5 > siege cast range 4 (125 deaths on the ladder)
#endif
#ifndef P_SIEGE_ESCORT
#define P_SIEGE_ESCORT 1
#endif
#ifndef P_SIEGE_TANK
#define P_SIEGE_TANK 1
#endif
#ifndef P_SIEGE_SAFE
#define P_SIEGE_SAFE 6
#endif
#ifndef P_SIEGE_CASSI_FARMS
#define P_SIEGE_CASSI_FARMS 0
#endif
#ifndef P_SIEGE_DRAFT
#define P_SIEGE_DRAFT 1
#endif

namespace Plan {

void setup_contest(MapPlan& p) {
    if (P_S3_CONTEST) {
        using D = cambc::champion_draft;
        S3C::enable();
        if (P_S3_DRAFT == 1) p.draft = {D::stellar, D::cassi_and_issac, D::ren, D::varg, D::ryth};
        p.init = S3C::init;
        p.pre = S3C::pre;
        p.buy = S3C::buy;
        return;
    }
    if (!P_SIEGE_CONTEST) return;
    Siege::Options o; o.on = true; o.escort = P_SIEGE_ESCORT; o.tank_capture = P_SIEGE_TANK; o.safe_tribute = P_SIEGE_SAFE; o.cassi_farms = P_SIEGE_CASSI_FARMS;
    Siege::enable(o);
    using D = cambc::champion_draft;
    if (P_SIEGE_DRAFT == 1) p.draft = {D::stellar, D::tavi, D::prism, D::cassi_and_issac, D::ryth};
    else if (P_SIEGE_DRAFT == 2) p.draft = {D::stellar, D::tavi, D::prism, D::ren, D::willow};
    p.init = Siege::init;
    p.pre = Siege::pre;
    p.mid = Siege::mid;
    p.buy = Siege::buy;
}

}  // namespace Plan
} // namespace va
