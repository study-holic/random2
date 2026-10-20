# w13: target value = expected gems per round of commitment (rate scoring). Verdict: REJECT

Builder w13, copy `bots/w13` (from v2a), 17:30 to 21:00 BST 19 Oct. Question: does scoring targets by expected gems per round of commitment raise our pace and beat v2? **No.** Both falsifiers fixed in the brief are met: the median win round fell on 6 of 17 maps (needed 10), and head-to-head against v2 is 39/102 (38%, needed 50%). The panel total fell from 151.7 to 131.3 of 204, and no map meets the admission rule.

## What changed (file:line, `bots/w13/src`)

- `score.cpp` (new). `Bot::init_econ` (score.cpp:48) caches per group: total HP and strength from the rules monster table, gold (`camp_group::gold_value()`), the Chebyshev distance to the nearest usable altar, and a danger integral `q` (score.cpp:78). `q` = sum over kills, in focus-fire order with the most strength per HP first, of (kill HP) x (strength still alive). Damage taken while clearing at D damage per round is then q/D. It also caches per roster champion a kit damage per round, `base_strength * camp_damage_multiplier + damage spell / 8` (the x-foundry-v16 estimate), and base health.
- `Bot::rate_self` and `Bot::target_rate` (score.cpp:89, :96). The terms:
  - t = Chebyshev distance; w = max(0, ready - round - t).
  - D = own current strength x multiplier + burst/8, plus the kit damage of each live squad mate when the group is a squad group.
  - c = HP/D; capture = `constants::capture_time` (5).
  - sorties = ceil((q/D) / pool), capped at 8, where pool = usable own health (current - 35% of max, at most 60% of max) plus 60% of each mate's base health. Each extra sortie costs `dist_ally + 12`.
  - bank = min(1, gems/36) x altar distance + ceil(gems/10).
  - The v2 own-half penalty after round 400 and the claim bonus (8) are subtracted from the rounds.
  - **Rate = (gems + 0.15 x gold) / rounds.** Highest wins.
- `squads.cpp:112-141` `choose_target`: with `score_mode == 1` the score is `-target_rate(...)`; the filters (own half before 400, contested, claims, taken) are unchanged. `score_mode == 0` is v2's `d + 2w - 0.05 gems` exactly; `score_mode == 2` is v2 plus `P_W13_GOLD_TILES` for gem-less groups (late variant w13e, below). `squads.cpp:119` adds a monolith hook for w9: `allow_monolith` (bot.h), default false. `bot.cpp:157-158` sets `score_mode = P_SCORE_MODE` and calls `init_econ()` (mode 1 only) before `Plan::init`, so a map file can set `b.score_mode` per map; a map switching to mode 1 must call `init_econ()` itself.
- `params.h:125-155` (P_W13_DBG at :154) `P_SCORE_MODE` (1), `P_W13_GOLD_LAMBDA` 0.15, `P_W13_POOL_FRAC` 0.6, `P_W13_SORTIE` 12, `P_W13_SORTIE_MAX` 8, `P_W13_SQUAD_DIV` 0, `P_W13_DPS_EFF` 1.0, `P_W13_CONTACT` 0, `P_W13_GOLD_TILES` 20, `P_W13_DBG` 0. Setting `P_SCORE_MODE 0` restores v2.
- Fuel, after the panel run:
  - The panel build had fuel_late 636 against v2's 241, all on lanes (360 vs 144), expanse (192 vs 96) and corners (84 vs 1). The fuel log (`team-a.fuel`) shows one extra 100,000-capped turn per champion right after the init turns, so the cause is the init cost, not the per-turn cost.
  - Reading own stats once per `choose_target` call (`rate_self`, score.cpp:89) left lanes and expanse unchanged against starter-cpp (13 and 8; `results/w13-fuelfix-starter.json`).
  - `bot.cpp:158` now runs `init_econ()` only in score mode 1. Its per-camp type reads repeat the ones in `Bot::init`; with 42 groups on lanes that is the likely extra 100k. If mode 1 is ever revived, fold the counts into the camp loop in `Bot::init`.

