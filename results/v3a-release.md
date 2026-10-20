# v3a release check (integrator, 19 Oct evening)

**release candidate: yes, if the lead waives the bot-error clause of test b. That clause fails: 34 vs 24 lines in 34 starter games, all caught engine error 97 and all on the four changed maps. Every other gate passes: 156/156 identical games on the unchanged maps at salt 0; starter 34/34 with 0 faults and no turn over 100,000 fuel; panel +6.67 W (each changed map dW +1 to +2, dGem > 0, dStrong >= 0); head-to-head vs v2 23/24 on the changed maps.**

## INTEGRATION

v3a is v2's two-bot dispatcher with its A side rebuilt from v2a plus the three admitted changes. v2 sends pilgrimage to B (b-v418d, `bots/v2/src/route.h`). A build from v2a alone would differ from v2 on pilgrimage and fail the leakage rule, so v3a keeps the dispatcher.

| Path | Source | Why |
|---|---|---|
| `bots/v3a/srcA/` | v2a + changes below | Editable A tree (plain bot layout) |
| `bots/v3a/src/main.cpp`, `route.h`, `b/` | byte-identical to `bots/v2/src` | Same dispatcher, same B, pilgrimage to B |
| `bots/v3a/src/a/` | generated from `srcA` by `bots/v3a/gen_dispatch.py` | Same transform as `bots/v2/gen_dispatch.py`. Check: given v2a and b-v418d, it reproduces `bots/v2/src/a` and `b` byte for byte |

Files in `srcA` that differ from v2a (`diff --strip-trailing-cr` checked file by file):

| Change | Files | Scope |
|---|---|---|
| 1. w7 part 1 | `w7_drafts.h`, `counter.h`, `counter_table.h` (new; `W7_SET 3`, `W7_CD_ALL 0`); `plan.cpp` (`draft_for`, `counter_on`); `plan.h` (`MapPlan::counter`); `draft.cpp` (counter hook, inactive) | `draft_p1` returns a list only for eight (D) and stampede (R) |
| 2. w9 siege | `siege.cpp`/`.h` (new); `util.cpp` (`mono_tight`/`block`, active only while the siege is on); `map_contest.cpp` (siege on, `P_SIEGE_DRAFT 1`); `map_monolith.cpp` (`P_SIEGE_MONOLITH 0`) | contest |
| 3. w8 build C | `plan.h`: `MapPlan::tavi_focus_field`. `map_monolith.cpp`: sets it. `plan.cpp` `Plan::init`: sets `Bot::tavi_focus_field` and `remote_shop`. `bot.h`: two members and `buy_items_w8`. `bot.cpp`: `buy_items` calls `buy_items_w8` when the flag is set; `buy_items_w8` is w8tf's code with `P_TAVI_DMG_FOCUS`/`P_FIELD_SHOP` replaced by the flag; the field-buy call site checks the flag; `can_shop` (dead code, `P_SHOP_RECALL 0`). `params.h`: `P_FIELD_SHOP_HEALTH 0`, `P_BUILD_DMG 0`, `P_BUILD_MON_FIRST 0` | monolith. Without the flag, v2a's `buy_items` runs unchanged. Note: w8tf ran with `P_FIELD_SHOP_HEALTH` 1. The knob is unreachable for Tavi, who always takes the damage path |

`scripts/integrate.sh v3a w7 w9` stopped on line-ending rejects in `draft.cpp` and `plan.cpp`, so I applied the changes by hand. w7 and w9 touch disjoint files, so their files were copied whole and then diffed against v2a.

## Leakage check (salt 0, panel of six, both sides; base `results/v2-panel-1016.json`)

