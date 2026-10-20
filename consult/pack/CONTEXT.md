# Team Xanarchy, Cambridge Battlecode 2026, Sprint 1: complete context for an outside reviewer

Written 23:05 BST, 18 October 2026, by the team's lead agent. This file is self-contained: the full game rules (`GameRules.md`) and a signature index of the C++ Bot API (`api-index.md`, 629 entries generated from the official `BotAPICpp.md`) are appended below it. Everything here is a fact or a measurement unless it is labelled as an interpretation.

## 1. Competition and stakes

- Cambridge Battlecode 2026 (event `cambc2026a`, 12 October to 18 November 2026). Each team writes a bot; ours is C++ compiled to WebAssembly.
- **Sprint 1** is a separate tournament, not the ladder: single elimination, 15 games a match, open to all eligible teams, and seeded from the ladder. The qualification snapshot is at 22:59Z = **23:59 BST tonight**. Source: `https://2026a.battlecode.co.uk/api/v1/tournaments`.
- Team Xanarchy: one person (Xans) plus AI coding agents. The team has **0 submissions and a 1,500 rating**. Xans's rule: **no upload until a version passes every release test**, and Xans approves each upload.
- Timeline from now: the code freeze is about 23:15, the planned single upload about 23:35, and the hard stop 23:50 BST.

## 2. The game in brief (full rules appended)

- **Draft.** The 11 champions are Ryth, Vesper Vale, Vendetta, Cassi & Issac (one pick, two bodies), Stellar, Prism, Marcella, Willow, Ren, Varg and Tavi. There are 3, 4 or 5 picks per team depending on the map. The map is fully visible during the draft.
- **Objective.** Tribute 1,000 gems first. Otherwise the team with more tributed gems at round 2,000 wins. The tiebreaks are lifetime gold earned, then kills, then Team B.
- **Camps.**
  - Types: wolves, Elks, gem clusters, gold clusters and monoliths (20,000 health, 200 Strength, range 3; 200 gems, 300 gold, 150-round respawn). Gold groups respawn after 50 rounds.
  - Capture: kill every monster in a cardinally connected group, then occupy a spawn tile uncontested for 5 rounds. Any enemy present resets the timer, and any occupied spawn tile delays the group's respawn.
- **Tribute.**
  - A tribute at an altar is a channel of max(1, ceil(gems/10)) of the owner's turns. Damage does not interrupt it; a stun, death or a new altar lock does.
  - A lock costs 10 gold and is placed within 2 tiles of an altar. It blocks the opponents' tributes there, and 3 basic attacks clear it.
- **Deaths and gold.** Carried gems pass to the killer on death; a kill pays 50 gold and the victim respawns after 60 rounds. Gold buys items (shards and emblems) on the spawn tiles.
- **Fuel.** Each champion is its own process with 100,000 fuel a turn (2× on its first turn, 10× on the first draft turn). Running out suspends execution, and it resumes from the same point next round.
- **Engine.** Engine 0.0.3, CLI cambc 2.2.5. Games are deterministic.

## 3. Maps (17 current; ladder data 9,245 ranked games, 09:20Z to 19:13Z 18 Oct)

| Map | Size | Champions per team | Top-50 winners' median rounds to 1,000 | Class (pre-patch kills/game) | Local file |
|---|---|---|---|---|---|
| ambush | 100×100 | 5 | 594 | mixed (4.3) | current |
| collision | 41×41 | 3 | 695 | n/a | **old layout** |
| contest | 50×50 | 5 | 1,602 (55 % of games reach round 2,000) | contact (56.3) | current |
| corners | 71×71 | 5 | 536 | n/a | **old layout** |
| eight | 41×30 | 5 | 640 | contact (23.1) | current |
| elkring | 30×30 | 5 | 577 | contact (14.7) | current |
| expanse | 80×80 | 3 | 513 | n/a | **old layout** |
| fragment | 30×30 | 3 | 772 | contact (19.2) | current |
| interlock | 40×40 | 4 | 663 | n/a | **old layout** |
| lanes | 90×30 | 5 | 468 | n/a | **old layout** |
| monolith | 50×50 | 5 | 447 | mixed (5.3) | current |
| packs | 31×31 | 3 | 528 | n/a | **old layout** |
| pilgrimage | 50×65 | 5 | 844 | contact (15.2) | current |
| rows | 50×50 | 4 | 386 | race (0.4) | current |
| rushbait_v2 | 30×80 | 4 | 660 | mixed (5.3) | current |
| stampede | 60×50 | 5 | 686 | contact (22.5) | current |
| swirl | 45×30 | 4 | 600 | contact (26.0) | current |

