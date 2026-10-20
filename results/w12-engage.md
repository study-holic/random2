# w12 engage: engagement rules against lone-champion feeding (contest, monolith, all maps)

Builder w12, 19 Oct (the machine slept 02:20 to 17:05; the lead moved the stop to 21:00 BST). Base: v2a. Opponents: the panel (x-v4-14, x-foundry-v16, x-hello, x-ifish, x-mite, x-shawn), both sides, salts 0, 1, 2, and v2 head-to-head (h2h), both sides, salts 0, 1, 2. Note: x-v4-14 is v2's code; its contest and monolith cells match the h2h cells exactly.

**Verdict: reject E1-E3 on every map.** The falsifier I fixed before testing was met: contest and monolith deaths were not down by a third against v2, and the gem margin against v2 was not up.
- E1+E2 on all 17 maps won 28 of 102 h2h games against v2, with dW -51 of 204 on the panel.
- The measurement holds: 60 of 79 ladder deaths were lone fights.
- The cause of those deaths is damage, not numbers. Focus-stacked enemy casters, mainly Tavi, win the fights (§2).
- Follow-ups in §5:
  - w12b (contact-free E1): 34/102 h2h; not admitted on monolith or interlock.
  - w12t (Tavi all-in Power Focus, not an engagement rule): 54/102, monolith 6/6; it duplicates w8's w8tf.

## 1. Deaths on the ladder (`results/w12-deaths.py`; rows in `results/w12-engage.json`)

There were 79 v2 deaths across four decoded ladder games: contest vs HGT `06490b46`, contest vs mcr `0a5e1356`, monolith vs Micron `4408dbb4`, monolith vs Howrah `76b15bb1` (0 deaths). Cassi and Issac die together, so each C&I death counts twice. Fight start is the first enemy-champion hit in an unbroken chain (gaps of 6 rounds or less) before the death. Radius 4 is Chebyshev.

| at fight start | contest HGT | contest mcr | monolith Micron | all |
|---|---|---|---|---|
| deaths | 58 | 11 | 10 | 79 |
| no ally within 4 | 42 | 9 | 9 | **60** |
| enemies >= 1 + allies (even or worse) | 42 | 7 | 8 | 57 |
| 1v1 | 20 | 3 | 8 | 31 |
| HP >= 50% | 55 | 11 | 8 | **74** |
| no gems carried | 58 | 11 | 9 | 78 |
| we hit first | 27 | 4 | **10** | 41 |

In v2a code, three things put champions in those lone fights:
- `squads.cpp:117` never targets a monolith group, so v2 has no gem source on contest.
- The solo-group `taken` rule gives each champion its own gold cluster, which spreads the team across the centre.
- `enemies.cpp:78` flees only from 2 or more enemies, so every 1v1 is fought.

## 2. Why even full-health 1v1s are lost (`results/w12-duels.py`; rows in `results/w12-duels.json`)

Totals over the 60 lone fatal fights, from the window from 2 rounds before fight start to the death:

| measure | our victim | enemy |
|---|---|---|
| damage dealt | 6,893 | 60,065 (8.7 times ours) |
| basic hits | 73 | 350 |
| casts | 37 | 45 |
| mean max HP | 1,146 | 1,145 |
| mean distinct item types | 2.7 | 3.1 |

- 33 of the 60 victims never cast. Tavi was among the hitters in 22 of the 60 fights.
- 11 non-C&I deaths, all against HGT, came in at most 2 rounds from a Tavi cast at full health.

**Contest vs HGT**
- The enemy Tavi's slot-1 spell (2.2 x Focus, range 4, one-round stun; GameRules, Tavi) hit for about 1,000 by round 250-500 and 2,400 by round 1,000.
- At r1076 a single cast from 4 tiles killed our full-health 2,400-HP Ryth.
- That Tavi bought in the field (kind-13 actions at r1070 and r1077). It held 32 Power Focus emblems (item 9) by r1490 and no Strength.
- Our Tavi's spell stayed at 220 all game. v2's `buy_items` gives Tavi Monster and Power Strength (items 0, 1, 6, 7), because the Focus list at `bot.cpp:167` is Cassi, Stellar and Prism only.

