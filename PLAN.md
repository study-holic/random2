# Sprint 1 plan v1 (lead, 22:25 BST 18 Oct; replaces draft v0)

Root: `C:/cambc-solo` (the OneDrive folder's docs were copied here at 21:44; the OneDrive copy is stale).
v0 review: `notes/eval-plan-v0.md` (critic, "proceed with named changes"). Each change from v0 names its reason. Consultant prompts on this plan: `consult/2225-plan-v1.md`.

## Facts this plan rests on

| Fact | Source |
|---|---|
| We are team **Xanarchy**: 1,500 rating, 0 submissions. MicroStella rows in `intel/` are another team's; the critic's "near 1,429" was MicroStella's and is void for us | Xans, 21:58 |
| **Decision 1: Sprint 1 is a separate tournament.** Single elimination, 15 games a match, all eligible teams (`topPercent` 100), seeded from the ladder (`seedingMode` ladder, 60-minute seeding window). Qualification snapshot `2026-10-18T22:59Z` = **23:59 BST**; seeding ends 23:59Z | `https://2026a.battlecode.co.uk/api/v1/tournaments`, fetched 22:03 |
| cambc CLI 2.2.5 (upgraded from 2.1.4), engine 0.0.3 | stats JSON `match.engineVersion` |
| **114 games a minute** at 13 jobs (204-game gate in 107 s); about 50 a minute while builds share the CPU | `results/v1.txt`, `results/b-*.txt` |
| Games are **deterministic** (a rerun is identical) **and fuel-chaotic**: v4.14 with no-op hooks (one extra first-turn `camp_groups()` call) flipped **+9 −6 of 204 cells**, on expanse (+3), lanes (+4 −2), corners, rushbait-v2; 10 maps identical. One-sample comparisons therefore admit a no-op change on 2 of 17 maps | `results/v1-hooks.json` vs `results/v1.json` |
| Maps: 11 of 17 current. collision, corners, expanse, interlock, lanes, packs are the **old** layouts (re-laid 16 Oct 11:40Z; `cambc maps pull` needs Xans's sign-in). `mapid.h` tells old and v2 apart; map files apply to both | md5 of map files; `intel/meta-2026-10-18.md` §2 |

Tournament consequence: an earlier upload buys only ladder seeding. **One upload, when a version passes every release test, with Xans's approval; target 23:35, hard stop 23:50.** After uploading, confirm `mySubmission` through the API.

## Baselines (panel x-v4-14, x-foundry-v16, x-hello, x-ifish, x-mite, x-shawn; 17 maps × 6 × 2 sides = 204 games, one sample)

| Bot | Wins | Maps passing 7/12 + 2/4 strong | Flips vs v4.14 |
|---|---|---|---|
| v4.14 | 143 | 9 | |
| v4.15 | 147 | 9 | +4 −0 (expanse +3, which the no-op also produced; rows +1) |
| v418d | 152 | 9 | +17 −8; vs v4.15: pilgrimage +4, interlock +2, lanes +2, collision +1, eight +1; loses to the strong pair on contest, elkring, fragment, stampede |
| microstella-next | 147 | 10 | +6 −2 |
| v4.14 vs starter-cpp | 34/34, 0 faults | | swirl 43.5 deaths a game even against starter-cpp |

**Base decision (22:20): v1 = v4.15 + hooks.** Reason: it never loses a cell to v4.14 and differs only on rows and expanse; v418d's gains are mixed by map, so it enters per map through the w3 dispatcher instead. Old v1 (v4.14 + hooks) kept as `bots/v1-414`. The base ensemble `results/v1-base.json` (3 salts, 612 games) is the reference for every comparison.

## Release tests (fixed 22:15, before any candidate ran; all must pass before an upload)

| Test | Pass condition | Games |
|---|---|---|
| R1 Safety | Against starter-cpp on 17 maps, both sides: 34 wins, 0 bot faults, 0 engine diagnostic codes, no "bot error" in stdout | 34 |
| R2 Fuel | No `fuelExhausted` event after round 2 in the R1 and R3 games (mapgate `fuelL` column). Opening suspensions on rounds 1 and 2 are counted and reported but allowed: the engine resumes a suspended bot next round (GameRules §Fuel), and v4.14 already suspends there on 9 of 17 maps (initialisation on large maps). **Xans to confirm this reading of CLAUDE.md's "no turn over 100,000 fuel"** | from R1, R3 |
| R3 Panel, no regression | Mean wins over 3 salts ≥ the base's; no map that passes the 7/12 gate for the base fails it; each changed map admitted by the changed-cell rule | 612 |
| R4 Held-out | tung-clone, hunter, raider2, locker, v5-base, 1 salt: total wins ≥ the base's − 2; no changed map loses more held-out games than the base | 170 |
| R5 Review | Critic verdict "proceed" on the candidate; one loss of the candidate watched in the visualiser (/watch) | |

The absolute per-map gate (7/12 with 2/4 against the strong pair) is reported but is **not** a release blocker: a coin-flip bot passes a map 34.9 % of the time and a 70 % bot passes all 17 only 5 % of the time (critic §1). v0's "target 17 of 17" is dropped. The base itself passes R3 and R4 by definition, so **the fallback upload is the base, if it passes R1, R2 and R5.**

**Changed-cell rule** (replaces v0's "+2 panel wins"), in `scripts/cmpcells.py`: per cell, the win rate and gem margin averaged over salts (salt s burns s × 4,000 loop iterations of first-turn fuel, giving another trajectory of the same bot). Admit a map's change when ΔW ≥ 1.5 games of 12, summed gem margin change > 0, and no fall against x-v4-14 and x-foundry-v16. With one sample the bar is ΔW ≥ 2. Reason: unchanged-trajectory cells carry no information, a no-op passed the one-sample rule on 2 maps, and best-of-several selection promotes a null change 61 % of the time (critic §1); R4 is the held-out check against that selection.

## Build wave (critic: one wave, 2 to 3 cards)

22:25 to 23:00, three builders, 4 game jobs each.

| Builder | Card | Mechanism and evidence | Maps | Falsifier (fixed 22:25) |
|---|---|---|---|---|
| w1 | Per-map drafts (IDEAS 11, refreshed) | `MapPlan::draft`. 18 Oct excess: stampede +Prism (z +4.0), ambush −Prism (−4.7), fragment −Varg (−3.4), rows +Willow (+3.4) and −Marcella (−3.9), pilgrimage −Vendetta (−2.8), collision −Varg (−2.2) (`notes/top10-by-map.md`). Builder checks that the code plays each champion's kit | all 17 | Not admitted by cmpcells on a map → that map keeps v4.15's draft. At most 2 variants a map |
| w2 | Swirl and pilgrimage death loop (IDEAS 13) | Swirl 45.8 deaths a game against the panel and 43.5 against starter-cpp: camps, not champions, kill us. Pilgrimage 10.2 deaths, median win round 1,208 vs top-10 692 | swirl, pilgrimage | Swirl deaths not cut by a third, or not admitted |
| w3 | Per-map dispatcher: v4.15 and v418d in one binary, routed by `mapid` | v418d beats v4.15 on pilgrimage, interlock, lanes, collision, eight (one sample) and loses elsewhere | routed maps | Not admitted with 3 salts → route that map to v4.15; starter check 34/34 |

Changed from v0, with reasons: the altar-lock tribute interrupt (v0 idea 1) moves to "if a builder finishes early". It is rule-backed (GameRules: a new lock cancels a channelling tribute), but it needs new state and has no local evidence, while the dispatcher reuses measured gains. Dropped: contest monolith chip (lost 6 of 6; v4.15 already wins contest 11/12 on gold), carrier hunting, capture denial, counter-draft, endgame all-in, bush ambush, crystals, fight evaluator, Willow heal. Each needs a fight model or state beyond 30 minutes. The EOQ bag cap (cross-domain C3) is the first parameter test if time remains (TUNING.md).

## Timeline (BST)

| Time | Lead | Agents | Xans |
|---|---|---|---|
| 22:25 | Plan v1; /consult prompts | w1, w2, w3 (stop 23:00) | Paste the prompts; sign in for `cambc maps pull` if possible |
| 22:30 to 23:00 | Base ensemble; fuel probe; held-out baseline of the base; /watch a v1 swirl loss; consultant answers | | Paste the answers |
| 23:00 | Integrate admitted per-map changes into v2 (VERSIONS.md) | | |
| 23:05 | R1 to R4 on v2 (about 816 games, 8 minutes at 13 jobs) | critic on v2 (R5) | |
| 23:20 | /watch one v2 loss; decision | | |
| 23:30 | Ask Xans to approve the upload of v2, or the base if v2 fails | | Approve |
| 23:35 | `cambc submit`; confirm `mySubmission` | | |
| 23:50 | Hard stop for any upload | | |

Freeze: no code after 23:15.

## Decision log

| Time | Decision | Evidence |
|---|---|---|
| 21:43 | cambc 2.1.4 → 2.2.5 | Cloud measurements used 2.2.5 |
| 21:44 | Root moved to `C:/cambc-solo`; infrastructure rebuilt (none of the cloud session's v1, mapgate or panel existed here) | Directory listing |
| 21:46 | Panel: x-v4-14 (16 Oct snapshot), x-foundry-v16 (Tarot main), four clones (Tarot `factory/clone-v3b`, 15 Oct 15:52Z); held-out tung-clone, hunter, raider2, locker, v5-base. All build | `logs/build-*.log` |
| 22:03 | Decision 1: tournament; one upload after the release tests | API record above |
| 22:15 | v0 gate replaced by R1 to R5 and the changed-cell rule; wave 2 and card quotas cut | `notes/eval-plan-v0.md` |
| 22:20 | Base = v4.15 + hooks | Baselines table |
| 22:22 | Salted ensembles (3 salts) for all comparisons | No-op flips +9 −6 |
| 22:25 | w3 = dispatcher (replaces the altar lock as a card) | v418d per-map flips |
| 23:00 | Wave 1 integrated as v2: corners draft, swirl exclusion, pilgrimage→v418d (collision_v2 stays on A) | `results/w1-drafts.md`, `w2-deaths.md`, `w3-dispatch.md` |
| 23:03 | Wave 2 at Xans's request: escape dash (w4) and death-loop breaker (w5), stop 23:30 | Xans 23:02 |
| 23:14 | Old-collision route removed (R4 per-map clause; not on the tournament layout) | `results/v2-heldout-cmp.txt`, critic `notes/eval-v2-rc.md` |
| 23:20 | Salted fuel overruns (expanse, lanes) shown to be the salt burn itself (rounds 1 to 3); the real build has one inherited round-20 overrun on corners | `results/fuelchk/` |
| 23:28 | Wave 2 admitted nothing; v2 is final | `results/w4-dash.md`, `results/w5-loop.md` |
| 23:33 | Xans approved uploading v2 now (R1 caught-error lines and the R2 round-20 overrun accepted as inherited); consultants: critic and Copilot "v2", DeepSeek "v2 minus corners" | `consult/2305-release-full.md` |
| 23:33 | v2 uploaded through the Submissions page (platform "v1") | platform page |
| 23:45 | v2 compiled on the platform (status Active / Live) and is the Sprint 1 entry; first ranked series lost 0–5 to Micron | Submissions page; `/api/v1/match-groups/23e14536-…` |
| 23:46 | v3 program opened (`PLAN-v3.md`): data scope, routing simulation and counter-draft agents running to 01:30; v3 consultation sent to ChatGPT, Copilot and DeepSeek (`consult/2340-v3-program.md`) | Xans 23:36 |
| 00:20 (19 Oct) | Agent cap lifted for v3; overnight work authorised | Xans ("use that to your hearts content", "keep going overnight") |
| 00:25 | CLI signed in (Xans approved in the browser; 7 days) | `logs/login2.log` |
| 01:20 | Wave 3 briefs corrected: the former teammate's six changes were on a different code base; hypotheses only | Xans 01:15 |
| 01:25 | Yardstick changed: panel wins no longer decide; pace vs the top-50, head-to-head vs v2, and real teams (ladder/unranked) do | ladder 10/50 vs panel 151.7/204 |
| 17:50 | Root causes R1–R6 (`PLAN-v3.md`); w8 retargeted to damage-only builds, w13 started on target scoring, w11 stopped | decoded ladder games, builder reports |
| 22:20 (19 Oct) | Session saved: resume from `NEXT.md`. Old workspace archived and recycled; current maps for the six old layouts recovered to `maps-1016/` | Xans ("save everything", clean-up request) |