Six maps were re-laid at 11:40Z on 16 October. Only the old layouts exist locally (the current files need a platform sign-in not yet done), so every local result on those six is on a layout the tournament will not use.

Ladder champion signals (rating-adjusted win excess, all teams; |z| ≥ 3 shown): stampede Prism +11.3 points (n 312), ambush Prism −25.0 (n 88), rows Willow +18.6 (n 84), rows Marcella −27.5 (n 49), fragment Varg −8.1 (n 436). Of 177 champion-map cells, 16 have |z| ≥ 2 where about 8 are expected by chance. Top-10 teams pick Stellar in about 100 % of games on most maps.

## 4. Our code

**Lineage.** v4.14 and v4.15 are an inherited C++ bot line (about 2,000 lines). The modules:

| File | Role |
|---|---|
| `bot.cpp` | Turn flow: shopping, retreat and recall, bank at a bag of about 36 gems, altar choice |
| `squads.cpp` | Squads and target choice; the squad claim lives in shared memory |
| `micro.cpp` | Farming and squad micro: shield, casts any ready damage or stun ability on camps or enemies |
| `enemies.cpp` | Enemy response |
| `threat.cpp` | Flee and threat layer |
| `cluster.cpp` | Cluster-first opening |
| `stuck.cpp` | Stuck and contested handling |
| `rows.cpp` | Rows wave relay (v4.15 only) |

- **Draft.** Fixed lists: 5 picks Vendetta, C&I, Varg, Ryth, Tavi (then Ren, Marcella, Stellar); 3 picks Vendetta, C&I, Stellar; rows C&I, Varg, Tavi, Stellar.
- **Unused kits.** The bot never casts Willow's heal or any dash, and has no Prism-specific play.
- **Hook layer (added tonight).**
  - `mapid.h` identifies each map from its dimensions and camp counts, with old and new layouts told apart.
  - `plan.h` defines a MapPlan per map: draft override, init, pre-turn, mid-turn, target override and buy hooks. Each map has a `map_<map>.cpp`; an empty one leaves v4.15's behaviour unchanged.