Rules and SDK lines relied on:
- `GameRules.md:179-184`: monster table (wolf 1,350 HP / 20 str / 90 gold / 22 gems; elk 2,400 / 60 / 160 / 36; clusters 500 HP, 0 str).
- `:189`: "awaken attacking the nearest champion every turn".
- `:193`: "a five-round timer ... Rewards are shared among allies present".
- `:206`: "All monsters have attack range 3".
- `:223`: tribute "max(1, ceil(starting gems / 10))".
- `:290`: "Shards cost 30 gold".
- `:361`: "The killer receives all carried gems".
- `sdk/cpp/imports.h:31-37`: `champion_stats::camp_damage_multiplier`.
- `cambc.h:3662`: `champion::base_stats()`; `cambc.h:4395/4398`: `gold_value()`/`gem_value()`; `cambc.h:86`: `constants::capture_time`.
- Measured in a smoke game (P_W13_DBG): `camp_damage_multiplier` = 1 for every drafted type. Kit damage per round: Vendetta 70, Cassi 47.5, Issac 40, Varg 50, Ryth 56.25, Tavi 35.

## Development runs (salt 0, 4 target maps)

| variant | panel W (stampede/pilgrimage/ambush/swirl) | h2h vs v2 |
|---|---|---|
| v2 (salt 0 of v2-panel) | 11 / 10 / 11 / 8 = 40 | - |
| rate / committed champions (`P_W13_SQUAD_DIV 1`) | 9 / 8 / 10 / 10 = 37 | 2/8 |
| rate, brief's formula (`P_W13_SQUAD_DIV 0`, the tested build) | 11 / 9 / 12 / 9 = 41 | 6/8 |

The third row ran as copy `w13b`; it is the same code as the final `w13`, and its salt-0 games replay identically in the full panel. The second row was the first `w13` build.

## Test 1: panel, 3 salts, all 17 maps (`results/w13-scoring.json`, base `results/v2-panel.json`)

| map | v2 W/12 | w13 W/12 | v2 strong/4 | w13 strong/4 | v2 medR | w13 medR | top-50 | v2 gemD | w13 gemD | v2 deaths | w13 deaths | h2h W/6 | h2h gemD |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| ambush | 10.67 | 11.33 | 2.67 | 3.33 | 899 | **843** | 594 | 851 | 875 | 2.1 | 2.0 | 3/6 | 46 |
| collision | 9.00 | **1.33** | 1.67 | 0.00 | 761 | 917 | 695 | 199 | -531 | 4.1 | 9.1 | 0/6 | -230 |
| contest | 10.00 | 9.67 | 2.00 | 1.67 | 2000 | 2000 | 1602 | 0 | 0 | 20.0 | 21.8 | 4/6 | 0 |
| corners | 6.00 | 4.67 | 1.33 | 1.00 | 466 | 486 | 536 | 25 | -50 | 2.9 | 2.8 | 3/6 | 15 |
| eight | 8.00 | 9.00 | 1.00 | 1.33 | 734 | **696** | 640 | 262 | 312 | 9.7 | 7.5 | 4/6 | 98 |
| elkring | 9.33 | 7.00 | 2.33 | 0.33 | 590 | 652 | 577 | 309 | 140 | 4.4 | 5.1 | 1/6 | -90 |
| expanse | 11.00 | 7.00 | 3.67 | 1.00 | 464 | 538 | 513 | 261 | 72 | 1.2 | 1.6 | 0/6 | -143 |
| fragment | 9.00 | 7.00 | 2.00 | 0.00 | 852 | **762** | 772 | 376 | 305 | 15.3 | 13.9 | 0/6 | -328 |
| interlock | 6.67 | 6.67 | 1.33 | 1.33 | 576 | 601 | 663 | -1 | -24 | 3.4 | 4.0 | 4/6 | 60 |
| lanes | 7.33 | **3.33** | 1.33 | 0.00 | 435 | 650 | 468 | 180 | -177 | 2.7 | 2.9 | 0/6 | -469 |
| monolith | 11.00 | 10.33 | 3.00 | 2.33 | 584 | 605 | 447 | 461 | 470 | 4.0 | 4.7 | 3/6 | 2 |
| packs | 10.00 | 9.00 | 2.00 | 1.00 | 611 | **592** | 528 | 291 | 216 | 4.5 | 4.3 | 3/6 | -54 |
| pilgrimage | 10.00 | 10.33 | 3.00 | 2.67 | 1278 | **1101** | 844 | 273 | 344 | 10.4 | 7.0 | 3/6 | -87 |
| rows | 11.67 | 11.67 | 4.00 | 4.00 | 481 | 488 | 386 | 623 | 605 | 0.6 | 0.9 | 3/6 | 0 |
| rushbait-v2 | 4.33 | 5.67 | 0.67 | 1.33 | 1006 | 1035 | 660 | -142 | -17 | 3.5 | 3.0 | 1/6 | -220 |
| stampede | 9.67 | 9.67 | 2.33 | 3.33 | 1752 | **1627** | 686 | 610 | 427 | 17.4 | 10.9 | 4/6 | 420 |
| swirl | 8.00 | 7.67 | 1.33 | 1.33 | 1249 | 1259 | 600 | 305 | 236 | 36.7 | 41.1 | 3/6 | -275 |
| **total** | **151.67** | **131.33** | | | | | | | | | | **39/102** | |

