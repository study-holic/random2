# v3 behaviour data: our 515 ranked games against the top-10 mutual games (partial report, 19 Oct 18:50 BST)

Everything here is decoded from platform replays with `tools/v3/` (engine v0_0_3 decoder validated: final gems and rounds equal the platform score in 1,456 of 1,456 games). Maps are keyed by map id (first 8 hex shown).

## 0. Samples and limits
- **Ours**: all 515 finished ranked games of xanarchy (104 groups), 18 Oct 22:40Z to 19 Oct 16:00Z, 235 wins (46%); round-level decode of all 515, full action log of 141 team-sides' games. The platform does not record bot version, so v2 and any later upload are mixed; our win rate by UTC hour is in 7.
- **Top-10 reference**: 1,011 games between two teams of the 18 Oct 22:39Z top 10 (2,022 team-sides), 18 Oct 13:20Z to 22:40Z: the newest 1,011 of the 4,640 such games, fetched before the machine slept. Per map 49-74 games. Full action log of 117 of them. Kill, purchase and draft rows in 3, 5 and 6 use the first 837 of the 1,011 (1,674 team-sides).
- **Resolution**: one state per round (round-level). Events are exact to a round. Killer attribution: the enemy champion whose kill counter rose and who stood nearest (<= 8 tiles); checked against the action log on 3,658 deaths in our full-action games: right in 3,152 (86%), no answer in 502, wrong in 4. Deaths with no answer are camp deaths (411 of the 3,658) or misses.
- Medians unless stated. A mark (†) means fewer than 30 games. Our win rates are not rating-adjusted because opponents are within a few rating points (mean expected score 50%); top-10 mutual rows have 50% win by construction.
- "Tributed per round" = final submitted gems / game rounds. "Collected" = sum of positive bag increments (camp captures and stolen bags).

