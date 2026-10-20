# econsim findings (S2 economy simulator, 21 maps, engine-1.0 camp values)

Details per map follow after the findings. Code in `tools/econsim/`, raw numbers in `results/econsim.json` and `results/econsim-sens.json`. Every number is a model output with no enemy, one trial per cell, not a measured result.

## Findings

1. **Gems per round are low everywhere**: best policy 0.16 (fragment) to 1.92 (elk_arena), median about 0.75; contest has no gem income except the monoliths (0.00 without them, 1.33 if a squad could absorb 200 strength per hit). 1,000 gems needs 1,300 rounds at 0.75, so the 2,000-round limit and kills matter.
2. **Best policy by map class** (ties are frequent: when two names share a value they pick the same route):
   - Elk-heavy maps where every group needs the full team (ambush, boss_battle, eight, expanse, fracture, corners, islands, elkring): the whole team as one blob choosing by value per round (team/all) or greedy; splitting loses (wait 40%+ in split squads).
   - Cluster-rich maps with many small gem/gold groups (lanes, gold_rush, monolith, collision, broken_curve, interlock, fragment, corridors, big_world): split squads or conveyor (blob clears, one capturer stays, rest move on) and the beam search win by 20 to 80%.
   - Maps with no gem income without a monolith (contest): economy is gold only; the monolith is the plan.
3. **Gain over greedy nearest**: lanes +79%, monolith +79%, gold_rush +73%, interlock +42%, collision +41%, corridors +30%, fracture +29%, eight +23%, corners +22%, big_world +20%; expanse +172% (greedy keeps chasing the far camp while the team farms elsewhere; 0.29 against 0.78). Zero gain on ambush, elk_arena, hooks, islands (greedy equals the best found). Mean across the 20 maps with gems: about +35%.
4. **Bank size** (per champion): best bank is 10 on 6 maps, 20 on 5, 30 to 60 on 8, 80 on 4 (corridors, big_world, interlock, concentrated_wealth). Rule of thumb from the model: the bank is where a trip costs less than one capture cycle: small banks (10 to 20) when the altar is within about 15 steps of the route; 60 to 80 when the altar is 30+ steps away. The curve is flat within about 5% between neighbouring values on most maps, so a 3-value table (10, 30, 60) is enough.
5. **Time budget of the best policy** (mean over maps): travel 27%, clearing 43%, capture 13%, tribute 2%, waiting for respawn 15%. Tribute time is negligible; clearing and walking dominate, so damage per round (gear) and route length matter more than bank size.
6. **Recall never helped** (<3% on every map; 2 of 21 maps change policy). Walking to an altar nearer than the spawn is always cheaper than a 5-round channel plus a walk back.
7. **Holes and islands**: unreachable on foot from our spawn: ambush 2 gem groups, big_world 4 groups, fracture 2 groups, islands 38 of 46 groups. Teleport crystals (range 3, ignoring terrain) or dashes open all of them; with an unlimited crystal supply the model gives ambush 0.70 -> 0.85, big_world 0.53 -> 1.06 (+100%), fracture 0.93 -> 0.98, islands 0.78 -> 1.16 (+49%). On islands, walking only reaches 8 groups, and a 1-champion-per-crystal cost (10 gold each, 3-round channel) must be paid per hop per champion: the real figure sits between. Dash (range 2 over holes, cooldown 8) reaches the same groups in the reachability test but cannot be used every hop, so crystals are the route on big_world and islands. Needs a check against the engine that the crystal landing tile is legal near each group.
8. **Assumptions that move the answer** (re-optimised policy per assumption, median change in best gems/round; policy family changes on the number of maps shown):
   - damage per champion per round: 30 -> x0.69 (10 of 21 maps change family), 80 -> x1.30 (10 maps). Biggest uncertainty; it also decides whether splitting pays.
   - damage budget a champion may absorb per group (feasibility): 300 -> x0.78 (8 maps), 1,500 -> x1.06 (5 maps; up to x3.25 where elks bar small squads).
   - monoliths allowed (a squad can absorb them): x1.36 median, up to x3.25 (contest 0 -> 1.33, corridors 1.16 -> 2.14, big_world 0.53 -> 1.52). The monolith is the largest income on 8 maps; its 200 strength and 20,000 HP make it a team-fight objective, not a farm.
   - gold counted at 0.1 gem per gold: no change in median (0.71 to 1.04).
   - Not modelled: monsters healing, camp damage persisting between visits (contest notes say monoliths keep damage), enemy contesting, deaths, the +33% Vesper gold. Camp values are engine-1.0 group data (still being confirmed in a game).