Caveat: v2 routes pilgrimage to b-v418d (`bots/v2/src/route.h`), while w13 is built on v2a, so the pilgrimage row compares v2a+change against b-v418d.

`cmpcells` (`results/w13-scoring-cmp.txt`): total dW -20.33. **Admitted: none.** The best maps are rushbait (+1.33 W, +1,499 gem change, +0.67 strong) and eight (+1.0, +591, +0.33); both fall short of dW >= 1.5. The worst are collision (-7.67, -8,768), expanse (-4.0), lanes (-4.0) and elkring (-2.33).

## Test 2: head-to-head against v2, 3 salts (`results/w13-h2h-merged.json`)

**39/102 (38%)**, mean gem margin -74.

- By salt: 17/34, 11/34, 11/34. By side: A 19/51, B 20/51.
- Per map (wins of 6, gem margin) is in the table above. Above 3/6: stampede 4/6 (+420), eight 4/6 (+98), interlock 4/6 (+60), contest 4/6 (0 gems on both sides; decided by gold). Shut out: collision, expanse, fragment, lanes (0/6 each).
- The first full run (`results/w13-h2h.json`) stopped with rc 1 after 41 games, and the missing statistics were counted as losses. Its 34 complete salt-0 games were kept, and salts 1-2 were re-run (`results/w13-h2h-s12.json`, 0 errors).

## Test 3: pace (median win round, panel)

The median win round fell on **6 of 17 maps**: ambush 899 to 843, eight 734 to 696, fragment 852 to 762, packs 611 to 592, pilgrimage 1,278 to 1,101, and stampede 1,752 to 1,627. It rose on 10 and stayed at 2,000 on contest. w13 beats the top-50 yardstick on corners (486 against 536), fragment (762 against 772) and interlock (601 against 663). v2 beat it on corners, expanse, interlock and lanes. The largest remaining gaps are still swirl (1,259 vs 600), stampede (1,627 vs 686) and pilgrimage (1,101 vs 844).

## Test 4: safety against starter-cpp (`results/w13-starter.json`, all 17 maps, both sides, panel build)

- 34/34 wins and 0 faults.
- "bot error" lines in our team's stdout: 35, all `engine error 97: Unit is not in vision`. v2's starter run has 27 (22 of kind 97, 4 of 98, 1 of 22), so there is no new kind.
- fuelExhausted after round 2: 21 events (expanse 8, lanes 13) against v2's 0, which fails the safety rule. The init cost (see Fuel above) is the likely cause; mode 2 skips that init. Separately, on stampede the starter game ends 0-0 at round 2,000 (v2 against starter: +810 gems), so we never took the block.

