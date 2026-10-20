# Unranked baseline of our live v2 against 8 real teams (136 distinct games of 160 queued, 17 maps; written 19 Oct 19:20 BST)

> Measured on engine 0.0.3 and the 17-map pool; engine 1.0 changes (docs/rules/ENGINE-1.0.md) that may affect it: 21-map pool (10 new maps, 6 retired), kept-map camp additions, cluster HP 120 and new per-camp rewards, base-stat rebalance; opponents' teams and drafts have likely changed too.

Source: `data/unranked/groups.csv` (32 groups of 5, queued by the lead 19 Oct 18:12-18:16 BST; games finished 17:12Z), `cambc match-groups download`, round-level decode (`tools/v3/analyze_unranked.py`; final gems equal the platform result in every game). Maps by map id (first 8 hex). Opponent ratings are the 18:00 ladder values in `data/unranked/panel.csv`. Each group is 5 games on 5 different maps. stampede, contest and monolith were queued twice per opponent with us on the same side, and the engine is deterministic: the repeated games gave identical results, so the 24 repeats are removed (the matrix in section 3 shows one game per cell). That leaves 8 games per map (one per opponent) and 17 per opponent, all with us on side a (136 games); every cell is below 30 games (†). This is a baseline, not a rating-adjusted comparison: the opponents differ in strength, so compare v3a only on the same opponent and map.

## 1. Overall and per opponent
Cells are us / them. Deaths are champions per game.
| group | games | our wins-losses | gems us / them | median rounds | tributed per round us / them | collected per round us / them | tributes us / them | first tribute us / them | deaths us / them |
|---|---|---|---|---|---|---|---|---|---|
| all | 136 | 27-109 | 563 / 919 | 618 | 0.89 / 1.52 | 1.07 / 1.62 | 11.8 / 22.3 | 76 / 70 | 12.0 / 4.8 |
| Micron (1524.5) | 17† | 3-14 | 592 / 926 | 699 | 0.91 / 1.45 | 1.06 / 1.55 | 11.8 / 31.5 | 81 / 54 | 12.0 / 5.4 |
| Deer Park (1510.5) | 17† | 5-12 | 708 / 915 | 716 | 1.03 / 1.36 | 1.19 / 1.46 | 14.6 / 24.3 | 88 / 78 | 8.3 / 6.6 |
| tvojtatko (1483.0) | 17† | 7-10 | 728 / 833 | 777 | 1.00 / 1.16 | 1.14 / 1.24 | 14.4 / 23.4 | 72 / 69 | 14.6 / 4.8 |
| 777 (1453.0) | 17† | 6-11 | 664 / 823 | 634 | 1.00 / 1.34 | 1.15 / 1.43 | 14.1 / 21.5 | 87 / 62 | 7.1 / 10.0 |
| Snowman Workshop (1563.8) | 17† | 2-15 | 493 / 955 | 615 | 0.84 / 1.67 | 1.05 / 1.77 | 10.6 / 23.6 | 82 / 36 | 15.5 / 3.8 |
| hackingnoises (1600.3) | 17† | 3-14 | 487 / 917 | 616 | 0.81 / 1.69 | 1.09 / 1.84 | 9.5 / 13.9 | 76 / 86 | 11.5 / 5.1 |
| Tozo's Disciples (1686.0) | 17† | 1-16 | 471 / 1013 | 547 | 0.77 / 1.61 | 0.93 / 1.70 | 10.8 / 20.0 | 76 / 95 | 14.8 / 0.8 |
| umamusume (1771.4) | 17† | 0-17 | 361 / 970 | 527 | 0.71 / 1.88 | 0.90 / 1.95 | 8.5 / 20.5 | 70 / 70 | 12.3 / 1.5 |

