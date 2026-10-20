// w3 dispatcher: one WASM holding two complete bots, A = v1 (namespace va, src/a/) and
// B = b-v418d (namespace vb, src/b/). Each process identifies the map once (va::mapid, one
// cached camp_groups() call that A's own code then reuses) and routes draft and play to one bot.
#include <cambc.h>
#include <iostream>
#include <vector>
namespace va {
#include "a/mapid.h"
void dispatch_init();
void dispatch_draft();
void dispatch_turn();
}
namespace vb {
void dispatch_init();
void dispatch_draft();
void dispatch_turn();
}
#include "route.h"

#if __has_include("salt.h")
#include "salt.h"   // written by scripts/mapgate.py --salts: a noise sample of the same bot
#endif
#ifndef P_SALT
#define P_SALT 0
#endif

int main() {
    const bool use_b = w3_route_b(va::mapid::current_map());
    if (use_b) vb::dispatch_init(); else va::dispatch_init();
    bool salted = false;
    while (true) {
        try {
            if (cambc::is_draft()) {
                if (use_b) vb::dispatch_draft(); else va::dispatch_draft();
            } else {
                if (!salted) {
                    salted = true;
                    if (P_SALT > 0) {   // burn a fixed amount of first-turn fuel: fuel-dependent code then takes another trajectory
                        volatile unsigned sink = 0;
                        for (unsigned i = 0; i < P_SALT * 4000u; ++i) sink = sink + i;
                    }
                }
                if (use_b) vb::dispatch_turn(); else va::dispatch_turn();
            }
        } catch (const std::exception& error) {
            std::cout << "bot error: " << error.what() << std::endl;
        } catch (...) {
            std::cout << "bot error: unknown exception" << std::endl;
        }

        cambc::end_turn();
    }
}
