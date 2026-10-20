# w5: in-game death-loop breaker (builder w5, 18 Oct, 23:04 to 23:30 BST)

## What I built

Copy: `bots/w5` = `bots/v2a/src` plus the following (src only).

- `bots/w5/src/loopbreak.h`, `bots/w5/src/loopbreak.cpp` (new module, namespace `LoopBreak`):
  - `observe_camps(const Bot&, const TurnView&)`: on each alive turn, right after `build_view`, records the group whose
    live damaging camps (wolf, Elk, Monolith) can reach our tile (spawn tile within range 3 + 1), the group with the
    largest summed `camp_strength`; falls back to `holding`. Stamps the round.
  - `observe_foes(view, enemies)`: stamps the last round an enemy champion was within `P_LB_FOE_RADIUS` (7) tiles.
  - `on_dead(Bot&)`: on the first dead turn (alive-to-dead transition), counts a **camp death** at the recorded group
    when the camp picture is at most `P_LB_STALE` (6) rounds old and no enemy champion was within 7 tiles in the last
    `P_LB_FOE_MEMORY` (8) rounds. After `P_LB_K` (2) camp deaths at one group within `P_LB_WINDOW` (500) rounds, sets
    `groups[g].excluded_until = max(existing, round + P_LB_N)` (N = 300), drops `target` if it is g, and clears the
    count (the next exclusion needs K fresh deaths).
  - `enabled()`: caches `Plan::current().loop_breaker`; `mark_alive()`.
  - State is file-static in `loopbreak.cpp`, not in `Bot` or `GroupInfo` (bot.h notes that `GroupInfo` size changes fuel
    use, EXP-082).
- `bots/w5/src/plan.h`: `MapPlan::loop_breaker` (default false).
- `bots/w5/src/bot.cpp` (`play_turn_inner`): four guarded calls: `on_dead` in the dead branch, `mark_alive` at the start
  of the alive branch, `observe_camps` after `build_view`, `observe_foes` after `update_contested`.
- Map files: `setup_<map>` sets `p.loop_breaker = true` during a test. Final state: off on every map (none admitted).

Shared memory: all 8 bytes are used (0..5 champion targets, 6..7 squad claims; GameRules: 8 bytes a team), so each
champion process learns from its own deaths only. The squad claim is not cleared on exclusion (the other members keep
farming the claimed group; only the champion that died twice leaves it).

## Tests

Panel x-v4-14, x-foundry-v16, x-hello, x-ifish, x-mite, x-shawn; both sides; salts 0,1,2; 6 jobs; base
`results/v1-base.json` (v2a plays these maps as v1). Two variants, the brief's cap.

- **Variant 1** (the card: camp deaths only, K 2, N 300, window 500, foe radius 7, foe memory 8, stale 6):
  stampede, fragment, eight, collision (contest left out to save time; games ran faster than estimated).
  `results/w5-loop.json`, `results/w5-loop.txt`, 144 games in 97 s.
- **Variant 2** (`P_LB_ANY 1`: champion kills count too; with no damaging camp in reach, a death within 6 tiles of the
  target counts at the target; other parameters unchanged): the same four maps plus contest.
  `results/w5-loop-v2.json`, `results/w5-loop-v2.txt`, 180 games in 123 s.

| map | base W/12 | v1 W | v1 dW | v1 dGem | v2 W | v2 dW | v2 dStrong | v2 dGem | deaths base / v1 / v2 | verdict |
|---|---|---|---|---|---|---|---|---|---|---|
| stampede | 9.67 | 9.67 | +0.00 | +0 | 8.33 | -1.33 | -0.67 | -2420 | 17.4 / 17.4 / 17.8 | reject |
| fragment | 9.00 | 9.00 | +0.00 | -188 | 9.00 | +0.00 | +0.00 | -82 | 15.3 / 14.8 / 13.6 | reject |
| eight | 8.00 | 8.00 | +0.00 | +0 | 8.00 | +0.00 | +0.00 | +22 | 9.7 / 9.7 / 9.5 | reject |
| collision | 6.67 | 6.67 | +0.00 | +0 | 6.67 | +0.00 | +0.00 | +23 | 5.1 / 5.1 / 5.2 | reject |
| contest | 10.00 | not run | | | 10.33 | +0.33 | +0.33 | +0 | 20.0 / - / 19.4 | reject |

