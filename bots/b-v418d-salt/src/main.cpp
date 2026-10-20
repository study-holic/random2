// Dispatch draft and gameplay; a failed turn is logged and still yields.
#include <cambc.h>
#include <iostream>
#include "bot.h"
#include "draft.h"

#if __has_include("salt.h")
#include "salt.h"   // written by scripts/mapgate.py --salts: a noise sample of the same bot
#endif
#ifndef P_SALT
#define P_SALT 0
#endif

int main() {
    Bot bot;
    bool salted = false;
    while (true) {
        try {
            if (cambc::is_draft())
                play_draft();
            else {
                if (!salted) {
                    salted = true;
                    if (P_SALT > 0) {   // burn a fixed amount of first-turn fuel: fuel-dependent code then takes another trajectory
                        volatile unsigned sink = 0;
                        for (unsigned i = 0; i < P_SALT * 4000u; ++i) sink = sink + i;
                    }
                }
                bot.play_turn();
            }
        } catch (const std::exception& error) {
            std::cout << "bot error: " << error.what() << std::endl;
        } catch (...) {
            std::cout << "bot error: unknown exception" << std::endl;
        }

        cambc::end_turn();
    }
}