## 2. Per map
| group | games | our wins-losses | gems us / them | median rounds | tributed per round us / them | collected per round us / them | tributes us / them | first tribute us / them | deaths us / them |
|---|---|---|---|---|---|---|---|---|---|
| ambush (699c96a0) | 8† | 4-4 | 918 / 950 | 826 | 1.11 / 1.17 | 1.22 / 1.26 | 9.6 / 14.4 | 115 / 136 | 2.0 / 2.0 |
| collision (2e9820ee) | 8† | 0-8 | 246 / 1043 | 566 | 0.38 / 1.83 | 0.87 / 1.89 | 6.4 / 29.4 | 59 / 60 | 8.9 / 1.0 |
| contest (3a5c2468) | 8† | 1-7 | 50 / 275 | 2000 | 0.03 / 0.16 | 0.03 / 0.20 | 0.2 / 1.4 | 1593 / 1304 | 59.6 / 24.4 |
| corners (e3c29b68) | 8† | 1-7 | 679 / 972 | 492 | 1.36 / 1.95 | 1.61 / 2.05 | 16.0 / 26.4 | 69 / 64 | 4.1 / 1.0 |
| eight (3e2df5be) | 8† | 0-8 | 169 / 1039 | 488 | 0.34 / 2.22 | 1.04 / 2.40 | 3.6 / 18.4 | 103 / 66 | 9.0 / 3.1 |
| elkring (bb12893c) | 8† | 0-8 | 562 / 1017 | 514 | 1.01 / 1.90 | 1.30 / 2.01 | 15.2 / 30.1 | 47 / 64 | 10.5 / 2.6 |
| expanse (81929320) | 8† | 4-4 | 921 / 947 | 724 | 1.29 / 1.36 | 1.35 / 1.43 | 27.9 / 27.5 | 72 / 52 | 0.0 / 0.8 |
| fragment (9925f416) | 8† | 0-8 | 308 / 1020 | 694 | 0.40 / 1.43 | 0.58 / 1.52 | 8.4 / 30.6 | 91 / 50 | 21.6 / 5.4 |
| interlock (b777c8c8) | 8† | 1-7 | 652 / 1004 | 650 | 0.95 / 1.52 | 1.12 / 1.60 | 13.9 / 30.2 | 43 / 62 | 8.5 / 2.9 |
| lanes (93852276) | 8† | 2-6 | 830 / 990 | 499 | 1.64 / 2.00 | 1.75 / 2.07 | 24.6 / 34.6 | 87 / 65 | 3.5 / 0.8 |
| monolith (3991a8b3) | 8† | 1-7 | 684 / 1006 | 472 | 1.43 / 2.17 | 1.68 / 2.31 | 18.5 / 28.2 | 66 / 58 | 5.4 / 0.4 |
| packs (b135e0fc) | 8† | 0-8 | 422 / 1015 | 501 | 0.86 / 2.09 | 0.87 / 2.18 | 8.1 / 25.5 | 51 / 36 | 6.2 / 1.2 |
| pilgrimage (7bfc2edd) | 8† | 1-7 | 557 / 1028 | 944 | 0.56 / 1.10 | 0.62 / 1.13 | 6.9 / 21.2 | 99 / 82 | 10.1 / 6.0 |
| rows (25d81311) | 8† | 5-3 | 949 / 858 | 459 | 2.20 / 2.06 | 2.28 / 2.19 | 14.4 / 17.5 | 97 / 58 | 0.0 / 0.0 |
| rushbait_v2 (df22386f) | 8† | 3-5 | 712 / 726 | 837 | 0.81 / 0.99 | 0.87 / 1.07 | 21.9 / 22.1 | 57 / 110 | 2.9 / 1.2 |
| stampede (f2b6903f) | 8† | 4-4 | 783 / 702 | 1488 | 0.56 / 0.48 | 0.66 / 0.57 | 3.0 / 4.0 | 894 / 598 | 17.5 / 10.1 |
| swirl (f930f43f) | 8† | 0-8 | 128 / 1030 | 764 | 0.14 / 1.46 | 0.25 / 1.59 | 1.9 / 18.1 | 345 / 144 | 34.2 / 18.0 |

