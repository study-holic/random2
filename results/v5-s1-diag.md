# v5 S1 diagnostics: v4 unopposed (20 Oct 2026, cloud)

Engine 1.0.0, CLI 3.0.1, 21-map pool, `--jobs 3`. Opponent `bots/idle` (drafts the first champion not taken, then passes every turn). 21 maps × 2 sides × salt 0 = 42 games per row set (74 s). Brief: tools/cloud/v5-s1-economy.md, step 1.

- `bots/v4` (unchanged): `results/s1/v4-vs-idle.json`.
- `bots/s1diag` = `bots/s1` (= v4) with `P_S1_DIAG 1` (bot.cpp `s1_diag`): each champion, at the end of its turn, finds the group whose spawn tile it stands on, reads the group state (SDK `camp_group::state`) and counts rounds: on a spawn tile (on), on a respawning group with timer 0 (self-block: arena test 3 says only the occupant then holds the respawn), on a reward-available group (capt), and with a second ally on the same group's tiles (shared, any state). Counts are champion-rounds summed over the team, mean of the two sides. The prints change fuel use, so the trajectories differ slightly from v4; gems and rounds came out identical on every map.
- "Round to 1,000" is the game's last round when 1,000 tributed gems were reached; otherwise projected as 2000 × 1000 / gems (inf with 0 gems). Mean of the two sides. Metric script: `tools/s1/diag.py`.

## v4 vs idle
| map | round to 1,000 (proj.) | gems | deaths |
|---|---|---|---|
| ambush | 1899 | 1032 | 6.0 |
| big_world | 2114 | 946 | 4.5 |
| boss_battle | inf | 0 | 1.5 |
| broken_curve | 17536 | 125 | 5.5 |
| collision | 1270 | 1014 | 2.5 |
| concentrated_wealth | 2651 | 776 | 0.0 |
| contest | inf | 0 | 3.5 |
| corners | 1160 | 1003 | 0.0 |
| corridors | 11364 | 198 | 7.0 |
| eight | 1292 | 1009 | 0.0 |
| elk_arena | 5093 | 396 | 1.0 |
| elkring | 854 | 1012 | 0.0 |
| expanse | 1454 | 1010 | 0.0 |
| fracture | 6327 | 317 | 12.0 |
| fragment | 1623 | 1004 | 0.5 |
| gold_rush | 7242 | 358 | 20.5 |
| hooks | 1254 | 1018 | 2.5 |
| interlock | 1141 | 1013 | 0.5 |
| islands | 1394 | 1028 | 1.5 |
| lanes | 1286 | 1006 | 6.0 |
| monolith | 1370 | 1009 | 0.5 |
median round to 1,000 over 21 maps: 1623; reached on 12 maps|

## Self-block and shared tiles (s1diag vs idle)
| map | round to 1,000 (proj.) | gems | deaths | on-tile | self-block | shared | capt | shared-capt |
|---|---|---|---|---|---|---|---|---|
| ambush | 1899 | 1032 | 6.0 | 496 | 0 | 12 | 367 | 0 |
| big_world | 2114 | 946 | 4.5 | 906 | 0 | 0 | 506 | 0 |
| boss_battle | inf | 0 | 1.5 | 248 | 0 | 0 | 78 | 0 |
| broken_curve | 17536 | 125 | 5.5 | 96 | 0 | 2 | 66 | 0 |
| collision | 1270 | 1014 | 2.5 | 970 | 0 | 1 | 656 | 0 |
| concentrated_wealth | 2651 | 776 | 0.0 | 1343 | 601 | 371 | 422 | 0 |
| contest | inf | 0 | 3.5 | 2418 | 3 | 0 | 1430 | 0 |
| corners | 1160 | 1003 | 0.0 | 632 | 0 | 4 | 501 | 0 |
| corridors | 11364 | 198 | 7.0 | 214 | 0 | 4 | 80 | 0 |
| eight | 1292 | 1009 | 0.0 | 400 | 0 | 4 | 276 | 0 |
| elk_arena | 5093 | 396 | 1.0 | 1347 | 2 | 12 | 1160 | 0 |
| elkring | 854 | 1012 | 0.0 | 299 | 0 | 14 | 293 | 0 |
| expanse | 1454 | 1010 | 0.0 | 852 | 0 | 0 | 686 | 0 |
| fracture | 6327 | 317 | 12.0 | 316 | 0 | 2 | 245 | 0 |
| fragment | 1623 | 1004 | 0.5 | 597 | 0 | 10 | 496 | 0 |
| gold_rush | 7242 | 358 | 20.5 | 398 | 0 | 0 | 232 | 0 |
| hooks | 1254 | 1018 | 2.5 | 1334 | 6 | 52 | 966 | 0 |
| interlock | 1141 | 1013 | 0.5 | 429 | 0 | 0 | 360 | 0 |
| islands | 1394 | 1028 | 1.5 | 374 | 0 | 2 | 235 | 0 |
| lanes | 1286 | 1006 | 6.0 | 1044 | 0 | 2 | 756 | 0 |
| monolith | 1370 | 1009 | 0.5 | 1336 | 0 | 4 | 924 | 0 |
median round to 1,000 over 21 maps: 1623; reached on 12 maps

## Reading
1. **Unopposed v4 reaches 1,000 gems on only 12 of 21 maps within 2,000 rounds.** Median projected round 1,623 (top-10 teams median 1,189 in contested wins, notes/top10-2026-10-20.md). **contest and boss_battle: 0 gems in 2,000 rounds with no opponent**; broken_curve 125, corridors 198, fracture 317, gold_rush 358, elk_arena 396, concentrated_wealth 776, big_world 946. These nine maps are the economy gap, before any enemy.
2. **Self-blocking (timer 0, our own champion on the tile) is rare**: 601 champion-rounds a game on concentrated_wealth, 0-6 elsewhere. P_S1_NO_SELF_BLOCK can matter on concentrated_wealth alone.
3. **Two allies on one group's tiles**: 371 champion-rounds on concentrated_wealth, 52 on hooks, at most 14 elsewhere. The counter includes clearing rounds, when allies stand on the dead members' tiles while fighting the rest; no capture-phase sharing was split out in this run.
4. Time on reward-available tiles is large where gems are scarce: contest 1,430 champion-rounds, elk_arena 1,160, hooks 966. On contest these rounds produced no tributed gems (gold-only groups or captures never completing): traced next.
