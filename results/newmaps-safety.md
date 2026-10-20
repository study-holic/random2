# New-pool map rebuild and safety gate (20 Oct, v3b)

## Map rebuild (job 1, done)
Method: ladder replay (`cambc matches download`) holds the engine's own map file at byte 14 of inner replay.repl26a (header `a62per` + 8 bytes, then `w62pam...`). Loaded with MapBuilder_FromFileBytes (engine 0.0.3 WASM), re-encoded: byte-identical to the embedded bytes for all 10 (so no field is guessed). Written to maps/<name>.map26a; sources kept at data/newmaps/dl/<name>/r.repl26a.
| map | size | champs/team | sym | camp groups | spawn A |
|---|---|---|---|---|---|
| big_world | 100x100 | 5 | 2 | 12 | 78,11-82,15 |
| boss_battle | 50x41 | 5 | 1 | 16 | 22,0-27,2 |
| broken_curve | 40x40 | 3 | 2 | 11 | 26,5-29,8 |
| concentrated_wealth | 61x61 | 5 | 2 | 6 | 1,29-4,32 |
| corridors | 51x40 | 5 | 1 | 12 | 24,0-27,3 |
| elk_arena | 50x50 | 5 | 2 | 15 | 12,23-16,27 |
| fracture | 61x71 | 5 | 2 | 14 | 1,1-5,5 |
| gold_rush | 31x31 | 3 | 2 | 8 | 1,16-4,19 |
| hooks | 31x31 | 4 | 2 | 6 | 1,1-4,4 |
| islands | 60x60 | 5 | 2 | 23 | 28,3-32,6 |
Limit: the replays are engine 1.0.0 (platform) / header 0.1.0; our WASMs are 0.0.1-0.0.3, so round-0 state could not be decoded independently. Map content is the embedded file itself.
Stale kept maps: re-extracting the 11 kept maps showed local walls/holes/altars/spawns identical, but **expanse had 3 champs locally, 5 in the current file** and **monolith had 11 camp groups locally, 10 now**. Both replaced from replays (old copies in maps-retired/expanse-old3champs, monolith-old). The other 9 kept maps match (camp counts differ only by half-vs-full camp storage; groups equal).

## Pool (job 2, done)
maps/ now holds exactly 21 files (`mapgate all` globs maps/*.map26a). packs, pilgrimage, rows, rushbait-v2, stampede, swirl are in maps-retired/.

## identify() (job 3)
WRONG (new map identified as old map, old per-map plan fires):
- big_world -> ambush; broken_curve -> interlock_old; gold_rush -> packs_old; hooks -> packs_v2.
Unknown (default plan): boss_battle, concentrated_wealth, corridors, elk_arena, fracture, islands. No new map routes to the B bot (only pilgrimage does).

## Safety gate (job 4), v3b vs starter-cpp, 21 maps, both sides, salt 0 (results/v3b-safety-21.json; expanse and monolith rerun after the fix, 4/4)
37/42 wins. Crashes/faults: 0 on all maps. Losses: boss_battle B (2000-round draw-out), corridors B (2000), gold_rush A and B (both lost, gems 614/46 vs 1012; packs_old plan), islands B (2000).
Fuel (fuelExhausted events, our team): no crash, but on big maps init spills over budget: rounds 1-3 (islands up to 4) for all champions on big_world, boss_battle, islands (old maps: round 1 only, ambush included); elk_arena also has one champion out of fuel at round 19 (mid-game turn over 100k). Corridors: init-only. 

Baseline (job 5) NOT run: stopped at the fuel finding. Cleanup: chk/ and probe bot zz-id sent to Recycle Bin.