## Champion-rounds by activity (A side, salt 0, 4 opponents per map; `results/w13-activity.py`, stride-20 extracts)

Classes: within 4 tiles of a gem-bearing group / of a gold-only group / other (walking, banking, healing) / dead.

| map | v2 gem / gold / other / dead | w13 gem / gold / other / dead |
|---|---|---|
| stampede | 12.1 / 49.0 / 33.2 / 5.6 | 9.1 / 47.6 / 36.3 / 7.0 |
| pilgrimage | 15.6 / 38.8 / 35.6 / 10.1 | 21.1 / 28.6 / 43.2 / 7.1 |
| ambush | 23.6 / 5.2 / 66.6 / 4.5 | 27.4 / 2.5 / 67.7 / 2.5 |
| swirl | 52.8 / 7.0 / 9.5 / 30.7 | 49.3 / 4.0 / 10.1 / 36.7 |
| collision | 66.9 / 18.8 / 13.3 / 1.1 | 61.7 / 12.7 / 12.2 / **13.4** |
| lanes | 57.0 / 0.0 / 38.9 / 4.0 | 38.1 / 10.2 / 49.4 / 2.4 |

The rule moved time from gold to gem groups where the gems are reachable (pilgrimage -10 points of gold, ambush). On stampede it did not move time: for a squad the static rate of the block is 0.71 gems/round against 0.30 for the best gold group, and the squads do walk there. In the stampede game against x-hello (A, salt 0) champions stand at (50-55, 17-27) at r100, r200, r250, r400, r550 and r650. Each time they walk back down the y=0 corridor to the spawn, and no capture happens by r650 (0-0); Issac holds 1,030 unspent gold at r650. This confirms w10: on stampede the bottleneck is the elk fight and the retreat rule (`bot.cpp` incoming x 4), not the target value.

## Traced losses (`tools/v3/extract_pilot.mjs --stride 1`, `derive_events.py`, files in `matches/w13-ex/`)

1. **Pilgrimage, w13 (A) vs v2, salt 0, lost 760-1,052 at r1,492** (`matches/w13b-183005/pilgrimage__v2__A__s0.repl26a`; the w13b build, identical code).
   - At r55 five of our six champions stood within 3 tiles of our own-side 2wolf+1gem+1gold group (4,44) (68 gems). The rate rule then sent them on to the higher-value axis groups (16,46) 2wolf+2gem and (24,58) 4elk+4gold (side 0): at r65 they were at (14-16, 53-54), and at r75 at (20-24, 49-61).
   - Four v2 champions arrived at the same tiles at r75. We lost Vendetta (r75), Cassi and Issac (r81) and Varg (r97) for two of theirs.
   - Meanwhile v2's Varg took its own-side (46,44) group and tributed 68 at r112. Our first tribute came at r351.
   - The change fired here: value per round prefers the 92-261-value groups on the symmetry axis, where the enemy also goes, over the near own-side group.
2. **Collision, w13 (A) vs x-hello, salt 0, lost 572-1,012** (`matches/w13-184115/collision__x-hello__A__s0.repl26a`).
   - The squad farms the far corner 1elk+3gem (38,3) (108 gems + 160 gold, 35 tiles; rate about 1.4 against about 0.5 for the 1wolf groups at 7-16 tiles that v2 farms). The trips home to heal run along y=0-3. A carrier holds 108 gems from r90 to r180.
   - Across the four salt-0 collision games, 30 deaths, 28 of them to enemy champions. Most were near (19-27, 16-23) on the walk to the centre, several with 108-152 gems. GameRules:361 gives those gems to the killer.
   - Collision deaths per game rose from 4.1 to 9.1 on the panel.

Likely cause, overall: the value model (1) ignores contact: high-value groups are central or far, and the enemy goes there too; (2) is optimistic about squad clears and treats gold as value although gold stays unspent (stampede champions held 1,000-2,000 gold, as w10 saw); and (3) builds bigger bags that are lost to kills at contested altars. v2's nearest-first habit farms small, close, safe groups with small bags, and that beats it on 4-body maps (collision, lanes, expanse, fragment).

