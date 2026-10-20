# v5 S2: quick wins (draft core, altar control, gold spending), 20 Oct 2026

Brief: `tools/cloud/v5-s2-quickwins.md` (with `tools/cloud/v5-common.md`). Copy: `bots/s2` = `bots/v4` + the switches below. Engine 1.0.0, CLI 3.0.1, 21 maps, `--jobs 3`. Every head-to-head is 21 maps × 2 sides × salts 0, 1, 2 = 126 games against `bots/v4`. Metrics come from `tools/cloud/metrics_s2.py`, which reads the run-batch statistics JSON: lock buys, our stdout "LK" placement lines, shards per body by round 200 (Power emblem = 6, other emblems = 5, as in `tools/v3/a2_fights.py`), and unspent gold per body from the gold timeline.

**Caveat on every number below:** these are self-play results against v4. Self-play misled before (68 % locally became 23 % against real teams). Variants with the same intent ranged 71 to 78 of 126, and per-map counts swing by 3 or more of 6 between near-identical builds. The live top-10 panel decides.

## Admission (fixed before testing)
| # | Line | Result | Verdict |
|---|---|---|---|
| 1 | `gate.py s2` PASS | 42/42 vs starter-cpp, 0 faults, mapid correct, max turn fuel 93,816, no fuelExhausted after round 1 (`results/gate-s2-20261020-130334Z-linux.json`) | **PASS** |
| 2 | Head-to-head vs v4 ≥ 45 % | **70/126 = 55.6 %**, gem margin +132 a game (`results/s2-vs-v4.json`). The pre-fuel-fix build of the same configuration scored 78/126, +146 (`results/s2-pre-fuelfix-vs-v4.json`) | **PASS** |
| 3a | Locks placed per game > 0 | 3.98 placed (9.1 bought) | **PASS** |
| 3b | Gear at round 200 ≥ 3.0 per champion | **2.71** (v4 2.25 in the same games; the top 10 reach 3.8) | **FAIL** (see "Gear" below) |
| 4 | Deaths per game vs v4 not up > 10 % | 17.0 vs v4's 22.9 in the same games (−26 %) | **PASS** |

Result: three of four lines pass; gear at round 200 fails (2.71 against 3.0). The candidate goes to the lead for the live panel with that line marked failed.

## Switches (all in `bots/s2/src/a/s2.h`; the candidate's defaults are shown)
| Switch | Default | Where | What |
|---|---|---|---|
| P_S2_DRAFT_CORE | 1 | plan.cpp:80, s2.h `core_draft` | top-10 core on every map |
| P_S2_CORE_VARG | 1 | s2.h `core_draft` | pick order C&I, Stellar, Varg, Ryth, Ren: 3 picks = C&I+Stellar+Varg, 4 = +Ryth, 5 = full core |
| P_S2_DRAFT_EXC | 0 | s2.h `keep_v4_draft`, plan.cpp:80 | per-map v4-draft exceptions (none admitted) |
| P_S2_ALTAR_LOCK | 1 | s2.cpp:50 (buy), s2.cpp:62 (place); bot.cpp:264, bot.cpp:576 | buy a lock on each shop visit; place it |
| P_S2_LOCK_FROM | 200 | s2.cpp:50 | no lock bought before round 200 (gold goes to shards first) |
| P_S2_TRIBUTE_GATE | 1 | s2.cpp:121, bot.cpp:477 | tribute stopping rule |
| P_S2_SPEND_NOW | 1 | bot.cpp:594 (recall after a tribute), s2.cpp:156 + bot.cpp:658 (detour) | shop after tributes and on short detours |
| P_S2_SHOP_GOLD / P_S2_SHOP_DIST | 90 / 12 | as above | gold that triggers a shop visit; maximum detour distance |
| P_S2_BANK_VIA | 0 | bot.cpp:426-435 | route a banking trip through the spawn (a no-op in test, left off) |
| P_S2_FUEL_FLOOR | 45,000 | s2.cpp | skip the lock and gate checks below this much fuel |
| P_S2_FUEL_STOP | 35,000 | bot.cpp:657 | end the turn before targeting, banking and farming below this much fuel |
| P_S2_FAST_CONTEST | 1 | stuck.cpp:35 | `update_contested` in one pass, same decisions |
| P_S2_LOCK_LOG | 1 | s2.cpp | prints "LK r<round> chan/pre" ('\n' only) for the metrics |
Files touched: `s2.h`, `s2.cpp` (new); `bot.h:129-135` (members); `bot.cpp:264, 426-435, 477, 576, 594-602, 657-658`; `plan.cpp:19, 80`; `stuck.cpp:17, 35-52`. Nothing else in v4 is changed.