9. **Calibration**: `python tools/econsim/calibrate.py --since 2026-10-18` prints real gems per round from `data/v3/decoded/econ.jsonl` (top-10 games, engine 0.0.3) against the sim. Real top-10 teams reach 1.0 to 2.2 gems/round on the old rewards (gem cluster 24 against 7 now; wolf 22 against 4), 2 to 6 times the engine-1.0 sim, in line with rewards 2 to 3.4 times larger, so the comparison is only a sanity check. Real check: run v4 base on the 21 maps, save `cambc matches summary <uuid> --json -o f.json`, pass them with `--summary`; the target is the sim "greedy" column within 10%. The summary schema was not available offline (the CLI needs a match UUID), so the reader searches for keys containing `tribut` or `score` and `rounds`; it may need one edit.

## What the plan compiler should store

Per champion slot (squad) a route of group ids and a bank threshold; map-specific facts come from the draft-time BFS, not memory.

- Header: 1 byte policy id (0 greedy, 1 team/blob value, 2 split, 3 conveyor), 1 byte bank code (bank/10), 1 byte first-squad size mask.
- Per slot: 1 byte length + one byte per group id (groups ordered as in `data/maps21.json` after mirroring; 46 groups on islands fits a byte). The routes in the per-map sections show the first captures of each squad: 8 to 10 ids, a repeating cycle of 2 to 4 groups after the first lap, so storing one lap plus a cycle start is enough.
- Total: 3 + 5 x (1 + 10) = 58 bytes for 5 slots, 3 + 3 x 11 = 36 bytes for 3 slots. Against 1,024 bytes of draft memory this is under 6%, leaving room for altar ids (1 byte each, up to 18 on lanes), the unreachable-group flags (1 bit per group, 6 bytes for 46) and crystal-hop waypoints (2 bytes per hop). Champions can also recompute the route from the stored policy id and bank with the BFS fields in about 5 turns of fuel, which would cut memory to under 10 bytes; storing the routes avoids that computation.

---

# econsim: S2 economy simulator results (engine-1.0 camp values, no enemies)

Generated by `tools/econsim/report.py` from `results/econsim.json`, `results/econsim-sens.json`. Code: `tools/econsim/{mapmodel,sim,run_all,sens,report,calibrate}.py`. Run: `python tools/econsim/run_all.py [maps]`, `python tools/econsim/sens.py`, `python tools/econsim/report.py`. Single process, about 6 min.

## Model and assumptions

- Grid: walls and holes block walking (king moves, 8 directions); BFS from group anchors (the camp nearest the group centroid), altars, spawn A. Spawn B = mirror of A. Our altars = those no farther from A than from B. Camp values from `data/maps21.json` group info (gems, gold, respawn; engine 1.0, kept as data, `gem_scale` swaps them).
- Horizon 900 rounds from round 0; metric = gems tributed per round (bags left at the end are lost). No enemy: contested groups are free (scenario `all`); scenario `half` keeps only groups at least as near to A as to B.
- Team = squads of k champions that move, clear and capture together. Clear time = camp HP (+ span overhead) / (k x dpr), dpr = 50 per champion per round. Feasibility: a squad of k may start a group only if monster strength x clear rounds / k <= 700 (damage per champion); this is what bars wolves/elks from solo play and excludes monoliths (20,000 HP, strength 200). Monsters are not simulated otherwise.
- Capture: 1 round to step on the tile + 5 rounds, reward split evenly among champions present (lowest ID takes the remainder), group respawn = group respawn value after the reward. A group left alone for 2 rounds heals fully (rule doc); the contest notes suggest monoliths keep damage, not modelled.
- Tribute: when any member bag >= bank b (per champion), walk to the nearest own altar, channel ceil(max bag / 10) rounds; optional recall = 5 ticks to spawn then walk to altar.
- Policies: greedy = one blob, nearest alive group, no value; team = one blob, value per round; value = squads (split swept), gems / (wait + travel + clear + capture + amortised tribute trip), exclusive claims; conveyor = blob that leaves one capturer (smallest bag) at each group when the rest can still take the next-best group, capturers farm on their own; orient = beam search (width 6, depth 3) over the next 3 groups with respawn windows. Best bank over {10,20,30,40,60,80} per row.

## Summary

| map | n | best policy/squads | bank | best g/r | greedy g/r | gain |
|---|---|---|---|---|---|---|
| ambush | 5 | greedy/all | 20 | 0.70 | 0.70 | +0% |
| big_world | 5 | conveyor/all | 80 | 0.53 | 0.44 | +20% |
| boss_battle | 5 | team/all | 10 | 0.80 | 0.76 | +5% |
| broken_curve | 3 | orient/half | 20 | 0.59 | 0.50 | +17% |
| collision | 3 | orient/half | 40 | 0.68 | 0.48 | +41% |
| concentrated_wealth | 5 | orient/all | 80 | 1.65 | 1.49 | +11% |
| contest | 5 | greedy/all | 10 | 0.00 | 0.00 | n/a |
| corners | 5 | value/half | 20 | 0.76 | 0.63 | +22% |
| corridors | 5 | conveyor/all | 80 | 1.24 | 0.95 | +30% |
| eight | 5 | team/all | 10 | 0.74 | 0.60 | +23% |
| elk_arena | 5 | greedy/all | 20 | 1.92 | 1.92 | +0% |
| elkring | 5 | conveyor/all | 60 | 0.76 | 0.73 | +3% |
| expanse | 5 | team/all | 20 | 0.78 | 0.29 | +172% |
| fracture | 5 | team/all | 10 | 0.93 | 0.72 | +29% |
| fragment | 3 | value/solo | 10 | 0.16 | 0.11 | +44% |
| gold_rush | 3 | conveyor/half | 30 | 0.55 | 0.32 | +73% |
| hooks | 4 | greedy/all | 10 | 0.88 | 0.88 | +0% |
| interlock | 4 | orient/solo | 80 | 0.37 | 0.26 | +42% |
| islands | 5 | greedy/all | 30 | 0.78 | 0.78 | +0% |
| lanes | 5 | conveyor/all | 60 | 1.03 | 0.57 | +79% |
| monolith | 5 | orient/half | 40 | 0.67 | 0.37 | +79% |

