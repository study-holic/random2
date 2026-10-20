// w14 duel-lab bot: dispatch draft and gameplay; a failed turn is logged and still yields.
#include <cambc.h>
#include <iostream>
void duel_draft();
void duel_turn();
int main() {
    while (true) {
        try {
            if (cambc::is_draft()) duel_draft(); else duel_turn();
        } catch (const std::exception& e) {
            std::cout << "bot error: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "bot error: unknown" << std::endl;
        }
        cambc::end_turn();
    }
}