- **v418d.** A later candidate from the same line with large changes: 890 changed lines in `bot.cpp`, a new `runner.cpp`, and a new draft.
- **Dispatcher (v2's structure).** v4.15+hooks (A) and v418d (B) are compiled into one WASM, each wrapped in its own C++ namespace. `main()` identifies the map once per process (one `camp_groups()` call, about 12,000 to 20,000 fuel) and routes the draft and every turn to A or B.

## 5. Test harness and the noise problem

- **Panel.** Six bots: an older own version (v4.14), another team's public bot (foundry-v16, roughly level with v4.14), and four public clones of top-ranked teams' policies (fidelity about 0.85). Each candidate plays every panel bot on all 17 maps from both sides: **204 games, about 2 minutes on 13 cores**.
- **Held-out panel.** Five older archetype bots (a farmer clone, a hunter, a raider, a locker, a stack bot): 170 games.
- **Determinism and fuel chaos.** A rerun is identical. But the bot's behaviour depends on fuel remaining, so a change that only spends about 15,000 extra fuel on turn 1 flipped 15 of 204 results (+9 −6); 10 maps stayed identical and 5 moved.
- **Salts.** Noise samples come from salted builds that burn s × 4,000 loop iterations on turn 1. Every comparison averages salts 0, 1 and 2 (612 panel games).
- **Admission rule** (`cmpcells.py`, fixed before any candidate ran). Per map: ΔW = Σ over cells of (candidate win rate − base win rate), in games of 12. Admit when ΔW ≥ 1.5, the summed gem-margin change is > 0, and there is no fall against the two strongest panel bots.

**Release tests (fixed at 22:15, before candidates ran).**

| Test | Condition |
|---|---|
| R1 | 34/34 against the starter bot, 0 bot faults |
| R2 | No `fuelExhausted` after round 2. Opening suspensions on rounds 1 and 2 are reported but allowed, pending Xans's confirmation |
| R3 | Mean panel wins over 3 salts ≥ the base's; no map that passed "7/12 with 2/4 against the strong pair" for the base fails it; every changed map admitted |
| R4 | Held-out total ≥ base − 2; no changed map loses more held-out games than the base |
| R5 | Reviewer verdict "proceed", plus one watched loss |

## 6. Results so far

| Bot | Panel wins /204 | Notes |
|---|---|---|
| v4.14 | 143 (1 sample) | 34/34 vs starter; fuel suspensions on rounds 1 and 2 on 9 of 17 maps (initialisation on large maps); swirl 43.5 deaths a game even vs the starter bot |
| v4.15 | 147 (1 sample) | +4 −0 flips vs v4.14, 3 of them on a map where the no-op change also scored +3 |
| v418d | 152 (1 sample) | +17 −8 flips vs v4.14; worse against the two strongest bots on contest, elkring, fragment and stampede |
| microstella-next (another line) | 147 (1 sample) | |
| **v1 = v4.15 + hooks (base)** | **142.7 (3 salts: 146, 141, 141)** | Passes 9 maps (ambush, contest, elkring, expanse, fragment, monolith, packs, rows, stampede). Held-out 139/170, 0 late fuel suspensions in held-out games. One late suspension (Issac, round 20) on corners vs foundry-v16, side B |

**Builder results against the base (3 salts each).**

| Change | Map | Base → candidate wins /12 | ΔW | ΔStrong | ΔGem | Status |
|---|---|---|---|---|---|---|
| Draft C&I, Stellar, Varg, Ryth, Ren | corners (old layout) | 3.67 → 6.00 | +2.33 | 0 | +1,762 | Admitted. All 3 salts gave identical games, so effectively one sample |
| Draft C&I, Stellar, Ren | collision (old) | 6.67 → 7.00 | +0.33 | 0 | +39 | Rejected |
| Draft with Stellar for Vendetta | eight | 8.00 → 6.00 | −2.00 | 0 | −2,772 | Rejected |
| Draft with Stellar for Vendetta | pilgrimage | 6.67 → 7.00 | +0.33 | +0.33 | +665 | Rejected |
| Swirl: permanently skip the corner camp group near (12,3) and its mirror | swirl | 6.33 → 8.00 | +1.67 | +0.33 | +4,093 | Admitted. Deaths/game 52.2 → 36.7 (−29.7 %, below the pre-set "cut by a third" falsifier); vs the starter bot deaths fell about 69 % |
| Route to v418d | pilgrimage | 6.67 → 10.00 | +3.33 | +2.00 | +2,609 | Admitted |
| Route to v418d | collision (old) | 6.67 → 9.00 | +2.33 | +0.67 | +1,301 | Admitted on the old layout only; the current layout stays on v4.15 |
| Route to v418d | eight / lanes / interlock | | +1.00 / +0.67 / −0.33 | | | Rejected (lanes: v418d hit the fuel limit 146 times in 36 games) |

Watched loss (v1 on swirl vs foundry-v16). Both spawns sit inside the central chamber, 4 to 6 tiles apart. v1 kept fighting the central gold camps beside the enemy spawn, and at round 292 had 0 gold and 0 gems against the opponent's 190 gems and 1,620 gold. 39 deaths in total, almost all in the chamber.

**Release candidate v2** = dispatcher over A (v4.15 + hooks + corners draft + swirl exclusion) and B (v418d), routing pilgrimage and old-layout collision to B.

RELEASE_RESULTS_PLACEHOLDER

## 7. Known issues and deviations

1. **R2 wording.** CLAUDE.md's original safety test said "no turn over 100,000 fuel". Every version suspends on rounds 1 and 2 on large maps, and the base has one suspension at round 20 on corners.
2. **Old layouts.** Six maps are old layouts locally; the corners draft was admitted on the old corners layout.
3. **Swirl.** The change missed its own pre-set death falsifier (−29.7 % against a −33 % bar) but passed the admission rule.
4. **Selection.** Three builders tested 1 to 2 variants on 4 to 5 maps each, so some admissions are expected by chance. The held-out panel is the only independent check.
5. **Engine errors.** The replays log caught engine errors (codes 97, 98, 22) at similar rates for v1 and the dispatcher (for example, 16 vs 26 across 34 starter games). They come from both bots' own code; none is a fault.