- On the 13 unchanged maps, **156 of 156 games are identical** to v2: same winner, final round, both teams' gems, kills and deaths.
- On the 4 changed maps, all 48 games differ, as expected.
- Salted runs (salts 1-2, from the panel in c): 295 of 312 unchanged-map games are identical. 17 differ: all 12 ambush games at salt 2 and 5 rushbait-v2 games at salt 1.
  - Same init-exhaustion counts as v2 in those cells; play starts after a round-2 spill. A few fuel units of added code (w9's `Siege::mono_tight`/`block` calls in util.cpp, the w8 flag checks) then shift fuel-dependent choices (`refresh_group` stops below 45,000 fuel, bot.cpp:202). This is fuel chaos (RESULTS 22:16), not a logic leak.
  - The ladder runs unsalted, so salt 0 is the relevant check, but divergence vs some opponents remains possible.
  - For bit-identical play on the unchanged maps, a three-way dispatcher would work: v2's own A tree for the 13 unchanged maps, the v3a A tree for the 4 changed maps, B for pilgrimage. Not built.

## a) Smoke: one game per side per changed map vs x-foundry-v16 (`results/v3a-smoke.json`)

All 8 games reproduce the builders' own runs exactly: eight matches w7-p1, stampede w7r, contest w9 safe6, monolith w8tf. 0 faults.

| Map | Result (A / B) |
|---|---|
| eight | L 330-1008 / L 22-1030 (same as w7) |
| stampede | W 1080-360 at round 1321 / W at round 1491 |
| contest | W 1000-0 at round 1597 / W 600-200 at round 2000 |
| monolith | W 1024-946 / W 1016-550 |

## b) Release safety: 34 games vs starter-cpp (`results/v3a-starter.json`)

| Check | v3a | v2 (old-map file plus the 1016 file for the six re-laid maps) |
|---|---|---|
| Wins | 34/34 | 34/34 |
| Crashes / faults | 0 | 0 |
| Late `fuelExhausted` (after round 2) | 0 | 0 |
| Normal play turns over 100,000 fuel (`.fuel` logs) | 0 | 0 |
| "bot error" lines | **34** | 24 |

- The extra lines are all on changed maps: contest +5, eight +2, stampede +2, monolith +1. The 13 unchanged maps are equal.
- They are engine error 97, "Unit is not in vision". The exception is caught; it comes from a handle to a unit our own spell had just killed (w7 Test 4: enemies.cpp:120 and `choose_altar`).
- Panel salt 0 shows the same pattern: 239 lines vs v2's 203, on eight, stampede and monolith (contest 32 vs 34).
- **This misses the brief's "no higher than v2" clause. It meets CLAUDE.md's upload rule (zero crashes, no turn over 100,000).** The existing fix (`P_ALLY_VISION_FIX`) is a behaviour change and was not admitted, so I did not apply it.

## c) Panel, 3 salts, all 17 maps (`results/v3a-panel.json` = salt 0 + salts 1-2; base `results/v2-panel-1016.json`)

| Map | v2 W/12 | v3a W/12 | v2 strong/4 | v3a strong/4 | v2 gem margin | v3a gem margin | v2 med. win round | v3a med. win round | top-50 rounds to 1,000 |
|---|---|---|---|---|---|---|---|---|---|
| ambush | 10.67 | 10.67 | 2.67 | 2.67 | +851 | +848 | 899 | 920 | 594 |
| collision | 5.67 | 5.67 | 1.00 | 1.00 | -34 | -34 | 810 | 810 | 695 |
| contest (changed) | 10.00 | 11.00 | 2.00 | 3.33 | +0 | +733 | 2000 | 1535 | 1602 |
| corners | 7.33 | 7.33 | 1.33 | 1.33 | +235 | +235 | 618 | 618 | 536 |
| eight (changed) | 8.00 | 10.00 | 1.00 | 2.00 | +262 | +486 | 734 | 666 | 640 |
| elkring | 9.33 | 9.33 | 2.33 | 2.33 | +309 | +309 | 590 | 590 | 577 |
| expanse | 10.33 | 10.33 | 2.33 | 2.33 | +413 | +413 | 789 | 789 | 513 |
| fragment | 9.00 | 9.00 | 2.00 | 2.00 | +376 | +376 | 852 | 852 | 772 |
| interlock | 9.00 | 9.00 | 1.67 | 1.67 | +309 | +309 | 794 | 794 | 663 |
| lanes | 10.33 | 10.33 | 2.33 | 2.33 | +273 | +273 | 569 | 569 | 468 |
| monolith (changed) | 11.00 | 12.00 | 3.00 | 4.00 | +461 | +483 | 584 | 570 | 447 |
| packs | 8.00 | 8.00 | 1.00 | 1.00 | +173 | +173 | 733 | 733 | 528 |
| pilgrimage | 10.00 | 10.00 | 3.00 | 3.00 | +273 | +273 | 1278 | 1278 | 844 |
| rows | 11.67 | 11.67 | 4.00 | 4.00 | +623 | +623 | 481 | 481 | 386 |
| rushbait-v2 | 4.33 | 5.00 | 0.67 | 1.00 | -142 | -97 | 1006 | 1043 | 660 |
| stampede (changed) | 9.67 | 11.67 | 2.33 | 3.67 | +610 | +840 | 1752 | 1371 | 686 |
| swirl | 8.00 | 8.00 | 1.33 | 1.33 | +305 | +305 | 1249 | 1249 | 600 |

Total W: v2 152.33, v3a 159.00 of 204. Faults v3a 0; fuel_late v3a 145 (v2 144).

cmpcells (`python scripts/cmpcells.py results/v2-panel-1016.json results/v3a-panel.json`):

```
salts: base 3, cand 3; admit when dW >= 1.5, dGem > 0, dStrong >= 0
map           base  cand    dW dStrong    dGem admit
ambush       10.67 10.67 +0.00   +0.00     -29 -
collision     5.67  5.67 +0.00   +0.00      +0 -
contest      10.00 11.00 +1.00   +1.33   +8800 -
corners       7.33  7.33 +0.00   +0.00      +0 -
eight         8.00 10.00 +2.00   +1.00   +2684 ADMIT
elkring       9.33  9.33 +0.00   +0.00      +0 -
expanse      10.33 10.33 +0.00   +0.00      +0 -
fragment      9.00  9.00 +0.00   +0.00      +0 -
interlock     9.00  9.00 +0.00   +0.00      +0 -
lanes        10.33 10.33 +0.00   +0.00      +0 -
monolith     11.00 12.00 +1.00   +1.00    +261 -
packs         8.00  8.00 +0.00   +0.00      +0 -
pilgrimage   10.00 10.00 +0.00   +0.00      +0 -
rows         11.67 11.67 +0.00   +0.00      +0 -
rushbait-v2   4.33  5.00 +0.67   +0.33    +539 -
stampede      9.67 11.67 +2.00   +1.33   +2760 ADMIT
swirl         8.00  8.00 +0.00   +0.00      +0 -
total dW +6.67; admitted: eight,stampede
```

- Only eight and stampede clear dW >= 1.5. Contest (+1.00) and monolith (+1.00) were admitted by the lead on their own evidence (contest is capped by v2's 10/12, monolith by 11/12).
- rushbait-v2 (+0.67) and ambush (dGem -29) are not code leaks; see the salted note under the leakage check.

## d) Head-to-head vs v2, 3 salts (`results/v3a-h2h.json`)

| Map | Wins/6 vs v2 | Gem margin a game | Median win round |
|---|---|---|---|
| ambush | 3/6 | +42 | 832 |
| collision | 3/6 | -47 | 846 |
| contest (changed) | 6/6 | +667 | 1932 |
| corners | 3/6 | -31 | 595 |
| eight (changed) | 6/6 | +336 | 666 |
| elkring | 3/6 | +0 | 590 |
| expanse | 3/6 | -61 | 759 |
| fragment | 3/6 | +0 | 926 |
| interlock | 3/6 | -6 | 795 |
| lanes | 3/6 | -75 | 558 |
| monolith (changed) | 6/6 | +175 | 515 |
| packs | 3/6 | +0 | 697 |
| pilgrimage | 4/6 | +106 | 1332 |
| rows | 2/6 | -11 | 460 |
| rushbait-v2 | 2/6 | -188 | 1044 |
| stampede (changed) | 5/6 | +480 | 1371 |
| swirl | 4/6 | +193 | 1328 |

Changed maps: 23/24. All maps: 62/102.

The unchanged maps are v2 against v2 (salted copies), so 2-4 of 6 there is noise.

## e) Pace

See the median-win-round columns in (c), next to the top-50 median rounds to 1,000 (`intel/meta-2026-10-18.md` §3a).

## Lanes late-fuel finding (lead, 23:10)

- **Cause: the init cost on the current lanes layout, not a per-turn cost.**
- Measured on an instrumented copy: lanes_v2 has 42 one-tile camp groups. Bot::init uses about 340k fuel there:
  - group loop about 200k: `camp_groups()` again 25k, `spawn_tiles()` and `positions()` 68k, ring 53k, move into `groups` 16k
  - reachable-altar distance field 28k; altar goals 16k
- The first play turn (200k budget) and the second (100k) both run out, and play starts in round 3. That gives `fuel_init` 12 a game; salt 0 has `fuel_late` 0.
- The salted copies add 20k-40k on turn 1, which pushes the spill into round 3. That accounts for all of v2's 144 (salts 1 and 2: 72 each, salt 0: 0) and v3a's 24 in the head-to-head.
- On the ladder (no salt) the cost is a two-round late start on lanes.
- **Not fixed.** The cheap savings found (reuse mapid's `camp_groups`, a cheaper ring) cut about 40-50k, which is at most one round. Any init change also alters fuel and so changes trajectories ("fuel-chaotic", RESULTS 22:16). It would make lanes a fifth changed map needing its own test.
- Lanes `fuel_late`: v2 144 and v3a 144 (salt 0: 0; salts 1 and 2: 72 each). The two are identical because lanes code is unchanged.

## Other notes

- One late `fuelExhausted` on stampede (vs x-v4-14 = vs v2, side A, salt 0, game ends at round 1476). This is the case w7 reported. The turn is suspended and resumes; no fault.
- Tools (scratch, not committed): per-cell identity compare, `.fuel` and stdout safety scan, salt merge.
