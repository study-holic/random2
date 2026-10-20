# w4: escape dash (bots/w4, from v2a)

## Which champions dash
GameRules.md kits: slot 1 is a 2-tile dash (can cross holes, cooldown 8, walking compatible) on Issac, Vesper Vale,
Vendetta and Ren. On the tested maps (default `small`/`large` draft lists in draft.cpp) we draft Vendetta first,
Cassi and Issac second, and Ren in the large list. v2a never cast a dash (only stun/damage in enemies.cpp and the
Varg shield in util.cpp).

## Trigger and action
- `Dash::escape(Bot&, const TurnView&, const Champs& enemies)` in `bots/w4/src/dash.h`.
- Fires when: not on our spawn; `b.retreating` (last turn's mode) or health below `P_RETREAT_HURT` (the threshold the
  "low and chased" branch uses with enemies in view); a live enemy champion within 3 tiles (Chebyshev); `can_cast()`;
  the dash ability `is_ready()`. One `abilities()` walk and one `valid_tiles()` call, only when the trigger holds.
- Destination: among `valid_tiles()` minus walls/holes/altars/enemy spawn, visible champions and camps, the tile with
  the lowest walking distance to our spawn (a `distance_field` to `ally_spawn` over `static_block`); ties go to the
  tile farthest from the nearest enemy. Taken only if it is at least 2 steps closer than standing still.
- After a successful dash the turn walks one step home (`move_towards_closest` to `ally_spawn`) and ends
  (the hook returns true). First version continued the turn and hit engine error 97 ("Unit is not in vision") on the
  stale enemy handles right after each dash (seen in the smoke game); fixed before mapgate.
- Flag: `MapPlan::escape_dash` (plan.h, default false), checked in `Plan::turn(Phase::pre)` (plan.cpp) before the map's
  own pre hook. Set in `setup_<map>()`.

## Smoke
stampede vs x-foundry-v16 (first build, with a temporary `DASH <round>` print): dash fired at rounds 124 and 677;
w4 won 1080-0 gems. Two "bot error 97" lines followed the dashes, which led to the end-the-turn fix. Print removed.

## Mapgate (flag ON on contest, stampede, fragment; 108 games, 3 salts, jobs 6, 23:16-23:18 BST)
`py -I scripts/mapgate.py w4 contest stampede fragment --jobs 6 --salts 0,1,2 --out results/w4-dash.json`
(eight and collision were enabled in code but dropped from the run for time.)

| map | base wins (v1-base) | w4 wins | dW | dStrong | dGem | deaths base -> w4 | flt | fuelL | admit |
|---|---|---|---|---|---|---|---|---|---|
| contest | 10.00 | 10.33 | +0.33 | +0.33 | +0 | 20.0 -> 17.2 | 0 | 0 | - |
| fragment | 9.00 | 9.00 | +0.00 | +0.00 | +1542 | 15.3 -> 10.4 | 0 | 0 | - |
| stampede | 9.67 | 10.67 | +1.00 | +0.33 | +1320 | 17.4 -> 13.5 | 0 | 0 | - |
| total | | | +1.33 | | | | | | none |

Admitted maps: none (no map reaches dW >= 1.5). bots/w4 is rebuilt with `escape_dash` off on every map (the setup
lines keep the enabling statement in a comment). Bot behaviour is therefore v2a's.

Signal: deaths fall on all three maps (-14 to -32 per cent) and gem margin rises on two, but wins move by at most one
game of 12. Worth a re-run with 6 salts on stampede (dW +1.0) and a first run on eight and collision before closing.

## Engine error 97 ("Unit is not in vision")
Pre-existing: the v1-base replays on the same three maps carry 227 such lines on our side; w4 carries 289 (+62).
The first build's dash-continue path produced them directly; after the end-the-turn fix the excess may still come
from the dash (e.g. a later handle refresh), or from more fights survived. Not resolved; check before any admit.

## Loss analysis
Planned: stampede vs x-v4-14, side B, salt 0 (lost 360-1080 gems at round 1256, 8 deaths, opponent banked to 1080).
`tools/replay/final.mjs` is not present under C:/cambc-solo/tools (only collect.py, kills_hist.py, mapids_hist.py,
meta.py), so no trace was run. Not watched in the visualiser. Likely cause from the summary alone: a gem race lost on
banking, not deaths (8 deaths is below the map's mean), so the dash is unlikely to be the cause.

## Parameters
Trigger radius 3 tiles; health gate `P_RETREAT_HURT` (0.45) or `retreating`; minimum gain 2 walking steps; one
variant only (no sweep).

## Risks
- Dashing ends the turn (no attack after the dash); a Vendetta that could finish a kill and reset cooldowns instead
  flees.
- Uses last turn's `retreating`; the "low" test ignores the incoming-damage term of `need` in bot.cpp.
- The distance field ignores holes as walls except via `static_block`; dash crosses holes, the field does not model it
  for the walk afterwards (harmless: field only ranks landing tiles).