## 3. Result matrix: our result, gems us-them, rounds (one cell per game; repeated games removed)
| map | Micron | Deer Park | tvojtatko | 777 | Snowman Workshop | hackingnoises | Tozo's Disciples | umamusume |
|---|---|---|---|---|---|---|---|---|
| ambush | W 1024-860 r877 | W 1024-928 r828 | W 1028-742 r840 | L 846-1010 r808 | L 894-1000 r819 | W 1024-972 r865 | L 952-1006 r824 | L 548-1086 r697 |
| collision | L 146-1018 r562 | L 394-1010 r631 | L 546-1084 r819 | L 444-1022 r634 | L 88-1040 r485 | L 182-1124 r517 | L 0-1032 r477 | L 168-1016 r570 |
| contest | L 0-800 r2000 | L 0-200 r2000 | L 0-0 r2000 | W 400-0 r2000 | L 0-0 r2000 | L 0-200 r2000 | L 0-1000 r1484 | L 0-0 r2000 |
| corners | L 698-1010 r474 | L 552-1026 r485 | W 1048-676 r479 | L 800-1028 r580 | L 602-1000 r450 | L 406-1022 r499 | L 550-1004 r511 | L 774-1012 r527 |
| eight | L 248-1064 r562 | L 370-1062 r531 | L 298-1024 r531 | L 44-1008 r445 | L 182-1042 r426 | L 116-1080 r379 | L 94-1022 r542 | L 0-1008 r405 |
| elkring | L 504-1026 r582 | L 882-1008 r600 | L 864-1008 r698 | L 576-1026 r508 | L 486-1044 r496 | L 468-1008 r519 | L 432-1008 r500 | L 288-1008 r450 |
| expanse | L 868-1000 r699 | L 956-1002 r716 | W 1014-936 r777 | W 1000-996 r724 | W 1000-964 r724 | W 1000-582 r724 | L 978-1050 r780 | L 554-1046 r517 |
| fragment | L 339-1020 r830 | L 670-1014 r908 | L 189-1013 r711 | L 264-1026 r642 | L 292-1020 r678 | L 466-1031 r830 | L 117-1028 r648 | L 124-1006 r590 |
| interlock | L 486-1018 r713 | W 1000-892 r780 | L 962-1016 r801 | L 668-1048 r613 | L 596-1004 r615 | L 756-1010 r677 | L 386-1018 r623 | L 364-1028 r558 |
| lanes | L 752-1010 r494 | L 832-1012 r497 | L 972-1016 r620 | W 1008-946 r572 | W 1022-898 r519 | L 628-1014 r422 | L 766-1012 r501 | L 664-1016 r417 |
| monolith | L 702-1032 r466 | L 738-1032 r477 | W 1038-936 r598 | L 716-1004 r505 | L 716-1016 r502 | L 402-1010 r393 | L 680-1012 r462 | L 482-1010 r377 |
| packs | L 572-1008 r524 | L 528-1012 r480 | L 176-1008 r547 | L 644-1032 r529 | L 224-1008 r406 | L 432-1028 r438 | L 436-1012 r522 | L 364-1012 r472 |
| pilgrimage | L 760-1072 r1032 | W 1064-932 r1301 | L 644-1068 r1106 | L 478-1032 r1007 | L 296-1036 r704 | L 348-1062 r830 | L 480-1000 r866 | L 388-1020 r881 |
| rows | W 1056-792 r459 | W 1056-528 r459 | W 1056-726 r459 | W 1056-726 r459 | L 858-1056 r418 | L 660-1056 r326 | W 1056-924 r459 | L 792-1056 r400 |
| rushbait_v2 | L 596-1016 r765 | W 1010-772 r1211 | W 1006-0 r1083 | W 1018-0 r964 | L 384-1010 r645 | L 304-1002 r616 | L 894-1004 r909 | L 480-1008 r615 |
| stampede | W 1080-0 r1475 | L 720-1080 r1806 | W 1260-900 r1604 | W 1080-0 r987 | L 720-1080 r1500 | W 1080-360 r1389 | L 180-1080 r1556 | L 144-1116 r1141 |
| swirl | L 236-1002 r745 | L 236-1044 r753 | L 280-1013 r1376 | L 252-1086 r1094 | L 18-1014 r775 | L 0-1028 r789 | L 0-1010 r547 | L 0-1044 r379 |