## ambush (5 champions, 12 groups, 2 own altars)

- **unreachable on foot from spawn: 2 groups** (2gem(31, 49), 2gem(68, 50)); with crystals (range 3, 3-round channel, 10 gold each) every one is reachable

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| greedy/all | 0.70 | 20 | 55% | 34% | 9% | 2% | 0% | 0.54 |
| team/all | 0.68 | 30 | 57% | 33% | 8% | 2% | 0% | 0.51 |
| value/all | 0.68 | 30 | 57% | 33% | 8% | 2% | 0% | 0.51 |
| orient/all | 0.68 | 20 | 57% | 33% | 8% | 2% | 0% | 0.56 |
| conveyor/all | 0.68 | 30 | 57% | 33% | 8% | 2% | 0% | 0.51 |

Best: **greedy/all**, bank 20, 0.70 gems/round; greedy 0.70 (+0%). Best on own half only: orient/all (0.56).

With crystal hops allowed (unlimited, 3 extra rounds per hop): best greedy/all 0.85 gems/round vs 0.70 on foot.

Sensitivity (best policy per assumption): base: greedy/all b20 0.70; dpr30: value/all b10 0.29; dpr80: greedy/all b20 0.70; budget300: value/all b20 0.33; budget1500: greedy/all b20 0.70; mono_ok: orient/pairs b40 0.87; gold0.1: value/all b40 0.68; recall: greedy/all b20 0.70

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [6, 11, 1, 0, 1, 0, 1, 0, 1, 0]. Draft-memory cost for these routes: 13 bytes.

## big_world (5 champions, 24 groups, 3 own altars)

- **unreachable on foot from spawn: 4 groups** (1gem+2gold(27, 22), 1gem+2gold(72, 77), 3gem(52, 28), 3gem(47, 71)); with crystals (range 3, 3-round channel, 10 gold each) every one is reachable
- monolith groups present (2): excluded unless a squad can absorb 200 strength; see sensitivity `mono_ok`

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| conveyor/all | 0.53 | 80 | 15% | 25% | 7% | 1% | 52% | 0.36 |
| orient/all | 0.50 | 30 | 48% | 39% | 12% | 1% | 0% | 0.50 |
| greedy/all | 0.44 | 10 | 52% | 35% | 11% | 2% | 0% | 0.44 |
| value/pairs | 0.42 | 60 | 26% | 15% | 8% | 1% | 51% | 0.35 |
| team/all | 0.39 | 80 | 12% | 25% | 7% | 1% | 55% | 0.39 |

Best: **conveyor/all**, bank 80, 0.53 gems/round; greedy 0.44 (+20%). Best on own half only: orient/all (0.50).

With crystal hops allowed (unlimited, 3 extra rounds per hop): best orient/solo 1.06 gems/round vs 0.53 on foot.

Sensitivity (best policy per assumption): base: conveyor/all b80 0.53; dpr30: value/solo b30 0.34; dpr80: orient/all b10 0.61; budget300: value/half b80 0.40; budget1500: conveyor/all b80 0.56; mono_ok: value/all b10 1.52; gold0.1: orient/all b30 0.50; recall: conveyor/all b80 0.53

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [8, 16, 12, 16, 12, 16, 16, 12, 16, 12]. Draft-memory cost for these routes: 13 bytes.

## boss_battle (5 champions, 31 groups, 1 own altars)

- monolith groups present (1): excluded unless a squad can absorb 200 strength; see sensitivity `mono_ok`

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| team/all | 0.80 | 10 | 22% | 58% | 17% | 3% | 0% | 0.68 |
| value/all | 0.80 | 10 | 22% | 58% | 17% | 3% | 0% | 0.68 |
| orient/all | 0.80 | 10 | 22% | 58% | 17% | 3% | 0% | 0.70 |
| greedy/all | 0.76 | 10 | 24% | 57% | 17% | 3% | 0% | 0.58 |
| conveyor/all | 0.51 | 10 | 36% | 43% | 15% | 2% | 5% | 0.26 |

