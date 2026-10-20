# v3b real-team panel, unranked (160 games, 8 teams x 20 maps; queued 20 Oct 06:12-06:16 BST, finished before 06:30)

Source: `data/unranked/groups.csv` (32 groups labelled v3b-*), `data/unranked/games_v3b.tsv` (per game), decode by `cambc matches summary --json` (`data/unranked/sumj/`) read by `tools/v3/analyze_v3b_panel.py` and `tools/v3/note_v3b_panel.py` (tables in `data/unranked/v3b_tables.txt`). Engine 1.0.0: `tools/v3/decode.mjs` rejects it (no WASM), so the platform summary replaces the round-level decode. Each team-map pair is played once (no repeats). Every cell below is n<=20 per team or n<=8 per map: all flagged †. Cells are us / them. Tributed per round = tributed gems / rounds played. Deaths = champion deaths per game.

## 0. Live check
v3b is live: on 00eight the draft is 3.4.10.0.8 or 3.4.9.0.8 (Cassi & Issac, Stellar, Tavi/Varg, Ryth, Ren) against v2's 2.3.9.0.10 (Vendetta, C&I, Varg, Ryth, Tavi); the 00eight game of group v3b-micron-A (17a34c9c) shows it, engine 1.0.0. Quota used 32 of 50 (the 00eight probe counted as group A of Micron). No wait needed.

## 1. Overall and per team (data)
| group | games | W-L | gems us / them | median rounds | tributed per round us / them | tributes us / them | deaths us / them |
|---|---|---|---|---|---|---|---|
| all | 160 | 29-131 | 383 / 781 | 1663 | 0.24 / 0.48 | 19.3 / 37.5 | 37.9 / 21.4 |
| Micron (1524.5) | 20† | 2-18 | 329 / 758 | 1467 | 0.21 / 0.49 | 17.1 / 51.2 | 37.8 / 22.7 |
| Deer Park (1510.5) | 20† | 3-17 | 384 / 812 | 1535 | 0.25 / 0.52 | 20.0 / 47.4 | 35.6 / 17.4 |
| tvojtatko (1483.0) | 20† | 5-15 | 446 / 722 | 2000 | 0.24 / 0.40 | 22.8 / 57.0 | 42.2 / 25.2 |
| 777 (1453.0) | 20† | 13-7 | 611 / 524 | 2000 | 0.34 / 0.29 | 29.4 / 38.2 | 33.6 / 32.6 |
| Snowman Workshop (1563.8) | 20† | 2-18 | 400 / 850 | 1712 | 0.24 / 0.52 | 18.1 / 26.1 | 40.9 / 28.6 |
| hackingnoises (1600.3) | 20† | 2-18 | 428 / 792 | 1639 | 0.26 / 0.48 | 20.8 / 19.1 | 34.0 / 27.1 |
| Tozo's Disciples (1686.0) | 20† | 1-19 | 309 / 900 | 1554 | 0.20 / 0.59 | 15.4 / 25.1 | 43.2 / 9.3 |
| umamusume (1771.4) | 20† | 1-19 | 161 / 888 | 1382 | 0.11 / 0.63 | 10.7 / 36.1 | 36.0 / 8.1 |

Total 29-131. Ratings are the 18:00 ladder values in panel.csv. Expected wins from the 400-point logistic over the 160 games: 55 / 65 / 75 if v3b were rated 1,450 / 1,500 / 1,550 (our rating at this time is not in the data; 1,500 is an assumption). Actual 29, so v3b is below its rating band on this panel (n=160). Gems per tribute are equal (19.9 us, 20.8 them); the gap is the number of tributes (19.3 vs 37.5 per game) and 38 vs 21 deaths per game.

