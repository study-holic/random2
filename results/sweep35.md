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
(filled in below)