## 4. Where champion-rounds go: dead / spawn / altar / gem camps / gold camps / transit, % of champion-rounds (us | them)
| group | games | us | them |
|---|---|---|---|
| all | 136 | 16 / 5 / 7 / 38 / 12 / 22 | 5 / 7 / 13 / 43 / 11 / 20 |
| Micron | 17† | 14 / 5 / 7 / 40 / 12 / 23 | 8 / 5 / 15 / 43 / 13 / 17 |
| Deer Park | 17† | 9 / 6 / 9 / 40 / 13 / 23 | 7 / 8 / 15 / 38 / 11 / 22 |
| tvojtatko | 17† | 15 / 6 / 8 / 38 / 11 / 22 | 5 / 10 / 12 / 42 / 11 / 21 |
| 777 | 17† | 11 / 6 / 9 / 38 / 14 / 22 | 8 / 11 / 11 / 44 / 8 / 18 |
| Snowman Workshop | 17† | 18 / 5 / 7 / 37 / 11 / 21 | 7 / 4 / 16 / 41 / 14 / 18 |
| hackingnoises | 17† | 18 / 5 / 7 / 36 / 12 / 22 | 6 / 6 / 9 / 47 / 9 / 24 |
| Tozo's Disciples | 17† | 21 / 6 / 6 / 35 / 10 / 22 | 1 / 6 / 14 / 44 / 10 / 24 |
| umamusume | 17† | 19 / 6 / 6 / 36 / 11 / 23 | 2 / 5 / 15 / 47 / 10 / 20 |
| ambush | 8† | 2 / 3 / 4 / 25 / 5 / 61 | 3 / 3 / 5 / 29 / 11 / 50 |
| collision | 8† | 21 / 7 / 6 / 46 / 13 / 8 | 2 / 7 / 21 / 49 / 18 / 1 |
| contest | 8† | 31 / 2 / 4 / 1 / 39 / 23 | 12 / 5 / 19 / 5 / 35 / 24 |
| corners | 8† | 7 / 5 / 7 / 53 / 2 / 25 | 2 / 6 / 20 / 45 / 3 / 24 |
| eight | 8† | 17 / 7 / 3 / 40 / 0 / 33 | 6 / 6 / 14 / 57 / 0 / 18 |
| elkring | 8† | 18 / 7 / 6 / 56 / 13 / 1 | 5 / 7 / 11 / 48 / 13 / 16 |
| expanse | 8† | 0 / 4 / 17 / 47 / 8 / 24 | 1 / 4 / 13 / 50 / 6 / 25 |
| fragment | 8† | 40 / 6 / 4 / 38 / 4 / 9 | 10 / 5 / 13 / 59 / 2 / 11 |
| interlock | 8† | 15 / 5 / 5 / 41 / 11 / 22 | 5 / 5 / 12 / 53 / 2 / 23 |
| lanes | 8† | 6 / 3 / 15 / 54 / 0 / 22 | 1 / 3 / 19 / 53 / 1 / 22 |
| monolith | 8† | 11 / 4 / 10 / 52 / 7 / 17 | 1 / 4 / 14 / 57 / 4 / 20 |
| packs | 8† | 18 / 8 / 9 / 56 / 3 / 5 | 4 / 4 / 20 / 59 / 6 / 8 |
| pilgrimage | 8† | 11 / 2 / 4 / 12 / 41 / 31 | 6 / 2 / 5 / 22 / 34 / 31 |
| rows | 8† | 0 / 5 / 8 / 65 / 0 / 22 | 0 / 4 / 11 / 62 / 1 / 22 |
| rushbait_v2 | 8† | 5 / 11 / 5 / 33 / 0 / 46 | 2 / 40 / 6 / 44 / 0 / 9 |
| stampede | 8† | 12 / 3 / 1 / 6 / 51 / 27 | 8 / 3 / 1 / 11 / 38 / 40 |
| swirl | 8† | 52 / 8 / 15 / 16 / 5 / 4 | 23 / 7 / 25 / 31 / 11 / 2 |

