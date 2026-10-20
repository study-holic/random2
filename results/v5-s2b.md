# v5 S2b: frozen-commit check of the S2 candidate (20 Oct 2026)

Frozen commit: `25ec32b2644b5c7d404857e46098cb61269899a0` (branch claude/nice-cray-9ju6gn; main not merged). `bots/s2` is built from a clean `.cambc`, with the defaults committed in `bots/s2/src/a/s2.h`: draft core in Varg order, altar lock from round 200, tribute gate, spend-now, fuel stop 35k, fast contest. No code changed from the start of the head-to-head to the end of the gate (`git diff HEAD -- bots/s2` empty before both). Engine 1.0.0, CLI 3.0.1, map set da4707523852 (equal to ground truth), `--jobs 3`.

Steps run, in order:
1. `python3 scripts/mapgate.py s2 all --jobs 3 --salts 0,1,2 --panel v4 --keep --out results/s2b-vs-v4.json`: 126 games.
2. `python3 tools/cloud/metrics_s2.py results/s2b-vs-v4.json`
3. `python3 scripts/gate.py s2`

## Admission table
| # | Line | Result | Verdict |
|---|---|---|---|
| 1 | gate.py PASS | 42/42 vs starter-cpp on 21 maps, 0 faults, mapid correct, max turn fuel 93,816, no fuelExhausted after round 1 (`results/gate-s2-20261020-151102Z-linux.json`) | **PASS** |
| 2 | Head-to-head vs v4 ≥ 45 % (rejection gate only) | **70/126 = 55.6 %**; gem margin +132 a game (727.8 vs 595.5); 0 faults, 0 errors, 0 fuelExhausted, max turn fuel 97,100 | **PASS** |
| 3a | Locks placed per game > 0 | **3.98** placed (9.10 bought); v4 0 | **PASS** |
| 3b | Gear at round 200 ≥ 3.0 per champion | **2.71** (v4 2.25 in the same games) | **FAIL**, waived for the panel upload |
| 4 | Deaths per game vs v4 not up > 10 % | **17.02** vs v4's 22.85 in the same games (−25.5 %) | **PASS** |

Unspent gold per body: ours 28.2 at round 100 and 31.2 at round 200; v4 31.3 and 35.3.

This head-to-head is identical, game for game in its totals, to the final run in results/v5-s2.md (70/126, +132, deaths 17.0, locks 3.98, gear 2.71). The engine is deterministic, so the frozen build reproduces it.

Why gear fails (results/v5-s2.md, Task 3): about 110 gold per body arrives by round 200, and ~31 of it stays unspent between shop visits. Lowering the shop threshold lost head-to-head games without raising gear. Reaching 3.0 needs more income (camp captures), not a different spending rule.

## Per map (6 games each)
| map | wins | our deaths | v4 deaths | gem margin | locks placed | gear@200 |
|---|---|---|---|---|---|---|
| ambush | 2/6 | 9.0 | 12.0 | -19 | 0.7 | 1.67 |
| big_world | 4/6 | 4.3 | 7.5 | +100 | 1.3 | 1.75 |
| boss_battle | 2/6 | 2.2 | 1.5 | +0 | 0.0 | 0.92 |
| broken_curve | 5/6 | 17.3 | 27.2 | +217 | 1.2 | 2.54 |
| collision | 3/6 | 12.3 | 19.3 | +210 | 10.3 | 5.33 |
| concentrated_wealth | 1/6 | 38.7 | 34.5 | -150 | 6.0 | 2.14 |
| contest | 2/6 | 42.0 | 37.2 | +0 | 0.8 | 1.53 |
| corners | 6/6 | 5.8 | 9.7 | +396 | 1.5 | 3.61 |
| corridors | 3/6 | 13.7 | 11.0 | +2 | 9.2 | 0.00 |
| eight | 5/6 | 24.3 | 27.7 | +325 | 5.0 | 2.33 |
| elk_arena | 4/6 | 28.2 | 42.3 | +64 | 3.3 | 4.81 |
| elkring | 3/6 | 13.3 | 15.2 | +70 | 5.5 | 3.22 |
| expanse | 0/6 | 0.0 | 1.5 | -92 | 0.5 | 3.17 |
| fracture | 2/6 | 21.7 | 22.8 | -92 | 4.7 | 1.33 |
| fragment | 6/6 | 18.3 | 65.5 | +810 | 4.0 | 6.62 |
| gold_rush | 2/6 | 21.7 | 26.7 | -7 | 1.3 | 0.83 |
| hooks | 3/6 | 46.3 | 57.2 | +255 | 13.7 | 4.70 |
| interlock | 4/6 | 12.0 | 22.7 | +225 | 2.7 | 2.57 |
| islands | 4/6 | 10.0 | 10.2 | +206 | 1.8 | 2.67 |
| lanes | 3/6 | 8.3 | 9.0 | -41 | 0.0 | 1.58 |
| monolith | 6/6 | 7.8 | 19.3 | +299 | 10.2 | 3.53 |

Caveat: these results are self-play against v4. Builds with the same intent ranged 70 to 78 of 126, and v4 never buys or clears locks. The live top-10 panel decides the release.