## 2. Per map (data)
| group | games | W-L | gems us / them | median rounds | tributed per round us / them | tributes us / them | deaths us / them |
|---|---|---|---|---|---|---|---|
| 00ambush | 7† | 1-6 | 711 / 946 | 1687 | 0.42 / 0.56 | 17.7 / 26.3 | 14.0 / 17.1 |
| 00collision | 8† | 2-6 | 442 / 890 | 1234 | 0.33 / 0.66 | 17.0 / 44.8 | 19.6 / 16.4 |
| 00contest | 8† | 0-8 | 0 / 325 | 2000 | 0.00 / 0.17 | 0.0 / 1.6 | 119.6 / 32.5 |
| 00corners | 8† | 3-5 | 680 / 868 | 1126 | 0.57 / 0.72 | 46.0 / 44.0 | 11.0 / 14.2 |
| 00eight | 7† | 1-6 | 444 / 889 | 1265 | 0.36 / 0.72 | 16.3 / 39.3 | 26.1 / 28.4 |
| 00elkring | 7† | 0-7 | 526 / 1010 | 807 | 0.60 / 1.14 | 14.7 / 44.0 | 19.6 / 10.4 |
| 00expanse | 8† | 5-3 | 859 / 752 | 1388 | 0.62 / 0.54 | 56.9 / 60.2 | 4.4 / 8.0 |
| 00fragment | 8† | 1-7 | 213 / 900 | 1831 | 0.12 / 0.51 | 16.2 / 61.4 | 64.9 / 16.8 |
| 00interlock | 8† | 3-5 | 649 / 880 | 1512 | 0.45 / 0.60 | 26.2 / 43.1 | 29.2 / 16.5 |
| 00lanes | 8† | 1-7 | 770 / 952 | 1230 | 0.62 / 0.77 | 48.1 / 59.9 | 11.6 / 9.8 |
| 00monolith | 7† | 2-5 | 720 / 944 | 1333 | 0.52 / 0.68 | 49.3 / 50.1 | 17.7 / 13.6 |
| 01big_world | 7† | 3-4 | 743 / 750 | 1936 | 0.40 / 0.41 | 36.3 / 31.0 | 10.6 / 4.4 |
| 01boss_battle | 8† | 3-5 | 0 / 0 | 2000 | 0.00 / 0.00 | 0.0 / 0.0 | 21.1 / 51.0 |
| 01broken_curve | 8† | 1-7 | 149 / 779 | 2000 | 0.08 / 0.42 | 5.9 / 44.6 | 76.8 / 29.2 |
| 01concentrated_wealth | 8† | 1-7 | 344 / 738 | 2000 | 0.18 / 0.39 | 21.9 / 31.0 | 34.2 / 18.5 |
| 01corridors | 7† | 1-6 | 190 / 707 | 2000 | 0.10 / 0.36 | 7.4 / 19.6 | 94.3 / 52.7 |
| 01elk_arena | 7† | 0-7 | 144 / 896 | 1773 | 0.09 / 0.54 | 2.1 / 29.9 | 27.0 / 18.6 |
| 01fracture | 8† | 0-8 | 216 / 740 | 2000 | 0.12 / 0.40 | 8.8 / 25.5 | 49.9 / 16.2 |
| 01gold_rush | 8† | 0-8 | 67 / 823 | 2000 | 0.03 / 0.42 | 5.0 / 55.0 | 73.9 / 25.2 |
| 01hooks | 8† | 1-7 | 175 / 968 | 1282 | 0.12 / 0.68 | 5.9 / 45.6 | 58.6 / 24.6 |
| 01islands | 7† | 0-7 | 70 / 717 | 2000 | 0.04 / 0.38 | 2.0 / 27.1 | 0.4 / 24.1 |

00 maps: 19-65 (n=84). 01 maps: 10-66 (n=76), of which 9 draws (01boss_battle, 0-0 gems, ended by tiebreak: 3 platform wins, 5 losses; 01islands 1 draw 0-0). Map ids are the current ids from the match listing (first 8 hex in games_v3b.tsv).

## 3. v3b vs v2 on the 11 kept maps, paired by team and map (data)
v2 = `data/unranked/games.tsv` (19 Oct 18:12 BST, engine 0.0.3); v3b = engine 1.0.0. Same 8 opponents, so rating adjustment cancels in the pairs. Pairs exist where v2 played that team on that map once (84 of 88).
| map | pairs | v2 W-L | v3b W-L | v2 gems us-them | v3b gems us-them | v2 tribute/rd | v3b tribute/rd | v2 deaths us | v3b deaths us |
|---|---|---|---|---|---|---|---|---|---|
| 00ambush | 7† | 3-4 | 1-6 | -79 | -234 | 1.10 | 0.41 | 2.3 | 14.0 |
| 00collision | 8† | 0-8 | 2-6 | -797 | -449 | 0.38 | 0.29 | 8.9 | 19.6 |
| 00contest | 8† | 1-7 | 0-8 | -225 | -325 | 0.03 | 0.00 | 59.6 | 119.6 |
| 00corners | 8† | 1-7 | 3-5 | -294 | -187 | 1.36 | 0.56 | 4.1 | 11.0 |
| 00eight | 7† | 0-7 | 1-6 | -850 | -445 | 0.39 | 0.34 | 9.3 | 26.1 |
| 00elkring | 7† | 0-7 | 0-7 | -455 | -483 | 0.99 | 0.58 | 10.9 | 19.6 |
| 00expanse | 8† | 4-4 | 5-3 | -26 | 106 | 1.29 | 0.61 | 0.0 | 4.4 |
| 00fragment | 8† | 0-8 | 1-7 | -712 | -687 | 0.40 | 0.11 | 21.6 | 64.9 |
| 00interlock | 8† | 1-7 | 3-5 | -352 | -231 | 0.95 | 0.44 | 8.5 | 29.2 |
| 00lanes | 8† | 2-6 | 1-7 | -160 | -182 | 1.64 | 0.62 | 3.5 | 11.6 |
| 00monolith | 7† | 1-6 | 2-5 | -325 | -224 | 1.43 | 0.51 | 5.1 | 17.7 |
| all 11 | 84 | 13-71 | 19-65 | -387 | -302 | 0.90 | 0.40 | 12.4 | 31.2 |
Per team (pairs, v2 wins, v3b wins): Micron 11, 1, 1; Deer Park 11, 2, 1; tvojtatko 10, 3, 3; 777 10, 3, 9; Snowman 10, 2, 2; hackingnoises 11, 2, 2; Tozo 11, 0, 1; umamusume 10, 0, 0. 777 accounts for 6 of the net 6 extra wins; excluding 777 v3b is 10-64 vs v2 10-64 (equal).
Caveat (data): the 11 maps carry new map ids since the v2 panel (e.g. ambush 699c96a0 -> 1efbe907) and a new engine. Rounds per game doubled for everyone (opponent tributed per round fell from 1.52 to 0.48), so per-round rates are not comparable across the two panels; only W-L, gem margin and deaths are shown as pairs. Deaths per game rose 12.4 -> 31.2 on these maps.
Interpretation: no measurable gain from v2 to v3b on the kept maps (19 vs 13 wins of 84 comes from one opponent, 777, and the engine/map change is a confound).

