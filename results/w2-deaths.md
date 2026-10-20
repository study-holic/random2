# w2: swirl and pilgrimage death loop (builder w2, 18 Oct, 22:21 to 23:00 BST)

## Cause of the deaths

Data: v1 stats JSON for swirl against starter-cpp (`matches/v1-221121/swirl__starter-cpp__{A,B}.json`) and against the
six-bot panel, salt 0 (`matches/v1-222117/swirl__*__s0.json`). Death entries give the killer (`context` camp or
champion), the round and the dead champion's tile.

- **Against starter-cpp most deaths are camp kills, not champion kills**: side A 30 of 35 deaths, side B 46 of 52.
- **They cluster at one corner group and its mirror.** Swirl is 45x30 with 180-degree symmetry. The deaths sit on
  tiles (10..14, 2..5) around (12,3), and on the mirror tiles (29..32, 24..26) around (32,26). Both sides die there, so
  the cause is the map, not one spawn. A second, smaller cluster is the centre camps (21..24, 13..17).
- **The deaths repeat on a fixed period.** Side B against starter-cpp: Vendetta died at (13,4) or (14,3) in rounds
  725, 853, 1034, 1134, 1236, 1337, 1440, 1541 and 1642, one death about every 100 rounds. The cycle is a 60-tick
  respawn, a walk back to the same group, and another death. Issac died at (13,4) or (14,3) on the same period. Each
  camp kill destroys the carried bag (GameRules 361, 365).
- **Cassi & Issac double the count.** They die together (GameRules 428). Every Issac camp death at (13,4) also logs a
  Cassi death wherever Cassi stood, for example (31,7), (24,0) or (25,0). Their 2.0 deaths per event inflate the
  per-game count.
- **Against the panel the split is roughly even** (camp kills 7 to 31 a game, champion kills 10 to 58). The champion
  kills are in the centre, (19..25, 9..16).
- **Pilgrimage is a different problem.** All of its 10.2 deaths a game are champion kills, spread across the map,
  and starter-cpp kills us 0 times there. No camp death loop exists on pilgrimage.

## What I changed

`bots/w2/src/map_swirl.cpp` only:
- `swirl_init` (Plan::init) sets `excluded_until = 1<<30` on every group with a spawn tile within Chebyshev 2 of
  (12,3) or of its mirror (W-13, H-4). `choose_target`, `claim_usable`, `resolve_target` and the cluster and fallback
  paths already skip excluded groups.
- `swirl_target` (Plan::target) is a backstop: it turns an excluded `chosen` into -1, and `farm` then runs
  `fight_any`.

Variants tried: swirl 2, pilgrimage 0 (no change; its deaths are champion fights, out of reach of a map-file
exclusion in the time left).

## Results

Swirl, 6-bot panel, both sides, salts 0,1,2, 4 jobs (`results/w2-deaths.json` against `results/v1-base.json`):

| swirl | wins /12 | strong /4 | deaths/game | median win round | gem margin | faults |
|---|---|---|---|---|---|---|
| v1 base | 6.33 | 1.00 | 52.2 | 1,652 | -35.9 | 0 |
| w2 variant 1 (corner exclusion) | 8.00 | 1.33 | 36.7 | 1,249 | +305.2 | 0 |

`cmpcells`: dW +1.67, dStrong +0.33, dGem +4093, **ADMIT**.

Falsifier check: deaths fell 52.2 to 36.7, a 29.7 per cent cut. The bar was a third, so on the brief's second
falsifier **the death-count criterion fails narrowly**, even though `cmpcells` admits.

Caveat: salt 1 replays salt 0 exactly, in both base and candidate (identical rounds game by game). The run therefore
has 2 independent samples, not 3. Per independent salt: base 7 and 5 wins, candidate 8 and 8.

Against starter-cpp (salt 0, kept stats, `matches/w2-224930/`): deaths 35 to 11 (side A) and 52 to 15 (side B).
No deaths remain at the corner groups. The remaining deaths sit in the central chamber (x 18..26, y 9..17): champion
kills, plus camps 26 and 27 by the spawns.

Pilgrimage: not changed and not re-run. Base 6.67/12, 10.2 deaths a game, median win round 1,230, all deaths to
champions.

Variant 2 (after the lead's watch) kept variant 1 and also excluded gold-only groups (`gems == 0`) with
`dist_enemy <= 8`. Result (`results/w2-deaths-v2.json`): 8.00/12, strong 1.33/4, 36.7 deaths, median win round
1,249, gem margin +305.2, identical game by game to variant 1. No group met the rule, or the bot never targeted one
that did, so it was a no-op. The central-chamber groups that kill us (camps 26 and 27 attack, so they are not pure
gold clusters) evidently carry gems or sit farther than 8 tiles from the enemy spawn centre. I reverted to variant 1.
Next rule to try: exclude by position, groups with any tile in the chamber box x 17..27, y 9..18, outside bank trips.
Dump `b.groups` once in a scratch copy first.

**State left:** `bots/w2` = v1 plus swirl variant 1 (`map_swirl.cpp` only), rebuilt at 22:54. `map_pilgrimage.cpp`
is unchanged. Faults 0 in all 86 w2 games (36 + 36 + 14).

## Remaining loss for the lead to watch

`C:/cambc-solo/matches/w2-224652/swirl__x-foundry-v16__A__s0.repl26a` (variant 1, side A, lost at round 1,181).
Kept stats: `C:/cambc-solo/matches/w2-224930/swirl__x-foundry-v16__A__s0.json`. 51 deaths, 47 of them to champions,
around (21..26, 11..14). The fight is in the central chamber beside the spawns, which matches the lead's watch of v1.
Tooling: `C:/cambc-review/tools/replay/final.mjs` failed with "load:" on the inner replay (engine WASM mismatch,
likely), so the loss analysis rests on the stats JSON.