## Variants tried after the panel (salt 0 only; each is noise-level, one sample)

| variant | maps | panel W (v2 s0 / w13 s0 / variant) | h2h vs v2 |
|---|---|---|---|
| w13d: effective damage x0.5, gold lambda 0.05 | collision, lanes, expanse, elkring, fragment, stampede, pilgrimage, eight | 75 / 59 / 56 of 96 | 4/16 |
| w13c: contact penalty, +15 rounds per tile of margin below 3 | collision, lanes, pilgrimage, swirl | 35 / 24 / 19 of 48 | 1/8 |
| w13e: v2 score + 20 tiles for gem-less groups (`P_SCORE_MODE 2`) | stampede, pilgrimage, ambush, swirl | 40 / 41 / 39 of 48 | 7/8 |

Neither calibration (w13d) nor a contact term (w13c) rescues the rate rule. The minimal change, w13e, looked good at salt 0, so it got the full test (next section).

## Addendum: w13e = v2's distance score + 20 tiles for gold-only groups (`P_SCORE_MODE 2`), full test

Copy `bots/w13e` = `bots/w13` src + `#define P_SCORE_MODE 2`; the change is `squads.cpp:140`. Panel `results/w13e-panel.json` (3 salts, 612 games, 0 faults), head-to-head `results/w13e-h2h.json` (102 games, rc 0, 0 errors), table `results/w13e-table.txt`, cmpcells `results/w13e-panel-cmp.txt`.

| map | v2 W/12 | w13e W/12 | v2 strong | w13e strong | v2 medR | w13e medR | top-50 | v2 gemD | w13e gemD | h2h W/6 | h2h gemD |
|---|---|---|---|---|---|---|---|---|---|---|---|
| ambush | 10.67 | 11.00 | 2.67 | 3.00 | 899 | 877 | 594 | 851 | 863 | 3 | 47 |
| collision | 9.00 | 5.67 | 1.67 | 0.33 | 761 | 757 | 695 | 199 | -50 | 4 | -4 |
| contest | 10.00 | 10.00 | 2.00 | 2.00 | 2000 | 2000 | 1602 | 0 | 0 | 2 | 0 |
| corners | 6.00 | 5.00 | 1.33 | 2.00 | 466 | 413 | 536 | 25 | -29 | 3 | 108 |
| eight | 8.00 | 8.00 | 1.00 | 1.00 | 734 | 734 | 640 | 262 | 262 | 3 | 0 |
| elkring | 9.33 | 9.33 | 2.33 | 2.33 | 590 | 590 | 577 | 309 | 309 | 3 | 0 |
| expanse | 11.00 | 10.33 | 3.67 | 3.33 | 464 | 467 | 513 | 261 | 250 | 4 | 9 |
| fragment | 9.00 | 9.00 | 2.00 | 2.00 | 852 | 852 | 772 | 376 | 376 | 3 | 0 |
| interlock | 6.67 | 7.67 | 1.33 | 1.67 | 576 | 572 | 663 | -1 | 55 | 4 | 45 |
| lanes | 7.33 | 7.33 | 1.33 | 1.33 | 435 | 436 | 468 | 180 | 187 | 3 | -55 |
| monolith | 11.00 | 11.00 | 3.00 | 3.00 | 584 | 538 | 447 | 461 | 529 | 3 | -26 |
| packs | 10.00 | 10.00 | 2.00 | 2.00 | 611 | 611 | 528 | 291 | 291 | 3 | 0 |
| pilgrimage* | 10.00 | 9.00 | 3.00 | 1.33 | 1278 | 1173 | 844 | 273 | 275 | 4 | 52 |
| rows | 11.67 | 11.67 | 4.00 | 4.00 | 481 | 481 | 386 | 623 | 623 | 2 | -11 |
| rushbait-v2 | 4.33 | 5.00 | 0.67 | 1.00 | 1006 | 1043 | 660 | -142 | -97 | 2 | -188 |
| stampede | 9.67 | 10.33 | 2.33 | 3.00 | 1752 | 1856 | 686 | 610 | 360 | 3 | -270 |
| swirl | 8.00 | 8.33 | 1.33 | 1.33 | 1249 | 1316 | 600 | 305 | 280 | **6** | **591** |
| **total** | **151.67** | **148.67** | | | | | | | | **55/102 (54%)** | |

