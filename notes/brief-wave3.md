# Wave 3 shared brief (lead, 01:10 BST 19 Oct). Every wave-3 builder reads this first.

## Why this wave exists

- Xanarchy's v2 is live and has lost about 96 rating since upload (0-5 to Micron, all five ended on Micron's 1,000 gems at rounds 450-747).
- The local panel (v4.14-era clones) sits at our own level, so panel wins overstated us. **Every test in this wave also runs head-to-head against v2 and reports pace.**
- Pace gap, v2's median win round on the panel vs the top-50 winners' median rounds to 1,000 (`intel/meta-2026-10-18.md` §3a): stampede 1,752 vs 686; swirl 1,249 vs 600; pilgrimage 1,278 vs 844; rushbait 1,006 vs 660; ambush 899 vs 594; monolith 584 vs 447; rows 481 vs 386; eight 734 vs 640; contest 2,000 (no win by gems) vs 1,602. The game is a race; we are slow even unopposed.
- Ladder evidence (a former teammate's report: six changes, "all tested both sides on every map before uploading", and the team then climbed). **Correction 01:20: their changes were made on a different code base that we do not have, not on v2.** A bug they fixed may not exist in our code, and a change that helped them may not transfer. Each item is a hypothesis about the game, not a recipe: first show, with file:line or a replay trace, that the mechanism exists in v2a (the bug, the missed opportunity), then build.
  1. Draft Tavi/Stellar/Ryth/Varg/Ren ("what the top 4 run"); Cassi & Issac first on small maps; Fish's roster on swirl only.
  2. Tavi buys shards in the field.
  3. Monolith play: casters poke from range 4 (monsters only hit at 3); on contest "that's the whole game", 0 to 400-1,000 gems.
  4. Contested rule fixed so gold clusters don't count as an alternative to a gem group ("why we scored 0 on stampede").
  5. An altar lock on every champion, dropped on any altar we pass.
  6. On seeing an enemy mid-tribute: lock, stun or hit it; focus whoever carries the most gems.
  They also reported as neutral or worse: a threat layer, wards everywhere, altar camping, copying an opponent's item build, counter-drafting Marcella.

## Setup

- Base: `cp -r bots/v2a bots/<you>; rm -rf bots/<you>/.cambc; rm -f bots/<you>/src/salt.h`. Build: `"$APPDATA/Python/Python314/Scripts/cambc.exe" build --release <you>` from `C:/cambc-solo`.
- Rules: `docs/rules/GameRules.md`, `docs/rules/BotAPICpp.md`, `sdk/cpp/cambc.h`. **Never invent an SDK method**; quote the rule or header line each mechanic relies on in your report.
- Edit only `bots/<you>/` and your own `results/<you>*` files. Do not touch `maps/` (frozen tonight: six maps are old layouts; the lead is fetching current ones separately), `scripts/`, other bots, or anything on the platform. No uploads.
- Fuel: stay under 80,000 a turn; no new per-turn full-map scans (cache in init); keep `std::endl` out of hot paths.

## Tests (at most 2 game jobs at any moment; seven agents share 13 cores)

1. Panel, 3 salts: `python scripts/mapgate.py <you> <maps|all> --jobs 2 --salts 0,1,2 --out results/<you>.json`, then `python scripts/cmpcells.py results/v2-panel.json results/<you>.json` (base = v2, 3 salts, all 17 maps). Admission: ΔW ≥ 1.5 of 12, ΔGem > 0, ΔStrong ≥ 0 per changed map.
2. **Head-to-head against v2**, 3 salts: `python scripts/mapgate.py <you> <maps> --jobs 2 --salts 0,1,2 --panel v2 --out results/<you>-h2h.json`. Report wins out of 6 per map (2 sides × 3 salts) and gem margin. A change that cannot beat v2 head-to-head on its maps is not admitted, whatever the panel says.
3. Pace: median win round per map from (1), next to the top-50 yardstick above.
4. Safety on the changed maps: `--panel starter-cpp`, both sides: all wins, 0 faults, no new "bot error" lines in `team-a.stdout`/`team-b.stdout`, no `fuelExhausted` after round 2 beyond v2's (corners round ~20 vs x-foundry-v16 is inherited).
5. Trace one loss with `node tools/replay/final.mjs`, `tools/replay/extract.mjs` + `deaths.py` / `timeuse.py`: find a moment where your change fired and say what happened.

Run the cheapest test that can falsify first (one salt on 2-3 target maps), fix, then the full runs.

## Report

`results/<you>-<topic>.md`: what changed (file:line), the rule lines relied on, tables for tests 1-4, the traced moment, the falsifier you fixed before testing and whether it was met, and a verdict (admit on maps X / reject). Stop at **04:30 BST**; report what you have by then, partial or not.
