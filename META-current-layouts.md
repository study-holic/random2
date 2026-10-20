# Current-layout meta (ranked games, 16 October, computed 18 October)

Source: `top100-intel/data/group_detail` (platform group details). Ranked matches on the 17 current map ids, with ratings taken from each group's `ratingBefore`. Six maps switched layout at 11:37 UTC on 16 October. Matches on old ids are excluded, so those six maps have about 3.4 hours of data and the other eleven about 7 hours. Samples involve top-100 teams and their opponents (ratings 1,471 to 2,007). Expected wins use a 400-point logistic on group ratings. Data file: `ranked_current_layouts.csv` (19,564 side-rows).

## Map ids

| Map | Current id | Layout change |
|---|---|---|
| ambush, contest, eight, fragment, pilgrimage, rushbait_v2, stampede, swirl | 699c96a0, 3a5c2468, 3e2df5be, 9925f416, 7bfc2edd, df22386f, f2b6903f, f930f43f | published 03:37 UTC 16 Oct |
| elkring, monolith, rows | bb12893c, 3991a8b3, 25d81311 | unchanged |
| collision, corners, expanse, interlock, lanes, packs | 2e9820ee, e3c29b68, 81929320, b777c8c8, 93852276, b135e0fc | replaced 11:37 UTC 16 Oct (old ids 3bf50c50, 8b48383d, 4705f2c8, 49a6f0b9, df106d8d, 81824dd7) |

## Pace yardsticks: winners' median rounds to 1,000 gems (n winning sides)

| Map | Rating ≥ 1,600 | Rating 1,450 to 1,600 | Games reaching round 2,000 |
|---|---|---|---|
| rows | 393 (587) | 431 (163) | 0 per cent |
| monolith | 449 (521) | 482 (177) | 0 |
| lanes | 487 (272) | 524 (77) | 0 |
| packs | 546 (253) | 556 (77) | 0 |
| elkring | 569 (522) | 615 (179) | 0 |
| corners | 582 (257) | 595 (74) | 0 |
| expanse | 586 (259) | 635 (87) | 0 |
| eight | 657 (531) | 693 (162) | 0 |
| swirl | 670 (506) | 705 (165) | 0 |
| rushbait_v2 | 697 (519) | 785 (156) | 6 |
| interlock | 702 (258) | 753 (95) | 0 |
| ambush | 712 (563) | 767 (168) | 0 |
| collision | 736 (275) | 755 (78) | 0 |
| fragment | 782 (566) | 828 (159) | 0 |
| pilgrimage | 902 (506) | 955 (165) | 0 |
| stampede | 1,085 (474) | 1,259 (141) | 10 |
| contest | 1,529 (174) | 1,583 (34) | 69 |

These are contested games. A bot whose uncontested local time to 1,000 exceeds the 1,450 to 1,600 column loses the race to the median opponent at that rating.

## Drafts: best broad sample per map (n ≥ 80 side-games, ≥ 15 teams)

| Map | Draft | n | Win | Expected | z | Most-picked draft (n), z |
|---|---|---|---|---|---|---|
| ambush | C&I, Ren, Ryth, Stellar, Varg | 341 | 60 | 51 | +3.6 | same |
| collision | C&I, Ren, Stellar | 251 | 55 | 50 | +1.7 | same |
| contest | C&I, Ryth, Stellar, Tavi, Varg | 127 | 54 | 50 | +0.9 | C&I, Ren, Stellar, Tavi, Varg (266), +0.7 |
| corners | C&I, Ren, Ryth, Stellar, Varg | 151 | 53 | 50 | +0.6 | same |
| eight | C&I, Ren, Ryth, Stellar, Varg | 260 | 58 | 51 | +2.4 | C&I, Ren, Stellar, Tavi, Varg (266), +0.3 |
| elkring | C&I, Ren, Stellar, Tavi, Varg | 216 | 54 | 50 | +1.0 | C&I, Ren, Ryth, Stellar, Varg (302), -1.2 |
| expanse | C&I, Varg, Vendetta | 224 | 56 | 50 | +1.8 | same |
| fragment | C&I, Ren, Stellar | 332 | 56 | 50 | +2.2 | C&I, Stellar, Varg (343), +0.2 |
| interlock | C&I, Ren, Ryth, Stellar | 124 | 56 | 50 | +1.3 | same |
| lanes | C&I, Ren, Stellar, Tavi, Varg | 122 | 48 | 50 | -0.6 | C&I, Ren, Ryth, Stellar, Varg (139), -0.7 |
| monolith | C&I, Ren, Ryth, Varg, Vendetta | 182 | 57 | 50 | +1.9 | C&I, Ren, Ryth, Stellar, Varg (343), -0.4 |
| packs | C&I, Ren, Stellar | 131 | 56 | 50 | +1.5 | C&I, Stellar, Varg (247), -0.4 |
| pilgrimage | C&I, Ren, Stellar, Tavi, Varg | 240 | 54 | 50 | +1.2 | C&I, Ren, Ryth, Stellar, Varg (306), +0.2 |
| rows | C&I, Ryth, Stellar, Varg | 182 | 59 | 50 | +2.5 | C&I, Stellar, Tavi, Varg (226), +1.5 |
| rushbait_v2 | C&I, Ryth, Varg, Vendetta | 149 | 64 | 50 | +3.4 | C&I, Ren, Ryth, Stellar (195), +1.7 |
| stampede | C&I, Ren, Ryth, Stellar, Varg | 257 | 56 | 50 | +2.1 | same |
| swirl | C&I, Ryth, Stellar, Varg | 145 | 57 | 50 | +1.5 | C&I, Stellar, Tavi, Varg (186), -0.5 |

Win and expected are per cent. Champion signals with |z| ≥ 2.5 (side drafted the champion): Vendetta -5.8 on eight, -3.5 on contest, -3.1 on stampede; Prism +4.4 on contest and +2.7 on eight; Varg -2.5 on contest; Ryth -3.5 on collision; Willow -3.6 on ambush and -3.4 on corners.

## Limits

- Ratings adjust for team strength alone; drafts and execution are confounded.
- The 17 maps × about 40 drafts give hundreds of cells, so |z| near 2 arises by chance in several of them.
- No platform kills, deaths, or gem statistics exist after 05:30 UTC on 16 October; this layer uses results, rounds, drafts, and ratings.
- Nothing after 15:01 UTC on 16 October is included. Your scraper run tonight extends it.
