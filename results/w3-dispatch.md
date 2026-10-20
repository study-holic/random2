# w3: per-map dispatcher over v1 (A) and b-v418d (B)

Builder w3, 18 Oct 22:21 to 23:00 BST. Status: complete. All three brief tests ran; final route table built into bots/w3 at 22:54.

## How the namespacing works

- `bots/w3/gen_dispatch.py` (run `py -I bots/w3/gen_dispatch.py`) regenerates `bots/w3/src/a/` from `bots/v1/src`
  and `bots/w3/src/b/` from `bots/b-v418d/src`. Neither source copy is touched.
- Each `.cpp` gets the union of that bot's system includes (`<cambc.h>`, `<vector>`, ... 11 for A, 12 for B, plus
  `<iostream>`) hoisted to global scope at the top, then the whole original file, local `#include "..."` lines
  included, wrapped in `namespace va { ... }` (A) or `namespace vb { ... }` (B). Headers are copied unchanged; they
  are only included inside the namespace, and their own system includes are no-ops there because the guards /
  `#pragma once` already fired at global scope. Macros (P_* params) stay per translation unit, so A and B never see
  each other's params.
- Each bot's `main.cpp` is dropped. A generated `entry.cpp` per bot defines `dispatch_init()` (heap `new Bot()`),
  `dispatch_draft()` (`play_draft()`) and `dispatch_turn()` (`Bot::play_turn()`) in that bot's namespace.
