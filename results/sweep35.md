# Sweep35: reliability on all 35 maps, the fuel limit, and a progress watchdog (cloud, 20 Oct evening)

Brief: tools/cloud/sweep35.md. Engine 1.0.0, CLI 3.0.1, 35 maps in `maps/`, `--jobs 3`. Each bot-opponent cell is 35 maps x 2 sides x seeds 0-2 = 210 games.

Scripts:
- `tools/cloud/stalls.mjs`: stalls from the replays.
- `tools/cloud/sweep35.py`: per-game rows, from the statistics JSON and `team-?.fuel`.
- `tools/cloud/sweep35_report.py`: the tables.
- `tools/cloud/wd_admission.py`: the admission lines.

Data:
- `results/sweep35/rows.json`: one row per game.
- `results/sweep35/tables-main.md`: per-map tables for v4, v5 and s1g1.
- `results/sweep35/*-vs-*.json`: the mapgate outputs.

**Definitions.**
- **Deaths by killer:** from the `death` events in the statistics JSON (owner = our champion; target kind camp = monster, champion = champion kill).
- **Fuel:** per-turn values from `team-?.fuel`, after round 1.
- **Stall:** a run of 20 or more rounds in which an alive champion's position, gems, gold, health and kills are all unchanged. Any channel, capture, fight, heal or purchase changes one of these within 20 rounds. A wait at a respawning camp longer than 20 rounds counts as a stall.
- **Altar stall:** a carrier (gems > 0) within 3 tiles of an altar for 20 or more rounds whose gems never fall.

**Seed caveat.** Seeds 1 and 2 burn 20k or 40k extra fuel on the bot's first turn (mapgate salt). In every s1g1-derived build (s1g1 and the first watchdog builds), S1's `s1.h` includes `../salt.h`. That revives a second burn in `Plan::init` (plan.cpp:94), which then lands in the last init stage on a 100k turn. So s1g1's turns at 100,000 at seeds 1-2 (all but 14-23 of about 1,000) are a measurement artifact. Fuel results below therefore use seed 0, which matches a ladder build.

## 1. Sweep: v4, v5, s1g1

| bot | opp | wins /210 | gems a game | deaths a game: monster / champion | seed-0 fuel peak | seed-0 turns >= 90k | turns at 100k, seed 0 / all | stall champion-rounds a game | altar stall a game | zero-gem games |
|---|---|---|---|---|---|---|---|---|---|---|
| s1g1 | idle | 210 | 749 | 5.8 / 0.0 | 100,000 | 109 | 14 / 347 | 545 | 45 | 19 |
| s1g1 | starter-cpp | 207 | 802 | 6.5 / 1.6 | 100,000 | 17 | 2 / 323 | 110 | 37 | 16 |
| s1g1 | v4 | 130 | 692 | 6.1 / 17.5 | 100,000 | 15 | 7 / 326 | 83 | 25 | 14 |
| v5 | idle | 207 | 716 | 5.4 / 0.0 | 100,000 | 25 | 1 / 3 | 1,025 | 79 | 23 |
| v5 | starter-cpp | 202 | 769 | 6.0 / 1.4 | 93,816 | 5 | 0 / 6 | 611 | 28 | 23 |
| v5 | v4 | 121 | 683 | 6.0 / 13.9 | 97,100 | 4 | 0 / 1 | 555 | 24 | 21 |
| v4 | idle | 209 | 709 | 6.1 / 0.0 | 100,000 | 218 | 4 / 18 | 1,061 | 31 | 24 |
| v4 | starter-cpp | 199 | 771 | 7.0 / 1.9 | 90,697 | 3 | 0 / 0 | 539 | 33 | 21 |
| v4 | v4 | 118 | 667 | 6.6 / 17.9 | 87,716 | 0 | 0 / 12 | 483 | 25 | 23 |