## 1. Gem income per round: us, our opponent, top-10 (answers the first lead question)
Cells are us / opponent / top-10. Deaths are own champions per game.
| map (id) | n | win | rounds | tributed per round | tributes per game | first tribute round | deaths us / opp |
|---|---|---|---|---|---|---|---|
| ambush (699c96a0) | 29† | 72% | 921 | 1.06 / 0.72 / 1.63 | 10.2 / 9.7 / 10.5 | 109 / 207 / 127 | 4.1 / 5.7 |
| collision (2e9820ee) | 26† | 54% | 810 | 0.82 / 1.06 / 1.12 | 16.6 / 25.6 / 17.8 | 84 / 58 / 58 | 9.7 / 9.6 |
| contest (3a5c2468) | 34 | 18% | 2000 | 0.06 / 0.12 / 0.17 | 0.6 / 1.1 / 1.5 | 1446 / 1136 / 1454 | 48.7 / 30.0 |
| corners (e3c29b68) | 33 | 45% | 574 | 1.50 / 1.56 / 1.52 | 19.2 / 25.3 / 18.2 | 59 / 70 / 64 | 4.1 / 6.1 |
| eight (3e2df5be) | 31 | 29% | 618 | 0.84 / 1.58 / 1.23 | 8.5 / 21.0 / 11.1 | 108 / 52 / 86 | 11.2 / 7.5 |
| elkring (bb12893c) | 21† | 52% | 649 | 1.41 / 1.17 / 1.63 | 23.9 / 21.0 / 16.4 | 49 / 69 / 75 | 10.1 / 11.6 |
| expanse (81929320) | 36 | 67% | 754 | 1.27 / 1.17 / 1.95 | 27.6 / 26.6 / 19.9 | 78 / 40 / 50 | 0.2 / 1.0 |
| fragment (9925f416) | 25† | 24% | 774 | 0.55 / 1.13 / 0.99 | 11.6 / 27.1 / 15.4 | 88 / 56 / 73 | 18.8 / 10.4 |
| interlock (b777c8c8) | 32 | 41% | 722 | 1.10 / 1.23 / 1.45 | 16.2 / 26.7 / 18.1 | 38 / 58 / 51 | 11.2 / 5.5 |
| lanes (93852276) | 32 | 72% | 578 | 1.59 / 1.39 / 2.21 | 26.5 / 28.6 / 30.5 | 87 / 71 / 63 | 5.7 / 4.4 |
| monolith (3991a8b3) | 39 | 62% | 581 | 1.59 / 1.57 / 2.22 | 23.5 / 22.7 / 16.5 | 69 / 58 / 80 | 5.6 / 3.2 |
| packs (b135e0fc) | 22† | 23% | 633 | 1.01 / 1.56 / 1.78 | 13.1 / 25.4 / 18.5 | 52 / 38 / 38 | 10.6 / 4.4 |
| pilgrimage (7bfc2edd) | 39 | 23% | 1112 | 0.51 / 0.86 / 1.29 | 7.3 / 18.1 / 12.1 | 112 / 96 / 119 | 9.2 / 8.9 |
| rows (25d81311) | 23† | 100% | 473 | 2.24 / 1.23 / 2.64 | 16.0 / 15.0 / 13.5 | 84 / 64 / 51 | 0.2 / 0.9 |
| rushbait_v2 (df22386f) | 35 | 43% | 996 | 0.68 / 0.80 / 1.49 | 20.5 / 22.0 / 16.7 | 104 / 77 / 97 | 5.5 / 4.5 |
| stampede (f2b6903f) | 38 | 37% | 1514 | 0.42 / 0.59 / 0.99 | 2.0 / 3.5 / 4.3 | 869 / 834 / 213 | 18.7 / 12.9 |
| swirl (f930f43f) | 20† | 15% | 1102 | 0.29 / 0.94 / 1.21 | 4.7 / 21.8 / 12.3 | 404 / 142 / 49 | 41.1 / 21.4 |
| all | 515 | 46% | 766 | 0.98 / 1.08 / 1.54 | 14.7 / 19.7 / 15.3 | 86 / 71 / 69 | 12.2 / 8.5 |
- Data: we out-earn our opponent on ambush, elkring, expanse, lanes, rows (and tie on monolith, corners); we trail by more than 0.5 gems per round on swirl (0.29 v 0.94), eight (0.84 v 1.58), fragment (0.55 v 1.13), packs (1.01 v 1.56); by 0.2 to 0.4 on pilgrimage and collision; by under 0.2 on interlock, stampede, rushbait_v2, contest.
- Data: on five of our six lowest-win maps (eight, fragment, packs, pilgrimage, swirl; the sixth is contest) we make far fewer tributes than the opponent (8.5 v 21.0, 11.6 v 27.1, 13.1 v 25.4, 7.3 v 18.1, 4.7 v 21.8). Bags are equal or larger (66 v 48 on eight; 80 v 58 on pilgrimage; 64 v 49 on swirl).