## Task 1: draft core
**Model** (draft as a simultaneous-move game; Battlecode and MOBA meta reading). Picks are simultaneous within a round and duplicates across teams are allowed (FACTS, draft line), so there is no denial term and the choice is a fixed best response to the field. Field = the top-10 champion usage (notes/top10 l.49-51: Stellar 96 %, C&I 95 %, Ren 72 %, Varg 70 %, Ryth 67 %, Vendetta 3 %). Decision rule: draft the first k entries of one ordered list, where k = team size.

**Kit support in v4's code** (read, then confirmed in games). Abilities are cast by kind (`cast_on_camp`, `cast_on_enemy`: damage, area, stun), attack resets by `reset_and_attack_again`, dashes by M3_DASH in fights, and Varg's shield by `cast_shield`.
- Ryth: area stun and reset are used.
- C&I: Cassi's damage-stun and Issac's reset and dash are used.
- Stellar: damage and area casts are used; Focus build.
- Varg: shield and reset are used.
- Ren: stun and dash are used.

Gaps:
1. Ren buys Strength, but his stun scales with Focus (1.25 × Focus).
2. Squads: with no Vendetta, squad 0 is C&I alone (2 bodies) and squad 1 has the other four.
3. The core draft raises "engine error 97: Unit is not in vision" bot errors from about 2 to about 6 a game. The rate is the same with no S2 code (t1: 6.1, against v4's 2.9 in the same games), so it is v4 play code hitting a target out of vision with these champions. It is untraced; each error costs one turn.
4. The core draft makes fights heavier on fuel (t1: 5 exhausted turns on fracture and concentrated_wealth). The S2 fuel fixes below handle this.

**Tavi:** v4's buy code does not assume Tavi. Field shopping needs the remote-shop passive (`remote_shop`), and the w8 path on monolith buys normally for every other champion.

**Cause trace (interlock, elkring; statistics JSON per champion, salt 0, both sides):**
- With the plain core order (Ren 3rd), the core went 0/6 on both maps.
- Interlock: v4's Varg collects 456-628 gems with 7-8 kills and 0-1 deaths. Our Ren in the same slot collects 84-112 with 4-5 deaths.
- Elkring: v4's Tavi solo-farms 506-561 gems. Our Ren and Ryth collect 0-242.
- So v4's squad code leans on Varg as tank and collector, and it plays Ren as a generic melee champion.
- Fix: P_S2_CORE_VARG. The 3- and 4-pick drafts become the top-10 second sets (C&I+Stellar+Varg: 16-30 sides a map, 100 % on broken_curve, n=16†; Ryth+C&I+Stellar+Varg on interlock and hooks: 49 % and 58 %).
- Result: 77/126 and deaths 16.8, against 75/126 and 19.1 for the same build with Ren 3rd (variants d and a).

**Per-map exceptions** (brief rule: a variant must beat the core by ≥ 2 of 6 on the map AND the overall head-to-head must not drop).
- Candidates were expanse (0/6) and contest (2/6, where the siege mode was built for its own draft).
- With v4's draft there (variant f): contest 3/6 (+1), expanse 0/6 (+0), overall 78 vs 77.
- No exception is admitted. Expanse is lost with either draft, so its cause is not the draft.

## Task 2: altar control
**Lock model** (objective denial, like a tower dive in MOBA play).
- Rule: a lock placed on altar a cancels enemy channels there and blocks new ones until 3 basic attacks remove it (GameRules l.193-199). Cost: 10 gold and no move or attack.
- Value: denying an enemy channel with bag B costs them ceil(B/10) + 3 turns. A lock placed in advance costs them 3 turns on the altar they are likely to use.
- Decision rule:
  1. Place on an altar where an enemy is channelling within 2 tiles.
  2. Walk in if one is within 4 tiles, while above 50 % health and carrying fewer than 20 gems.
  3. Otherwise place on the "enemy altar" (the last enemy tribute seen, else the altar nearest the enemy spawn) when passing within 2 tiles.
- A lock is bought on every shop visit from round 200 (P_S2_LOCK_FROM).
- Placements in the first build: 101 on enemy channels, 522 in advance. Only 10 % were before round 200, so delaying the buys costs little denial and adds about +0.3 gear at round 200.
- (a) Detecting enemy locks: v4 already does it. Locks seen are remembered for 40 rounds, a locked altar scores +3 in the altar choice (the cost of clearing, against the distance to another altar), and a lock within reach is attacked. Kept unchanged.

**Tribute gate** (an optimal-stopping rule; tributes take T = max(1, ceil(g/10)) turn ends).
- Do not start when any of these holds:
  - stun: an enemy within 5 has a stun ready (cooldown ≤ 1), since stun cancels the channel;
  - lock: an enemy within 3 of the altar may carry a lock (unknown inventory counts as yes);
  - kill: the sum over enemies within 4 of (Strength + 1.5 × Focus) × (T + 1) is at least my health, since death drops the bag.
- Damage alone never holds the tribute (GameRules l.181). After 3 held turns the tribute starts anyway.
- When held, `bank` returns false and v4's enemy response takes the turn.

## Task 3: gold spending
**Model** (inventory replenishment, a periodic-review policy from operations research).
- Shards cost 30 gold and can only be bought on spawn tiles or while dead (FACTS, Shop). The lock is bought first, which keeps 10 gold for it.
- The lever is the number of shop visits. The decision rule triggers a visit when either:
  - (i) a tribute has just finished, gold ≥ 90, there is no incoming damage and no enemy within 8: recall, or walk if the spawn is within 6 (the top-10 recall-and-shop loop, 62 recalls a game);
  - (ii) there is no bag, gold ≥ 90, the spawn is within 12 tiles and no enemy is in view: walk in.
- Build order unchanged from v4 (Monster emblem of the main stat, then Power shards of Health and the main stat).

**Unspent gold v4 holds** (v4 side of the core-draft run, `results/s2-t1-vs-v4.json`, per body): **28.9 at round 100, 35.0 at round 200**. Across all runs it stays at 29-36 at round 200. The candidate holds 28.2 / 31.2.

**Gear:**
| | Spent gold by round 200 | Gear (points) | Unspent at round 200 |
|---|---|---|---|
| Candidate, per body | ~81 | 2.71 | 31 |
| v4, per body | ~68 | 2.25 | 35 |

- Spending every coin would give at most ~3.7 points, and the unspent figure always includes the remainder below one shard.
- Lowering the trigger hurts: SHOP_GOLD 60 → 71/126, and 30 with a 16-tile detour → 65/126, at gear 2.60-2.63. Detours cost income.
- Routing banking trips through the spawn (P_S2_BANK_VIA) did nothing: gold is rarely ≥ 60 when a bag starts.
- **The 3.0 target is limited by gold income, not by the spending rule.** About 110 gold per body arrives by round 200. The top-10's 3.8 comes with more camps captured (15.0 groups by round 300 against our 10-11, notes/top10 §2), which is the S1 economy module's job.

## Ablations (one switch off from the candidate, 126 games each vs v4)
Candidate configuration before the two fuel-only edits (lock early exit and FAST_CONTEST):
| Variant | Wins/126 | Gem margin/game | Deaths/game (v4's) | Locks placed | Gear@200 |
|---|---|---|---|---|---|
| candidate | 78 | +146 | 16.9 (22.8) | 4.1 | 2.71 |
| lock off | 72 | +117 | 16.9 (23.4) | 0 | 2.71 |
| gate off | 76 | +98 | 18.4 (22.7) | 4.6 | 2.63 |
| spend off | 61 | +52 | 19.8 (22.0) | 4.4 | 2.28 |
| core off (v4 drafts) | 73 | +86 | 20.4 (23.6) | 4.5 | 2.62 |
| **final candidate (with fuel edits)** | **70** | **+132** | **17.0 (22.9)** | **4.0** | **2.71** |

First configuration (core with Ren 3rd, locks from round 0):
| Variant | Wins | Margin | Deaths | Gear@200 |
|---|---|---|---|---|
| all on | 78 | +107 | 18.9 | 2.33 |
| lock off | 74 | +85 | 18.6 | 2.65 |
| gate off | 77 | +126 | 18.1 | 2.35 |
| spend off | 67 | +64 | 21.3 | 1.96 |
| core draft only, nothing else (s2-t1) | 58 | +7 | 20.8 | 2.23 |

Other variants: lock-from-200 alone (a) 75 / +96; with SHOP_GOLD 60 (b) 71 / +54; with SHOP_GOLD 30 and a 16-tile detour (c) 65 / +59; Varg order (d) 77 / +150 / 16.8 deaths; d plus exceptions (f) 78 / +148; bank-via (e) the same as a.

Reading:
- Spend-now is the largest effect: −13 to −17 wins and −55 to −95 margin when off.
- The lock adds +4 to +6 wins in both configurations, and the brief's ablate-on-its-own condition is met. Caveat: v4 never locks or clears locks, so self-play flatters the lock.
- The gate is +1 to +2 wins in both passes. In the second pass it is −1.5 deaths a game, in the first +0.8: it is close to noise and is kept because it is cheap.
- The core draft is +5 wins and −3.5 deaths against v4's drafts. The draft decision rests on the live panel.

## Final head-to-head per map (candidate vs v4, 6 games a map)
| map | wins | our deaths | v4 deaths | gem margin | locks placed | gear@200 us | gear@200 v4 |
|---|---|---|---|---|---|---|---|
| ambush | 2/6 | 9.0 | 12.0 | -19 | 0.7 | 1.67 | 1.58 |
| big_world | 4/6 | 4.3 | 7.5 | +100 | 1.3 | 1.75 | 1.17 |
| boss_battle | 2/6 | 2.2 | 1.5 | +0 | 0.0 | 0.92 | 2.92 |
| broken_curve | 5/6 | 17.3 | 27.2 | +217 | 1.2 | 2.54 | 2.25 |
| collision | 3/6 | 12.3 | 19.3 | +210 | 10.3 | 5.33 | 4.33 |
| concentrated_wealth | 1/6 | 38.7 | 34.5 | -150 | 6.0 | 2.14 | 1.61 |
| contest | 2/6 | 42.0 | 37.2 | +0 | 0.8 | 1.53 | 0.53 |
| corners | 6/6 | 5.8 | 9.7 | +396 | 1.5 | 3.61 | 3.56 |
| corridors | 3/6 | 13.7 | 11.0 | +2 | 9.2 | 0.00 | 0.31 |
| eight | 5/6 | 24.3 | 27.7 | +325 | 5.0 | 2.33 | 2.22 |
| elk_arena | 4/6 | 28.2 | 42.3 | +64 | 3.3 | 4.81 | 4.03 |
| elkring | 3/6 | 13.3 | 15.2 | +70 | 5.5 | 3.22 | 2.11 |
| expanse | 0/6 | 0.0 | 1.5 | -92 | 0.5 | 3.17 | 3.50 |
| fracture | 2/6 | 21.7 | 22.8 | -92 | 4.7 | 1.33 | 0.75 |
| fragment | 6/6 | 18.3 | 65.5 | +810 | 4.0 | 6.62 | 2.67 |
| gold_rush | 2/6 | 21.7 | 26.7 | -7 | 1.3 | 0.83 | 0.46 |
| hooks | 3/6 | 46.3 | 57.2 | +255 | 13.7 | 4.70 | 4.37 |
| interlock | 4/6 | 12.0 | 22.7 | +225 | 2.7 | 2.57 | 1.57 |
| islands | 4/6 | 10.0 | 10.2 | +206 | 1.8 | 2.67 | 1.58 |
| lanes | 3/6 | 8.3 | 9.0 | -41 | 0.0 | 1.58 | 2.00 |
| monolith | 6/6 | 7.8 | 19.3 | +299 | 10.2 | 3.53 | 3.83 |

## Fuel (traced)
- **Trace:** the gate first FAILED on lanes as B (`results/gate-s2-20261020-122615Z-linux.json`), and the head-to-head had 6 exhausted turns, all on lanes as B: Varg, rounds 406-413.
- **Profile** of the busiest Varg turn on lanes, in thousands of fuel at each checkpoint: view 12, stuck 13, shield 15, vision 21, contested 49, lock 58, retreat 60, enemy 68. So v4's `update_contested` costs 28k, and S2's lock check costs 8-10k because it queries every enemy's position.
- **Fixes:**
  - the lock check returns early, with no SDK calls, when no altar is within 4;
  - it reads each enemy's position once;
  - the lock and gate checks are skipped below 45k fuel left;
  - the turn ends before targeting, banking and farming below 35k left (P_S2_FUEL_STOP);
  - `update_contested` runs in one pass (P_S2_FAST_CONTEST, same decisions).
- **After the fixes:** gate PASS, max turn 93.8k. Final head-to-head: 0 exhausted turns, max 97.1k on lanes. These peaks are above the 80k guideline and come from v4's turn under the heavier core-draft fights. The S2 code itself costs < 2k when no altar is near.

## What still fails or is open
- **Gear at round 200 is 2.71, not 3.0.** It is limited by income (see Task 3). It needs more camp captures (S1).
- **The core draft raises "Unit is not in vision" errors** to ~6 a game (v4 ~2). These come from v4 play code with the new champions. Untraced: the next step is to find which call throws, with Stellar or Ren most likely.
- **expanse is 0/6** with every draft and setting. v4 also lost it 0/6 to v3b-e1 (results/v4-cloud.md). Untraced.
- **Not done:** /watch of a loss (no browser here), and the live top-10 panel (run by the lead).
- **Self-play caveats:** v4 never buys locks, so the gate's lock term and the lock-clearing code are not exercised. Real top-10 teams lock 6.4 times a game and stun more, so the gate's value is untested where it matters.