Per-map tables (gems, stalls and fuel per opponent; deaths by killer vs v4) and the full flagged list are in `results/sweep35/tables-main.md`.

### Flagged map-bot pairs
**Zero-gem games (of 18 per map: 3 opponents x 2 sides x 3 seeds).** These maps are won or lost on the gold tiebreak.

| bot | boss_battle | contest | shortcut | stampede | other |
|---|---|---|---|---|---|
| v4 | 18 | 18 | 18 | 14 | – |
| v5 | 18 | 18 | 18 | 10 | rushbait_v2 3 |
| s1g1 | 18 | 18 | 1 | 11 | elk_arena 1 |

**Games with a stall over 200 champion-rounds (of 18).**

| bot | maps with 9 or more such games | fewer |
|---|---|---|
| v4 | big_world, boss_battle, contest, gold_rush, packs, rows, shortcut (18 each), split 13, islands 10, rushbait_v2 9 | pilgrimage 8, reverse_spiral 8 |
| v5 | boss_battle, contest, gold_rush, rows, shortcut (18 each), reverse_spiral 15, islands 14, packs 14, stampede 14, rushbait_v2 12, big_world 11 | pilgrimage 8 |
| s1g1 | big_world 18, rows 18, islands 9, rushbait_v2 9 | reverse_spiral 8, elk_arena 6, gold_rush 5, broken_curve 4 |

**Longest stalls.** Each was traced in a replay.

| map | bots | worst stall | cause |
|---|---|---|---|
| rushbait_v2 | v4 and s1g1 | 1,999 rounds from round 1 | Cassi, Issac and Varg never leave the spawn. The enemy spawn is within 5 tiles. The bot gives them no target, and they spend every turn attacking idle enemies on the enemy spawn, which heal every turn. |
| reverse_spiral | both | 1,912 from round 88 | Cassi stands on the spawn all game. Her target flips between a group and none. |
| gold_rush, broken_curve, elk_arena (vs idle) | v4 and s1g1 | 1,500-1,900 | not traced |
| rows | all bots | – | Many 20-60-round waits at wolf packs (respawn 40). These are deliberate respawn waits. |

**Turns at or over 90,000 after round 1, seed 0 (games of 18; with 100k counts).**

| bot | maps |
|---|---|
| v4 | islands 3 (217 turns over the 6 seed-0 games; 4 at 100k), yellow_brick_road 2, lanes 1 |
| v5 | lanes 3, islands 2, starvation 2, concentrated_wealth 1 (one 100k turn), corners 1, fracture 1, starfish 1 |
| s1g1 | split 6, starvation 6 (20 turns at 100k), islands 2 (12 at 100k), yellow_brick_road 2, lanes 2 (2 at 100k) |

**Round 1** (limit 200,000) is close to the limit:
- s1g1: 199,036 on monolith and 192k on shortcut;
- v4: 186,807;
- v5: 184,343.

### Fuel cases: code paths
Found with the S1 fuel stamps (`P_S1_FUELDBG`; stage costs within `play_turn_inner`, s1g1 copy vs idle).