## 2. Where champion-rounds go, and why income differs (second lead question)
Share of all alive-or-dead champion-rounds, % (us / opponent / top-10). Position classes at each round start: dead; spawn (within 3 of the spawn centre); altar (within 2 of a tile where tributes completed on that map, 5 or more times); gem camps (within 4 of a wolf, elk, gem or monolith camp); gold camps (within 4 of a gold-only camp); transit (the rest, walking).
| map | dead | spawn | altar | gem camps | gold camps | transit |
|---|---|---|---|---|---|---|
| ambush | 4 / 6 / 2 | 3 / 3 / 3 | 5 / 5 / 5 | 24 / 20 / 31 | 6 / 12 / 4 | 57 / 54 / 55 |
| collision | 16 / 16 / 13 | 5 / 5 / 6 | 10 / 14 / 13 | 48 / 44 / 48 | 17 / 17 / 19 | 4 / 4 / 1 |
| contest | 25 / 15 / 23 | 2 / 3 / 3 | 7 / 13 / 15 | 1 / 6 / 3 | 42 / 38 / 29 | 22 / 25 / 26 |
| corners | 6 / 9 / 9 | 5 / 4 / 5 | 8 / 17 / 19 | 50 / 44 / 43 | 3 / 3 / 1 | 28 / 22 / 24 |
| eight | 16 / 11 / 15 | 7 / 4 / 5 | 6 / 12 / 10 | 41 / 55 / 53 | 0 / 0 / 0 | 31 / 18 / 18 |
| elkring | 13 / 17 / 10 | 6 / 9 / 8 | 7 / 7 / 9 | 46 / 43 / 48 | 9 / 12 / 14 | 19 / 11 / 11 |
| expanse | 0 / 2 / 0 | 3 / 4 / 5 | 16 / 14 / 13 | 44 / 50 / 57 | 11 / 5 / 3 | 25 / 25 / 23 |
| fragment | 31 / 18 / 20 | 5 / 7 / 7 | 6 / 12 / 7 | 46 / 50 / 55 | 2 / 4 / 1 | 10 / 10 / 9 |
| interlock | 17 / 9 / 9 | 5 / 4 / 5 | 7 / 12 / 11 | 39 / 48 / 50 | 8 / 4 / 3 | 24 / 23 / 23 |
| lanes | 8 / 6 / 2 | 3 / 4 / 4 | 15 / 18 / 17 | 52 / 49 / 54 | 0 / 4 / 1 | 21 / 20 / 22 |
| monolith | 9 / 5 / 5 | 3 / 4 / 4 | 12 / 14 / 10 | 49 / 51 / 58 | 8 / 6 / 3 | 18 / 19 / 20 |
| packs | 22 / 10 / 10 | 6 / 5 / 5 | 14 / 18 / 14 | 46 / 53 / 54 | 3 / 6 / 7 | 8 / 8 / 10 |
| pilgrimage | 8 / 8 / 6 | 2 / 4 / 3 | 4 / 4 / 4 | 10 / 17 / 25 | 49 / 36 / 32 | 27 / 30 / 30 |
| rows | 0 / 2 / 0 | 5 / 4 / 5 | 8 / 9 / 9 | 67 / 55 / 65 | 0 / 4 / 0 | 20 / 26 / 21 |
| rushbait_v2 | 6 / 5 / 4 | 26 / 30 / 7 | 4 / 6 / 6 | 36 / 34 / 64 | 0 / 0 / 0 | 28 / 25 / 18 |
| stampede | 11 / 9 / 8 | 3 / 5 / 3 | 1 / 1 / 2 | 6 / 8 / 22 | 39 / 38 / 26 | 39 / 37 / 38 |
| swirl | 45 / 23 / 29 | 7 / 11 / 10 | 17 / 25 / 22 | 21 / 25 / 24 | 8 / 15 / 14 | 3 / 2 / 1 |
| all | 13 / 10 / 9 | 6 / 6 / 5 | 8 / 11 / 11 | 36 / 37 / 45 | 14 / 13 / 8 | 24 / 23 / 21 |