- `bots/w3/src/main.cpp` includes `a/mapid.h` inside `namespace va` (the same inline `va::mapid::current_map()`
  that A's own code uses, so the cached camp_groups() result is shared with A), calls `w3_route_b()` from
  `src/route.h` once per process before the loop, constructs only the chosen bot, and keeps the original loop:
  try/catch with `bot error:` logging, then `cambc::end_turn()`.
- Salt: main.cpp includes `salt.h` when present (mapgate writes it to `src/`) and burns `P_SALT*4000` iterations
  of a volatile sink once, on the first play turn, before the chosen bot's turn. A's `plan.cpp` copy has its salt
  block disabled (`#if __has_include("salt.h")` replaced by `#if 0`), so P_SALT is 0 inside A (it could not see
  `src/salt.h` from `src/a/` anyway). B had no salt code.
- Only edit to the copies: that one line of A's plan.cpp. No edits to B.

## Route table

Final table, built into `bots/w3` (debug and release) at 22:54, in `bots/w3/src/route.h`:

| map ids | bot | cmpcells v1-base (3 salts) vs w3-dispatch (3 salts) | verdict |
|---|---|---|---|
| pilgrimage | B | base 6.67, cand 10.00, dW +3.33, dStrong +2.00, dGem +2609 | ADMIT |
| collision_old, collision_v2 | B | base 6.67, cand 9.00, dW +2.33, dStrong +0.67, dGem +1301 | ADMIT |
| eight | A | base 8.00, cand 9.00, dW +1.00, dStrong 0.00, dGem +548 | not admitted (dW < 1.5) |
| lanes_old, lanes_v2 | A | base 7.67, cand 8.33, dW +0.67, dStrong -0.33, dGem +71 | not admitted; B also hit fuel exhaustion 146 times (fuelL) |
| interlock_old, interlock_v2 | A | base 6.67, cand 6.33, dW -0.33, dStrong +0.67, dGem -1045 | not admitted |
| all others, unknown | A | not tested on B (v1 code path) | default |

Salt 0 alone had all five maps ahead (total dW +9.33; interlock +1.33, lanes +1.33). Salts 1 and 2 took
interlock and lanes back to about zero, which fits the 7% no-op flip rate in the brief: single-salt panel results
overstate. Total over the five maps at 3 salts: dW +7.00, of which pilgrimage and collision give +5.67.

## Tests

### 1. Identity (A path), `results/w3-identity.json`, salt 0, panel of six, 22:43

Routing checked from the replays: A prints `map <name>` once per process; B never does. Every rows and swirl game
shows the A marker.

| map | v1-base (3 salts) | w3 (salt 0) | dW | dStrong | dGem |
|---|---|---|---|---|---|
| rows | 11.67/12 | 12/12 | +0.33 | 0 | +348 |
| swirl | 6.33/12 | 7/12 | +0.67 | 0 | +1447 |

Within the noise band (a no-op flips about 7% of cells). The A path behaves as v1.

### 3. Safety, starter-cpp panel, all 17 maps (first run on the initial table, 22:45)

34/34 wins, flt 0 on every map, fuelL 0. Replay markers confirm the routes: B on collision, eight, interlock,
lanes, pilgrimage; A on the other 12. Caught `bot error:` lines (each ends that unit's turn, no crash): 26 across
the 34 games, all engine errors 97 (unit not in vision), 98 (no visible entity on tile) and 22 (known occupied
destination). v1's own starter run (`results/v1-starter.json`) logged 16 of the same three kinds, and v1's panel
run logged 10 on swirl alone, so these come from the bots' code, not from the dispatcher. On the B maps:
collision 6, interlock 2, eight 1.

Rerun on the FINAL route table, 22:54: 34/34 wins, 0 faults, fuelL 0; markers show B on collision and
pilgrimage only, A on the other 15 maps; 26 caught engine errors of the same three kinds.

### 2. Routed, `results/w3-dispatch.json`, salts 0,1,2, panel of six, 22:48 (initial table: B on 5 maps)

| map | wins | strong | gemD | flt | fuelL | gate |
|---|---|---|---|---|---|---|
| pilgrimage | 10.0/12 | 3.0/4 | 272.9 | 0 | 0 | PASS |
| interlock | 6.33/12 | 2.0/4 | -88.2 | 0 | 0 | fail |
| lanes | 8.33/12 | 1.33/4 | 202.5 | 0 | 146 | fail |
| collision | 9.0/12 | 1.67/4 | 199.3 | 0 | 0 | fail |
| eight | 9.0/12 | 1.0/4 | 308.2 | 0 | 0 | fail |

v1-base on the same maps (3 salts): pilgrimage 6.67 (1.0 strong), interlock 6.67 (1.33), lanes 7.67 (1.67),
collision 6.67 (1.0), eight 8.0 (1.0). cmpcells verdicts are in the route table above. The salted copies
`bots/w3-s1`, `bots/w3-s2` were pre-built by me with mapgate's own copy recipe, so its rebuild only recompiled
main.cpp; mapgate rewrites them on each run.

### Loss analysis (one loss)

`tools/replay/final.mjs` does not exist in this repository (tools/ holds only collect.py, kills_hist.py,
mapids_hist.py, meta.py), so I read the statistics JSON for
`matches/w3-224840/interlock__x-foundry-v16__B__s0` (B route, w3 as team B). x-foundry-v16 ran five
champions (Cassi, Issac, Varg, Stellar, Vendetta) against B's four (Vendetta, Varg, Ryth, Tavi). B lost Ryth at
round 49 at (26,20), and by round 145 had three deaths in the central band x 23 to 29, y 13 to 22. Kills 4,
deaths 7. Foundry banked steadily (202 gems by round 122, 868 by round 467) while B banked in two lumps (160 by
round 171, 320 by round 358); final 1006 to 436 at round 501. Likely cause: being one champion short in the
mid contest. B gave up the centre early and could not bank between fights. This fits interlock not being
admitted for B. No replay was watched (/watch is outside this brief and time).

## Binary size

Release `bot.wasm`: w3 3,470,026 bytes; v1 3,342,742; b-v418d 3,371,988. The second bot adds about 127 KB
(most of each binary is libc++ and the SDK, shared once). Release build of w3 took about 8 minutes on the loaded
machine (44 objects), against roughly half that for one bot.

## Risks

- Fuel: on B maps main.cpp pays one camp_groups() call (12k to 20k fuel, first turn of each process) that B
  does not reuse (B calls camp_groups() itself in bot.cpp:109 and draft.cpp:33). On A maps there is no extra cost:
  A's plan code reuses the same `va::mapid` cache. The B-route numbers therefore include a small first-turn
  perturbation, about one salt's worth.
- Static init: both bots' namespace-scope statics are constructed at start-up even though one bot runs; a grep
  found no namespace-scope initialiser that calls the cambc API (all are constexpr or type aliases). Only the
  chosen Bot is constructed (heap), as `Bot bot;` was before the loop in each original main.
- Exceptions: a throw inside either bot unwinds to main's catch as before. mapid's identification catches its own
  failures and returns `unknown`, which routes to A.
- Map identification: route keys are mapid ids; `unknown` goes to A. collision, interlock and lanes route both
  the `_old` ids (the local map files) and the `_v2` ids (the ladder layouts) the same way, so the ladder layouts
  inherit a decision measured on old layouts.
- Maintenance: a change to v1 or b-v418d needs `gen_dispatch.py` rerun and a rebuild of w3.
- Fuel on lanes: B on lanes ran out of fuel 146 times over 36 games (A: 0). One more reason lanes stays on A.
- The win-rate gains for pilgrimage and collision are measured on the local map files. Collision is an OLD layout
  (collision_old); the ladder's collision_v2 is routed to B without a test on it.

## Parameters

No tunables changed. Salt burn: P_SALT*4000 volatile-sink iterations on the first play turn (the same as v1's).
Route rule: cmpcells ADMIT (dW >= 1.5 of 12, dGem > 0, dStrong >= 0, 3 salts against 3).