## 5. Who kills us (nearest enemy champion whose kill counter rose; checked at 86% coverage and 4 wrong in 3,658 deaths on ranked games)
1633 of our champion deaths in 136 games; 1337 attributed, 296 with no enemy kill nearby. Killer champion: Stellar 323, C&I 300, Varg 163, Ren 157, Ryth 149, Tavi 104, Vendetta 79, Vesper 40, Marcella 15, Prism 4, Willow 3.
- Micron: Varg 50, Vendetta 41, Ren 28
- Deer Park: Stellar 53, C&I 28, Varg 13
- tvojtatko: Stellar 95, C&I 38, Tavi 29
- 777: Stellar 39, C&I 19, Varg 14
- Snowman Workshop: C&I 54, Tavi 44, Ren 43
- hackingnoises: Ryth 52, Vendetta 35, C&I 29
- Tozo's Disciples: C&I 72, Stellar 63, Ren 39
- umamusume: C&I 41, Stellar 35, Ren 32

## 6. Drafts (pick order)
Ours (champion ids 0 Ryth, 1 Vesper, 2 Vendetta, 3 Cassi and Issac, 4 Stellar, 5 Prism, 6 Marcella, 7 Willow, 8 Ren, 9 Varg, 10 Tavi): 2.3.9.0.10 x64; 2.3.4 x32; 2.3.9.0 x24; 3.4.9.0.8 x8; 3.9.10.4 x8
Per-game rows: `data/unranked/games.tsv`.

## 7. All 160 queued games, and which faults each map points to (added 19 Oct 22:50 BST)
- As queued: 33 wins in 160 (21%); distinct games: 27 in 136 (20%). The 24 repeats (stampede, contest, monolith, same side) were identical, so sections 1-6 count each once; all 17 maps are in sections 2 and 4 (8 distinct games each, below 30).
- "F1-F6" is not defined in `PLAN-v3.md`; I use its root causes R1-R6 (R1 target value ignores gems, R2 monoliths never targeted, R3 health-heavy builds, R4 lone even fights, R5 retreat formula, R6 drafts) and F1 (engagement). This is interpretation: the pointer is the nearest match between the numbers above and the cause, not a test.

| map | numbers (us v them) | points to |
|---|---|---|
| swirl | dead 52% v 23%, deaths 34 v 18, tributed 0.14 v 1.46 | R4/F1, R5, R3 |
| fragment | dead 40% v 10%, deaths 21.6 v 5.4, 0-8 | R4/F1, R5, R3 |
| collision | dead 21% v 2%, deaths 8.9 v 1.0, tributes 6.4 v 29.4 | R4/F1, R3 |
| packs | dead 18% v 4%, tributes 8.1 v 25.5 | R4/F1, R5 |
| elkring | dead 18% v 5%, deaths 10.5 v 2.6 | R4/F1, R3 |
| eight | tributes 3.6 v 18.4, first tribute 103 v 66, transit 33% v 18%, dead 17% v 6% | R1, R4/F1 |
| contest | gem camps 1% v 5%, gems 50 v 275, deaths 60 v 24 | R2 |
| pilgrimage | gem camps 12% v 22%, gold camps 41% v 34%, tributes 6.9 v 21.2 | R1 |
| stampede | gold camps 51% v 38%, gem camps 6% v 11%, first tribute 894 v 598, deaths 17.5 v 10.1 | R1, R5 |
| rushbait_v2 | transit 46% v 9%, income 0.81 v 0.99 (they sit on spawn 40%) | R1 (walking, not gems) |
| corners, interlock, lanes, monolith | camp shares close; tributes 16.0 v 26.4, 13.9 v 30.2, 24.6 v 34.6, 18.5 v 28.2; deaths 4.1-8.5 v 0.4-2.9 | R3 (weaker hits), R4/F1 |
| ambush, expanse, rows | parity (4-4, 4-4, 5-3); deaths 0-2 each | none |
- Fault R6 (drafts) is not visible per map; all 136 games use our draft against theirs (Stellar killed us most, see section 5).
- Section 4 gives dead / spawn / altar / gem camps / gold camps / transit per map for us and them.