Best: **team/all**, bank 10, 0.80 gems/round; greedy 0.76 (+5%). Best on own half only: orient/all (0.70).

Sensitivity (best policy per assumption): base: value/all b10 0.80; dpr30: value/all b10 0.38; dpr80: value/all b20 1.07; budget300: value/all b10 0.46; budget1500: value/all b80 0.83; mono_ok: value/half b80 0.99; gold0.1: value/all b10 0.80; recall: value/all b10 0.80

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [12, 21, 22, 13, 21, 12, 22, 13, 21, 12]. Draft-memory cost for these routes: 13 bytes.

## broken_curve (3 champions, 22 groups, 2 own altars)

- monolith groups present (2): excluded unless a squad can absorb 200 strength; see sensitivity `mono_ok`

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| orient/half | 0.59 | 20 | 23% | 38% | 21% | 2% | 17% | 0.41 |
| greedy/all | 0.50 | 10 | 26% | 36% | 35% | 3% | 1% | 0.34 |
| team/all | 0.50 | 60 | 55% | 24% | 19% | 2% | 0% | 0.39 |
| value/all | 0.50 | 60 | 55% | 24% | 19% | 2% | 0% | 0.39 |
| conveyor/all | 0.48 | 10 | 14% | 9% | 8% | 2% | 68% | 0.32 |

Best: **orient/half**, bank 20, 0.59 gems/round; greedy 0.50 (+17%). Best on own half only: orient/half (0.41).

Sensitivity (best policy per assumption): base: orient/half b20 0.59; dpr30: orient/half b10 0.51; dpr80: orient/half b40 0.76; budget300: orient/all b40 0.55; budget1500: value/all b30 0.77; mono_ok: value/half b30 0.94; gold0.1: value/half b10 0.60; recall: orient/half b20 0.59

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [14, 19, 14, 20, 14, 21, 14, 19, 18, 15]; s1: [15, 18, 15, 19, 15, 18, 15, 14, 19, 15]. Draft-memory cost for these routes: 24 bytes.

## collision (3 champions, 20 groups, 1 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| orient/half | 0.68 | 40 | 20% | 52% | 15% | 3% | 10% | 0.57 |
| value/half | 0.65 | 40 | 16% | 49% | 15% | 2% | 18% | 0.57 |
| conveyor/half | 0.64 | 40 | 12% | 49% | 9% | 2% | 27% | 0.54 |
| team/all | 0.63 | 30 | 24% | 48% | 26% | 3% | 0% | 0.63 |
| greedy/all | 0.48 | 10 | 25% | 46% | 25% | 3% | 1% | 0.48 |

Best: **orient/half**, bank 40, 0.68 gems/round; greedy 0.48 (+41%). Best on own half only: orient/all (0.66).

Sensitivity (best policy per assumption): base: orient/half b40 0.68; dpr30: orient/all b40 0.51; dpr80: value/half b40 0.82; budget300: orient/all b40 0.66; budget1500: orient/half b40 0.70; mono_ok: orient/solo b30 0.77; gold0.1: value/half b40 0.68; recall: orient/half b40 0.68

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [6, 7, 6, 4, 5, 4, 5, 4, 7, 6]; s1: [4, 5, 7, 6, 7, 6, 5, 7, 6, 7]. Draft-memory cost for these routes: 24 bytes.

## concentrated_wealth (5 champions, 12 groups, 1 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| orient/all | 1.65 | 80 | 26% | 44% | 24% | 4% | 1% | 1.62 |
| team/all | 1.58 | 10 | 37% | 34% | 13% | 4% | 11% | 1.58 |
| value/all | 1.58 | 10 | 37% | 34% | 13% | 4% | 11% | 1.58 |
| greedy/all | 1.49 | 10 | 36% | 39% | 20% | 4% | 0% | 1.49 |
| conveyor/half | 0.88 | 10 | 32% | 36% | 8% | 2% | 22% | 1.45 |

Best: **orient/all**, bank 80, 1.65 gems/round; greedy 1.49 (+11%). Best on own half only: orient/all (1.62).

Sensitivity (best policy per assumption): base: orient/all b80 1.65; dpr30: value/all b10 1.42; dpr80: orient/all b10 1.78; budget300: orient/all b80 1.65; budget1500: orient/all b80 1.65; mono_ok: orient/all b80 1.65; gold0.1: orient/all b80 1.65; recall: orient/all b80 1.65

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [0, 1, 9, 8, 0, 1, 9, 8, 0, 1]. Draft-memory cost for these routes: 13 bytes.

## contest (5 champions, 14 groups, 2 own altars)

- monolith groups present (2): excluded unless a squad can absorb 200 strength; see sensitivity `mono_ok`

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| greedy/all | 0.00 | 10 | 0% | 0% | 0% | 0% | 100% | 0.00 |
| team/all | 0.00 | 10 | 0% | 0% | 0% | 0% | 100% | 0.00 |
| value/all | 0.00 | 10 | 0% | 0% | 0% | 0% | 100% | 0.00 |
| orient/all | 0.00 | 10 | 0% | 0% | 0% | 0% | 100% | 0.00 |
| conveyor/all | 0.00 | 10 | 0% | 0% | 0% | 0% | 100% | 0.00 |

