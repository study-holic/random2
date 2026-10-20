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
// map_interlock.cpp: interlock plan. Empty = v4.14 behaviour.
#include "plan.h"
#include "bot.h"

namespace Plan {

void setup_interlock(MapPlan& p) { (void)p; }

}  // namespace Plan
} // namespace va