**Local game vs x-foundry-v16 (w12 smoke test)**
- Enemy casts dealt 95,139 damage against our 9,375.
- Their Tavi cast reached 2,794 and their Stellar 1,199; their Tavi had 750 max HP and 17 kills.
- Only 18 of our 47 deaths there were lone. Locally the gap is damage, not isolation.

**Monolith vs Micron.** We started all 10 fatal fights, mostly against a lone Ren. Ren's retaliation (0.2 x Focus per hit taken) cost only 340 over 19 hits; the fights were lost to Ren's own damage and stun.

**Answer: the enemy out-damages us with Focus casters, through items and ability use, mainly Tavi.** Our Tavi buys no Focus. An engagement rule cannot stop a one-shot from range 4.

## 3. What was built

- **E1** `Bot::engage_pre` (`bots/w12/src/engage.cpp`, called at `bot.cpp:378` after the Plan pre hook):
  - Fires when enemies within 6 are at least 1 + allies within 4.
  - Steps to the nearest visible ally or the spawn (`step_to_friends`, threat.cpp).
  - A melee champion strikes the weakest enemy in range first (GameRules, "A champion turn": melee may move and attack in one turn).
  - Skipped when retreating, on the spawn, when a carrier is in submit range, or to finish a lone enemy below 25% HP.
- **E2** `Bot::engage_target` (`bot.cpp:440`): while alone, groups within 6 tiles of the last enemy sighting are excluded for 25 rounds and the target is re-chosen.
- **E3** (`squads.cpp:127`, contest only): solo groups may be shared, so the team farms as one body.
- **Fix:** after a melee kill in E1, stale handles threw "engine error 97: Unit is not in vision". `keep_visible(enemies)` after the strike fixed it.
- **Errors:** across the w12all h2h games our side logged 52 bot-error lines (97, 98, 22) and v2's side logged 76. All of these types are inherited from v2.
- **Copies:**
  - `bots/w12`: E1+E2 on contest and monolith, E3 on contest.
  - `bots/w12all`: E1+E2 on all maps (`P_ENGAGE_ALL 1`), no E3.

## 4. Results

### 4a. w12 (contest and monolith only), panel, 3 salts, against `results/v2-panel.json`

| map | wins v2 -> w12 | deaths a game | gem margin | gold margin | median win round | dW (cmpcells) |
|---|---|---|---|---|---|---|
| contest | 10.0 -> 8.67 | 20.0 -> 16.8 (-16%) | 0 -> 0 | 7,119 -> 2,286 | 2000 -> 2000 | -1.33 |
| monolith | 11.0 -> 10.33 | 4.0 -> 4.3 (+9%) | 461 -> 334 | 2,413 -> 1,964 | 584 -> 665 | -0.67 |

