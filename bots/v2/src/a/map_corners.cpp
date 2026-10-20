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
#include <variant>
#include <vector>
namespace va {
// map_corners.cpp: corners plan. w1 per-map draft: 5 champs; top-10 modal C&I+Ren+Ryth+Stellar+Varg (n=14); Stellar 100% of top-10 games
#include "plan.h"
#include "bot.h"

namespace Plan {

void setup_corners(MapPlan& p) {
    using D = cambc::champion_draft;
    p.draft = {D::cassi_and_issac, D::stellar, D::varg, D::ryth, D::ren, D::tavi};
}

}  // namespace Plan
} // namespace va
