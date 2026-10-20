# w6: counter-drafting from the opponent's revealed picks (builder w6, 18-19 Oct, 23:37-01:30 BST)

Base code: `bots/v2a/src` copied to `bots/w6/src` (src only). Card: IDEAS.md v3 counter-draft.

## What was built

- `bots/w6/src/counterdraft.h` (new): `counter::pick(mine, prefs, size, mask, out)`. It reads
  `cambc::draft::current_draft(team::enemy)`, projects our roster from the static per-map list, and returns a
  counter pick at **our last pick only**. Rules and sources are in the header comment and below.
- `bots/w6/src/draft.cpp`: `play_draft` calls `counter::pick` after choosing the static list (v2a's lists; corners
  keeps its own `p.draft`) and before the static loop.
- `bots/w6/src/plan.h`, `plan.cpp`: `MapPlan::counter` (rule bits; default `W6_CD_DEFAULT`) and `Plan::counter()`.
- Map files: see "State left".

## Decision rules (written before the test)

Mechanics: picks are simultaneous and revealed as they go; both teams may take the same champion
(GameRules, Drafting). With no denial, waiting costs nothing, so every rule fires at our last pick, when
team_size - 1 enemy picks are visible, and replaces the static list's last champion.

| Rule | Bit | Fires when (at our last pick) | Pick | Source |
|---|---|---|---|---|
| R1 Stellar parity | 1 | enemy has Stellar, our projected roster has none | Stellar | watch-v2-pilgrimage-2340 (9 of 15 deaths to the enemy Stellar, we had none); top10-by-map (Stellar 94-100% of top-10 games on 13 of 17 maps); kit: 2xF single + 1.4xF area vs Tavi's 2.2xF |
| R2 Stun answer | 2 | team size >= 4, enemy shows >= `W6_CD_STUNS` stunners (A: 3; B, admitted: 2) (C&I, Ryth, Prism, Marcella, Tavi, Ren), no Marcella projected | Marcella | mining/others.md (ELTheo77 main-4 draft.cpp:199 uses 2+); threshold 3 because Marcella's ladder excess is negative on 9 of 11 maps with n >= 30 (rows -27.5 z -3.9, eight -14.8 z -2.8) |
| R3 Ranged-burst floor | 4 | projected roster has no Stellar, Tavi or Prism | Stellar | card; top-10 Stellar share |
| R0 Playability | always | a counter pick is never Willow or Vesper | - | w1 kit check (heal and dashes unused) |

Priority R1 > R2 > R3. Fuel: two `current_draft` calls and short list scans per draft turn; one log line
("cd pick k/n enemy ... static X rule R?") at the last pick, '\n' only.

## Which rule fired (draft log `cd pick ...`, identical across sides A/B and salts 0-2)

Variant A (R2 needs 3 stunners). Smoke: 102 two-round games, every map against every panel bot; the mapgate
bundles show the same rule in all 6 games of each cell.

| Map | size | v4-14 | foundry-v16 | hello | ifish | mite | shawn |
|---|---|---|---|---|---|---|---|
| ambush, contest, eight, pilgrimage, stampede | 5 | - | R1 Stellar for Tavi | R1 | R1 | R1 | R1 |
| elkring, monolith | 5 | - | R1 | R1 | R1 | R1 | - |
| lanes | 5 | - | R1 | R1 | - | R1 | - |
| corners | 5 | - | - | R2 Marcella for Ren | R2 | - | - |
| interlock | 4 | R3 Stellar for Ryth | R1 Stellar for Ryth | R1 | R1 | R3 | R3 |
| rushbait-v2, swirl | 4 | R3 Stellar for Ryth | R1 Stellar for Ryth | R1 | R1 | R1 | R1 |
| collision, expanse, fragment, packs (3), rows (4) | | - | - | - | - | - | - |