Contest by opponent:
- x-v4-14 (v2's code): deaths 30.5 -> 28.7, kills 25.0 -> 13.8, gold margin +425 -> -4,422, wins 2 -> 0.
- x-foundry-v16: deaths 46.5 -> 41.5, wins 4 -> 2.

### 4b. w12 h2h against v2 (contest and monolith), 3 salts

| map | wins of 6 | gem margin | gold margin | our deaths | our kills |
|---|---|---|---|---|---|
| contest | **0** | 0 | -4,422 | 28.7 | 13.8 |
| monolith | 2 | -73 | -594 | 0.5 | 0.8 |

### 4c. w12all (E1+E2 on all 17 maps, no E3)

H2h against v2: **28/102**, gem margin -132 a game, gold margin -1,340.

| map | h2h wins of 6 | h2h our D / K | panel W v2 -> w12all | panel deaths v2 -> w12all | panel gem margin v2 -> w12all | median win round v2 -> w12all | top-50 yardstick |
|---|---|---|---|---|---|---|---|
| ambush | 0 | 1.2 / 0.0 | 10.7 -> 11.0 | 2.1 -> 1.8 | 851 -> 846 | 899 -> 907 | 594 |
| collision | 4 | 3.2 / 3.7 | 9.0 -> 5.0 | 4.1 -> 3.3 | 199 -> 23 | 761 -> 775 | |
| contest | 0 | 34.8 / 7.8 | 10.0 -> 3.3 | 20.0 -> 27.1 | 0 -> 0 | 2000 -> 2000 | 1602 |
| corners | 3 | 3.0 / 2.0 | 6.0 -> 2.0 | 2.9 -> 2.3 | 25 -> -335 | 466 -> 557 | |
| eight | 0 | 9.0 / 4.0 | 8.0 -> 3.0 | 9.7 -> 9.3 | 262 -> -264 | 734 -> 820 | 640 |
| elkring | 3 | 3.0 / 1.5 | 9.3 -> 5.3 | 4.4 -> 5.3 | 309 -> -61 | 590 -> 667 | |
| expanse | 2 | 0.0 / 0.0 | 11.0 -> 9.0 | 1.2 -> 0.8 | 261 -> 104 | 464 -> 507 | |
| fragment | 3 | 11.5 / 9.0 | 9.0 -> 7.0 | 15.3 -> 14.8 | 376 -> 5 | 852 -> 1005 | |
| interlock | 1 | 2.5 / 0.5 | 6.7 -> 2.0 | 3.4 -> 3.4 | -1 -> -364 | 576 -> 635 | |
| lanes | 2 | 1.2 / 0.2 | 7.3 -> 6.3 | 2.7 -> 1.8 | 180 -> 40 | 435 -> 488 | |
| monolith | 2 | 0.5 / 0.8 | 11.0 -> 10.3 | 4.0 -> 4.3 | 461 -> 334 | 584 -> 665 | 447 |
| packs | 0 | 5.5 / 1.5 | 10.0 -> 4.0 | 4.5 -> 6.0 | 291 -> -156 | 611 -> 716 | |
| pilgrimage | 0 | 17.7 / 8.7 | 10.0 -> 6.7 | 10.4 -> 10.0 | 273 -> -13 | 1278 -> 1384 | 844 |
| rows | 2 | 0.0 / 0.0 | 11.7 -> 11.3 | 0.6 -> 0.8 | 623 -> 577 | 481 -> 502 | 386 |
| rushbait-v2 | 2 | 2.5 / 2.3 | 4.3 -> 1.7 | 3.5 -> 2.2 | -142 -> -595 | 1006 -> 1216 | 660 |
| stampede | 2 | 20.5 / 6.3 | 9.7 -> 7.7 | 17.4 -> 20.4 | 610 -> 255 | 1752 -> 1795 | 686 |
| swirl | 2 | 39.3 / 19.0 | 8.0 -> 5.0 | 36.7 -> 47.6 | 305 -> -158 | 1249 -> 1553 | 600 |

Panel total dW -51.0 of 204; no map admitted. The median win round rose on 16 of 17 maps (ambush 899 -> 907), so the pace gap gets worse. Panel faults 0. `fuelExhausted` after round 2: lanes 146, expanse 96, corners 18. I have not checked these against v2's starter run, and no starter-cpp safety run was made, because the change is rejected.

### 4d. Traced moment where E1 fired (w12 vs v2, contest, side A, salt 0; decoded with `tools/v3/extract_pilot.mjs` + `results/w12-deaths.py`)

`tools/replay/final.mjs` is not in C:/cambc-solo, so this trace uses the v3 decoder.

1. r472: v2's Ryth (2,000 HP, melee) stuns and hits our lone Varg (1,200 HP) at (16,22).
2. E1 fires: it is 1v1 with no ally within 4. Varg strikes (85 damage) and steps toward the spawn every turn: (16,22) -> (15,21) -> ... -> (9,15).
3. Ryth follows and hits for 118 every round, because a melee champion may move and attack in the same turn.
4. Varg dies at r482: 10 rounds of flight, 510 damage dealt, 1,200 taken.

Two things decided the fight:
- Running from a melee chaser gains nothing. It only moves the death 8 tiles closer to home.
- The 2,000 vs 1,200 HP gap came from gold: E3's shared targets halved our contest gold (6,810 vs 14,375), so v2 out-itemised us. Deaths then gave v2 kill gold, and the gap grew.

## 5. Follow-up variants (built after the falsifier was met; not admitted unless the h2h says so)

**w12b** (`bots/w12b`) is E1 without running from contact: no disengage once an enemy is within 2 (`P_ENG_CONTACT 3`), and no melee pre-strike. E2 only marks groups hot when E1 actually backed off. No E3. On all maps.
- H2h against v2: **34/102**, gem margin -120 a game.
  - At least 4 of 6: monolith 4/6 (+22 gems), interlock 4/6 (+45).
  - Contest 2/6, deaths 29.8 against kills 22.2.
- Panel on those two maps, 3 salts:
  - monolith: wins 11.0 -> 10.33, deaths 4.0 -> 3.7, gem margin 461 -> 417, dW -0.67.
  - interlock: wins 6.67 -> 4.33, dW -2.33.
  - **Not admitted anywhere.**

**w12t** (`bots/w12t`, not an engagement rule) is v2a plus Tavi buying only Power Focus shards (`bot.cpp`, `buy_items`), with `P_FIELD_SHOP 1` so Tavi buys in the field. It answers §2.
- H2h against v2: **54/102**, gem margin -17 a game, gold margin -4.
  - monolith 6/6, +175 gems; lanes 5/6; contest, interlock and swirl 4/6.
  - collision, expanse, pilgrimage, rows and rushbait 2/6.
- It duplicates builder w8's `w8tf`. On the 9 maps both ran, the h2h numbers are identical.
- My 8 extra maps for w8:

| map | wins of 6 | gem margin | gold margin |
|---|---|---|---|
| collision | 2 | -203 | +32 |
| corners | 3 | +26 | -820 |
| expanse | 2 | -24 | -48 |
| fragment | 3 | 0 | 0 |
| interlock | 4 | +69 | -27 |
| packs | 3 | 0 | 0 |
| rushbait | 2 | -188 | -873 |
| swirl | 4 | +193 | +531 |

- Other builders' changes on v2a scored 50-53/102 against v2 (w11), so this baseline is about 50%. On that baseline w12t is neutral overall and helps on monolith.
- Bot errors: ours 94, v2's 89, all of the inherited types (97, 98, 22).
- I left the w12t panel to w8, which has `results/w8tf.json`.

## 6. Falsifier and verdict

- **Falsifier, fixed before testing:** contest and monolith deaths not down by a third against v2, or gem margin against v2 not up, means reject.
- **Outcome: met.**
  - Panel deaths: contest -16% (w12) and +35% (w12all); monolith +9%.
  - Gem margin against v2: monolith -73 a game in the h2h; contest 0.
  - H2h wins: 2 of 12 for w12 (contest and monolith), 28 of 102 for w12all.
- **Verdict: reject E1, E2 and E3 on all maps.**

What the evidence points to instead:
1. Our Tavi buys Strength while the best enemy Tavis stack Focus and one-shot from range 4. That is an items and draft question, w12t in §5.
2. On contest the tiebreak is gold. Any rule that cuts farming spread (E3) or backs off from clusters (E1, E2) loses it, even when deaths fall.

## Files

- Measurement: `results/w12-deaths.py`, `results/w12-duels.py`, `results/w12-engage.json` (ladder death rows), `results/w12-duels.json`.
- Tests:
  - `results/w12-smoke.json`, `results/w12-cm-s0.json`
  - `results/w12-cm.json`, `results/w12-cm-h2h.json` (w12)
  - `results/w12all.json`, `results/w12all-h2h.json` (w12all)
  - `results/w12b-h2h.json`, `results/w12b-mi.json` (w12b)
  - `results/w12t-h2h.json` (w12t)
- Comparison helper: `results/w12-cmp.py`.
- Bots: `bots/w12`, `bots/w12all`, `bots/w12b`, `bots/w12t`, plus the salted copies that mapgate creates.
