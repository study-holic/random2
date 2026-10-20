Read tools/cloud/v5-common.md first and follow it. Your copy: bots/w1. Module: CAMP SAFETY (monster deaths) and v4's three lost maps.

Why: in v3b's 160 real-team games, 48 % of our deaths (2,897, 18 a game) were to monsters near camps or altars, against 22 % for top-10 teams (notes/step0-deaths.md); they cost time more than gems (23 gems destroyed a game). Measured rules (results/arena-1020.md): first-damage range wolf 3, elk 4, monolith 5; a monster hits the NEAREST champion of either team; asleep camps never attack; monsters see and hit through bushes.
v4 still loses corners 0/6, expanse 0/6 and lanes 1/6 to v3b-e1 (results/v4-final-h2h.json), untraced.

Tasks:
1. Trace one v4 loss each on corners, expanse and lanes vs v3b-e1: cause of the gem deficit (deaths? routes? idle time? a plan from map_corners/map_expanse/map_lanes written for the old layout?). Fix the cause behind a switch; if a per-map plan is the cause and the safe default plays better there, the fix may be to use the default on that map.
2. Camp threat model: v4 treats elk as range 3 for paths (P_CAMP_RANGE 2; range 4 for all kinds cost wins in the cloud ablation, results/v4-cloud.md). Find out why range 4 hurt (probably stand or wait tiles excluded) and build a model that uses the true ranges where it matters: never be the nearest champion to an awake camp unless attacking it with enough HP to finish (HP needed = camp HP / our team dps x the camp's damage per round); hold and queue outside range; wake a camp only when the squad can kill it.
3. Measure deaths by cause on your head-to-heads with the step0 method (tools/step0/) for v4 and your copy.
Expected effect: monster deaths per game down by a third without fewer gems per round.
