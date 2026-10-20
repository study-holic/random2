# Replay decoding on engine 1.0.0

Why `tools/v3/decode.mjs` rejects 1.0.0 replays (found 20 Oct):
1. Header check. Replay bytes 8..10 are `0,0,patch` for engine 0.0.x but `1,0,0` for 1.0.0, so `bytes.subarray(8,10).some(b => b !== 0)` throws "unsupported engine header [1,0,0]".
2. Engine cache. `data/v3/engine/cache2_2_5` (from CLI 2.2.5) holds only v0_0_1..3 wasm; the 1.0.0 wasm ships with the CLI 3.0.1 visualiser.

Fix (already in place, tools/top10): `tools/top10/decode.mjs` accepts header `1,0,0` (patch code 100), loads `v1_0_0-*.wasm` from `data/top10/engine` (also holds v0_0_1..4 and v0_1_0), and its header-decoder regex covers the new snapshot ABI. Use it for any engine: `import { openEngine, openReplay, replayBytes } from 'tools/top10/decode.mjs'`; `tools/top10/extract_pilot.mjs` is the extractor.

Verified 20 Oct against `cambc matches summary` totals:
- Own 1.0.0 game (`cambc run`, 1792 rounds): decoder submittedGems [1009, 391] = statistics JSON [1009, 391].
- Ladder replay `data/top10/dl/00c7d730-...zip`: decoder [369, 1010] = `cambc matches summary` [369, 1010].
- Seek to the end with `R.seek(R.size)`, not `R.size - 1` (the latter stops one input early: 988 instead of 1010).

Summary path (no decoder, fastest for totals): `cambc matches summary <matchId> --json` writes `match-<id>.json` into the current directory (run it from a scratch dir; it refuses to overwrite). Fields: `teams[].submittedGems`, `events` (death, kill, fuelExhausted, ...), `championTimelines` (earnedGold ...), `match` (engineVersion, outcome, roundsStarted), `diagnostics.botFaults`. `scripts/mapgate.py` reads the same format from `cambc run-batch --statistics`.

Per-turn fuel: each replay bundle holds `team-a.fuel` / `team-b.fuel` (magic `a62leuf\0`, 5 header bytes, then u64 values: 8 limits, 5 draft turns, then one value per living champion per round). `scripts/mapgate.py` reads them (`fuel_max`, `fuel_max_r1`).
