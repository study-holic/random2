# v4 base release (bots/v4, source of bots/v3c; no .cambc)

Built against SDK 3.0.1, engine 1.0.0. Do-no-harm pass: no draft, build or economy changes. NOT committed.

## Measured (local, before the cloud move)
| test | result |
|---|---|
| v3b-e1 (v3b source, SDK 3.0.1) vs starter-cpp, 21 maps x 2, salt 0 (results/v3b-e1-starter.json) | 36/42. Lost broken_curve/A, contest/A, corridors/B, gold_rush A+B, islands/B. 0 faults. Init fuelExhausted rounds 1-3 on big_world, boss_battle, islands, lanes (6 units each) |
| v3c = v4 base vs starter-cpp, last local gate.py (results/gate-v3c-0756.json) | 40/42, only corridors A+B lost; max turn fuel 93,423; no fuelExhausted after round 1; mapid printed and correct on all 21 maps |
| v3c vs v3b-e1 h2h, 3 salts (results/v3c-h2h.json) | 24.33/42 = 57.9 % but that run used death-loop mode 1, camp_range=all kinds; later changes (below) were checked only on 7-map subsets (noise level, 6 games per cell) |

## Confirmed engine facts (API probe, bots/probe, deleted)
Camp attack_range from the SDK: wolf 3, elk 4, monolith 5, gem/gold cluster 3. Group rewards: wolf 40 gold/4 gems, elk group of 2 + 3 gem clusters 180 gold/51 gems, monolith 300 gold/200 gems, gem cluster 7 gems each, gold cluster 20 gold each: matches ENGINE-1.0.md. NOT confirmed in play: the range at which an elk or monolith first hits (only the API value; gold_rush deaths sit at Chebyshev 4-5 from the monolith, consistent with 5). Golem: no golem camp exists on any of the 21 maps (probe).

## Fixes (switch; file:line in bots/v4/src/a)
1. Exact map id (no switch): mapid.h identify_e1 (l.59): dimensions, plus camp-group count only where two maps share a size (50x50, 100x100, 30x30, 40x40, 31x31). Returns the v2 ids for the 5 kept maps with changed layouts (collision, corners, expanse, interlock, lanes), new enum ids for the 10 new maps (no per-map plan -> default plan), unknown otherwise. map_name returns the pool name; plan.cpp:80 prints `mapid=<name>`. Kept-map counts changed (extra gem/gold cluster camps), which is why v3b mislabelled them; group counts and positions are unchanged.
2. Fuel: FIX_STAGED_INIT (bot.cpp:99 init(), phases 0-5, each checks fuel_remaining and yields; play_turn yields too when init leaves < 80k). FIX_LAZY_RING (bot.h:16, GroupInfo::ring() built on first use, saved ~4k per group). cached_groups() (mapid.h:79) one camp_groups() load per process. Opening-assign and altar reach share one distance field (init_opening_assign now takes field, reach). Result: init fuelExhausted gone (rounds 1-3), none after round 1 in the last gate. NOT met: the lead's 70,000 per turn target. Max measured 93k (gate), typical heavy turns 61-78k; farm() costs 45-58k on those turns (trace on concentrated_wealth, gold_rush, lanes); not profiled further.
3. Unreachable groups (FIX_REACH, bot.cpp:220 phase 4): a group none of whose attack tiles (own range) is walkable from spawn gets excluded_until = 1<<30. Fixes islands (0 gems -> 1,028, wins 2/2).
4. Monolith range (FIX_CAMP_RANGE util.h:9 `P_CAMP_RANGE` 2 = monolith and golem from the API; 1 = all kinds; 0 = off): danger_zone, incoming_damage, micro stand tiles use camp_range(). Elk is kept at 3 on purpose: with 1 (elk 4) collision/eight/expanse h2h dropped (30-game ablation, weak evidence). Elk range 4 handling is open for v5.
5. FIX_MONO_ZONE (util.cpp:135/158, `P_MONO_ZONE_ALWAYS` 1, `P_FIX_MONO_RING` 1): path steps avoid tiles within monolith range; if that leaves no step toward goals outside the zones the walk is retried through it. Stand tiles inside monolith range make a group unwanted (bot.cpp:241). gold_rush: 0/2 -> 2/2 vs starter (deaths 76 -> 36).
6. FIX_SIEGE_OFF (map_contest.cpp:25 P_SIEGE_CONTEST 0): siege cast range 4 is inside monolith range 5; contest deaths 125 -> 17-39, 2/2 vs starter, but 0 gems (default plan farms gold clusters only).
7. FIX_DEATH_LOOP (bot.cpp:494, `P_DEATH_LOOP` 3 = any death at the target group excludes it for P_DEATH_EXCLUDE 500 rounds; 1 = only repeat death within 300 rounds; 2 = death with no enemy champion in view; 0 = off). Dead champions still get play_turn calls (confirmed), so the death is detected there. Mode 3 won most vs starter (broken_curve 2/2, gold_rush) but in the h2h subsets the modes were within noise (7.3 / 7.7 / 6.7 of 14). Generic loop-breaker; it does not fix corridors.
8. Heart of Ruin / enum: no array or switch in the code is indexed by champion type or draft id (grep); M3 tables default to false for id 11. Dash code (enemies.cpp:128,143) uses ability valid_tiles(): no change needed; Heart of Ruin's landing damage is not modelled. Direct me.move_towards only in util.cpp (guarded by a not-blocked goal), so the blocked-target throw cannot be hit there.

## Known failures / not done
- corridors: lost on A and B vs starter (gems near equal, 83-86 deaths, all at y=16, 3 tiles from camps in 3-wide vertical corridors; ranged/queued units stand inside camp range). Not fixed.
- broken_curve/A and gold_rush/B flip between runs with the death mode; contest gives 0 gems.
- Fuel 70k/turn target not met (see 2).
- Per-map plans of the 5 kept maps were not re-validated against the changed (extra camp) layouts beyond the h2h.

## Tests NOT run (for the cloud)
gate.py on the final source (last gate was with death mode 3: 40/42, fuel max 93k); the 126-game h2h v4 vs v3b-e1 on the final switches (Falsifier from the brief: reject if < 50 % or > 3 of 6 lost on a kept map; the earlier h2h lost collision, eight, elkring, expanse, monolith 4+ of 6 with mode 1, partly noise, partly the elk/death-loop switches); panel21 for v3b-e1 and v4; /watch of a loss. Switches to ablate first: P_DEATH_LOOP (0/1/2/3), P_CAMP_RANGE (1/2), P_FIX_MONO_RING, P_MONO_ZONE_ALWAYS.
