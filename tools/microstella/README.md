# MicroStella tools

Put this folder's contents in the cambc workspace root (the folder with `bots/`, `maps/`, `cambc.toml`).

| File | Use |
|---|---|
| pace.py <bot> [opp=v20] [maps...] | Runs bot vs opp on both sides (default: the 01 maps + interlock); prints gems@300/600, first tribute, deaths, winner. Caches per bot name in replays/pace_<bot>_vs_<opp>/ — delete that folder after changing the bot's source. Paths inside are absolute to /home/claude/bc: change the prefix in the f.write line to your workspace. |
| evalall.sh <bot> | 168-game benchmark: full pool x 2 sides x v19/v20/v16/v13 (those bots must be built). The only number to trust. |
| addtrace.py <bot> | Patches bots/<bot>/src to print a per-turn state line (T r<round> a<idx> c<type> pos tg hold bank retr hp bag gold ...) plus the DBG trace. Use on a copy of the bot. TRACE_EVERY in bot.cpp sets the interval. |
| trace.sh <botA> <botB> <map> <outdir> | Runs one game and extracts the trace lines to <outdir>/ta.txt and tb.txt. |
| summarize.py replays/<dir> | Results table from --statistics JSONs. |
| reach.py <probe team-a.stdout> | Walk vs one-teleport reachability of every group and altar from the probe's ASCII map. |
| bots/probe | Prints map dims, altars, spawns, groups with walking distance, and the ASCII map (8 rows per turn). Run it vs itself on a map, unzip the replay bundle, read team-a.stdout. |
| bots/elkprobe | Monster wake/regen experiment (MODE 0/1/2 in bot.cpp). |
| bots/lab | Engine-constant probe (TP crystal, recall, capture). |

Bots here are source only: `cambc build probe` works once the folder sits under bots/ (cambc generates its own build files).