Best: **greedy/all**, bank 10, 0.00 gems/round; greedy 0.00 (n/a). Best on own half only: greedy/all (0.00).

Sensitivity (best policy per assumption): base: value/all b10 0.00; dpr30: value/all b10 0.00; dpr80: value/all b10 0.00; budget300: value/all b10 0.00; budget1500: value/all b10 0.00; mono_ok: value/all b10 1.56; gold0.1: value/all b10 0.00; recall: value/all b10 0.00

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): . Draft-memory cost for these routes: 2 bytes.

## corners (5 champions, 30 groups, 3 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| value/half | 0.76 | 20 | 23% | 57% | 15% | 2% | 3% | 0.60 |
| orient/half | 0.76 | 20 | 23% | 57% | 15% | 2% | 3% | 0.61 |
| team/all | 0.73 | 30 | 39% | 38% | 21% | 2% | 0% | 0.57 |
| conveyor/half | 0.73 | 30 | 29% | 54% | 15% | 2% | 0% | 0.65 |
| greedy/all | 0.63 | 30 | 17% | 53% | 27% | 2% | 0% | 0.50 |

Best: **value/half**, bank 20, 0.76 gems/round; greedy 0.63 (+22%). Best on own half only: conveyor/half (0.65).

Sensitivity (best policy per assumption): base: value/half b20 0.76; dpr30: value/all b10 0.59; dpr80: conveyor/half b20 1.08; budget300: value/all b10 0.73; budget1500: value/pairs b10 0.91; mono_ok: orient/half b40 1.03; gold0.1: value/half b20 0.76; recall: value/half b20 0.76

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [26, 27, 19, 26, 27, 19, 26, 27, 19, 26]; s1: [28, 29, 28, 29, 28, 29, 28, 29, 28, 29]. Draft-memory cost for these routes: 24 bytes.

## corridors (5 champions, 24 groups, 1 own altars)

- monolith groups present (2): excluded unless a squad can absorb 200 strength; see sensitivity `mono_ok`

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| conveyor/all | 1.24 | 80 | 25% | 33% | 10% | 3% | 29% | 0.72 |
| value/half | 1.21 | 80 | 26% | 46% | 12% | 2% | 14% | 0.74 |
| orient/half | 1.21 | 80 | 25% | 46% | 12% | 2% | 14% | 0.80 |
| team/all | 1.16 | 80 | 27% | 24% | 13% | 3% | 32% | 0.78 |
| greedy/all | 0.95 | 60 | 55% | 28% | 15% | 3% | 0% | 0.64 |

Best: **conveyor/all**, bank 80, 1.24 gems/round; greedy 0.95 (+30%). Best on own half only: orient/half (0.80).

Sensitivity (best policy per assumption): base: conveyor/all b80 1.24; dpr30: value/all b80 1.16; dpr80: orient/pairs b80 1.37; budget300: value/all b80 1.16; budget1500: value/pairs b60 1.30; mono_ok: value/all b10 2.38; gold0.1: conveyor/all b80 1.24; recall: value/half b60 1.25

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [2, 3, 10, 2, 11, 3, 10, 2, 11, 10]. Draft-memory cost for these routes: 13 bytes.

## eight (5 champions, 8 groups, 1 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| team/all | 0.74 | 10 | 44% | 42% | 12% | 2% | 0% | 0.53 |
| value/all | 0.74 | 10 | 44% | 42% | 12% | 2% | 0% | 0.53 |
| orient/all | 0.74 | 10 | 44% | 42% | 12% | 2% | 0% | 0.57 |
| greedy/all | 0.60 | 80 | 25% | 52% | 21% | 2% | 0% | 0.57 |
| conveyor/all | 0.35 | 10 | 27% | 28% | 7% | 1% | 37% | 0.19 |

Best: **team/all**, bank 10, 0.74 gems/round; greedy 0.60 (+23%). Best on own half only: greedy/all (0.57).

Sensitivity (best policy per assumption): base: value/all b10 0.74; dpr30: orient/half b10 0.33; dpr80: value/all b20 1.18; budget300: value/half b10 0.40; budget1500: value/all b60 1.05; mono_ok: value/half b30 1.07; gold0.1: value/all b10 0.74; recall: value/all b10 0.74

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [0, 1, 0, 1, 0, 1, 0, 1, 0, 1]. Draft-memory cost for these routes: 13 bytes.

## elk_arena (5 champions, 30 groups, 1 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| greedy/all | 1.92 | 20 | 5% | 32% | 16% | 5% | 42% | 0.96 |
| team/all | 1.92 | 20 | 5% | 32% | 16% | 5% | 42% | 0.96 |
| value/all | 1.92 | 20 | 5% | 32% | 16% | 5% | 42% | 0.96 |
| orient/all | 1.92 | 20 | 5% | 32% | 16% | 5% | 42% | 0.96 |
| conveyor/all | 1.92 | 20 | 5% | 32% | 16% | 5% | 42% | 0.96 |