Variant B (R2 needs 2 stunners, ELTheo77's rule) adds R2 Marcella in 18 cells: v4-14 on all nine 5-pick maps
(it shows Vendetta, C&I, Varg, Ryth), corners vs mite and shawn, elkring/monolith vs shawn, lanes vs ifish and
shawn, interlock vs mite (in place of R3), rows vs hello and mite. Held-out panel: R1 vs hunter and raider2 on
pilgrimage; R2 vs tung-clone, locker, v5-base on pilgrimage and vs all five on corners.

R3 is not a dead safety net: on 4-pick maps the large list's first four hold no ranged burst, so it fires there.

## Mapgate (salts 0,1,2; panel of six, both sides)

Deviation from the brief: variant A ran on the 12 maps where a rule fires, not `all`. On the other five no rule
fires against any panel bot, so the draft and the game code equal v2a's (= v1's) and the games are identical
(checked: every unchanged cell, e.g. pilgrimage and lanes vs v4-14, matches v1-base in wins, gems, deaths and
round). Variant B re-ran only the 18 changed cells (108 games, `results/w6b-cells.json`) and was merged with A's
results for unchanged cells (`results/w6b-counter.json`). Bot faults 0 in all 432 + 108 + 30 games.

`py -I scripts/cmpcells.py` (base v1-base, except corners and swirl against v2-panel; pilgrimage v2a = v1, so
v1-base is its base). Admit: dW >= 1.5, dGem > 0, dStrong >= 0.

| Map | base | A wins | A dW | A dStrong | A dGem | B wins | B dW | B dStrong | B dGem | Decision |
|---|---|---|---|---|---|---|---|---|---|---|
| ambush | 10.67 | 10.67 | +0.00 | +0.00 | -17 | 11.33 | +0.67 | +0.67 | +403 | off |
| contest | 10.00 | 10.33 | +0.33 | +0.33 | 0 | 10.33 | +0.33 | +0.33 | 0 | off |
| corners (v2) | 6.00 | 7.00 | +1.00 | -0.33 | +535 | 8.00 | +2.00 | +0.67 | +473 | **ON (B)** |
| eight | 8.00 | 9.00 | +1.00 | +0.00 | +1612 | 9.00 | +1.00 | +0.00 | +1712 | off |
| elkring | 9.33 | 8.00 | -1.33 | -1.33 | -972 | 9.00 | -0.33 | -0.33 | -660 | off |
| interlock | 6.67 | 4.33 | -2.33 | -0.33 | -2385 | 4.33 | -2.33 | -0.33 | -2397 | off |
| lanes | 7.67 | 8.67 | +1.00 | -0.67 | +417 | 9.00 | +1.33 | -0.67 | +267 | off |
| monolith | 11.00 | 11.00 | +0.00 | +0.00 | +832 | 10.00 | -1.00 | -1.00 | +713 | off |
| pilgrimage | 6.67 | 8.33 | +1.67 | +0.67 | +1815 | 8.67 | +2.00 | +1.00 | +2324 | **ON (B)** |
| rows | 11.67 | = base | | | | 10.67 | -1.00 | +0.00 | -979 | off |
| rushbait-v2 | 5.00 | 5.00 | +0.00 | +0.33 | -1082 | = A | | | | off |
| stampede | 9.67 | 10.33 | +0.67 | +0.67 | +840 | 10.33 | +0.67 | +0.67 | +1440 | off |
| swirl (v2) | 8.00 | 7.33 | -0.67 | +0.33 | -1023 | = A | | | | off |
| collision, expanse, fragment, packs | = base | | 0 | | | | 0 | | | off |

Totals over the 12 run maps: A dW +1.33, B dW +3.67 (+2.67 including rows, where only B changes cells). Two rule-set variants used (the cap).

Caveats.
- **Corners has one effective sample.** Every w6b corners cell is identical across salts 0-2 (as w1 found for
  w1), so its +2.00 is one sample per cell against a base with salt variation; it meets the single-sample bar
  (2.0) exactly and no more. Gains: hello B (0 to 3/3, R2 from A), mite B, v4-14 B, ifish B; loss: mite A (2/3 to 0).
- **Pilgrimage in v2 routes to v418d**, which scored 10.00/12 on the panel and 9/10 held-out (v2-panel,
  v2-heldout). w6b's 8.67 and 7/10 are below that, so the pilgrimage flag only helps a build that plays
  pilgrimage with the v2a code.
- **4-pick maps:** Stellar for Ryth (R1/R3) costs games (interlock -2.33; swirl -0.67): Ryth is the only tank
  stun there.
