# /evaluate: release candidate v2 (critic, 23:00 to 23:20 BST, 18 Oct)

Inputs: PLAN.md v1 (release tests R1 to R5, changed-cell rule, build cards), RESULTS.md, VERSIONS.md, `results/w1-drafts.md`, `w2-deaths.md`, `w3-dispatch.md`, `notes/watch-v1-swirl-2245.md`, `results/v2-panel.{txt,json}`, `v2-starter.{txt,json}`, `v2-heldout.*` (see section 2, R4), `results/v1-base.json`, `v1-heldout.json`, `results/fuelchk/`, replay bundles in `matches/v2-230053` (panel) and `matches/v2-230855` (starter). I ran no games; every figure below is recomputed from the JSON files.

Candidate: v2 = dispatcher (`bots/v2/src/main.cpp`, `route.h`) over A = `bots/v2a/src` and B = `bots/b-v418d/src`. `diff -rq bots/v1/src bots/v2a/src` lists two files, `map_corners.cpp` (identical to w1's) and `map_swirl.cpp` (identical to w2's). `route.h` sends `pilgrimage` and `collision_old` to B and everything else, `collision_v2` included, to A. The build matches the description.

## 1. Fact checks

| # | Claim | Check | Result |
|---|---|---|---|
| 1 | Camp kills destroy the carried bag; C&I die together (w2's mechanism) | GameRules lines 361, 365, 367 and 428 | **True.** "Camp kills destroy carried gems"; "If either Cassi or Issac dies, the other dies too" |
| 2 | Fuel exhaustion suspends the bot, which resumes next round (R2's reading) | GameRules §Fuel, line 387 | **True.** "Its execution is suspended … The next round, this bot's turn will resume from the point of suspension." Line 389: 2x fuel on the first play turn |
| 3 | v1 has a late `fuelExhausted` at round 20 (Issac, corners vs x-foundry-v16, v1 as B) | `results/fuelchk/v1.json` events | **True.** teamB Issac at rounds 1, 2 and **20**. w1's copy has it at round 21 |
| 4 | w2: swirl panel deaths 52.2 to 36.7 (29.7 %); salt 1 replays salt 0 | `v1-base.txt`, `w2-deaths.txt`, cell comparison | **True.** (52.2 − 36.7) / 52.2 = 29.7 %. Salt 0 equals salt 1 in all 12 cells, base and candidate alike |
| 5 | w1: all 3 corners salts identical | Cell comparison of `w1-drafts.json` | **True** for w1 (12 of 12 cells equal across all salts). In **v2** the corners salts are no longer all identical (salt 0 differs; salts 1 and 2 match), so v2 gives 2 effective samples: 6 and 6 wins against the base's 3 and 4 |
| 6 | "Ladder top-10 data on current corners supports Stellar/Ren" | `notes/top10-by-map.md` line 22; `intel/meta-2026-10-18.md` §4 (corners `e3c29b68`, 1,032 side-games) | **Half true.** Top-10 pick share: Stellar 100 %, Ren 78 %. The **rating-adjusted excess points the other way**: Vendetta +3.3 pp and Tavi +1.6 pp (the two picks the change drops), Stellar +1.4 pp and Ren +1.0 pp (the two it adds). All four are inside noise. Top-10 modal draft C&I+Ren+Ryth+Stellar+Varg: n = 14, 57 %. The ladder neither supports nor contradicts the swap |
| 7 | w3: "B on lanes ran out of fuel 146 times over 36 games (A: 0)" | `w3-dispatch.json` late fuel by salt | **Misattributed.** Every event is in salts 1 and 2 (12 games each); salt 0 has none. v2's A path on lanes shows the same pattern (144 events, salts 1 and 2 only). `v1-base.json` has no `fuel_late` field, so "A: 0" was never measured. This is the dispatcher's salt burn, not B (section 2, R2). Lanes was not admitted anyway (dW +0.67), so no route changes |

## 2. Release tests against PLAN.md's exact wording

| Test | Wording | v2 result | Pass? |
|---|---|---|---|
| R1 Safety | 34 wins, 0 bot faults, 0 engine diagnostic codes, no "bot error" in stdout | 34/34 wins, 0 faults, 0 codes, 0 late fuel. **27 caught `bot error:` lines in our stdout** (22 of engine error 97 "unit not in vision", 4 of 98, 1 of 22). v1's own starter run (`matches/v1-221121`) has 21 of the same kinds | **Fails as written; passes the intent.** The base fails the same clause, so it cannot be the fallback under the literal wording either. These are caught exceptions that end one unit's turn, not crashes. Across the panel, v2 logs the same three kinds at the base's rate (541 lines vs 586 over comparable games; the B map collision has 75 vs 71) |
| R2 Fuel | No `fuelExhausted` after round 2 in R1 and R3 games | R1: 0. R3: **241 events**: expanse 96 and lanes 144 (**salts 1 and 2 only**, every champion at **round 3**), corners 1 (**salt 0**, Issac at round 22 vs x-v4-14 as B, a win) | **Fails as written.** 240 of the 241 events are a test artefact. The dispatcher burns `P_SALT × 4,000` iterations on the first play turn (`main.cpp` line 40), and on the two widest maps this pushes initialisation past round 2. The shipped build has `P_SALT = 0`, and its salt-0 games on all 13 unchanged maps are **identical to the base's salt 0, cell for cell**. The one real event is the same kind as v1's round-20 Issac event on corners (one suspended turn, resumed next round). v1 fails R2 as written too |
| R3 Panel | Mean ≥ base's; no base-passing map fails; every changed map admitted | **151.67 vs 142.69** (+9.0). All 9 base-passing maps still pass, and pilgrimage now passes too (10/17). cmpcells admits collision (+2.33), corners (+2.33, dStrong +0.33), pilgrimage (+3.33, dStrong +2.00) and swirl (+1.67). Unchanged maps: 0.00 on 11 maps, expanse +0.33, lanes −0.33, rushbait-v2 −0.67, all from salts 1 and 2 (salt 0 identical to the base) | **Pass** |
| R4 Held-out | Total ≥ base − 2 (139 − 2 = 137 of 170); no changed map loses more held-out games than the base (base: corners 8/10, swirl 8/10, pilgrimage 7/10, collision 9/10) | **142/170** (+3). corners **10** (+2, dGem +3,804), pilgrimage **9** (+2), swirl **8** (0; two raider2 flips cancel), **collision 8 vs 9** (one flip: hunter, v2 as B, lost at round 895). The 13 unchanged maps match the base cell for cell. 0 faults, 0 codes; 1 late fuel event (corners vs v5-base, as B) | **Fails as written, on collision alone.** Total and three of the four changed maps pass |
| R5 Review | Critic "proceed"; one v2 loss watched | This note; no v2 loss watched yet | Open |

**Per-map regressions** (passed 7/12 + 2/4 for the base, fails for v2): **none.** The fragile passes are contest, fragment and packs, each at exactly 2.0/4 strong; all three are identical to the base.

**Faults (`flt`):** 0 in all 646 R1 and R3 games.

**R4 note:** see section 7.

## 3. Lenses

**Causation.**
- **Swirl.** The causal chain is the best-documented of the four. Replays show deaths at (12,3) and its mirror (32,26), about 100 rounds apart; the exclusion removes that cluster; deaths against starter-cpp fall from 43.5 to 13.0 a game (−70 %, `v2-starter.txt`). The link to the panel wins rests on correlation. Panel deaths are mostly champion kills in the central chamber (watch note: 32 of 39), which the change does not touch, so the +1.67 wins come from fewer camp deaths and bag losses, which is plausible but measured on 2 effective samples (8, 8 against 7, 5).
- **Corners.** The chain runs draft, then lineup, then gem race. The rule link holds: generic casting uses Stellar's and Ren's stuns (w1's kit check). The evidence link is correlational: top-10 pick share. The local gain is on the **old** layout, while the draft also fires on `corners_v2`, which was never tested. The loss to foundry-v16 got worse (188 vs 1,032 gems).
- **Pilgrimage → B.** There is no mechanism, only selection of a black box. The effect is the largest and most consistent of the four: 10, 11 and 9 wins against 6, 5 and 9 over 3 independent salts, with gains against both strong bots (v4-14 0.67 → 2.00, foundry 0.33 → 1.00).
- **collision_old → B.** No causal path to the tournament exists. `mapid` sends a gemless 41×41 map to `collision_v2`, and that id stays on A.

**Limitations.**
- **Selection.** About 10 map-variants were tried across the three builders (w1 4, w2 1 plus a no-op, w3 5), and 4 were admitted. With the 7 % no-op flip rate, a null change clears dW ≥ 1.5 on some map in that many tries, so one or two of the four may be noise. Pilgrimage (+3.33 at 3 independent salts) is the least likely to be noise; swirl (+1.67, 2 samples) and corners (one w1 sample, 2 in v2) are the most likely.
- **Layouts.** Two of the four changes were measured only on old layouts: corners (the draft ships to `corners_v2`) and collision (dead on the ladder). Of the +9.0 R3 gain, **+2.33 (collision) cannot transfer**, and +2.33 (corners) transfers only if the draft effect survives the re-layout. The gain that transfers on current layouts is pilgrimage +3.33 and swirl +1.67, about **+5.0**.
- **Panel strength.** foundry-v16 is still 0/2 on corners, swirl and collision, and the panel is our own ancestor plus 15 Oct clones. Single-elimination opponents seeded from a 1,500 start are likely stronger.
- **Determinism.** 12 deterministic games a map, and salts collapse (s0 = s1 on swirl, s1 = s2 on corners and collision).

**Falsification.**
- **w1 (corners)** survives its pre-set falsifier (admitted, dW +2.33, now also dStrong +0.33 in v2).
- **w3** survives: pilgrimage and collision were admitted; eight, lanes and interlock were routed back to A, as the falsifier required.
- **w2 (swirl)** **triggered** its pre-set falsifier on deaths (29.7 % < 33.3 %). The lead's override has a defensible reason: the falsifier targeted the camp-death loop, and that loop is gone against starter-cpp (−70 %). Overriding a pre-registered falsifier after the result is still the move pre-registration exists to stop. The honest resolution is to let **R4's swirl cell** decide: keep the change only if held-out swirl is at least the base's 8/10.
- **What would drop v2:** a held-out total below 137, or any changed map below its base held-out count, or a v2 loss in the visualiser showing a new failure mode (for example B stalling on pilgrimage).

## 4. Missing (what a strong team would have)
1. **R1 and R2 are unpassable as written, by the base as well.** Write the reading down before the upload decision, not after: R1 = 0 faults, 0 codes, no `bot error:` kinds beyond engine errors 97, 98 and 22, at a rate no higher than the base's. R2 = no `fuelExhausted` after round 2 in **salt-0** games, except isolated single-champion events of the kind the base already has. Xans has to confirm, as PLAN.md already asks.
2. **The salt burn breaks R2 on wide maps.** Any future R2 count must exclude salts above 0, or move the burn to round 3 or later.
3. **No test on the ladder layouts** of corners or collision (`cambc maps pull` needs a sign-in). The corners draft is the one change with no evidence on the layout it will play.
4. **No fingerprint check that ladder collision has `gem == 0`.** If ladder collision carries any gem cluster, `mapid` calls it `collision_old` and routes it to B, untested. B has a clean 34/34 starter record, so this risks quality, not safety.
5. **No v2 loss watched yet** (R5). The best candidate is a pilgrimage loss on the B path, the largest behavioural change.

## 5. Verdict per change

| Change | Verdict | Reason |
|---|---|---|
| Pilgrimage → B | **Proceed** | Panel +3.33 at 3 independent salts, held-out +2 (9 vs 7), gains against both strong bots, current layout, 0 faults, 0 late fuel |
| Swirl exclusion | **Proceed** | Mechanism verified (starter-cpp deaths −70 %). Panel +1.67; held-out level (8 vs 8), which meets R4. The pre-set death falsifier tripped narrowly (29.7 %); the R4 tie-break holds, but log the gain as "+1.67 panel, 0 held-out" |
| Corners draft | **Proceed** | Held-out 10 vs 8 (dGem +3,804) independently backs the panel +2.33. The caveat stands: both measurements are on the **old** layout, and the ladder excess slightly disfavours the swap |
| collision_old → B | **Revert to A before upload (named change)** | Fails R4 as written (8 vs 9 held-out). Its +2.33 panel gain cannot reach the tournament (ladder collision is `collision_v2` and stays on A). Keeping it buys nothing and keeps a misidentification risk (a ladder collision with any gem cluster would route to B untested). The fix is one line in `route.h`: drop `case M::collision_old:`. Then rebuild, rerun R1 (25 s), and confirm that collision at salt 0 matches `v1-base` salt 0 cell for cell (12 games), which proves the A path is v1 there |

## 6. Overall

**Upload v2 with collision_old routed to A (call it v2 with that one-line change, or v3 per VERSIONS.md), not v1, and not nothing.** With that revert, v2 passes R3 (151.67 − 2.33 ≈ 149.3 expected, still above 142.69) and R4 (142 − (8 − 9) = 143 ≥ 137; every changed map at or above the base), and it passes the intent of R1 and R2.

Conditions:
1. Xans confirms the R1 and R2 readings in section 4 (both are unpassable as written, by the base as well).
2. The rebuild, R1 rerun and collision identity check finish by **23:35**. If they do not, upload **v2 as built** with the R4 collision deviation logged (a one-game held-out difference on a route that never fires on the ladder layout). Do not upload v1 over v2 for that reason alone: v2 beats v1 on the three transferable maps in both panel and held-out.
3. One v2 loss watched (R5), preferably a pilgrimage loss on the B path.

Uploading nothing is never better: the tournament needs a submission, and both bots run 34/34 with 0 faults.

Expected tournament gain over v1 on current layouts: about +5 of 204 panel wins (pilgrimage +3.33, swirl +1.67) and +2 of 170 held-out, plus the corners effect if it survives the re-layout (+2.33 panel and +2 held-out on the old layout).

## 7. R4 detail (from `results/v2-heldout-cmp.txt`, 1 salt)

| Changed map | Base | v2 | Flips | Verdict on R4 |
|---|---|---|---|---|
| corners | 8 | 10 | +hunter A, +v5-base A | pass |
| pilgrimage | 7 | 9 | +hunter B, +raider2 B, +v5-base B, −raider2 A | pass |
| swirl | 8 | 8 | +raider2 B, −raider2 A | pass (equal) |
| collision | 9 | 8 | −hunter B | **fail** |

Unchanged maps: 13 of 13 identical to the base. Total 142 against the threshold of 137. It is one salt, so each changed-map figure is one deterministic sample.