Best: **greedy/all**, bank 20, 1.92 gems/round; greedy 1.92 (+0%). Best on own half only: greedy/all (0.96).

Sensitivity (best policy per assumption): base: value/all b20 1.92; dpr30: value/all b10 1.68; dpr80: value/all b10 1.92; budget300: value/all b20 1.92; budget1500: value/all b20 1.92; mono_ok: value/all b20 1.92; gold0.1: value/all b20 1.92; recall: value/all b20 1.92

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [0, 1, 0, 1, 0, 1, 0, 1, 0, 1]. Draft-memory cost for these routes: 13 bytes.

## elkring (5 champions, 10 groups, 2 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| conveyor/all | 0.76 | 60 | 28% | 41% | 20% | 2% | 9% | 0.61 |
| greedy/all | 0.73 | 30 | 32% | 44% | 22% | 2% | 0% | 0.73 |
| team/all | 0.73 | 10 | 38% | 39% | 20% | 3% | 0% | 0.73 |
| value/all | 0.73 | 10 | 38% | 39% | 20% | 3% | 0% | 0.73 |
| orient/all | 0.73 | 30 | 34% | 43% | 21% | 2% | 0% | 0.73 |

Best: **conveyor/all**, bank 60, 0.76 gems/round; greedy 0.73 (+3%). Best on own half only: value/half (0.75).

Sensitivity (best policy per assumption): base: conveyor/all b60 0.76; dpr30: value/all b30 0.61; dpr80: value/half b60 1.37; budget300: conveyor/all b60 0.76; budget1500: value/half b20 1.03; mono_ok: value/solo b30 1.22; gold0.1: conveyor/all b60 0.76; recall: conveyor/all b60 0.76

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [2, 0, 4, 9, 7, 3, 1, 5, 8, 6]. Draft-memory cost for these routes: 13 bytes.

## expanse (5 champions, 24 groups, 5 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| team/all | 0.78 | 20 | 35% | 51% | 11% | 3% | 0% | 0.78 |
| value/all | 0.78 | 20 | 35% | 51% | 11% | 3% | 0% | 0.78 |
| orient/all | 0.78 | 20 | 35% | 51% | 11% | 3% | 0% | 0.78 |
| conveyor/all | 0.78 | 20 | 35% | 51% | 11% | 3% | 0% | 0.78 |
| greedy/all | 0.29 | 10 | 50% | 30% | 19% | 1% | 0% | 0.29 |

Best: **team/all**, bank 20, 0.78 gems/round; greedy 0.29 (+172%). Best on own half only: team/all (0.78).

Sensitivity (best policy per assumption): base: value/all b20 0.78; dpr30: orient/half b10 0.36; dpr80: value/all b80 1.03; budget300: orient/half b80 0.40; budget1500: value/half b60 0.85; mono_ok: value/pairs b40 0.93; gold0.1: value/all b20 0.80; recall: value/all b40 0.80

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [9, 5, 9, 5, 9, 5, 9, 5, 9, 5]. Draft-memory cost for these routes: 13 bytes.

## fracture (5 champions, 28 groups, 2 own altars)

- **unreachable on foot from spawn: 2 groups** (2gem(13, 32), 2gem(47, 38)); with crystals (range 3, 3-round channel, 10 gold each) every one is reachable
- monolith groups present (2): excluded unless a squad can absorb 200 strength; see sensitivity `mono_ok`

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| team/all | 0.93 | 10 | 30% | 46% | 13% | 4% | 7% | 0.49 |
| value/all | 0.93 | 10 | 30% | 46% | 13% | 4% | 7% | 0.49 |
| orient/all | 0.93 | 10 | 30% | 46% | 13% | 4% | 7% | 0.53 |
| greedy/all | 0.72 | 10 | 31% | 46% | 20% | 3% | 0% | 0.38 |
| conveyor/half | 0.44 | 80 | 35% | 39% | 9% | 1% | 16% | 0.29 |

Best: **team/all**, bank 10, 0.93 gems/round; greedy 0.72 (+29%). Best on own half only: orient/all (0.53).

With crystal hops allowed (unlimited, 3 extra rounds per hop): best team/all 0.98 gems/round vs 0.93 on foot.

Sensitivity (best policy per assumption): base: value/all b10 0.93; dpr30: value/all b20 0.32; dpr80: orient/all b10 1.02; budget300: value/all b40 0.38; budget1500: value/all b10 0.98; mono_ok: value/all b10 1.56; gold0.1: value/all b10 0.93; recall: value/all b10 0.93

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [25, 24, 25, 24, 25, 24, 25, 24, 25, 24]. Draft-memory cost for these routes: 13 bytes.

