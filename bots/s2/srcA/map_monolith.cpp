// map_monolith.cpp (w9): optional Monolith siege on the corner Monoliths (1,1)/(48,48), each ringed by eight gem
// clusters in one group (200 + 8 x 24 gems). Off by default (P_SIEGE_MONOLITH 0 = v2 behaviour).
// v3a: w8 build C on (Tavi takes the Focus damage build and buys in the field; results/w8-builds.md, monolith 12/12,
// head-to-head 6/6). The flag is read only by Plan::init, so every other map keeps v2a's buying.
#include "plan.h"
#include "bot.h"
#include "siege.h"
#include "params.h"

#ifndef P_SIEGE_MONOLITH
#define P_SIEGE_MONOLITH 0
#endif

namespace Plan {

void setup_monolith(MapPlan& p) {
    p.tavi_focus_field = true;   // w8 build C
    if (!P_SIEGE_MONOLITH) return;
    Siege::Options o; o.on = true;
    Siege::enable(o);
    p.init = Siege::init;
    p.pre = Siege::pre;
    p.mid = Siege::mid;
    p.buy = Siege::buy;
}

}  // namespace Plan