Income chain: collected gems per 100 gem-camp champion-rounds, tributed per collected gem, recall estimate (5 rounds per teleport-to-spawn, % of champion-rounds), recalls per game. us / opponent / top-10.
| map | collected per 100 gem-camp rounds | tributed per collected | recall % | recalls per game |
|---|---|---|---|---|
| ambush | 78 / 66 / 96 | 0.92 / 0.93 / 0.92 | 3.7 / 2.2 / 3.4 | 40.9 / 23.6 / 23.1 |
| collision | 64 / 80 / 77 | 0.71 / 0.80 / 0.80 | 3.7 / 3.2 / 5.6 | 23.5 / 19.0 / 29.7 |
| contest | 85 / 53 / 143 | 0.84 / 0.59 / 0.67 | 1.1 / 2.7 / 3.5 | 25.1 / 59.3 / 76.8 |
| corners | 60 / 69 / 73 | 0.84 / 0.90 / 0.80 | 5.3 / 3.8 / 6.1 | 36.8 / 24.6 / 40.5 |
| eight | 51 / 55 / 49 | 0.72 / 0.85 / 0.78 | 5.4 / 4.1 / 5.1 | 41.0 / 30.4 / 33.1 |
| elkring | 59 / 62 / 67 | 0.85 / 0.79 / 0.84 | 2.7 / 3.3 / 5.2 | 21.7 / 25.0 / 32.3 |
| expanse | 75 / 66 / 91 | 0.97 / 0.95 / 0.95 | 3.0 / 3.8 / 6.2 | 18.1 / 21.3 / 23.9 |
| fragment | 44 / 68 / 57 | 0.75 / 0.92 / 0.87 | 1.6 / 3.0 / 3.9 | 10.2 / 17.3 / 19.8 |
| interlock | 63 / 58 / 66 | 0.88 / 0.93 / 0.89 | 4.2 / 3.4 / 5.1 | 30.4 / 23.3 / 29.4 |
| lanes | 55 / 53 / 72 | 0.92 / 0.93 / 0.95 | 3.6 / 3.0 / 5.0 | 26.0 / 21.2 / 25.8 |
| monolith | 62 / 57 / 71 | 0.88 / 0.91 / 0.90 | 3.0 / 3.5 / 5.3 | 20.4 / 23.0 / 26.3 |
| packs | 65 / 79 / 88 | 0.88 / 0.95 / 0.95 | 4.2 / 3.4 / 5.8 | 21.4 / 16.0 / 22.6 |
| pilgrimage | 99 / 95 / 95 | 0.89 / 0.91 / 0.90 | 1.1 / 1.9 / 2.9 | 15.2 / 23.7 / 23.0 |
| rows | 70 / 51 / 87 | 0.96 / 0.91 / 0.94 | 5.2 / 3.2 / 5.3 | 24.3 / 14.8 / 19.1 |
| rushbait_v2 | 45 / 55 / 53 | 0.86 / 0.89 / 0.88 | 2.6 / 2.2 / 4.7 | 25.6 / 19.1 / 28.0 |
| stampede | 168 / 149 / 80 | 0.70 / 0.89 / 0.92 | 2.3 / 2.3 / 2.8 | 41.2 / 38.4 / 20.8 |
| swirl | 50 / 90 / 111 | 0.60 / 0.86 / 0.90 | 0.6 / 1.3 / 2.6 | 6.7 / 12.8 / 15.0 |
| all | 65 / 67 / 74 | 0.85 / 0.89 / 0.89 | 3.1 / 3.0 / 4.6 | 25.9 / 25.2 / 28.0 |
- Data: over all maps we spend 13% of champion-rounds dead (opponent 10%, top-10 9%) and 36% at gem camps (top-10 45%); we spend 14% at gold camps (top-10 8%).
- Data, per map (n as in 1): swirl 45% dead (opponent 23%) with 0.60 of collected gems reaching an altar; fragment 31% dead, 0.75 reaching; packs 22% dead; interlock 17% dead; stampede 0.70 reaching (top-10 0.92); collision 0.71 (0.80). Pilgrimage: 10% at gem camps and 49% at gold camps (top-10 25% and 32%) while yield per gem-camp round equals top-10 (99 v 95). Rushbait: 36% at gem camps against 64% for top-10, and 26% of our champion-rounds on spawn against 7% (the opponent is also 30%). Eight: 31% transit against 18%, 41% gem camps against 53%. Stampede: 6% gem camps against 22%.
- Interpretation: swirl, fragment, packs, interlock lose income to deaths and bag loss; pilgrimage, rushbait, stampede, eight lose it to where champions stand (gold camps, spawn, walking), not to yield per camp-round.

