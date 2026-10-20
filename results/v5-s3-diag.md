# v5 S3 diagnostics: v4's gem funnel, deaths, locks and time use (20 Oct)

Data: the 90 real top-10 games (`data/real/v4-panel/v4top-t02..t10-A/B`, ranks 2-10, sets A and B, engine 1.0.0), decoded at stride 1 with `tools/top10/extract_pilot.mjs`; our side from `data/unranked/matches.jsonl`. Self-play: `results/s3-v4-vs-v3b-e1-s0.json` (v4 vs v3b-e1, 21 maps x 2 sides, salt 0, 42 games, 32-10). All numbers come from `tools/cloud/metrics.py` (field definitions are in its docstring). It reproduces `notes/panel-v4-top10-AB.md`: gems 159 / 943, deaths 22.7 / 6.0, gems lost to kills 62, W-L 1-89, and the win flag agrees with the platform result in 90/90 games.

Commands:
```
python3 -I tools/cloud/metrics.py --panel <dir of decoded v4top-*.json> --out f.json [--flip]   # --flip measures the opponent
python3 -I tools/cloud/metrics.py --mapgate results/<x>.json --out f.json                        # decodes the replays itself (3 node jobs)
```

## 1. Gem funnel per game (mean; the residual is 0 in 88 of 90 top-10 games and in every self-play game: every gem is traced)

| set | n | acquired from camps | gained from kills | tributed | destroyed by monster deaths | lost to champion kills | carried at the end | conversion (tributed / acquired) |
|---|---|---|---|---|---|---|---|---|
| v4 vs the top 10 | 90 | **269** | 3 | 159 | 15 | 62 | 35 | 59 % |
| top 10 vs v4 | 90 | **1,112** | 63 | 943 | 89 | 4 | 140 | 85 % |
| v4 vs v3b-e1 (self-play) | 42 | 791 | 60 | 744 | 14 | 66 | 30 | 94 % |
| v3b-e1 vs v4 | 42 | 566 | 62 | 527 | 11 | 64 | 27 | 93 % |

**ChatGPT's test.** Against the top 10, v4 acquires 269 gems a game: about half the 550 in the "production" branch and far from the 950 in the "conversion" branch. **The problem is production.** Conversion is weaker too (59 % against 94 % in self-play), but if v4 converted at the top teams' 85 % it would still bank only 229. The 62 gems lost to kills are 23 % of what v4 acquires. Self-play does not show the production gap: v4 acquires 791 there.

## 2. v4's deaths against the top 10, by cause (step0 method: champion if an enemy action hit the victim in the death round or the one before; Cassi and Issac share fate)

| cause | per game | share | at spawn | at an altar (<= 8) | at a camp (<= 8) | elsewhere | rounds 0-200 | 200-600 | 600+ | gems lost per game |
|---|---|---|---|---|---|---|---|---|---|---|
| champion | 18.5 | **81 %** | 2.9 | 8.0 | 4.0 | 3.6 | 1.8 | 5.9 | 10.7 | 62.2 |
| monster | 3.8 | 17 % | 0.9 | 1.1 | 1.9 | 0.0 | 0.8 | 1.6 | 1.4 | 14.2 |
| other | 0.3 | 1 % | | | | | | | | 0.6 |
| all | 22.7 | | 3.8 (17 %) | 9.1 (40 %) | 5.9 (26 %) | 3.8 (17 %) | 2.7 (12 %) | 7.6 (34 %) | 12.4 (54 %) | |

- **Claude's deciding fact holds:** 81 % of v4's deaths against the top 10 are champion kills, so fights rise in priority. The top 10 lose 6.0 champions a game, mostly to monsters (4.1 monster, 0.5 champion, 1.4 other).
- 78 % of the champion-killed v4 champions carried no gems (1,299 of 1,667). Only 95 carried 20 or more. A kill therefore costs us mostly the 60-turn respawn, not the bag: dead is 22 % of v4's champion-rounds against the top teams' 5 %.
- Killers: C&I 334, Heart of Ruin 298, Ren 298, Stellar 285, Ryth 159, Varg 95, Tavi 88, Prism 40.
- Per map, champion deaths per game: contest 47.9, gold_rush 30.2, eight 24.6, concentrated_wealth 24.3, collision 22.7, corners 11.1, ambush 8.7, corridors 8.2, broken_curve 7.3, boss_battle 0.2.
- Self-play for comparison: 16.8 deaths a game (12.9 champion), 52 % at altars.

## 3. Lock awareness

