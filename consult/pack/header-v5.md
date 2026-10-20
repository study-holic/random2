# Team Xanarchy, Cambridge Battlecode 2026: context for an outside reviewer (v5, 20 Oct 2026, engine 1.0.0)

Supersedes header-v4 for game facts. Written by the lead agent; facts verified 20 Oct against the SDK 3.0.1 and the engine tables (docs/rules/ENGINE-1.0.md, FACTS.md). The site rules page lags the engine; where they differ the engine and SDK are right. Appendices A to C of v4 describe the 17-map, engine 0.0.3 world and are out of date on maps, rewards and base stats.

## 1. Situation

- Cambridge Battlecode 2026: each team writes a bot (ours is C++ compiled to WebAssembly). Ranked ladder plus tournaments. Sprint 2 submission lock: Sat 24 Oct 23:59 BST. Team Xanarchy = one player (Xans) with AI agents; consultants (DeepSeek, Copilot) are run by Xans.
- Engine 1.0.0 went live overnight 19-20 Oct (CLI 3.0.1, SDK 3.0.1). Our live bot is v3b (uploaded 06:05 BST 20 Oct as platform "bot version 2"). v3c (engine-1.0 fixes) is being built.
- The map pool is 21 maps (was 17): kept ambush, collision, contest, corners, eight, elkring, expanse, fragment, interlock, lanes, monolith; new big_world, boss_battle, broken_curve, concentrated_wealth, corridors, elk_arena, fracture, gold_rush, hooks, islands; retired packs, pilgrimage, rows, rushbait-v2, stampede, swirl. Champions per side: 3 on broken_curve, collision, fragment, gold_rush; 4 on hooks, interlock; 5 on the other 15. Nine kept maps had only camps added (more gem and gold clusters); no map has a golem. Overview: notes/maps-21.md, data/maps-21.png.

## 2. Game facts that changed in engine 1.0 (details in docs/rules/ENGINE-1.0.md)

- 12 draftable champions (Heart of Ruin added; Cassi and Issac remain one pick, two bodies). Heart of Ruin: melee, 1000 HP (+100), strength 40, Focus 100 (+15); passive up to +50% to all damage by missing health; slot 0 self shield 1.5 x Focus for 5 turns; slot 1 dash range 2 with landing damage 1.75 x Focus in radius 1 and a 1-turn stun (first dash with landing effects); all cooldowns 8.
- Prism gains 1.0 Focus per carried gem (was 0.2). Vesper Vale gold bonus 33% (was 20%). Marcella gains 2 gold per gem tributed. Tavi's stun 1.4 x Focus (was 2.2).
- Base stats were rebalanced for 11 of the 12 champions (examples: Ryth strength 50 to 45, Vesper 45 to 50, Varg 50 to 40, Willow strength 45 to 55 and Focus 105 to 120, Cassi health 700 to 600, Stellar Focus 100 to 85, Tavi strength 35 to 45).
- Camps: gem and gold clusters 120 HP (were 500); elk range 4 and monolith range 5 (were 3); a golem camp type exists (5,000 HP, strength 120, range 4) but is on no map. Per-camp rewards read from the map group data: wolf 4 gems / 40 gold, elk 15 / 90, gem cluster 7 gems, gold cluster 20 gold, monolith 200 / 300 (the site table still shows 22/90, 36/160, 24 gems, 35 gold); some map groups carry a x2 multiplier.
- Unchanged: 1,000 gem win or round 2,000 (tiebreaks gems, lifetime gold, kills, Team B); tribute max(1, ceil(gems/10)) turns, interrupted by stun, death or a new altar lock; recall 5 turns (3 with Home); death 60 rounds, killer takes carried gems plus 50 gold; shards 30 gold, emblem at 5; vision 8; fuel 100k per turn (10x first draft turn, 2x first play turn); 8 bytes shared memory, 1,024 bytes draft memory.

## 3. Our bot

v3b = v3a plus the per-champion micro table from the duel lab (no 45%-health walk-home in a fight, spells from full range, Varg shield at engage, dashes in fights). Local results (17-map set, engine 0.0.3, 3 salts): head-to-head vs v3a 63/102, vs v2 60/102; panel 168.0 of 204 (v2 152.3); gem margin up on 14 of 17 maps; elkring -1.00, monolith -0.67, pilgrimage -0.33. Starter 34/34, 0 faults (VERSIONS.md). Not tested on the 10 new maps or on engine 1.0 before upload; a safety run on the new maps is in results/newmaps-safety.md. Earlier (v4 header) our v2 sat at 1,417 rating after 515 games (46% wins).

## 4. What the measurements do not cover

Every local test, duel table, draft table, ladder yardstick (intel/) and map-economics figure before 20 Oct was measured on engine 0.0.3 and the 17-map pool. Prism, Marcella, Vesper, Willow, Tavi and Heart of Ruin figures and the cluster HP and reward figures are the most affected. Treat those results as hypotheses with stated test conditions.
