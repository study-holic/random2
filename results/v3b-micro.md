# v3b: w14 per-champion micro table on v3a (19 Oct)

## Built (bots/v3b; dispatcher untouched, patched identically in src/a and src/b)
- params.h (end of file): `P_M3` master switch, `P_M3_OFF` ablation bitmask (1 retreat, 2 earlycast, 4 shield, 8 dash, 16 kite), `m3_on(item, champion_type)` table (retreat: all but Willow; earlycast: all but Marcella; shield: Varg; dash: Ren, Vendetta, Issac, Vesper; kite: Vesper, Prism).
- bot.cpp after `RETREAT-FIRE` (a:~474, b:~423): item 1, retreating cleared when an enemy is within 4, gems < 40, and not (2+ foes within 4 with no ally within 4).
- enemies.cpp enemy_response: items 4 (dash-in for melee at foe distance 2-3; Vesper dash-out from adjacent melee foe), 2 (`cast_on_enemy` before the in-range test), 5 (kite for Vesper/Prism, skipped when the foe is Vendetta). v3a has P_RANGED_KITE 0, so kite is newly on for those two champions.
- util.cpp cast_shield: item 3, Varg shields when an enemy champion is within 4.
- Ablation copy bots/v3b-nr (P_M3_OFF 1). Debug copy bots/v3b-dbg (prints item 1 fires).

## a) Safety
Build clean. Starter-cpp 34/34 wins, 0 faults, fuel_late 0 (results/v3b-starter.json). Smoke (3 maps vs foundry) ran, no faults.

## b, c) Head-to-head, 3 salts x 2 sides x 17 maps (102 games), wins out of 6
| map | v3b vs v3a | v3b-nr vs v3a | v3b vs v2 |
|---|---|---|---|
| ambush | 4/6 | 4/6 | 4/6 |
| collision | 5/6 | 3/6 | 5/6 |
| contest | 6/6 | 6/6 | 6/6 |
| corners | 3/6 | 4/6 | 3/6 |
| eight | 6/6 | 0/6 | 3/6 |
| elkring | 3/6 | 3/6 | 3/6 |
| expanse | 3/6 | 3/6 | 3/6 |
| fragment | 3/6 | 6/6 | 3/6 |
| interlock | 3/6 | 3/6 | 3/6 |
| lanes | 4/6 | 3/6 | 4/6 |
| monolith | 4/6 | 3/6 | 3/6 |
| packs | 3/6 | 0/6 | 3/6 |
| pilgrimage | 3/6 | 4/6 | 3/6 |
| rows | 2/6 | 2/6 | 2/6 |
| rushbait-v2 | 4/6 | 3/6 | 4/6 |
| stampede | 4/6 | 5/6 | 5/6 |
| swirl | 3/6 | 3/6 | 3/6 |

| run | wins | deaths/g (ours) | opp deaths/g | gem margin/g | faults |
|---|---|---|---|---|---|
| v3b vs v3a | 63/102 = 61.8% | 8.83 | 7.85 | +120 | 0 |
| v3b-nr vs v3a (item 1 off) | 55/102 = 53.9% | 7.90 | 6.64 | +62 | 0 |
| v3b vs v2 | 60/102 = 58.8% | 8.71 | 7.51 | +133 | 0 |

fuel_late 24 in all runs (lanes, inherited from v3a). Gems carried at death are not in mapgate output; not reported.

## Falsifier
Reject if wins < 55% of 102 vs v3a or deaths +15% with no gem gain. Result: 61.8% wins; our deaths 8.83 vs v3a's 7.85 (the opponent's deaths = our kills) = +12.5%, below 15%, with gem margin +120/game. Not rejected. Caveat: 63/102 is about 1.2 sd above 50%; item 1 off gives 53.9%, so item 1 carries the gain (+8 points), the other items are about neutral-to-positive (v3b-nr alone is below the 55% bar). The ablation did not isolate items 2-5.

## Traced item 1 (corners, v3b-dbg, side A, salt 0, a won game, 527 rounds)
r50 champion 5 (Stellar), hp 710, one foe within 4, no ally: kept fighting; hp 110 at r54 and 50 at r55, still fighting (the walk-home would have fired). Cassi (type 3) fired r121-123 at hp 700 to 580. I did not trace whether Stellar died.

## Verdict
Keep the table (all items on) as a v3b candidate: passes both falsifier clauses. Panel regression (d) not run. No upload. Deaths rise 12.5%, close to the limit; a 2nd 102-game run would tighten the estimate.