## fragment (3 champions, 6 groups, 1 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| value/solo | 0.16 | 10 | 29% | 46% | 9% | 1% | 15% | 0.09 |
| orient/solo | 0.16 | 10 | 37% | 46% | 9% | 1% | 7% | 0.09 |
| conveyor/all | 0.16 | 10 | 31% | 46% | 9% | 1% | 14% | 0.10 |
| greedy/all | 0.11 | 10 | 50% | 31% | 18% | 1% | 0% | 0.11 |
| team/all | 0.11 | 10 | 29% | 30% | 18% | 1% | 22% | 0.11 |

Best: **value/solo**, bank 10, 0.16 gems/round; greedy 0.11 (+44%). Best on own half only: greedy/all (0.11).

Sensitivity (best policy per assumption): base: value/solo b10 0.16; dpr30: conveyor/all b10 0.10; dpr80: value/all b20 0.64; budget300: value/all b10 0.11; budget1500: value/all b20 0.52; mono_ok: value/all b20 0.52; gold0.1: value/solo b10 0.16; recall: value/solo b10 0.16

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [4, 3, 5, 2, 3, 5, 2, 4, 3, 3]; s1: [2, 4, 3, 5, 2, 4, 3, 5, 2, 4]; s2: [5, 2, 4, 4, 3, 5, 2, 4, 5, 5]. Draft-memory cost for these routes: 35 bytes.

## gold_rush (3 champions, 16 groups, 2 own altars)

- monolith groups present (2): excluded unless a squad can absorb 200 strength; see sensitivity `mono_ok`

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| conveyor/half | 0.55 | 30 | 31% | 51% | 10% | 2% | 6% | 0.38 |
| value/solo | 0.51 | 30 | 35% | 50% | 10% | 2% | 3% | 0.38 |
| orient/half | 0.46 | 60 | 33% | 47% | 14% | 2% | 4% | 0.39 |
| team/all | 0.34 | 20 | 41% | 33% | 20% | 2% | 4% | 0.34 |
| greedy/all | 0.32 | 10 | 49% | 31% | 19% | 1% | 0% | 0.34 |

Best: **conveyor/half**, bank 30, 0.55 gems/round; greedy 0.32 (+73%). Best on own half only: value/half (0.39).

Sensitivity (best policy per assumption): base: conveyor/half b30 0.55; dpr30: conveyor/all b10 0.30; dpr80: value/solo b60 0.66; budget300: conveyor/all b20 0.35; budget1500: conveyor/half b30 0.55; mono_ok: conveyor/half b30 0.55; gold0.1: value/solo b80 0.39; recall: value/solo b30 0.51

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [10, 2, 3, 8, 9, 10, 8, 2, 10, 8]; s1: [8, 10, 2, 3, 9, 3, 8, 10, 2, 8]. Draft-memory cost for these routes: 24 bytes.

## hooks (4 champions, 12 groups, 1 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| greedy/all | 0.88 | 10 | 16% | 55% | 25% | 4% | 0% | 0.54 |
| team/all | 0.88 | 10 | 16% | 55% | 25% | 4% | 1% | 0.54 |
| value/all | 0.88 | 10 | 16% | 55% | 25% | 4% | 1% | 0.54 |
| orient/all | 0.88 | 10 | 16% | 55% | 25% | 4% | 0% | 0.54 |
| conveyor/all | 0.88 | 10 | 16% | 54% | 25% | 4% | 2% | 0.55 |

Best: **greedy/all**, bank 10, 0.88 gems/round; greedy 0.88 (+0%). Best on own half only: conveyor/all (0.55).

Sensitivity (best policy per assumption): base: value/all b10 0.88; dpr30: value/all b10 0.68; dpr80: orient/all b10 1.17; budget300: value/all b10 0.88; budget1500: value/half b10 0.93; mono_ok: value/half b10 0.93; gold0.1: value/all b10 0.88; recall: value/all b10 0.88

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [4, 2, 3, 5, 4, 2, 3, 5, 4, 2]. Draft-memory cost for these routes: 13 bytes.

## interlock (4 champions, 14 groups, 3 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| orient/solo | 0.37 | 80 | 23% | 47% | 10% | 1% | 19% | 0.23 |
| conveyor/half | 0.36 | 10 | 34% | 33% | 7% | 1% | 24% | 0.24 |
| value/half | 0.34 | 80 | 44% | 31% | 13% | 1% | 12% | 0.26 |
| greedy/all | 0.26 | 10 | 43% | 31% | 23% | 2% | 1% | 0.26 |
| team/all | 0.24 | 10 | 66% | 19% | 14% | 1% | 0% | 0.23 |

Best: **orient/solo**, bank 80, 0.37 gems/round; greedy 0.26 (+42%). Best on own half only: greedy/all (0.26).

