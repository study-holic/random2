# v4 vs the top 2-10, sets A and B (90 games, one per team and map, queued 20 Oct 09:51 BST, engine 1.0.0)

Source: `v4top-t02..t10-A/B` in `data/unranked/groups.csv`; `tools/v3/panel_v4_top10.py` (reuses `panel_v4_vs_v3b.py`). Sets: A = 00eight, 00contest, 00ambush, 00collision, 00corners; B = 01corridors, 01broken_curve, 01gold_rush, 01boss_battle, 01concentrated_wealth (fragment not in A/B). Rank labels t02-t10 are the queue labels. All cells n<=10 per team or n=9 per map (flagged †). Ratings are not needed: this is a single bot against nine opponents. Gems lost = `gemLoss` entries with context champion (owner reading unchecked). Each cell is us / them.

## 1. Overall (data)
| n | W-L | gems | deaths | tributes | gems lost to kills | draws (equal gems) |
|---|---|---|---|---|---|---|
| 90† (n>=30 overall) | 1-89 | 159 / 943 | 22.7 / 6.0 | 8.1 / 28.8 | 62 / 3 | 6 |
The one win is Mite on 01boss_battle (0-0 gems, tiebreak).

## 2. Per team
| team | n | W-L | gems | deaths | tributes | gems lost to kills |
|---|---|---|---|---|---|---|
| t02 procless | 10† | 0-10 | 132 / 906 | 22.0 / 1.0 | 5.9 / 27.4 | 73 / 0 |
| t03 Nyx | 10† | 0-10 | 169 / 857 | 26.5 / 8.7 | 7.6 / 27.5 | 70 / 4 |
| t04 Mite | 10† | 1-9 | 237 / 804 | 20.4 / 4.2 | 12.4 / 35.0 | 38 / 6 |
| t05 Sponge | 10† | 0-10 | 146 / 1020 | 22.9 / 3.1 | 8.8 / 25.0 | 66 / 3 |
| t06 Pantheon | 10† | 0-10 | 186 / 911 | 19.5 / 5.6 | 9.7 / 34.9 | 72 / 1 |
| t07 Tartarus | 10† | 0-10 | 133 / 1035 | 21.7 / 1.4 | 6.7 / 26.3 | 52 / 1 |
| t08 Univardisty of Shingtonwash | 10† | 0-10 | 157 / 1020 | 23.1 / 16.2 | 8.0 / 29.2 | 47 / 5 |
| t09 NEETCoders | 10† | 0-10 | 119 / 914 | 25.6 / 5.0 | 6.2 / 29.3 | 60 / 3 |
| t10 Sneaky Bush Campers | 10† | 0-10 | 154 / 1020 | 22.5 / 9.0 | 7.6 / 24.7 | 83 / 3 |

## 3. Per map
| map id | n | W-L | gems | deaths | tributes | gems lost to kills |
|---|---|---|---|---|---|---|
| 00ambush | 9† | 0-9 | 484 / 1012 | 13.8 / 4.6 | 12.0 / 23.3 | 50 / 4 |
| 00collision | 9† | 0-9 | 82 / 1014 | 24.8 / 0.9 | 6.4 / 38.7 | 142 / 1 |
| 00contest | 9† | 0-9 | 0 / 822 | 48.7 / 7.3 | 0.0 / 4.1 | 0 / 0 |
| 00corners | 9† | 0-9 | 239 / 1009 | 12.1 / 1.0 | 20.8 / 44.1 | 76 / 0 |
| 00eight | 9† | 0-9 | 92 / 1011 | 25.7 / 2.9 | 3.6 / 28.3 | 115 / 10 |
| 01boss_battle | 9† | 1-8 | 0 / 479 | 1.2 / 12.7 | 0.0 / 4.1 | 5 / 0 |
| 01broken_curve | 9† | 0-9 | 214 / 1020 | 17.8 / 4.6 | 7.4 / 39.9 | 27 / 5 |
| 01concentrated_wealth | 9† | 0-9 | 78 / 1022 | 25.4 / 1.8 | 5.8 / 34.4 | 64 / 1 |
| 01corridors | 9† | 0-9 | 139 / 1017 | 17.7 / 14.1 | 6.3 / 18.4 | 57 / 8 |
| 01gold_rush | 9† | 0-9 | 265 / 1024 | 39.8 / 10.4 | 18.7 / 52.7 | 87 / 0 |

## 4. Contact maps, per top team (data; one game each †; cells us / them)
| team | contest gems | contest deaths | gold_rush gems | gold_rush deaths | eight gems | eight deaths | collision gems | collision deaths |
|---|---|---|---|---|---|---|---|---|
| t02 procless | 0 / 1000 | 39 / 0 | 187 / 1001 | 52 / 0 | 118 / 1009 | 24 / 1 | 64 / 1003 | 22 / 0 |
| t03 Nyx | 0 / 400 | 82 / 11 | 264 / 1001 | 50 / 17 | 152 / 1036 | 24 / 4 | 56 / 1000 | 25 / 1 |
| t04 Mite | 0 / 0 | 67 / 2 | 308 / 1001 | 38 / 1 | 115 / 1007 | 21 / 1 | 162 / 1000 | 14 / 0 |
| t05 Sponge | 0 / 1000 | 31 / 0 | 176 / 1007 | 33 / 11 | 11 / 1007 | 28 / 2 | 72 / 1009 | 26 / 0 |
| t06 Pantheon | 0 / 1000 | 32 / 4 | 297 / 1001 | 37 / 14 | 48 / 1001 | 23 / 6 | 164 / 1035 | 23 / 2 |
| t07 Tartarus | 0 / 1000 | 44 / 5 | 396 / 1098 | 33 / 3 | 70 / 1008 | 21 / 0 | 23 / 1011 | 29 / 0 |
| t08 Shingtonwash | 0 / 1000 | 51 / 36 | 253 / 1082 | 33 / 19 | 107 / 1004 | 28 / 2 | 83 / 1033 | 28 / 4 |
| t09 NEETCoders | 0 / 1000 | 47 / 4 | 77 / 1014 | 50 / 9 | 96 / 1021 | 34 / 6 | 68 / 1009 | 25 / 0 |
| t10 Sneaky Bush | 0 / 1000 | 45 / 4 | 429 / 1007 | 32 / 20 | 107 / 1005 | 28 / 4 | 46 / 1030 | 31 / 1 |

Top teams reach about 1,000 gems (a 1,000 total ends the game early: rounds 970-1,750 on contest, 720-960 on eight and collision) while losing 0-6 champions on eight and collision.

## 5. Interpretation (labelled)
- The top 10 are a different class: v4 takes about 16 % of their gem total and loses 23 champions a game to their 6. The result matches the v3b top-end (Tozo 1-19, uma 1-19).
- On contest the top teams finish the game with 4 tributes (monolith), so contest is lost by the monolith race, not by feeding; our 0 gems and 49 deaths there is the same pattern as against mid teams.
- Gems we lose to kills on collision (142) and eight (115) are the largest; the top teams lose almost none.