## 3. Who kills us
- 6,281 of our champion deaths in 515 games (12.2 per game); 5,071 attributed. Our champion types dying: Cassi and Issac bodies 3,196 (51%), Vendetta 1,040 (17%), Varg 676, Ryth 668, Tavi 490, Stellar 201. 869 of our deaths (14%) had a bag of 20 gems or more; 563 opponent deaths had a bag of 20 or more.
- Killer champion type among the 5,071: Stellar 1,060 (21%), Cassi and Issac 904 (18%), Varg 801 (16%), Ryth 640 (13%), Tavi 573 (11%), Ren 441 (9%). Largest pairs: Stellar kills our Cassi and Issac 452, Cassi and Issac kills Cassi and Issac 368, Varg kills Cassi and Issac 337, Stellar kills Vendetta 227.
- Opponent champions died 4,393 times against our 6,281: victims Cassi and Issac 1,768, Varg 518, Stellar 461, Ryth 447.
- By map, our deaths (leading killers): swirl 823 (Cassi and Issac 21%, Stellar 19%, Ryth 16%); contest 1,655 (Stellar 14%, Varg 13%, Ryth 12%); stampede 712 (Stellar 15%, Ryth 12%); fragment 470 (Stellar 21%, Cassi and Issac 18%, Varg 18%); pilgrimage 359 (Tavi 14%, Varg 14%, Ren 12%); interlock 358 (Stellar 18%); eight 346 (Stellar 21%); packs 233 (Stellar 27%, Cassi and Issac 21%); elkring 212 (Varg 27%); collision 252 (Cassi and Issac 22%); monolith 218 (Stellar 20%, Ren 13%).
- Data: Stellar is the top killer on 7 of the 11 maps with 200 or more of our deaths (contest, eight, fragment, interlock, monolith, packs, stampede). In the first 837 top-10 mutual games Stellar made 3,286 of 8,586 attributed kills (38%), and 1,977 of 2,022 top-10 drafts (98%) contain Stellar against 165 of 515 of ours (32%).

## 4. Three largest shortfalls against the top-10 per map (ratio us / top-10, 1.0 equal; lower-is-better factors inverted)
| map | n | tributed per round us v top-10 | three largest shortfalls |
|---|---|---|---|
| ambush | 29† | 1.06 v 1.63 | dead share 0.62; tributed per round 0.65; collected per round 0.65 |
| collision | 26† | 0.82 v 1.12 | first tribute 0.69; tributed per round 0.73; dead share 0.80 |
| contest | 34 | 0.06 v 0.17 | collected per round 0.30; tributed per round 0.37; tributes per game 0.41 |
| corners | 33 | 1.50 v 1.52 | collected per gem-camp round 0.82; collected per round 0.93; tributed per round 0.99 |
| eight | 31 | 0.84 v 1.23 | tributed per round 0.68; tributes per game 0.76; collected per round 0.77 |
| elkring | 21† | 1.41 v 1.63 | dead share 0.76; collected per round 0.84; tributed per round 0.86 |
| expanse | 36 | 1.27 v 1.95 | collected per round 0.63; first tribute 0.65; tributed per round 0.65 |
| fragment | 25† | 0.55 v 0.99 | tributed per round 0.56; collected per round 0.65; dead share 0.66 |
| interlock | 32 | 1.10 v 1.45 | dead share 0.56; tributed per round 0.75; collected per round 0.76 |
| lanes | 32 | 1.59 v 2.21 | dead share 0.37; tributed per round 0.72; first tribute 0.73 |
| monolith | 39 | 1.59 v 2.22 | dead share 0.57; tributed per round 0.72; collected per round 0.73 |
| packs | 22† | 1.01 v 1.78 | dead share 0.46; tributed per round 0.57; collected per round 0.62 |
| pilgrimage | 39 | 0.51 v 1.29 | tributed per round 0.40; share at gem camps 0.40; collected per round 0.41 |
| rows | 23† | 2.24 v 2.64 | first tribute 0.61; dead share 0.74; collected per gem-camp round 0.80 |
| rushbait_v2 | 35 | 0.68 v 1.49 | tributed per round 0.46; collected per round 0.46; share at gem camps 0.56 |
| stampede | 38 | 0.42 v 0.99 | first tribute 0.25; share at gem camps 0.26; tributed per round 0.43 |
| swirl | 20† | 0.29 v 1.21 | first tribute 0.12; tributed per round 0.24; collected per round 0.36 |
- Caveat: top-10 rows come from 18 Oct 13:20Z-22:40Z and ours from later; the top teams may have moved since.