- **Caught "bot error" lines rise with Stellar** (engine error 97, "Unit is not in vision", caught in main.cpp;
  each costs that champion the rest of its turn). Per 36 games, base vs A: ambush 16 vs 62, swirl 23 vs 97,
  stampede 46 vs 74, pilgrimage 16 vs 27. A bug in the casting path, outside this card's files; not fixed.
- lanes fuel_late 144 is pre-existing (12 per cell, also in cells identical to the base).

## Held-out check (tung-clone, hunter, raider2, locker, v5-base; 1 salt; both sides)

| Map | base | A | B | Cells changed (B) |
|---|---|---|---|---|
| pilgrimage (base v1-heldout) | 7/10, gemD 506 | 7/10, gemD 495 | 7/10, gemD 525 | hunter A lost (R1), raider2 B won (R1) |
| corners (base v2-heldout) | 10/10, gemD 732 | not run | 10/10, gemD 525 | none flipped; deaths up vs hunter B (10 to 23) and raider2 (2-5 to 13-16) |

R4 (no changed map loses more held-out games than its base) passes on both maps. Corners' held-out gem margin
fell by 2,070 summed, and Marcella-for-Ren raised deaths against the stun-heavy hunter and raider2, which is
the opposite of the rule's intended effect; treat corners as a marginal admission.

## Loss analysis: pilgrimage vs x-shawn, side A, salt 1 (base won 3/3; w6 lost this salt)

Re-run with statistics (`C:/Users/willi/AppData/Local/Temp/claude/C--Users-willi-OneDrive-Desktop-cambc-solo/a868a45e-2afb-4da7-93b5-574919305849/scratchpad/final/loss_w6s1_shawn_A.json`; base `.../final/base_v1s1_shawn_A.json`, same directory). Replay from mapgate: `matches/w6-235235/pilgrimage__x-shawn__A__s1.repl26a`.
Rule R1 swapped Tavi for Stellar (shawn shows C&I, Varg, Stellar, Vendetta). w6 lost 449 to 1,032 at round 1,132;
the base won 1,039 to 607 at round 1,218.
- Our deaths: 13 (12 of them credited to the **enemy Stellar**: Cassi and Issac 4 times each, linked deaths,
  plus Varg 2, our Stellar, Vendetta, Ryth); base 8 (5 Stellar, 3 Tavi).
- Deaths cluster at x 21-23, y 8-14 (rounds 291-659); our Stellar died first (round 291) and C&I followed 13 rounds later.
- Likely cause: the swap drops Tavi's 2.2 x Focus stun (range 4), one of our two stuns, and our code does not
  focus the enemy Stellar, so "Stellar parity" gives no answer to her. She kills the fragile, linked C&I pair
  repeatedly and snowballs. The counter rule raised damage but removed the control that kept the base alive.
  Not watched in the visualiser (time); the analysis rests on the stats JSON.

## Parameters

`W6_CD_STUNS` 2 (variant A 3); last pick only; stunners = C&I, Ryth, Prism, Marcella, Tavi, Ren; bursts =
Stellar, Tavi, Prism; rule priority R1 > R2 > R3; `MapPlan::counter = CD_ALL` (7) on admitted maps.

## State left

- `bots/w6` built (debug and release): `W6_CD_DEFAULT 0u` in plan.h; `p.counter = counter::CD_ALL` only in
  `map_corners.cpp` and `map_pilgrimage.cpp`; `W6_CD_STUNS 2`. A two-round check on all 17 maps against
  x-v4-14 and x-foundry-v16 logs a rule only on corners and pilgrimage (R2 vs v4-14; R1 vs foundry on pilgrimage).
- `bots/w6b` (+ `-s1`, `-s2`): variant B with the flag on every map, used for the B runs; delete or ignore.
- `bots/w6-s1`, `w6-s2` still hold variant A (threshold 3, all maps on).
- Results: `results/w6-counter.json` (A), `results/w6b-cells.json`, `results/w6b-counter.json` (B merged),
  `results/w6-heldout.json` (A, pilgrimage), `results/w6b-heldout.json` (B, pilgrimage and corners).

Falsifier check: two maps admitted, and neither loses held-out games, so the card's falsifier is not met. The
margin is thin (corners rests on one effective sample, and on both maps the result is "no worse" held-out).