- Totals and falsifier:
  - Panel dW -3.0. **Admitted: none**: no map reaches dW >= 1.5; interlock is +1.0, +671 gems, +0.33 strong.
  - Median win round fell on 6 of 17 maps (ambush, collision, corners, interlock, monolith, pilgrimage), so the brief's falsifier is met for w13e too.
  - Head-to-head is 54%, but maps with no gold-only group play v2a's code and give the mirror noise: eight, elkring, fragment, packs at 3/6, rows at 2/6, rushbait at 2/6. On the 12 maps with gold-only groups the head-to-head is 41/72 = 57%.
- \* Pilgrimage: v2 routes pilgrimage to b-v418d (`bots/v2/src/route.h`), while every wave-3 copy is v2a. Pilgrimage rows here compare v2a+change against b-v418d, not against v2a.
- Safety: fuel_late is 243 against v2's 241, so skipping `init_econ` outside mode 1 confirms that diagnosis. 0 faults.
- Activity on 4 maps (A, salt 0): pilgrimage gold 38.8% to 30.9% and gem 15.6% to 19.9%; ambush gold 5.2% to 0.9%. Stampede is unchanged (gold 54%).
- Lead worth a follow-up: **swirl**.
  - Extra salts 3-5 (20:20): panel 9.0/12, strong 2.67/4, medR 1,303, gemD +325 (`results/w13e-swirl-s345.json`; no v2 run at these salts). Head-to-head 5/6, +364 gems (`results/w13e-swirl-h2h-s345.json`).
  - Swirl head-to-head over salts 0-5 is therefore **11/12**. The panel is neutral (+0.33 W and -300 gem margin over salts 0-2), and the median win round is slower (1,249 to 1,316).
  - The panel does not meet dW >= 1.5, so it is not admitted under the rule. It is the one candidate for a per-map mode: `b.score_mode = 2` in `map_swirl.cpp`'s init.
  - Collision is the clearest harm (-3.33 W), so mode 2 must stay off there.

## Falsifier (fixed before testing) and verdict

- Falsifier: median win round falls on fewer than 10 of 17 maps, or head-to-head against v2 is below 50% overall. **Met on both counts** (6/17; 38%).
- Verdict: **reject `P_SCORE_MODE 1` on every map.** `bots/w13` keeps it behind the flag; `P_SCORE_MODE 0` is v2.
- **Reject `P_SCORE_MODE 2` (w13e) globally** on the same falsifier (6/17 maps faster) and the admission rule (no map dW >= 1.5).
- Swirl only: head-to-head 11/12 against v2 (salts 0-5) with a neutral panel. That is the lead's call under the head-to-head-first rule; I do not admit it under the cmpcells rule.
- Useful by-products:
  - `results/w13-activity.py`: champion-round classes from extracts.
  - The per-group danger integral and kit-damage cache in `score.cpp`.
  - The `allow_monolith` hook.
  - The finding that on stampede the target value already favours the block, so the fix there must be in the fight and retreat rules.

## Parameters

P_SCORE_MODE 1, P_W13_GOLD_LAMBDA 0.15, P_W13_POOL_FRAC 0.6, P_W13_SORTIE 12, P_W13_SORTIE_MAX 8, P_W13_SQUAD_DIV 0, P_W13_DPS_EFF 1.0, P_W13_CONTACT 0, P_BANK_BAG 36, P_CLAIM_BONUS 8 (inherited). Sweep: SQUAD_DIV {1, 0}; DPS_EFF/lambda {(1, 0.15), (0.5, 0.05)}; CONTACT {0, 15}; all at salt 0 except the final build.
