# v5r, R1 and R2a release builds (20 Oct 2026, cloud)

Brief: tools/cloud/v5r-release.md (with the lead's two added sections). Engine 1.0.0, CLI 3.0.1, the **35-map pool** in maps/ on main, `--jobs 3`. `scripts/preflight.py` needs a platform sign-in, which this session may not use. The gates therefore ran against a local ground truth whose pool is the 35 maps in maps/ (map-set hash 60fe3909bfbe, not checked against the platform; state/ground-truth.json was not committed).

## Admission (rules fixed in the brief)

### v5r = v5 + BANK_REACH (altar walks only) + NSB walk-out + 35-pool map ids + retry fuel guard

| # | Rule | Result | Verdict |
|---|---|---|---|
| 1 | gate.py PASS on 35 maps | 69/70: **lost gold_rush/B** (about 407-429); shortcut 2/2 (520-0, 330-11); mapid correct on all 35; no fuel exhaustion after round 1, max 93,874 (`results/gate-v5r-20261020-194918Z-linux.json`) | **FAIL** |
| 2a | Unopposed: no map more than 10 % slower than v5 | broken_curve +40 % at salt 0 (672 → 480 gems) and +19 % over 3 salts (669 → 547); islands +19 % at salt 0, +6 % over 3 salts | **FAIL** |
| 2b | Unopposed: shortcut above 0 gems | 0 and 0 at salt 0; mean 7 over 3 salts (one game) | **FAIL** at salt 0 |
| 3 | Deaths vs v4 ≤ v5's 17.0 + 10 % (18.7) | 16.4 on the old 21 maps (19.8 on all 35; v4's own salted copies 24.5 on 35) | **PASS** |
| 4 | vs v4 ≥ 45 % | 138/210 (65.7 %); old 21: 80/126. Null: v4's salted copies vs v4 118/210, 73/126 | **PASS** |

Against starter-cpp over 3 salts × 2 sides (6 games a map): shortcut 6/6 (v5 0/6), gold_rush 4/6 (v5 5/6), starvation 6/6, starfish 6/6, reverse_spiral 6/6.

### R1 = bots/s1g1 + altar-only zone entry + 35-pool map ids + retry fuel guard

| # | Rule (brief's R1 section) | Result | Verdict |
|---|---|---|---|
| 1 | gate.py PASS on 35 maps | 68/70: **lost reverse_spiral/B** (v4 also loses it: `results/gate-v4-20261020-194436Z-linux.json`) and **starfish/B** (318-357); shortcut 2/2 (299-38, 317-98); no fuel exhaustion, max 93,297 (`results/gate-r1-20261020-194758Z-linux.json`) | **FAIL** |
| 2 | r1 vs v4, reject below 62/126 on the old 21 | 68/126 on the old 21; 124/210 on 35 (null 118/210); deaths 24.0 (null 24.5) | **PASS** |
| 3 | Unopposed vs idle on 35 (vs v4) | median 2,109 → 1,874 (−11 %); 15 maps faster. Slower: starvation +33 % at salt 0 (−11 % over 3 salts), starfish +17 % at salt 0 (+14 % over 3 salts). shortcut 0 → 10 gems | reported, no rule |

Against starter-cpp, 6 games a map: shortcut 6/6 (v4 0/6), gold_rush 6/6, starvation 6/6, starfish 5/6 (v4 6/6), reverse_spiral 5/6 (v4 5/6).

### R2a = v5r with P_S2_DRAFT_CORE 0
**Gate PASS: 70/70** on 35 maps, max fuel 93,874 (`results/gate-r2a-20261020-200307Z-linux.json`). No other test was asked for or run.

## What was built (all switches on in the delivered builds)
- **P_S1_BANK_REACH**, ported by hand from bots/s1 with the same comments, plus the helper `awake_tiles` (util.cpp):
  - build_view leaves an asleep Monolith outside the target out of the view (bot.cpp ~l.83-90);
  - an altar whose only submit tiles are Monolith spawn tiles is dropped; with none left the team keeps its bag and farms (bot.cpp, init phase 2);
  - an asleep Monolith zone may be entered when every goal lies inside zones (util.cpp move_towards_closest).
