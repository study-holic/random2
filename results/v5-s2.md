# v5 S2 quick wins: draft core, altar control, gold spending (20 Oct 2026, cloud)

Brief: tools/cloud/v5-s2-quickwins.md. Copy: bots/s2 (from bots/v4; edits are in `src/a`, `srcA` is v4's unchanged plain copy). Engine 1.0.0, CLI 3.0.1, 21-map pool, `--jobs 3`. Every head-to-head (h2h) is 21 maps x 2 sides x salts 0-2 = 126 games against bots/v4. Metrics come from `tools/cloud/s2metrics.mjs` (replay decoder; field names as in v5-common) and `tools/cloud/s2summ.py`.

## Admission (fixed before testing)
| line | rule | result | verdict |
|---|---|---|---|
| 1 | gate.py PASS | 42/42 vs starter-cpp, 0 faults, mapid correct, max turn fuel 97,728 (`results/gate-s2-20261020-112935Z-linux.json`) | PASS |
| 2 | h2h vs v4 >= 45 % | 82/126 = 65.1 % (`results/s2-vs-v4.json`) | PASS |
| 3a | locks placed per game > 0 | 5.3 (v4 0.0) | PASS |
| 3b | gear at round 200 >= 3.0 | 2.32 shard-equivalents per champion (v4 2.32) | **FAIL** |
| 4 | deaths per game not up > 10 % vs v4 | 17.4 vs v4's 23.5 in the same games (-26 %) | PASS |

**The candidate fails line 3b.** Gear did not move: the gap to the top 10 (3.8) is in gold income, not in unspent gold (below). Lines 1, 2, 3a and 4 pass. Gem margin is +84 a game. The upload decision is the lead's.

## Runs (126 games each, vs v4)
| build | wins | deaths us/v4 | locks | gear at r200 us/v4 | unspent gold r100 / r200 us (v4) | gem margin |
|---|---|---|---|---|---|---|
| s2d: draft core only, play code unchanged | 54 (42.9 %) | 21.1 / 21.3 | 0 | 2.14 / 2.42 | 27 / 36 (28 / 35) | -21 |
| s2 core on all maps (allcore) | 77 (61.1 %) | 17.5 / 23.4 | 5.2 | 2.25 / 2.33 | 23 / 24 (29 / 36) | +81 |
| s2 + v4 draft on 6 maps (keep6) | 80 (63.5 %) | 17.3 / 23.9 | 5.4 | 2.32 / 2.33 | 25 / 26 (29 / 34) | +99 |
| **s2 candidate: v4 draft on boss_battle, gold_rush** | **82 (65.1 %)** | **17.4 / 23.5** | **5.3** | **2.32 / 2.32** | 24 / 25 (29 / 36) | **+84** |

## Ablations (one switch off from the candidate)
| switch off | wins | delta | deaths us/v4 | locks | gear r200 | gem margin |
|---|---|---|---|---|---|---|
| none (candidate) | 82 | – | 17.4 / 23.5 | 5.3 | 2.32 | +84 |
| P_S2_ALTAR_LOCK | 68 | -14 | 16.7 / 25.1 | 0 | 2.54 | +42 |
| P_S2_TRIBUTE_GATE | 78 | -4 | 17.7 / 22.9 | 5.2 | 2.31 | +83 |
| P_S2_SPEND_NOW | 61 | -21 | 21.2 / 21.6 | 5.0 | 2.00 | +27 |
| P_S2_DRAFT_KEEPV4 (= allcore) | 77 | -5 | 17.5 / 23.4 | 5.2 | 2.25 | +81 |
| P_S2_DRAFT_CORE, all else on | not run; draft-only build above shows core alone is -4 vs v4's draft | | | | | |

Games are deterministic per salt: allcore and the candidate agree map for map on the 19 maps whose draft is the same. Each switch is worth wins. The gate's -4 is within noise for 126 games (about +/-11 at 1 s.d.). Locks cost 0.2 gear points (10 gold each) and still add 14 wins. Spending (role emblem + shop trips) cuts deaths from 21.2 to 17.4 a game.

## Switches and file:line (bots/s2/src/a)
- params.h:9-24: P_S2_DRAFT_CORE, P_S2_DRAFT_KEEPV4, P_S2_ALTAR_LOCK, P_S2_TRIBUTE_GATE, P_S2_SPEND_NOW (all 1).
- **P_S2_DRAFT_CORE**, draft.cpp:55-63: a fixed table, 5 or 4 picks Ryth, C&I, Stellar, Ren, Varg (in order); 3 picks C&I, Stellar, Ren. It overrides map-file and w7 lists. No denial order is needed: picks are simultaneous and duplicates across teams are allowed (FACTS.md).
- **P_S2_DRAFT_KEEPV4**, draft.cpp:26-37: v4's draft is kept on boss_battle and gold_rush (see Draft).
- **P_S2_ALTAR_LOCK**: buy at bot.cpp:263-266; place or chase at s2.cpp:51-101, called at bot.cpp:576. Enemy-lock detection and clearing were already in v4 (bot.cpp:432-461: locked altars remembered for 40 rounds, +3 in altar scoring, lock attacked when in submit range). That covers brief item (a), so no change was made there.
- **P_S2_TRIBUTE_GATE**: s2.cpp:104-139, called in bank() at bot.cpp:478.
- **P_S2_SPEND_NOW**: role emblem at bot.cpp:273-279; emblem-progress bookkeeping at bot.cpp:306-311; shop trip at s2.cpp:141-176, called at bot.cpp:595.
- bot.h:124-139 holds the state and declarations; s2.cpp is a new file with the same dispatcher wrapper as bot.cpp.

## Mechanisms as models
**Draft.** Objective: maximise win probability. A fixed lookup is sufficient because no pick can deny the opponent (game theory: with simultaneous moves and no shared pool, the best response does not depend on the opponent's picks). Per-map exception rule, fixed before testing: a variant must beat the core by >= 2 of 6 on that map, and the overall h2h must not drop.

**Altar lock (value of denial).** The lock is placed on argmax_i V_i over unlocked altars within 2 tiles (GameRules l.193-199), when V > 0. V_i = 3 if an enemy is channelling a tribute at i (the channel is cancelled, then 3 attacks are needed to clear the lock); 2 if an enemy tributed at i in the last 150 rounds; 1 if i is the altar nearest the enemy spawn. When an enemy is channelling at an altar 3 to 2+k tiles away (k = ceil(gems/10) remaining channel turns) and our HP >= 50 %, the champion steps into the 2-ring. One lock is carried at all times and bought first at the shop, which keeps 10 gold for it. Sources: OR marginal-value task allocation; MOBA vision and objective denial.

**Tribute gate (expected loss).** With k = max(1, ceil(gems/10)), do not start a tribute if any of these holds:
- stun threat: k >= 2 and an enemy with a ready CC ability is within 5 (cast range 4 plus one step);
- lock threat: k >= 2, enemy locks have been seen this game, and an enemy is within 3 of the altar;
- kill threat: no ally is adjacent and sum_e [str_e * max(0, k+1 - max(0, d_e - r_e)) + 2 focus_e if a damage ability is ready within 5] >= our HP.

Damage alone never aborts a tribute (GameRules l.181/229). Source: decision analysis, from the cost of a cancelled channel vs a lost bag.

**Spending (back timing).** A shop trip costs 5 recall turns plus the walk back, and buys floor(gold/30) shards. Trip iff all of:
- gold >= 30 * max(2, shards left to the current emblem), or 5 shards;
- no gems carried;
- no enemy within 8 and no incoming damage;
- not retreating or holding a capture;
- 40+ rounds since the last trip, and round < 1850.

A spawn closer than 8 tiles is walked to instead of recalled to. Recall is interrupted by damage; walking is not. The first emblem is Durability-Health for the melee front line (Varg, Ryth, Ren, Issac, Vendetta, Marcella, Heart of Ruin), for -10 % damage taken. Monster stays first for the ranged farmers. Source: MOBA "back when the item completes".

## Draft: support check and the forced-draft comparison
- **v4 code support for the core:** abilities are cast generically. Damage, AoE and CC are used on camps and on enemies (util.cpp:102-113, enemies.cpp:75; CC is saved when an enemy champion is within 8). Dashes are used in fights for Ren and Issac (enemies.cpp:130-147, M3_DASH), and Varg's shield is used (util.cpp:116). Retreat and buying are type-agnostic; only the stat list in buy_items depends on type (Focus for Cassi, Stellar, Prism).
- **Gap:** Ryth's AoE, Ren's stun and Issac's kit scale with Focus, but these champions buy Strength (v4), or Durability-Health first (S2).
- **No Tavi dependency:** buying happens on spawn tiles or while dead; field shopping only applies with Tavi's passive. Monolith's tavi_focus_field is inert without Tavi.
- **Forced draft, play code unchanged** (s2d, 54/126): the core loses 0/6 on boss_battle, corners and interlock, and 1/6 on elk_arena, gold_rush and islands. It wins 6/6 on corridors and monolith, and 5/6 on collision and elkring. Deaths are equal (21.1 vs 21.3).
- **Re-test with locks, gate and spending on** (allcore vs keep6, wins of 6 with v4's draft vs core):

  | map | v4 draft | core | decision |
  |---|---|---|---|
  | boss_battle | 3 | 0 | keep v4 draft |
  | gold_rush | 3 | 1 | keep v4 draft |
  | interlock | 4 | 3 | core (below the +2 bar) |
  | elk_arena | 6 | 5 | core (below the +2 bar) |
  | corners | 6 | 6 | core (below the +2 bar) |
  | islands | 2 | 6 | core |

  The overall h2h rose 77 -> 82. Caveat: choosing exceptions from these games and scoring them on the same games is optimistic. The live panel decides.

## Gold: v4's unspent gold (brief task 3)
- **Unspent gold per champion, v4 side:** 5 to 13 at rounds 100 and 200 in the first test games, and 28-29 at round 100 and 34-36 at round 200 over 504 h2h games. v4 already buys with all its gold on every spawn visit (bot.cpp buy_items), so it does not hoard.
- **Cause of low gear (traced, eight A, rounds 20-220):** only 2 of 6 champions earn any gold before round 220 (capture gold goes to the capturer). Stellar held 200 gold from round 80 to 150 while carrying a bag. The shop trip fixes the holding; it cannot fix the income.
- **Ambush (0/6 for the candidate):** our team earns no gold before round 160, and gear at round 200 is 0.7 vs v4's 1.7-2.2. The draft-only build shows the same (0.8), so the cause is the core draft's early capture income (S1 economy). Losses are close (-66 to -349 gems) with fewer deaths than v4.

## What still fails
- Gear at round 200 is 2.32 (target 3.0): the gap is income, not unspent gold. S1's controller, or gold-cluster routing, is the lever.
- Weak maps: ambush 0/6 (-154 gems a game), lanes 1/6, broken_curve 2/6, fracture 2/6.
- Max turn fuel is 97.7k in the gate, above the 80k guideline. The source was not traced; v4's own peak was 90.7k, outside farm().
- Self-play only: v4-cloud's 68 % locally became 23 % against real teams. The live top-10 panel (A-B-A vs v4) decides the release.
- Not done: /watch of a loss (no browser in the cloud); the Tavi-swap draft variant (the code was written but never run, and has been removed).
