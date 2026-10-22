import sys
b=sys.argv[1]; d=f'bots/{b}/src/'
p=d+'bot.cpp'; s=open(p).read()
if 'T r" << rnd' not in s:
    s=s.replace('#if P_ROWS_DBG || P_RUN_DBG\n#include <iostream>\n#include <string>\n#endif','#include <iostream>\n#include <string>')
    s=s.replace('''#else
    play_turn_inner();
#endif
    attacked = could_attack''','''#else
    play_turn_inner();
#endif
    { const int rnd = static_cast<int>(cambc::round_number());
      if (rnd % TRACE_EVERY == 0) { std::array<std::uint8_t, 8> mm{}; cambc::shared_memory.read(0, std::span{mm.data(), mm.size()});
        std::cout << "T r" << rnd << " a" << ally_index << " c" << int(me.champion_name()) << " alive" << me.is_alive() << " pos" << me.position().x << "," << me.position().y
                  << " tg" << target << " hold" << holding << " bank" << banking << " retr" << retreating << " regrp" << regrouping << " wait" << wait_rounds << " mono" << mono_mode
                  << " hp" << (me.is_alive() ? int(me.health()) : -1) << " bag" << int(me.gem_count()) << " gold" << int(me.gold_count())
                  << " mem" << int(mm[0]) << "," << int(mm[1]) << "," << int(mm[2]) << "," << int(mm[3]) << "," << int(mm[4]) << "," << int(mm[5]) << "," << int(mm[6]) << "," << int(mm[7]);
        if (target >= 0 && target < (int)groups.size()) std::cout << " tgready" << groups[target].ready_round << " tgexcl" << groups[target].excluded_until << " tgcont" << groups[target].contested_round;
        std::cout << g_dbg << std::endl; } }
    attacked = could_attack''')
    s=s.replace('#include "params.h"','#include "params.h"\n#ifndef TRACE_EVERY\n#define TRACE_EVERY 5\n#endif',1)
    open(p,'w').write(s)
u=d+'util.h'; t=open(u).read()
if '#include "params.h"' not in t:
    t=t.replace('#include <cambc.h>\n#include <vector>','#include <cambc.h>\n#include "params.h"\n#include <vector>',1); open(u,'w').write(t)
o=d+'params_override.h'; t=open(o).read()
if 'P_ROWS_DBG' not in t: open(o,'a').write('#define P_ROWS_DBG 1\n')