cmpcells admitted no map in either variant. Faults (flt) 0 and fuelL 0 on every map in both runs (base fuelL is not
in v1-base.txt; 0 cannot exceed it).

Smoke check: our `team-*.stdout` in the first w5 run (`matches/w5-231511/`) shows `bot error: engine error 97: Unit is
not in vision.` once or twice in 51 of 69 games scanned, on all four maps. It is not from this module: another agent's
v2 run at 23:15 (`matches/v2-231503/`, collision) shows it in 12 of 12 games, and mapgate counts no fault. Someone
should trace it in the v2a line (a handle read after a kill; `keep_visible` covers allies and enemies only after
`enemy_response`).

## Why variant 1 is a no-op here: the camp death loop does not exist on these maps

Diagnostic run (variant 1, salt 0, `--keep`, `matches/w5-231714/`), our deaths by killer `context` in the stats JSON:

| map | champion kills | camp kills |
|---|---|---|
| stampede | 180 | 0 |
| contest | 260 | 0 |
| fragment | 169 | 8 |
| eight | 112 | 4 |

Against the panel, 96 to 100 per cent of our deaths are champion kills, so the camp-death trigger (2 camp deaths at one
group) almost never fires: variant 1 played stampede, eight and collision game for game like the base, and fragment
changed in a few games (deaths 15.3 to 14.8, gems -188). The swirl loop (w2: 30 of 35 and 46 of 52 deaths to camps
against starter-cpp) is a swirl property, already handled by the hand fix there.

## Loss analysis (variant 2)

`matches/w5-231939/stampede__x-ifish__B__s1.repl26a` (base won this cell 3/3; variant 2 lost salts 1 and 2).
`final.mjs` (C:/cambc-review/tools/replay) fails with "load:" as it did for w2, so I re-ran the same game with
statistics (`bots/w5-s1` still held the variant 2 build): x-ifish submitted 1,080 gems, we submitted **0**. Our 19
deaths are all champion kills, spread over the south (y 30..44), and only one dropped a full bag (360 gems, round 936).
Likely cause: variant 2 treats a lost fight at a group as a reason to leave it, so after two such deaths each champion
abandons its farm group for 300 rounds and walks to farther or contested groups, where it is caught again before it
fills a bag. Excluding a group after champion kills hands the area to the opponent; that is the opposite of what
stampede needs.

## Parameters

K 2, N 300, window 500 rounds, foe radius 7, foe memory 8 rounds, stale 6 rounds; variant 2 adds `P_LB_ANY 1` with a
6-tile target radius. All are `#ifndef` macros in `bots/w5/src/loopbreak.h`.

## State left

`bots/w5` built (release) with `loop_breaker` off on every map and `P_LB_ANY 0` (variant 1 code in place, inert).
With the flag off, the only change from v2a is a cached-bool check four times a turn, so fuel-dependent paths may differ
slightly from v2a; not measured.

## Risks and next steps

- Per-champion learning only (no free shared-memory byte): each champion must die twice itself before it avoids a group.
- Not clearing the squad claim keeps squads together but leaves the dead champion's mates on the deadly group.
- The module is worth a test only where camp kills dominate deaths: swirl (already fixed by hand) and possibly
  maps not tested here (ambush, elkring, packs, lanes with 1 to 4 deaths a game are unlikely). Check the camp/champion
  split with `--keep` stats before spending games.
- Variant 2 is a loss on stampede (-1.33 W, -2420 gems); do not enable champion-kill counting.
