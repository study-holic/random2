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
// map_expanse.cpp: expanse plan. Empty = v4.14 behaviour.
#include "plan.h"
#include "bot.h"

namespace Plan {

void setup_expanse(MapPlan& p) { (void)p; }

}  // namespace Plan
} // namespace va