## 5. What the top 10 do (1,011 mutual games; side-rows = 2 per game)
Rounds to submitted gems (median over team-games that reached it), gems at round 300, tributes per team-game, bag per tribute, first tribute round, deaths and dead champion-rounds per team-game, recalls, altar-lock buys and uses, shards.
| map | rows | 250 | 500 | 750 | 1000 | g300 | trib | bag | first | deaths | dead | recalls | lock buy/use | shards |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| ambush | 124 | 233 | 380 | 485 | 558 | 332 | 10.7 | 89 | 127 | 1.3 | 74 | 23.1 | 2.8 / 0.7 | 19.5 |
| collision | 100 | 301 | 476 | 638 | 656 | 238 | 18.2 | 22 | 58 | 6.3 | 338 | 29.7 | 3.3 / 1.9 | 43.9 |
| contest | 98 | 1582 | 1628 | 1565 | 1579 | 0 | 1.5 | 200 | 1454 | 43.2 | 2571 | 76.8 | 4.2 / 2.0 | 82.6 |
| corners | 112 | 226 | 368 | 470 | 544 | 356 | 18.8 | 36 | 64 | 5.4 | 305 | 40.5 | 3.7 / 2.0 | 44.1 |
| eight | 112 | 252 | 368 | 460 | 534 | 242 | 11.4 | 44 | 86 | 8.8 | 492 | 33.1 | 2.4 / 1.0 | 28.1 |
| elkring | 104 | 200 | 337 | 432 | 506 | 396 | 16.9 | 36 | 75 | 5.6 | 305 | 32.3 | 4.7 / 2.5 | 40.4 |
| expanse | 128 | 202 | 312 | 416 | 484 | 424 | 20.2 | 44 | 50 | 0.1 | 6 | 23.9 | 2.0 / 0.7 | 33.9 |
| fragment | 136 | 299 | 440 | 568 | 658 | 226 | 15.6 | 44 | 73 | 9.8 | 531 | 19.8 | 2.8 / 1.3 | 35.6 |
| interlock | 116 | 252 | 408 | 485 | 573 | 336 | 18.3 | 44 | 51 | 4.6 | 257 | 29.4 | 3.5 / 1.8 | 40.3 |
| lanes | 148 | 170 | 274 | 368 | 428 | 563 | 31.4 | 22 | 63 | 1.1 | 62 | 25.8 | 2.5 / 1.2 | 45.3 |
| monolith | 104 | 176 | 275 | 362 | 416 | 556 | 16.9 | 44 | 80 | 2.2 | 119 | 26.3 | 2.5 / 1.4 | 45.5 |
| packs | 144 | 176 | 312 | 418 | 492 | 470 | 18.7 | 44 | 38 | 3.6 | 192 | 22.6 | 2.5 / 0.9 | 26.5 |
| pilgrimage | 106 | 309 | 472 | 568 | 665 | 228 | 12.4 | 68 | 119 | 4.3 | 248 | 23.0 | 2.1 / 0.5 | 26.4 |
| rows | 138 | 131 | 230 | 304 | 355 | 726 | 13.6 | 66 | 51 | 0.0 | 2 | 19.1 | 0.7 / 0.1 | 31.4 |
| rushbait_v2 | 124 | 248 | 408 | 519 | 600 | 308 | 16.9 | 44 | 97 | 2.5 | 140 | 28.0 | 2.9 / 1.2 | 41.5 |
| stampede | 128 | 224 | 388 | 523 | 540 | 0 | 4.5 | 90 | 213 | 5.4 | 318 | 20.8 | 2.8 / 0.5 | 30.3 |
| swirl | 100 | 252 | 385 | 506 | 580 | 267 | 12.6 | 44 | 49 | 15.6 | 895 | 15.0 | 3.1 / 1.2 | 27.8 |