## 4. Three biggest failure patterns
| # | pattern | evidence | replay id (round) |
|---|---|---|---|
| 1 | New maps are not identified. v3b logs `map unknown` on 6 of 10 new maps (boss_battle, concentrated_wealth, corridors, elk_arena, fracture, islands) and a wrong old-map plan on 4 (big_world as ambush, broken_curve as interlock_old, gold_rush as packs_old, hooks as packs_v2). 0 of 10 correct. On 01islands v3b tributes 0 gems in 6 of 7 games, with 0-2 deaths: the team neither fights nor farms. | 01 maps 10-66 (n=76) vs 00 maps 19-65 (n=84); 01islands 0-7, 70 vs 717 gems | 325a86d9-fad6-4bd5-9859-984707146712 (01islands vs Micron: 0 tributes, 0 deaths by round 1715, they win 1014-0 at round 1715) |
| 2 | Early wipe then feeding. Four of six champions are dead by round 68 in 01gold_rush and 5-6 by round 56 in 00contest, and they keep dying: 01gold_rush 74 deaths per game (all 8 games have 5-8 deaths before round 200); 00contest 120 deaths per game and 0 tributes; 00fragment 65; 01corridors 94; 01broken_curve 77. Killers collect carried gems and 50 gold per kill. | 00contest 0-8, gems 0 vs 325; 01gold_rush 0-8, 67 vs 823 | d010981e-8c50-4018-abdb-2eae3d6a9451 (01gold_rush vs Micron: deaths at rounds 62, 62, 63, 68, then 166-174, 260; 74 total; 99 vs 695 gems); 5f070dad-dcc3-426a-b4dd-8cf96cd15c1f (00contest vs Micron: first deaths 41, 48, 52, 56, 56; 125 total; 0 gems vs 200) |
| 3 | Low tribute count on identified maps, even with fewer deaths. 00elkring 0-7: 14.7 tributes vs 44.0, 526 vs 1010 gems; 00eight 1-6: 16.3 vs 39.3; 00collision 17.0 vs 44.8. Our first tribute is on time (round 69-70) but the second wave is late: at round 300 we hold 56-93 gems against their 53-153, at round 500 56-297 against 224-495. | 00elkring Micron: 17 vs 61 tributes, 605 vs 1016 gems | 33e36682-75cf-4818-bf1e-f0ecd543cd41 (00elkring vs Micron, ends round 946: 88 vs 209 at 300, 220 vs 400 at 500); a0ab9698-6354-4497-8aaa-f7601c3ecf5d (00eight vs tvojtatko: 56 gems at round 500 vs 256) |

## 5. Interpretation (labelled)
- Pattern 1 is the cheapest fix: ten new maps are 48% of the pool (10 of 21) and v3b scores 10-66 on them against 19-65 on the 11 old ones; wrong-map plans (packs_old, interlock_old) look worse than the unknown fallback only where deaths are high (gold_rush 74, broken_curve 77), which suggests a safe default plan beats a wrong specific one. Not tested.
- Pattern 2 looks like fighting a stronger team at their camps; the 00contest result (0 tributes, 120 deaths) matches the v2 baseline (0-8 there too, 60 deaths), so v3b did not change contest behaviour except more deaths.
- 777 is the only team v3b beats (13-7, n=20); the other seven take 2-18 or worse at the same time (Tozo and uma 1-19).
- Draws: 10 games ended with equal gems; the platform tiebreak (gold, then kills) decides them. In 01boss_battle our three wins come from that tiebreak with no gems.