**Code.** `bots/v4/src/a/bot.cpp:432-461` (`Bot::bank`) reads `cambc::vision::altar_locks(team::enemy)` every banking turn and remembers a locked altar for 40 rounds (`lock_until`). `choose_altar` (bot.cpp:58) adds +3 to a locked altar's score, and `stuck.cpp:116` adds +1000. A carrier in submit range of a locked altar basic-attacks the lock when no enemy is within 4, or after 3 turns of waiting (`lock_wait`), then tributes. So **v4 does detect enemy locks**. It never buys or places one: `buy_items` has no consumable purchase, and the decoded games show 0 locks placed by v4 in all 90.

**Trace** (v4top-t08-B-01, 01corridors, v4 = side A, opponent Shingtonwash). At r679 their Varg tributes 104 gems at the altar (13,19). At r680 a lock appears at (14,21), within 2 of the altar. At r721-723 v4's Tavi walks in carrying 16 gems with their C&I 4 tiles away. At r724-728 Tavi makes 5 basic attacks on the lock (no champion or camp hit; the site doc says 3, and the SDK reads `altar_lock_hits` at runtime). Tavi takes 80 damage meanwhile (710 -> 630). At r729 Tavi starts the tribute, which completes by r732 (16 gems). The lock delayed a 16-gem bag by 5 turns under fire.

**Across the 90 games:** the top 10 placed 146 locks (1.6 a game). Only 5 of them cancelled a v4 tribute in progress (bags of 15-23, 94 gems in total). The other 141 were placed on altars with no v4 channel running: most go down right after the opponent's own tribute or early on the central altars, so they deny the altar rather than interrupt a channel. v4's champions attacked a lock in 26 of the 146 cases. On contest the top 10 placed 0.9 locks a game.

## 4. Time use (share of champion-rounds; v4 vs the top 10, and the top 10 measured the same way)

| set | dead | walking | clearing camps | capturing | fighting | tributing | recalling | idle |
|---|---|---|---|---|---|---|---|---|
| v4 vs the top 10 | **22.2 %** | 48.5 % | 10.2 % | 3.5 % | 0.8 % | 0.4 % | 4.6 % | 9.8 % |
| top 10 vs v4 | 5.1 % | 46.3 % | **17.1 %** | 2.4 % | 1.4 % | 2.2 % | **15.8 %** | 9.7 % |
| v4 vs v3b-e1 | 11.3 % | 55.4 % | 13.1 % | 3.6 % | 1.8 % | 1.2 % | 4.3 % | 9.4 % |

Definitions: dead = alive flag 0. Tributing = from a tribute start (step kind 12, aimed at an altar) for ceil(bag/10) rounds. Recalling = 5 rounds from a recall start (kind 11). Either channel ends early at the champion's next move or hit. Fighting = an attack or cast that hit a champion. Clearing = one that hit a camp. Walking = a move or a dash. Capturing = a capture step (kind 23) or standing on a camp spawn tile. Idle = the rest. Engine 1.0 step kinds were checked on decoded games: 4 move, 5 attack, 6 cast, 11 recall (the champion appears at spawn 5 rounds later), 12 tribute start (the target is the altar tile), 13 shop, 23 capture. The kind list in `notes/v3-data-scope.md` (11 channel, 12 ward, 20 consumable) is engine 0.0.x and no longer holds.

Per map, v4 is dead 31-37 % of the time on gold_rush, collision, contest and eight. The top teams are dead 1-11 % on every map. They spend 70 % more rounds clearing camps (17 % vs 10 %) and recall 3.4 times as often (15.8 % vs 4.6 %): heal and shop at spawn instead of dying.

## 5. Other fields (v4 / top 10, per game)

| field | v4 | top 10 |
|---|---|---|
| gear points per champion at round 200 | 1.9 | 5.3 |
| fights opened | 3.7 | 19.6 |
| share of those fights opened with an ability | 47 % | 79 % |
| locks placed | 0.0 | 1.6 |
| games reaching 1,000 banked | 0 / 90 | 83 / 90 (median round 1,086) |

## 6. What this says for S3

1. Contest (v4 0 gems in 9/9 against the top 10; 0-0 against v3b-e1 in self-play, won on the gold tiebreak). The top teams bank whole monoliths: in v4top-t02-A-02, procless hits the (17,31) Monolith from r470, takes 200 gems at r745 and tributes them from (25,26), beside the central altars, at r794. They repeat on alternate Monoliths about every 95 rounds (5 tributes of 200 by r1,166). v4 deals 0 damage to either Monolith and dies 39 times there. Both of contest's altars sit in the centre, (25,24) and (24,25), so every 200-gem carrier channels for 20 turns in the same place: interdiction (a) has a fixed target.
2. The top teams' advantage is production plus staying alive (dead 5 % vs 22 %), not locks: only 5 of their 146 locks cancelled a v4 tribute.
3. Self-play vs v3b-e1 hides the production gap (791 acquired against 269). Admission numbers from self-play are a rejection gate only, as v5-common.md says.