Per team (all maps; win v rating-expected, exact to the pre-group ratings in the group JSON):
| team | rows | win v exp | 250 | 500 | 750 | 1000 | first tribute | deaths | recalls | lock buy/use | shards |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Intercontinental Fish | 253 | 72% v 65% | 212 | 340 | 432 | 499 | 74 | 4.5 | 25.6 | 0.0 / 0.0 | 31.3 |
| Mite | 250 | 52% v 58% | 190 | 320 | 424 | 501 | 62 | 4.6 | 28.8 | 5.5 / 2.9 | 36.3 |
| Nyx | 233 | 50% v 52% | 220 | 340 | 417 | 532 | 74 | 6.8 | 33.9 | 0.0 / 0.0 | 47.1 |
| Univardisty of Shingtonwash | 205 | 44% v 42% | 224 | 357 | 452 | 526 | 81 | 6.4 | 23.5 | 7.0 / 3.2 | 34.5 |
| procless | 246 | 50% v 52% | 205 | 348 | 443 | 507 | 68 | 8.5 | 28.7 | 0.0 / 0.0 | 39.9 |
| Sneaky Bush Campers | 203 | 45% v 44% | 229 | 380 | 489 | 610 | 67 | 6.1 | 28.4 | 0.0 / 0.0 | 40.1 |
| hello | 155 | 45% v 43% | 233 | 375 | 482 | 612 | 68 | 7.1 | 31.1 | 3.6 / 0.2 | 35.8 |
| Tartarus | 174 | 43% v 42% | 207 | 339 | 451 | 552 | 60 | 6.9 | 29.2 | 3.0 / 0.4 | 40.4 |
| new team(4) | 218 | 44% v 45% | 216 | 360 | 459 | 567 | 70 | 7.3 | 24.9 | 5.7 / 2.9 | 31.8 |
| Pantheon | 85 | 41% v 44% | 212 | 344 | 429 | 491 | 57 | 7.3 | 24.7 | 6.0 / 3.0 | 31.7 |
- Data: Fish wins 72% against 65% expected (+7 points, 253 team-games) and buys no altar locks. Four of the ten (Mite, Univardisty of Shingtonwash, new team(4), Pantheon) buy 5.5 to 7.0 locks per game and place 2.9 to 3.2; Fish, Nyx, procless, Sneaky Bush Campers buy none. Excess of the lock users: -6, +2, -1, -3 points; of the non-users: +7, -2, -2, +1.
- Data, action log of 117 top-10 games (234 team-sides): 40.7 stun casts per team-game, 7% landing within 1 tile of an enemy carrying 20 or more gems; 93 damaging hits per team-game, 21% on carriers against 27% of enemy champion-rounds being carrier-rounds (no carrier focus); 1.03 lock placements per team-game, 14% (33 of 240) with an enemy holding a bag within 2 tiles of the lock tile. Per team hits on carriers: Fish 34%, Tartarus 26%, procless 25%, Mite 24%, Sneaky Bush Campers 12% (each below 30 games).
- Ours (141 full-action games): 53.3 stun casts per game, 2% near carriers; 301 damaging hits per game, 7% on carriers against 22% baseline; 0 lock placements.