- **Deviation, measured: P_S1_BANK_REACH_ALTAR_ONLY 1.** The zone entry applies only to bank()'s walk to an altar (flag `g_bank_move`). With it on every move, as in bots/s1 and s1g1 (kept as `bots/v5r-port`), farmers walked into gold_rush's Monolith zone after gold clusters. Results with it on every move: unopposed gold_rush 451 → 116 gems; against starter, 8/12 vs 10/12 for the altar-only build (12 salted games each).
- **P_S1_NO_SELF_BLOCK, walk-out only** (P_S1_NSB_WALK 1; the turn-end step-off is not ported, P_S1_NSB_STEP 0).
- **P_S1_RETRY_FUEL 1 (starvation fuel fix).** move_towards_closest times its first attempt and runs the walk-out and zone retries only if the fuel left covers that cost plus 10k. On starvation (80 × 80) a field costs about 24k. A failed bank or farm move then ran three attempts, 65-77k for that stage alone, and the gate ran out of fuel (r1 side A: 2 turns; stage stamps from bots/s1's P_S1_FUELDBG). After the fix, the gate max is 93,297 (r1) and 93,874 (v5r), with no exhaustion.
- **Map ids:** 14 new MapId values (`packs_p35`, `pilgrimage_p35`, `rows_p35`, `stampede_p35`, `swirl_p35` and the new names) with map_name = the pool name. The exact-table keys are the laptop's {w,h,groups}; there are no setup functions. The retired ids (rows, pilgrimage, stampede, swirl, packs_v2, rushbait) are untouched, so route.h still sends only the old pilgrimage layout to bot B.
- Not ported: AUCTION, THREAT_COST, ONE_CAPTURER, SPAN, the measurement salt.

## v5r vs v5 (210 games): which maps changed
At salt 0 the two games of a map are exact mirrors (v5r behaves as v5) on 19 maps; mirror (19): ambush, boss_battle, contest, corners, eight, elkring, expanse, fragment, hooks, interlock, lanes, monolith, pilgrimage, quadrants, reverse_spiral, rows, rushbait_v2, swirl, the_farm
- They differ on 16 maps; differ (16): big_world, broken_curve, collision, concentrated_wealth, corridors, elk_arena, fracture, gold_rush, islands, packs, shortcut, split, stampede, starfish, starvation, yellow_brick_road

BANK_REACH should change only maps with an asleep Monolith near an altar. The differing set is wider: it also holds maps where the NSB walk-out acts (spawn-tile blocks: concentrated_wealth, packs, split and others), plus maps where any change of fuel or timing alters a deterministic game. Totals: 123/210 for v5r, deaths 22.6, gem margin +7. On the 14 new maps, 44/84 wins.

## The 14 new maps
v5r vs v4 (6 games a map; null = v4's salted copies vs v4):

| map | wins/6 | deaths/game | gem margin/game | null wins/6 | null deaths |
|---|---|---|---|---|---|
| packs | 6 | 9.7 | +590 | 4 | 19.5 |
| pilgrimage | 5 | 26.7 | +180 | 1 | 37.3 |
| quadrants | 3 | 15.3 | +45 | 3 | 17.8 |
| reverse_spiral | 6 | 34.5 | +101 | 3 | 44.0 |
| rows | 5 | 0.7 | +106 | 2 | 0.7 |
| rushbait_v2 | 2 | 17.0 | +37 | 5 | 10.2 |
| shortcut | 6 | 8.8 | +82 | 5 | 7.2 |
| split | 3 | 53.2 | -84 | 3 | 55.5 |
| stampede | 4 | 18.3 | +93 | 3 | 32.2 |
| starfish | 3 | 64.0 | -24 | 3 | 53.3 |
| starvation | 1 | 28.8 | -190 | 4 | 25.3 |
| swirl | 6 | 51.3 | +574 | 3 | 65.5 |
| the_farm | 6 | 4.7 | +261 | 3 | 10.7 |
| yellow_brick_road | 2 | 14.8 | -126 | 3 | 18.2 |

R1 vs v4:

| map | wins/6 | deaths/game | gem margin/game | null wins/6 | null deaths |
|---|---|---|---|---|---|
| packs | 4 | 15.3 | +290 | 4 | 19.5 |
| pilgrimage | 5 | 34.7 | +83 | 1 | 37.3 |
| quadrants | 3 | 17.5 | +36 | 3 | 17.8 |
| reverse_spiral | 3 | 44.3 | -74 | 3 | 44.0 |
| rows | 3 | 0.7 | -20 | 2 | 0.7 |
| rushbait_v2 | 4 | 20.2 | +64 | 5 | 10.2 |
| shortcut | 6 | 19.2 | +144 | 5 | 7.2 |
| split | 3 | 51.3 | +11 | 3 | 55.5 |
| stampede | 5 | 29.3 | +80 | 3 | 32.2 |
| starfish | 6 | 49.3 | +472 | 3 | 53.3 |
| starvation | 4 | 20.7 | +313 | 4 | 25.3 |
| swirl | 2 | 70.0 | -261 | 3 | 65.5 |
| the_farm | 4 | 9.7 | +12 | 3 | 10.7 |
| yellow_brick_road | 4 | 16.5 | +48 | 3 | 18.2 |

v5r vs v5:

| map | wins/6 | deaths/game | gem margin/game |
|---|---|---|---|
| packs | 4 | 12.3 | +182 |
| pilgrimage | 5 | 29.8 | +42 |
| quadrants | 1 | 13.3 | -123 |
| reverse_spiral | 5 | 14.3 | +64 |
| rows | 4 | 0.7 | +45 |
| rushbait_v2 | 3 | 20.8 | +123 |
| shortcut | 6 | 20.5 | +218 |
| split | 1 | 52.7 | -55 |
| stampede | 4 | 25.7 | +80 |
| starfish | 1 | 65.8 | -630 |
| starvation | 1 | 26.2 | -168 |
| swirl | 3 | 62.8 | +99 |
| the_farm | 3 | 5.3 | -73 |
| yellow_brick_road | 3 | 16.7 | +64 |

Unopposed vs idle, salt 0 (A / B): v5 → v5r

| map | base gems (A/B) | new gems (A/B) | base last round | new last round |
|---|---|---|---|---|
| packs | 884 / 1004 | 1020 / 1020 | 2000 / 1258 | 974 / 1013 |
| pilgrimage | 243 / 298 | 243 / 268 | 2000 / 2000 | 2000 / 2000 |
| quadrants | 1038 / 1066 | 1038 / 1066 | 758 / 1080 | 758 / 1080 |
| reverse_spiral | 326 / 254 | 326 / 254 | 2000 / 2000 | 2000 / 2000 |
| rows | 1028 / 1007 | 1028 / 1007 | 1597 / 1556 | 1597 / 1556 |
| rushbait_v2 | 161 / 0 | 161 / 0 | 2000 / 2000 | 2000 / 2000 |
| shortcut | 0 / 0 | 0 / 0 | 2000 / 2000 | 2000 / 2000 |
| split | 910 / 654 | 855 / 593 | 2000 / 2000 | 2000 / 2000 |
| stampede | 80 / 320 | 480 / 640 | 2000 / 2000 | 2000 / 2000 |
| starfish | 1028 / 1019 | 1003 / 1024 | 1744 / 1629 | 1439 / 1990 |
| starvation | 400 / 476 | 524 / 564 | 2000 / 2000 | 2000 / 2000 |
| swirl | 679 / 763 | 679 / 763 | 2000 / 2000 | 2000 / 2000 |
| the_farm | 1001 / 1011 | 1001 / 1011 | 1388 / 1595 | 1388 / 1595 |
| yellow_brick_road | 1009 / 1000 | 1009 / 1006 | 1311 / 1296 | 1311 / 1490 |

Unopposed vs idle, salt 0: v4 → R1

| map | base gems (A/B) | new gems (A/B) | base last round | new last round |
|---|---|---|---|---|
| packs | 804 / 824 | 1008 / 1034 | 2000 / 2000 | 1091 / 1024 |
| pilgrimage | 250 / 254 | 187 / 301 | 2000 / 2000 | 2000 / 2000 |
| quadrants | 1002 / 1023 | 1006 / 1035 | 818 / 1030 | 866 / 1096 |
| reverse_spiral | 296 / 318 | 296 / 318 | 2000 / 2000 | 2000 / 2000 |
| rows | 1014 / 1019 | 1071 / 1000 | 1605 / 1636 | 1688 / 1699 |
| rushbait_v2 | 739 / 210 | 771 / 210 | 2000 / 2000 | 2000 / 2000 |
| shortcut | 0 / 0 | 0 / 21 | 2000 / 2000 | 2000 / 2000 |
| split | 1021 / 750 | 1000 / 693 | 1841 / 2000 | 1974 / 2000 |
| stampede | 0 / 0 | 0 / 0 | 2000 / 2000 | 2000 / 2000 |
| starfish | 1042 / 1012 | 1000 / 815 | 1692 / 1598 | 1516 / 2000 |
| starvation | 692 / 676 | 444 / 584 | 2000 / 2000 | 2000 / 2000 |
| swirl | 1003 / 884 | 1003 / 884 | 1981 / 2000 | 1981 / 2000 |
| the_farm | 1018 / 1061 | 1018 / 1061 | 1597 / 1765 | 1597 / 1765 |
| yellow_brick_road | 1002 / 1019 | 1002 / 1020 | 1066 / 1504 | 1066 / 1439 |

All 35 maps, unopposed, salt 0 (round to 1,000 = 1000 ÷ mean of gems ÷ last round):

v5 → v5r:

| map | base r1000 | new r1000 | change | base gems | new gems | base deaths | new deaths |
|---|---|---|---|---|---|---|---|
| ambush | 1810 | 1810 | +0 % | 1025 | 1025 | 3.0 | 3.0 |
| big_world | 1790 | 1773 | -1 % | 1022 | 1005 | 2.0 | 1.0 |
| boss_battle | 100000 | 100000 | +0 % | 0 | 0 | 3.0 | 3.0 |
| broken_curve | 2974 | 4162 | +40 % | 672 | 480 | 6.5 | 3.5 |
| collision | 1203 | 1209 | +0 % | 1028 | 1006 | 2.0 | 1.5 |
| concentrated_wealth | 3077 | 2948 | -4 % | 650 | 678 | 2.0 | 1.0 |
| contest | 100000 | 100000 | +0 % | 0 | 0 | 5.5 | 0.0 |
| corners | 1177 | 1177 | +0 % | 1010 | 1010 | 0.0 | 0.0 |
| corridors | 5076 | 4338 | -15 % | 394 | 461 | 2.5 | 3.5 |
| eight | 1143 | 1143 | +0 % | 1005 | 1005 | 0.5 | 0.5 |
| elk_arena | 1694 | 1692 | -0 % | 1008 | 1008 | 0.0 | 0.0 |
| elkring | 887 | 887 | +0 % | 1012 | 1012 | 0.0 | 0.0 |
| expanse | 1557 | 1557 | +0 % | 1014 | 1014 | 0.0 | 0.0 |
| fracture | 5495 | 4635 | -16 % | 364 | 432 | 17.5 | 8.5 |
| fragment | 1577 | 1577 | +0 % | 1000 | 1000 | 0.0 | 0.0 |
| gold_rush | 4435 | 3497 | -21 % | 451 | 572 | 20.5 | 5.0 |
| hooks | 1234 | 1234 | +0 % | 1006 | 1006 | 2.0 | 2.0 |
| interlock | 1270 | 1270 | +0 % | 1010 | 1010 | 0.0 | 0.0 |
| islands | 2523 | 3008 | +19 % | 738 | 665 | 1.0 | 0.0 |
| lanes | 1277 | 1277 | +0 % | 1015 | 1015 | 3.5 | 3.5 |
| monolith | 1278 | 1278 | +0 % | 1005 | 1005 | 0.0 | 0.0 |
| packs | 1613 | 974 | -40 % | 944 | 1020 | 0.0 | 0.0 |
| pilgrimage | 7394 | 7828 | +6 % | 270 | 256 | 0.5 | 0.0 |
| quadrants | 849 | 849 | +0 % | 1052 | 1052 | 21.5 | 21.5 |
| reverse_spiral | 6897 | 6897 | +0 % | 290 | 290 | 12.0 | 12.0 |
| rows | 1549 | 1549 | +0 % | 1018 | 1018 | 1.0 | 1.0 |
| rushbait_v2 | 24845 | 24845 | +0 % | 80 | 80 | 0.0 | 0.0 |
| shortcut | 100000 | 100000 | +0 % | 0 | 0 | 0.5 | 0.5 |
| split | 2558 | 2762 | +8 % | 782 | 724 | 40.0 | 44.0 |
| stampede | 10000 | 3571 | -64 % | 200 | 560 | 2.0 | 0.0 |
| starfish | 1646 | 1651 | +0 % | 1024 | 1014 | 16.5 | 13.5 |
| starvation | 4566 | 3676 | -19 % | 438 | 544 | 2.0 | 0.0 |
| swirl | 2774 | 2774 | +0 % | 721 | 721 | 10.0 | 10.0 |
| the_farm | 1476 | 1476 | +0 % | 1006 | 1006 | 0.0 | 0.0 |
| yellow_brick_road | 1298 | 1384 | +7 % | 1004 | 1008 | 17.0 | 21.0 |
median 1790 -> 1773 (-1.0 %; rule: <= -15 %); worst map broken_curve +40 % (rule: <= +10 %); total gems 25261 -> 25702; deaths 5.6 -> 4.6

v4 → R1:

| map | base r1000 | new r1000 | change | base gems | new gems | base deaths | new deaths |
|---|---|---|---|---|---|---|---|
| ambush | 1836 | 1836 | +0 % | 1032 | 1032 | 6.0 | 6.0 |
| big_world | 2113 | 1988 | -6 % | 946 | 1002 | 4.5 | 1.5 |
| boss_battle | 100000 | 100000 | +0 % | 0 | 0 | 1.5 | 1.5 |
| broken_curve | 16000 | 9324 | -42 % | 125 | 214 | 5.5 | 3.5 |
| collision | 1251 | 1176 | -6 % | 1014 | 1014 | 2.5 | 0.5 |
| concentrated_wealth | 2577 | 2291 | -11 % | 776 | 873 | 0.0 | 0.0 |
| contest | 100000 | 100000 | +0 % | 0 | 0 | 3.5 | 0.0 |
| corners | 1154 | 1154 | +0 % | 1003 | 1003 | 0.0 | 0.0 |
| corridors | 10101 | 7313 | -28 % | 198 | 274 | 7.0 | 7.5 |
| eight | 1278 | 1278 | +0 % | 1009 | 1009 | 0.0 | 0.0 |
| elk_arena | 5051 | 4630 | -8 % | 396 | 432 | 1.0 | 1.5 |
| elkring | 844 | 844 | +0 % | 1012 | 1012 | 0.0 | 0.0 |
| expanse | 1437 | 1437 | +0 % | 1010 | 1010 | 0.0 | 0.0 |
| fracture | 6309 | 3089 | -51 % | 317 | 648 | 12.0 | 15.5 |
| fragment | 1607 | 1607 | +0 % | 1004 | 1004 | 0.5 | 0.5 |
| gold_rush | 5594 | 3600 | -36 % | 358 | 556 | 20.5 | 7.0 |
| hooks | 1229 | 1198 | -3 % | 1018 | 1006 | 2.5 | 1.5 |
| interlock | 1125 | 1125 | +0 % | 1013 | 1013 | 0.5 | 0.5 |
| islands | 1350 | 1362 | +1 % | 1028 | 1015 | 1.5 | 2.0 |
| lanes | 1275 | 1251 | -2 % | 1006 | 1006 | 6.0 | 5.0 |
| monolith | 1358 | 1358 | +0 % | 1009 | 1009 | 0.5 | 0.5 |
| packs | 2457 | 1034 | -58 % | 814 | 1021 | 0.0 | 0.0 |
| pilgrimage | 7937 | 8197 | +3 % | 252 | 244 | 0.0 | 0.0 |
| quadrants | 902 | 950 | +5 % | 1012 | 1020 | 24.5 | 25.5 |
| reverse_spiral | 6515 | 6515 | +0 % | 307 | 307 | 10.0 | 10.0 |
| rows | 1594 | 1635 | +3 % | 1016 | 1036 | 0.0 | 1.0 |
| rushbait_v2 | 4215 | 4077 | -3 % | 474 | 490 | 0.0 | 0.0 |
| shortcut | 100000 | 100000 | +0 % | 0 | 10 | 2.5 | 9.0 |
| split | 2151 | 2344 | +9 % | 886 | 846 | 38.0 | 45.0 |
| stampede | 100000 | 100000 | +0 % | 0 | 0 | 0.0 | 0.0 |
| starfish | 1601 | 1874 | +17 % | 1027 | 908 | 19.5 | 18.5 |
| starvation | 2924 | 3891 | +33 % | 684 | 514 | 1.0 | 2.0 |
| swirl | 2109 | 2109 | +0 % | 944 | 944 | 19.5 | 19.5 |
| the_farm | 1615 | 1615 | +0 % | 1040 | 1040 | 3.0 | 3.0 |
| yellow_brick_road | 1236 | 1213 | -2 % | 1010 | 1011 | 15.5 | 14.5 |
median 2109 -> 1874 (-11.1 %; rule: <= -15 %); worst map starvation +33 % (rule: <= +10 %); total gems 24742 -> 25522; deaths 6.0 -> 5.8

## Cause traces
**shortcut (v5: 0-0 against starter, 2,000 rounds).** Altars at (32,12) and (32,27), each beside a Monolith at (31,12) and (31,27) with range 5. Every submit tile is inside a zone, and `P_MONO_ZONE_ALWAYS` walls the zones off even while the Monoliths sleep. Bot trace (v5 + prints, side A against starter): carriers in `banking` with 21-42 gems stood at (18,27) and (19,27) from round 300 to at least 600 and never submitted. With BANK_REACH they walk in while the Monoliths sleep: 520-0 and 330-11 in the gate, 6/6 against starter over 3 salts.

*Unopposed shortcut stays at 0 for v5r.* Trace against idle (spawns: ours at (18,25), theirs at (18,12)): three carriers waited at (17-19,17) next to the idle team's five parked champions. v4's bank safety rule does not walk to an altar with enemies within 4 tiles of it, and idle champions never leave. The fourth carrier stood 4 tiles from altar (32,27) with no enemy in view, cause not found. Rule 2b therefore fails in a way that an idle opponent may cause; the starter results are the better guide.

**gold_rush/A (first v5r build, lost 209-440 to starter).** From round 800 to 2000 all four champions alternated between two tiles each beside the Monolith at (15,15): (16,11)↔(15,12) and (16,16)↔(15,17). Decision trace (P_ROWS_DBG with return markers): on every other turn v5's S2 shopping detour (`s2_shop_detour`, P_S2_SPEND_NOW; ≥ 90 gold, spawn within 12 tiles) moved them one step toward the spawn. The path bends away around the Monolith zone, so the spawn left the 12-tile radius and the next turn's farm step went back. Making the detour sticky lost both gold_rush games (4/12 against starter), so it was dropped. The trigger was BANK_REACH's zone entry for every move, which brought farmers inside that zone; restricting it to altar walks (above) took gold_rush to 10/12 and the gate loss moved to gold_rush/B. **The S2 detour oscillation is still in v5 and v5r** whenever a path to the spawn leaves the radius.

**starvation fuel** (lead's probe: s1g1 exhausted 2 turns): see P_S1_RETRY_FUEL above. bank stage 65-73k, farm stage 73-77k on starvation before the fix.

## Still failing / for the lead
- No build passes all of its own rules. **R2a is the only gate PASS** (70/70). R1's gate losses are reverse_spiral/B (v4's own loss) and starfish/B (5/6 against starter over 3 salts). v5r's is gold_rush/B (4/6 over 3 salts).
- The gate plays one deterministic game per map and side, and maps like gold_rush flip on any change of timing. The 6-game starter rows above are the better measure of each build.
- Fuel-exhausted turns against v4 at salt 0: R1 2 in 70 games, v5r vs v5 1. The gate (vs starter) has none.
- Not done: /watch (no browser here).
