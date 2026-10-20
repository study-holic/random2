# v4 baseline against the top 2-10 (A-B-A reference for every v5 candidate)

180 games = 9 teams (ranks 2-10 by queue label) x 20 maps (the 21-map pool minus 01big_world, which is in no set), one game each, sets A-D (A: 00eight 00contest 00ambush 00collision 00corners; B: 01corridors 01broken_curve 01gold_rush 01boss_battle 01concentrated_wealth; C: 00interlock 00elkring 01hooks 00monolith 00expanse; D: 00lanes 00fragment 01elk_arena 01fracture 01islands). Queued 20 Oct 09:51 (A, B) and 10:49 (C, D) BST; engine 1.0.0; live bot v4 (platform version 3). Per-game rows: `data/real/v4-panel/baseline-v4top.csv` (pair by team + map). Code: `tools/v3/panel_v4_top10_full.py`. Per-map cells have n=9 (flagged †, fewer than 30); per-team n=20 (†); only the overall row (n=180) is above 30. "Gems" = submitted (tributed) gems; us / them in every cell. Gems lost = `gemLoss` entries with context champion, owner assumed to be the loser (not checked in a replay). Rating adjustment is not applicable (one bot, fixed opponents; a later panel against the same teams and maps is paired directly). Gear points at round 200 and locks placed are NOT decoded (the summary's buyItem amount fields are ambiguous; no cheap reading).

## Tables (data)
| group | n | W-L | gems us / them | gems per round us / them | deaths us / them | tributes us / them | gems lost to kills us / them | mean rounds |
|---|---|---|---|---|---|---|---|---|
| all | 180 | 2-178 | 201 / 976 | 0.18 / 0.85 | 23.4 / 4.3 | 11.6 / 33.6 | 58 / 3 | 1142 |

### Per team
| group | n | W-L | gems us / them | gems per round us / them | deaths us / them | tributes us / them | gems lost to kills us / them | mean rounds |
|---|---|---|---|---|---|---|---|---|
| t02 procless | 20 | 0-20 | 174 / 961 | 0.16 / 0.89 | 22.2 / 0.8 | 9.7 / 31.4 | 59 / 0 | 1075 |
| t03 Nyx | 20 | 0-20 | 212 / 935 | 0.18 / 0.81 | 25.9 / 6.0 | 10.4 / 30.4 | 58 / 2 | 1158 |
| t04 Mite | 20 | 1-19 | 290 / 906 | 0.23 / 0.73 | 19.8 / 4.3 | 18.1 / 41.4 | 45 / 5 | 1241 |
| t05 Sponge(Albert and Bob) | 20 | 0-20 | 159 / 1021 | 0.14 / 0.93 | 25.2 / 2.3 | 10.3 / 30.2 | 66 / 3 | 1096 |
| t06 Pantheon | 20 | 0-20 | 199 / 962 | 0.18 / 0.88 | 19.9 / 3.6 | 12.4 / 41.0 | 66 / 0 | 1095 |
| t07 Tartarus | 20 | 0-20 | 176 / 1022 | 0.16 / 0.93 | 22.7 / 1.8 | 10.3 / 32.0 | 58 / 6 | 1095 |
| t08 Univardisty of Shingtonwash | 20 | 1-19 | 237 / 997 | 0.19 / 0.81 | 23.6 / 11.0 | 12.4 / 30.7 | 47 / 7 | 1238 |
| t09 NEETCoders | 20 | 0-20 | 163 / 965 | 0.15 / 0.86 | 27.6 / 3.5 | 9.1 / 34.5 | 60 / 2 | 1122 |
| t10 Sneaky Bush Campers | 20 | 0-20 | 199 / 1013 | 0.17 / 0.87 | 23.9 / 5.5 | 11.7 / 30.9 | 66 / 2 | 1159 |

### Per map id
| group | n | W-L | gems us / them | gems per round us / them | deaths us / them | tributes us / them | gems lost to kills us / them | mean rounds |
|---|---|---|---|---|---|---|---|---|
| 00ambush | 9† | 0-9 | 484 / 1012 | 0.36 / 0.75 | 13.8 / 4.6 | 12.0 / 23.3 | 50 / 4 | 1342 |
| 00collision | 9† | 0-9 | 82 / 1014 | 0.08 / 1.04 | 24.8 / 0.9 | 6.4 / 38.7 | 142 / 1 | 977 |
| 00contest | 9† | 0-9 | 0 / 822 | 0.00 / 0.57 | 48.7 / 7.3 | 0.0 / 4.1 | 0 / 0 | 1448 |
| 00corners | 9† | 0-9 | 239 / 1009 | 0.29 / 1.22 | 12.1 / 1.0 | 20.8 / 44.1 | 76 / 0 | 823 |
| 00eight | 9† | 0-9 | 92 / 1011 | 0.12 / 1.27 | 25.7 / 2.9 | 3.6 / 28.3 | 115 / 10 | 796 |
| 00elkring | 9† | 0-9 | 254 / 1008 | 0.38 / 1.52 | 18.6 / 1.9 | 7.4 / 28.8 | 161 / 2 | 662 |
| 00expanse | 9† | 0-9 | 622 / 1014 | 0.56 / 0.91 | 3.2 / 1.6 | 46.3 / 49.1 | 13 / 0 | 1110 |
| 00fragment | 9† | 0-9 | 32 / 1009 | 0.02 / 0.75 | 52.7 / 1.9 | 5.1 / 49.1 | 36 / 7 | 1339 |
| 00interlock | 9† | 0-9 | 175 / 1017 | 0.17 / 1.00 | 25.2 / 2.0 | 13.4 / 34.1 | 73 / 0 | 1015 |
| 00lanes | 9† | 0-9 | 392 / 1009 | 0.40 / 1.04 | 16.6 / 3.4 | 28.8 / 54.4 | 54 / 5 | 971 |
| 00monolith | 9† | 0-9 | 324 / 1029 | 0.34 / 1.07 | 22.1 / 1.3 | 25.6 / 38.6 | 107 / 6 | 962 |
| 01boss_battle | 9† | 1-8 | 0 / 479 | 0.00 / 0.27 | 1.2 / 12.7 | 0.0 / 4.1 | 5 / 0 | 1786 |
| 01broken_curve | 9† | 0-9 | 214 / 1020 | 0.21 / 1.00 | 17.8 / 4.6 | 7.4 / 39.9 | 27 / 5 | 1022 |
| 01concentrated_wealth | 9† | 0-9 | 78 / 1022 | 0.07 / 0.91 | 25.4 / 1.8 | 5.8 / 34.4 | 64 / 1 | 1118 |
| 01corridors | 9† | 0-9 | 139 / 1017 | 0.10 / 0.72 | 17.7 / 14.1 | 6.3 / 18.4 | 57 / 8 | 1412 |
| 01elk_arena | 9† | 0-9 | 10 / 1010 | 0.01 / 0.89 | 23.2 / 2.8 | 0.2 / 21.8 | 0 / 2 | 1137 |
| 01fracture | 9† | 0-9 | 156 / 1022 | 0.12 / 0.82 | 16.8 / 5.8 | 12.0 / 30.1 | 42 / 6 | 1252 |
| 01gold_rush | 9† | 0-9 | 265 / 1024 | 0.17 / 0.64 | 39.8 / 10.4 | 18.7 / 52.7 | 87 / 0 | 1603 |
| 01hooks | 9† | 0-9 | 2 / 1007 | 0.00 / 0.96 | 55.1 / 4.0 | 0.1 / 46.2 | 26 / 4 | 1052 |
| 01islands | 9† | 1-8 | 461 / 958 | 0.45 / 0.94 | 7.9 / 1.7 | 12.0 / 31.7 | 30 / 0 | 1016 |



The first row (all) is the overall figure.

Wins (2 of 180): v4top-t04-B 01boss_battle (0-0 gems, tiebreak) and v4top-t08-D 01islands (1006 vs 549 gems). Draws with equal gems: 6 (only 1 won).
Split: 11 old maps (00*, n=99) 0-99, gems 245 / 996, deaths 23.9 / 2.6; 9 new maps (01*, n=81) 2-79, gems 147 / 951, deaths 22.8 / 6.4.

## Interpretation (labelled)
- The reference to beat is not win rate (2/180) but the gap measures: our tributed gems 201 vs 976, deaths 23.4 vs 4.3, tributes 11.6 vs 33.6. A candidate that stays 0-180 can still be compared by those and by (team, map) pairs.
- Top teams end most games near 1,000 gems (a 1,000-gem finish), so their "gems per round" mostly tracks game length: games last a mean 1,142 rounds, and 00elkring ends at 662.
- Deaths cluster on 01hooks (55.1 us a game, 0.1 gems), 00fragment (52.7), 00contest (48.7), 01gold_rush (39.8); the mid-team panel showed the same four. 00expanse (46.3 vs 49.1 tributes), 01islands (461 vs 958 gems, one win) and 00ambush (484 vs 1012) are the closest maps.
- Fuel-chaotic noise: a no-op change moves roughly 4 % of local cells (PLAN.md), so a candidate needs a gem or death shift across many maps, not one map.