## 6. Top-team behaviours we do not have
- Data: **Tavi buys shards in the field**: 78% of Tavi's 4,655 shards (1,674 top-10 team-games) were bought more than 6 tiles from spawn; ours 0% of 2,234 (515 games). Tavi shards per team-game: ours 4.3, top-10 2.8.
- Data: **draft**: 98% of top-10 drafts contain Stellar (ours 32%); first picks Cassi and Issac 523, Varg 476, Ren 382, Stellar 209 of 1,674. Ours: Vendetta first in 459 of 515 games; our usual five-set is Cassi and Issac + Ryth + Tavi + Varg + Vendetta (263 games, 48% win, -2 points against expected). Rating-adjusted top-10 sets: Cassi and Issac + Ren + Stellar 57% (n=215, +5), Cassi and Issac + Ren + Ryth + Stellar + Varg 52% (376, -1), with Tavi and Varg 39% (100, -8). Our opponents open with Cassi and Issac 250 of 515.
- Data: **shard choice**: first shards of top-10 champions are power 2,186, jungle 1,931, durability 1,765, bruiser 1,072, treasure 754, monster 316 (stat focus 3,209, strength 2,549, health 2,322); ours are monster 1,340 and power 1,200 only, strength 1,631, focus 571, health 338. Stellar's first shard: power/focus 39% for top-10, 52% ours (plus monster/focus 42%).
- Data: **earlier income**: first tribute round 69 against our 86; median rounds to 250 gems 213 and 500 gems 349 across the top-10 rows.
- Data: **staying alive**: top-10 rows spend 9% of champion-rounds dead against our 13%, and lose 1 in 9 collected gems against our 1 in 7 (tributed per collected gem 0.89 against 0.85).
- Data: **altar locks** are not a common trait: 4 of 10 top teams use them, Fish does not.
- Not shown by the data: carrier focus, ward use (not decoded), or stuns on carriers; the top-10 hit carriers less than their share of the field.

## 6b. Interpretation (mine, not data)
- Where income is lost differs by map: deaths and bag loss on swirl, fragment, packs, interlock, collision, stampede; standing at gold camps or spawn on pilgrimage, stampede, rushbait_v2; late first tribute on swirl (404 against 49), stampede (869 against 213), eight (108 against 86). A single pace change will not fix all three families.
- The Tavi field-buy gap is the one purchase behaviour that is large (78% against 0%) and cheap to copy; the lock gap is not (Fish wins without locks).
- Our Cassi and Issac bodies take half of our deaths, and Stellar plus Varg kill them most; the top-10 drafts we do not play (Stellar in 98%) are the same champions that kill us most.
- The top-10 reference and our games are from different days, so a gap of under about 15% on a single map with fewer than 30 games should not drive a change.

## 7. Our win rate by hour (UTC, 18 Oct 22Z to 19 Oct 16Z), wins/games (30 games per hour; align with upload times)
18Oct 22Z 0/10 | 23Z 7/30 | 19Oct 00Z 14/30 | 01Z 18/30 | 02Z 17/30 | 03Z 13/30 | 04Z 13/30 | 05Z 13/30 | 06Z 10/30 | 07Z 21/30 | 08Z 17/30 | 09Z 12/25 | 10Z 14/30 | 11Z 13/30 | 12Z 19/30 | 13Z 15/30 | 14Z 13/30 | 15Z 13/25 | 16Z 3/5.

## 8. Reproduce and extend
`py -I tools/v3/fetch_team.py <teamId> <label>`; `py -I tools/v3/fetch_replays.py <ids.txt>` (one download at a time, about 0.2 to 0.4 per second, HTTP 409/429 handled); `decode_loop.py <ids> <dir> --stride 13|1`; `analyze_rounds.py`, `analyze_econ.py`, `aggregate_econ.py`, `killers.py`, `behaviour_extras.py`, `action_extras.py`, `pace_gaps.py`. Tables: `data/v3/decoded/econ_tables.txt`, `tables_s13.txt`, `killers_ours.txt`. Not done: lock cancellations of tributes and tribute interruptions by stun (the channel outcome classifier gave unusable results in a 24-game trial: 5,343 of 9,107 top-10 channel starts ended as unclassified), exact time in recall/tribute channels, per-map action-level comparisons (full-action sample per map is below 30).
