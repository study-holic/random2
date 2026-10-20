#include <string>
// Dispatch draft and gameplay; a failed turn is logged and still yields.
#include <cambc.h>
#include <iostream>
#include "bot.h"
#include "draft.h"

#include <iostream>
using namespace cambc;
static void probe(){ static bool done=false; if(done) return; done=true;
 std::cout<<"PROBE"; for(auto a: map::altars().positions()) std::cout<<" ALTAR "<<a.x<<","<<a.y; std::cout<<" SP "; for(auto a: map::spawn_region(team::ally).positions()) std::cout<<a.x<<","<<a.y<<";"; std::cout<<" RANGE "<<constants::submit_gems_range;
 int seen[16]={0};
 for(auto grp: map::camp_groups()){
  std::string k; int hp=0, r=0; int n=0; std::string nm;
  for(const auto& c: grp.camps()){ ++n; int kd=(int)c.kind(); nm+=to_string(c.kind())+":"+std::to_string(c.attack_range())+" ";  }
  std::cout<<" | "<<nm<<" n="<<n<<" gold="<<grp.gold_value()<<" gems="<<grp.gem_value()<<" respawn="<<grp.max_respawn_timer();
 }
 std::cout<<std::endl;}
int main() {
    Bot bot;
    while (true) {
        try {
            if (cambc::is_draft())
                play_draft();
            else
                { probe(); }
        } catch (const std::exception& error) {
            std::cout << "bot error: " << error.what() << std::endl;
        } catch (...) {
            std::cout << "bot error: unknown exception" << std::endl;
        }

        cambc::end_turn();
    }
}