Sensitivity (best policy per assumption): base: orient/solo b80 0.37; dpr30: orient/half b40 0.33; dpr80: orient/all b20 0.99; budget300: value/half b80 0.34; budget1500: conveyor/all b40 0.81; mono_ok: value/half b60 1.08; gold0.1: value/solo b40 0.39; recall: orient/solo b80 0.37

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [12, 12, 12, 4, 10, 12, 4, 10, 12, 4]; s1: [13, 13, 11, 5, 12, 4, 10, 12, 4, 10]; s2: [10, 12, 5, 10, 13, 11, 5, 13, 11, 5]; s3: [4, 11, 4, 10, 13, 11, 5, 13, 11, 5]. Draft-memory cost for these routes: 46 bytes.

## islands (5 champions, 46 groups, 2 own altars)

- **unreachable on foot from spawn: 38 groups** (1wolf+2elk+1gem(26, 28), 1wolf+2elk+1gem(33, 31), 1gem(34, 24), 1gem(25, 35)...); with crystals (range 3, 3-round channel, 10 gold each) every one is reachable

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| greedy/all | 0.78 | 30 | 46% | 42% | 11% | 2% | 0% | 0.78 |
| orient/all | 0.78 | 40 | 48% | 39% | 11% | 2% | 0% | 0.78 |
| team/all | 0.68 | 20 | 51% | 38% | 9% | 2% | 0% | 0.68 |
| value/all | 0.68 | 20 | 51% | 38% | 9% | 2% | 0% | 0.68 |
| conveyor/all | 0.68 | 20 | 51% | 38% | 9% | 2% | 0% | 0.68 |

Best: **greedy/all**, bank 30, 0.78 gems/round; greedy 0.78 (+0%). Best on own half only: greedy/all (0.78).

With crystal hops allowed (unlimited, 3 extra rounds per hop): best team/all 1.16 gems/round vs 0.78 on foot.

Sensitivity (best policy per assumption): base: orient/all b40 0.78; dpr30: value/all b10 0.00; dpr80: value/all b60 0.88; budget300: value/all b10 0.00; budget1500: orient/all b40 0.78; mono_ok: orient/half b60 1.07; gold0.1: value/all b30 0.78; recall: orient/all b10 0.78

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [38, 2, 38, 2, 38, 2, 38, 2, 38, 2]. Draft-memory cost for these routes: 13 bytes.

## lanes (5 champions, 42 groups, 9 own altars)

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| conveyor/all | 1.03 | 60 | 26% | 54% | 16% | 2% | 2% | 0.82 |
| value/half | 0.95 | 30 | 29% | 49% | 19% | 2% | 0% | 0.87 |
| orient/half | 0.93 | 60 | 25% | 51% | 20% | 2% | 1% | 0.89 |
| team/all | 0.73 | 60 | 30% | 41% | 27% | 2% | 0% | 0.68 |
| greedy/all | 0.57 | 10 | 32% | 35% | 31% | 2% | 0% | 0.57 |

Best: **conveyor/all**, bank 60, 1.03 gems/round; greedy 0.57 (+79%). Best on own half only: orient/half (0.89).

Sensitivity (best policy per assumption): base: conveyor/all b60 1.03; dpr30: value/half b10 0.71; dpr80: value/pairs b80 1.39; budget300: orient/pairs b60 0.77; budget1500: orient/pairs b60 1.09; mono_ok: conveyor/all b60 1.16; gold0.1: conveyor/half b60 0.99; recall: conveyor/all b60 1.03

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [6, 8, 30, 32, 34, 6, 31, 8, 33, 30]. Draft-memory cost for these routes: 13 bytes.

## monolith (5 champions, 20 groups, 4 own altars)

- monolith groups present (2): excluded unless a squad can absorb 200 strength; see sensitivity `mono_ok`

| policy / squads | gems/round (all) | bank | travel | clear | capture | tribute | wait | gems/round (half) |
|---|---|---|---|---|---|---|---|---|
| orient/half | 0.67 | 40 | 38% | 45% | 13% | 2% | 2% | 0.67 |
| value/half | 0.66 | 40 | 38% | 46% | 14% | 2% | 1% | 0.64 |
| conveyor/half | 0.62 | 10 | 40% | 48% | 10% | 2% | 0% | 0.46 |
| team/all | 0.53 | 60 | 45% | 34% | 20% | 1% | 0% | 0.53 |
| greedy/all | 0.37 | 20 | 43% | 33% | 23% | 1% | 0% | 0.37 |

Best: **orient/half**, bank 40, 0.67 gems/round; greedy 0.37 (+79%). Best on own half only: orient/half (0.67).

Sensitivity (best policy per assumption): base: orient/half b40 0.67; dpr30: orient/half b20 0.46; dpr80: orient/pairs b40 0.96; budget300: value/all b60 0.53; budget1500: orient/pairs b40 0.76; mono_ok: value/all b30 1.42; gold0.1: orient/half b60 0.66; recall: orient/half b40 0.67

First captures per squad (group ids, in order of the map's group list in `data/maps21.json` after mirroring): s0: [11, 19, 11, 19, 11, 19, 11, 5, 11, 3]; s1: [5, 3, 5, 3, 5, 3, 18, 4, 2, 4]. Draft-memory cost for these routes: 24 bytes.