| map (side) | turn total | stage that dominates | path |
|---|---|---|---|
| islands (B) | 90-94k | `update_contested`, 32-48k | stuck.cpp:33-38: the `P_THREAT_LAYER` loop runs enemies x groups x `min_distance` over every spawn tile, with no distance prefilter (S1's prefilter covers only the second loop, and only with the auction on) |
| yellow_brick_road (A) | 98.7k | `update_contested` 43k + `farm` 25k | same |
| starvation (A) | 96-98k | `bank` about 73k, or `farm` about 77k | util.cpp `move_towards_closest`: one path attempt costs about 30k on this 80x80 maze. When the first attempt fails, the wrapper retries up to twice more (NSB walk-out, then zone skip), at 62-72k per call (measured). |
| lanes (B) | 91-97k | `bank` 36k + `resolve_target` 20k | the same move retries, plus target choice |
| shortcut (A) | 88-92k | `bank` 22k, retreat/threat 15k, `resolve_target` 10k | spread over the turn |
| corners (A) | 100k once (round 234) | inside `farm` with about 70k left | not resolved: the debug prints shift fuel and it stops reproducing |
| round 1, all maps | up to 199k of 200k | `Bot::init` phases | each staged-init phase checks a fixed fuel floor (22-70k) that is below the phase's own cost on large maps |

## 2. Arena: past 100,000 fuel
Full table: `results/arena-1020.md`, arena 7; one line in FACTS.md. Probe `bots/fuelprobe` vs idle on elkring. Every champion steps once, then burns a set amount; on rounds 50 and 1.

- **Actions are kept.** Actions taken before the limit are applied.
- **The bot is not removed.** No botFault, and the game runs to round 2,000.
- **Execution resumes later from the same point.**
- **Freeze length:** the champion is frozen for about 10 rounds, plus about 1 round per 10,000 fuel of overrun. Measured: 1k over gives 11 rounds, 10k gives 12, 50k gives 16. Each frozen turn is recorded at 100,000 with one fuelExhausted event.
- **Round 1 (limit 200,000) behaves the same.**
- **Implication:** every turn at 100,000 in the tables above is about 10 rounds of a champion standing still.

## 3. Progress watchdog (`P_W_WATCHDOG`) and fuel fixes (`P_W_FUELCAP`) in bots/wd (= bots/s1g1 + both)

### Admission (fixed before testing; baseline bots/s1g1; `tools/cloud/wd_admission.py`; 630 games each)
| line | rule | result | verdict |
|---|---|---|---|
| 1 | stall champion-rounds fall by at least 50 % over the sweep | 154,911 -> 81,335 (-47 %) | **FAIL** |
| 2 | no map more than 10 % slower unopposed (tributed gems per round vs idle, mean of 6) | none slower | PASS |
| 3 | shortcut won on both sides vs starter-cpp, seeds 0-2 | 6/6 (A 260-14, 124-10, 124-10; B 153-5, 268-20, 268-20) | PASS |
| 4 | peak fuel at most 90,000 on all 35 maps | seed 0: one turn at 100,000 (corners, side A vs idle, round 234); every other map at most 87,922 | **FAIL** |
| 5 | self-play vs v4 on the 21 old maps >= 62/126 | 75/126 (s1g1 74) | PASS |

**Line 1, more detail.** By seed: -49 %, -48 %, -44 %. By opponent:
- vs idle: 114,452 -> 38,936 (-66 %);
- vs starter-cpp: +5 %;
- vs v4: +4 %.

Against real opponents the stall total is dominated by rows (about 32k of the 81k), where champions wait 20-60 rounds at wolf packs for respawn (wolf respawn 40). That is deliberate respawn waiting, not a fault, and the watchdog does not touch it.

**Line 4, more detail.** One turn at 100,000 in the 210 seed-0 games: corners, round 234, champion 2, inside `farm()` with about 70k left. It stops reproducing once debug prints are added, because they shift fuel, so the exact branch is not named. Over all 630 games:
- turns at 100,000: 2, against about 1,000 for s1g1;
- turns >= 90,000: 15, against about 1,540 for s1g1. All but the corners turn are at seeds 1-2: lanes, starvation, starfish, shortcut and gold_rush, 90-100k.

**Other results.**
- Wins 542/630 (s1g1 547); gems 765 a game (s1g1 747).
- Deaths vs v4: 5.9 monster and 18.1 champion kills a game (s1g1 6.1 / 17.5).
- Zero-gem games remain on boss_battle, contest and stampede, where the game is won on gold.
- Shortcut now scores 124-268 gems.

### Watchdog model (wd.cpp)
Lyapunov-style progress certificate (Khalil, *Nonlinear Systems*, ch. 4), run as a watchdog timer; source: Copilot, consult 1946.

**Potential Phi, by objective:**
- banking: walking distance to the altar, from the altar table (below), or Chebyshev distance until the table exists;
- travelling (target more than 4 tiles away): Chebyshev distance to the target group's spawn tiles;
- clearing (within 4): remaining HP of the group's visible camps.

**Progress event:** any of these:
- Phi falls below its best;
- gems or gold change;
- health rises on our spawn;
- an enemy champion within 5 tiles, not on its own spawn, with blows exchanged (we attacked or our health changed);
- a known respawn wait of at most 30 rounds;
- a step onto a tile not among the last 6 positions.

**Trigger and escalation.** The stall counter fires after W = 12 rounds (travel), 20 (clearing) or 10 (within 3 tiles of the altar). One level per trigger, reset by progress:
1. Soft hazards for W rounds: Monolith zones, the siege block and threat detours are dropped from pathing.
2. Release the objective and block it for 60 rounds (`excluded_until`, `altar_skip_until`).
3. Drop the bank decision and force the nearest open non-Monolith group for 60 rounds, ignoring claims. The watchdog walks the champion there itself, until within 4 tiles or until hit.
4. Walk to our spawn for 8 rounds.

Cost: about 2k fuel a turn; skipped below 20k fuel left.

**Fixes found by testing.** Each is commented in wd.cpp.
- **Shortcut side B scored 0.** Chebyshev Phi does not fall on a detour round a wall, so the watchdog released the altar for every carrier. Fix: a fresh tile counts as progress, and the altar potential uses walking distance.
- **Rushbait_v2.** Champions attacking idle enemies on the enemy spawn counted as fighting forever. Fix: an enemy on its own spawn is no fight. Level 3 now also walks to the forced group, because the threat layer had held the champions in place before target choice.
- **Reverse_spiral.** A target flipping between a group and none reset the timer every few rounds. Fix: an objective change no longer counts as progress.

### Fuel fixes (`P_W_FUELCAP`, from the stamps in section 1)
| fix | file:line | why |
|---|---|---|
| distance prefilter in `update_contested`; the second loop's prefilter now also applies with the auction off (same result) | stuck.cpp:33-60 | 32-48k on islands and yellow_brick_road |
| altar tables: walking distance to each altar's submit tiles, one built per turn while 75k is left at the end of the turn; `bank()` steps by lookup | wd.cpp `wd_tables_step`, `wd_step_altar`; bot.cpp `bank()` | `bank()` cost 36-73k a turn. A full field costs 13k on starvation and 24k on big_world, plus about the same to export (bots/fcost). |
| move retries only while the fuel left covers another attempt plus 20k | util.cpp `move_towards_closest` | three attempts at about 30k each on starvation |
| a move is skipped (the champion waits a turn) below max(45k, dearest attempt this game + 15k), capped at 60k | util.cpp `move_towards_closest` | the last big spend of a turn, in `farm()` and `bank()` |
| the play turn after init needs 100k (was 80k); each init phase needs min(floor + 30k, 90k) | bot.cpp `play_turn`, bot.h `init_fuel_ok` | round 1 reached 199,036 of 200,000 (monolith); it now peaks at 156k |
| duplicate seed burn in `Plan::init` disabled | plan.cpp:94 | test artifact (seed caveat above) |

A tried fix that failed: stepping with the SDK's `move_towards` when a field would not fit. It builds a full field in our own fuel (cambc.h:5194) and made starvation worse (765 turns >= 90k in a debug run), so it was removed.

## What still fails
- Line 1: -47 % against the -50 % target. The remainder is mostly rows respawn waits, plus 7-11 long stalls on reverse_spiral, islands and rushbait_v2.
- Line 4: one 100,000 turn on corners at seed 0, with the branch not identified. At seeds 1-2, 90-100k turns remain on lanes, starvation, starfish, shortcut and gold_rush.
- Not done: /watch in the visualiser (no browser in the cloud).
