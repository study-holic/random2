# CONTEXT (CAMBC 2026 lead chats 1 and 2, Fri 23 Oct; merged 11:13 BST)

Chats 1 and 2 ran in parallel from 09:34 to 11:10. This file is chat 2's CONTEXT with chat 1's later results added (sections 3f to 3i, conclusion 7, and the open list). Where both chats measured the same thing, chat 2's larger index governs and chat 1's figure is labelled.

Labels follow A2.11. Every result names engine, pool, build, sample, and opponents. Times are BST.

## 1. Environment (KNOWN, 23 Oct 07:52 to 08:00)

- The claude.ai sandbox runs the engine with no settings change: 2026a.battlecode.co.uk answers directly. cambc's own downloader rejects the proxy certificate for github.com, so scripts/setup_engine.sh fetches wasi-sdk 33, cmake 3.31.8, and ninja 1.12.1 with curl into cambc's cache. Two cores, 7 GB RAM; a release game takes 2 to 10 s.
- Workspaces: /home/claude/ws305 (cambc 3.0.5, SDK 3.0.5, engine 1.0.3, primary) and /home/claude/ws (cambc 3.0.4, SDK 3.0.4, engine 1.0.2).
- The platform pool fetched by `cambc init` at 07:56 holds 40 maps, byte-identical to random2/maps. Platform file names use a 00/01 prefix and hyphens (01big-world).
- github.com answers the sandbox's curl through the proxy for release downloads. From chat 2 (09:34) the proxy serves anonymous git reads of random2, so A2.3's sparse clone works; pushes need Xans.
- Each chat starts in a new sandbox: setup_engine.sh (about 2 minutes) and the sparse clone rebuild the workspace, and data/lead is linked to /home/claude/data.

## 2. Engine version

- KNOWN: cambc 3.0.5 (PyPI, 22 Oct 07:44 BST) runs engine 1.0.3. Part B treats 1.0.2 as current.
- DERIVED: 1.0.2 and 1.0.3 share design tables (13 champions, 6 camps), the 407-entry export list, and embedded strings. SDK 3.0.5 changes one cambc.h comment (debug drawings persist until the drawer's next turn).
- DERIVED: v4 against both starters gives 160 of 160 identical games on 1.0.2 and 1.0.3; v5r vs v4 scores 52/80 on both. Evidence against full equivalence: the 1.0.3 change list is unpublished in any source here, and one bot pairing cannot exercise every code path.
- KNOWN: the sandbox reproduces B4 exactly on 1.0.2 (v4 74/80 against random2's starter-cpp, same two overruns, same six losses). That older starter logs 21,170 overrun events in 80 games.
- KNOWN: the platform runs 1.0.3 with the same build hash as cambc 3.0.5 (ranked match 7ebae7cd, 23 Oct 07:42 UTC, patchId 1.0.3:fcb6c3e8...). The platform's patch-notes page lists nothing after 1.0.2. The ranked index places the switch from 1.0.2 to 1.0.3 between 22 Oct 07:00 and 07:10 UTC.

## 2a. Event and platform (KNOWN, platform pages, 23 Oct 08:41 to 08:50)

- Sprint 2 locks Sat 24 Oct 23:59 BST (tournament 25 Oct); Sprint 3 locks Sat 31 Oct 23:59 GMT (1 Nov); International qualifiers 4 Nov; UK qualifiers 11 Nov; finals 18 Nov at the Cambridge Union, 16 teams (8 UK, 8 international). Xanarchy (team of 1) is eligible for the sprints, the UK qualifiers, and the main prize pool.
- Ladder at 09:16: 752 teams (168 UK). Xanarchy 1,344, UK 130 of 168. UK ranks 1, 8, 16: 2,084, 1,825, 1,752. Top UK picks centre on Marcella, Heart of Ruin, Vesper, Ryth, Vendetta, Tavi; v4 drafted Cassi and Issac in 830 of 830 games.
- Every sprint tournament uses unseen maps outside the active pool. Map-specific code earns nothing there; sprint builds need a held-out map test.
- One person in two teams with shared code: allowed as a team merger (Xans, 23 Oct 09:03). Which record and rating a merged team carries is OPEN.
- No 1.0.3 change note on Discord as of 09:03.
- Xanarchy: rating 1,354, ladder #563, season ranked 310-312 (08:43). Platform numbering: v1 = our v2, v2 = our v3b, v3 = our v4, v4 = our v5r. v5r uploaded 08:53, compiled and live from 09:03 BST. Rollback: Make Live on platform v3 if v5r wins 40 or fewer of its first 100 ranked games, or 0 of its first 6 on road_of_riches (scripts/lead/rollback_check.py; first run due 12:25 BST; chat 1 also runs scripts/lead/team_record.py at 12:35). v5r: 11 of its first 20 ranked games (08:52 UTC); Xanarchy 1,347, overall 566. Interim at 11:05 BST (public API, chat 1): 34 of 60 (0.567; v4 on 1.0.3 0.492 over 750), pace 40.2 gems per 100 rounds (v4 38.0), opponents' mean rating 1,352; great_wall 3 of 4 (v4 3 of 23).
- Public API without login: /api/v1/match-groups/<id> gives map, rounds, end reason, engine build, drafts, scores, and ratings. Match summaries, replays, and the match database need a login; database parts redirect to signed CloudFront URLs, and the 206 range test is blocked from the sandbox and the browser.

## 3. Engine facts (KNOWN, census bot on 40 maps, engine 1.0.3; data/ENGINE-FACTS.json)

- Constants: capture_time 5, camp_sleep_time 2, respawn_time 60, recall_time 5, kill_gold 100, assist_gold 0, passive_gold 0, altar_lock_hits 5, tp_crystal_time 3, tribute duration max(1, 0.1 g rounded up) within range 1, spawn_healing 1, tile_reveal_rounds 1, shared memory 8 bytes, draft memory 1,024 bytes.
- Wards: vision radius 6, placement radius 2, lifetime -1, cooldown 10, one active, one charge. A control ward also exists (vision 6, placement 2).
- Passive constants: Prism 1.5 Focus per gem, Varg splash radius 1, Vesper +33 per cent gold, Marcella 2 gold per gem tributed, Ryth 0.01 Strength per bonus health, Ren retaliation 0.2 x Focus, Heart of Ruin up to +50 per cent damage from missing health.
- Draft: team_size is 5 on 26 maps, 4 on 8, and 3 on 6. Cassi and Issac take one pick and add one body. Both teams may draft the same champion.
- Camps: one base_stats variant per kind across 2,741 camps on 40 maps; every growth field is 0. All 836 groups pay exactly the documented sum of their members (no reward multiplier in the 40-map pool) and respawn on their longest member's timer. Golems (13) appear on great_wall, lost_world, quadrants, reverse_spiral, road_of_riches, and split.
- Fuel overrun (arena, release and debug identical byte for byte): 50k over at round 50 suspends the champion and resumes the same turn at round 66; 950k over leaves it suspended past round 120.
- Arena p09 to p16 match results/arena-1020.md (engine 1.0.0): respawn rounds 69, 189, 258 and 148; contested reward at round 60; adjacent enemies do not contest; adjacent allies take no share; the Monolith does not regenerate; nearest means Chebyshev, and ties alternate.
- Wake and sleep (lab3 M1): damage to one member wakes the whole group, including clusters; each member sleeps 2 rounds after the last round a champion stood within its own range. Camps never regenerate and never grow (round 1,990).
- Turn order (lab3 M2): champions interleave across teams in within-team order (A1, B1, A2, B2, ...); the starting team alternates each round, Team A on odd rounds. Within a team, priority then draft order.
- Ranged champions (lab3 M3): a move uses the attack allowance; an attack reset restores it, so move + reset + attack and attack + reset + attack both work, every 8 rounds.
- Capture, recall, crystal (lab3 M4): paid 5 rounds after the first capture phase on the tile; recall 5 turn ends; crystal 10 gold, range 3, refuses walls, holes, and occupied tiles, channels 3 turn ends.
- Golem groups pay their group values (lab3 M7: 36 gems, 240 gold). No inventory or stat cap within 21 shard types and 44 shards (M8); gold binds first.
- Gold bonuses (lab3 M4, M6): Vesper's +33 per cent applies to capture gold (40 -> 53) and kill gold (100 -> 133). Marcella's 2 gold per gem counts her own tributes alone (M5). A tribute of up to 10 gems completes in one turn end.
- Rewards and capture split reconfirmed (p03): gem cluster 7, gold cluster 20, wolf 4 gems and 40 gold, a 21-gem group paid 11 and 10. Ranges reconfirmed (p00 to p02): wolf 3, elk 4, Monolith 5. Targeting reconfirmed (p04 to p08): nearest champion, immediate switch, alternation between equidistant champions.

## 3a. Maps (DERIVED, data/MAPS.csv, data/MAP-GROUPS.csv, data/renders)

- Every map has a walking route between the spawns. Altars not walkable from spawn A: collision 1 of 2; islands 6 of 10; boss_battle's single altar is usable only from the Monolith tile (24,20), so no team tributes there before a Monolith dies.
- B4 map rows on 1.0.3: islands holds; boss_battle holds; elk_arena's centre group pays 72 gems (B4's 216 is the pre-1.0 value); corridors' "no tile in reach of fewer than two elk" fails (8 tiles); v4 tributes 0 gems on boss_battle, contest, and shortcut in every game (6 per map).
- The supply ceiling (gems / (respawn + 5)) ignores clear and travel time and runs from 1.1 to 14.6 gems per round across the 40 maps; S3 needs a clear-time version before it binds.
- PATCH-NOTES-1.0.md matches the engine on all 35 champion stat values, every ability and passive change, all camp values, kill gold, ward cooldown, and lock hits.

## 3b. Held-out proxy for unseen sprint maps (MEASUREMENT, 1.0.3, 40 maps x 2 seats)

- Builds v4-u and v5r-u force the map id to unknown (the plan they run on any unseen map). v5r-u vs SDK starter 77/80 and v4-u 75/80, each with the same losses as the unforced build. v5r-u vs v5r 38/80 (noise band): v5r's map plans add nothing measurable. v5r-u vs v4-u 49/80 (two-SD line).
- Only 3 of 23 distinct non-pool maps in random2 are unknown to v5r; the rest share a pool key, so they are no held-out set.

## 3c. Data path (KNOWN)

- Public API, no login, from the sandbox: ladder (with each team's picks over its last 50 games), ranked groups per team, per-match map, drafts, scores, rounds, end reason, engine build. Index in data/lead/s2 (scripts/lead/s2_index.py; 6,244 groups in about 10 minutes with 6 parallel requests). Draft ids decode in the order ryth, vesper_vale, vendetta, cassi_and_issac, stellar, prism, marcella, willow, ren, varg, tavi, heart_of_ruin.
- Per-match summaries need a login: presigned S3 links, readable in the browser, same CAMSTAT format as local summaries. scripts/s2_metrics.py fetch runs on Xans's laptop (`cambc login` once) and returns per-team metrics; validated on local summaries.
- Each team plays a ranked group of 5 games about every 10 minutes (about 30 ranked games an hour); every indexed team has 305 or 306 ranked groups between 21 Oct 05:30 UTC and 23 Oct 08:38 UTC.
- Summary fetch pending on the laptop: data/lead/s2/fetch_ids.csv (500 ids, engine 1.0.3: 8 top-20 vs top-20 games per map, 2 UK top-8 games per map, the 100 latest Xanarchy losses).

## 3d. Population (S2; ranked, index of 23 Oct 09:36 to 09:48; data/lead/s2)

- Sample: ladder top 20, UK top 8, Xanarchy, MicroStella; 31,220 ranked matches since 21 Oct 05:30 UTC (engine 1.0.2: 15,775; 1.0.3: 15,445). End reasons: gems 27,725, tiebreak gems 3,102, gold 360, second team 32, kills 1.
- Top-20 pick shares, 1.0.3 (15,300 team-games): Vesper 0.85, Marcella 0.84, Heart of Ruin 0.77, Ryth 0.73, Vendetta 0.57, Tavi 0.17, Willow 0.16, Cassi and Issac 0.15, Prism 0.13, Varg 0.10, Stellar 0.03, Ren 0.00. On 1.0.2 (14,795): Heart of Ruin 0.79, Ryth 0.75, Marcella 0.71, Vesper 0.57, Vendetta 0.38, Stellar 0.29, Cassi and Issac 0.22, Ren 0.22. Win share with any pick lies between 0.50 and 0.58.
- Nine of the top 20 play Heart of Ruin, Marcella, Ryth, Vendetta, and Vesper as their modal 5-pick draft on 1.0.3. Per-map shares follow team size: each team truncates one preference list, and Ryth falls to 0.14 to 0.22 on 3-pick maps. UK top 8 on 1.0.3 (5,630 team-games): Heart of Ruin 0.77, Marcella 0.69, Vendetta 0.61, Ryth 0.58, Vesper 0.57. Xanarchy's v4 draft (Cassi and Issac, Ryth, Tavi, Varg, Vendetta; modal share 0.91) shares two picks with the top core.
- Pace, top 20 against top 20 on 1.0.3 (5,735 games): median round of gem wins per map from 564 (quadrants) to 1,776 (starvation), median over maps 1,126 (finish_by_map.csv).
- Xanarchy ranked: v4 on 1.0.2 379/765 (opponents' mean rating 1,386); v4 on 1.0.3 369/750 (1,363); v5r 11/20. Per map in xanarchy_by_map.csv.

## 3e. Production model and archetypes (S3, DERIVED; scripts/lead/s3_model.py, s3_archetypes.py)

- Model v1: squads of the drafted bodies cycle camp groups and one altar at base stats; clear time from total camp health over summed damage per round (strength, attack resets, damaging abilities at cooldown 8); capture 6 rounds from arrival; tribute ceil(gems / bodies / 10); a recall when a cycle's damage taken exceeds 60 per cent of squad health; respawn bounds the cycle period. "Half" counts groups nearer our spawn. Output data/lead/S3-PRODUCTION.csv.
- Calibration: combined tributed gems per 100 rounds in top-20 vs top-20 games over twice the model's half-map rate (top core draft) has median 1.02 across 40 maps (0.62 boss_battle to 2.09 rows; 30 maps within 0.67 to 1.5).
- v5r against the SDK starter (1.0.3, 80 games, statistics kept): reaches 1,000 in 53 of 80 games, median round 1,417; 57.2 gems per 100 rounds; 851 gems from camps and 49 from kills per game. Its production is 0.76 of a top-20 team's production against top-20 opponents (median over maps) and under 0.6 on boss_battle and contest (0), road_of_riches 0.16, corridors 0.17, shortcut 0.25, pilgrimage 0.27, reverse_spiral 0.35, split 0.41, rushbait_v2 0.45, fracture 0.47, stampede 0.47, and broken_curve 0.57. Ten of these twelve hold a Monolith or golem, or an altar 50 or more tiles from spawn A.
- Model sensitivities (median ratio of half-map rate over 40 maps, v5r draft): damage x1.25 gives 1.11 (p10 1.01, p90 1.28); damage x0.8 gives 0.92; one squad forced gives 1.00 (p10 0.81); recall threshold 90 per cent gives 1.05 (p90 1.30); capture 5 rounds slower gives 0.91. Drafts: v5r's draft gives 1.05 of the top core's rate (Cassi and Issac add a body); the model carries no fight, kill, or gold effect, so draft differences in fights stay OPEN until the summaries arrive.
- Archetypes (k-means, k = 6, 8 standardised features; S3-ARCHETYPES.csv). Top-20 median finish and v5r's local median round to 1,000: A1 concentrated_wealth, eight, elkring, gem_fields, gemfight, lost_world, quadrants, reverse_spiral (893; 1,103). A2 ambush, corners, elk_arena, expanse, islands, lanes, rushbait_v2, yellow_brick_road (1,050; 1,485). A3 road_of_riches, shortcut (949; v5r reaches 1,000 in 0 of 4). A4 broken_curve, collision, fragment, gold_rush, hooks, interlock, packs, swirl, the_farm (1,228; 1,423). A5 big_world, boss_battle, contest, corridors, fracture, great_wall, monolith, rows, split, starvation (1,152; 1,652, 9 of 20 reach 1,000). A6 pilgrimage, stampede, starfish (1,662; 1,699, 2 of 6).

## 3f. Chat 1's index and the Monolith-altar maps (DERIVED, chat 1 index of 10:00: top 20, Xanarchy, MicroStella; 1.0.3, 10,580 matches; a subset of chat 2's index)

- Pace (gems tributed per 100 rounds): top 20 60.9 to 89.0 (ranks 1 and 2: 81.6 and 89.0); Xanarchy under v4 38.0 (win share 0.492); MicroStella 35.9 (0.494). Median round to 1,000 in gem wins: top 20 993 to 1,243; Xanarchy 1,490.
- Xanarchy's worst maps under v4 on 1.0.3 (6 or more games): packs 0 of 18, fragment 0.11, gem_fields 0.11, gemfight 0.12, great_wall 0.13 (pace 0), broken_curve 0.19, contest 0.19, swirl 0.19. Best: the_farm 0.83, islands 0.86, monolith 0.86, rows 0.90, interlock 0.92.
- Xanarchy's pace ratio to the top 20 per map correlates with Monolith count (r = -0.41), altar count (+0.37), narrowest cut width (+0.37), and spawn-to-altar distance (-0.33); median ratio 0.50. It is 0 on boss_battle, great_wall, and shortcut, the three maps where every altar lies within 3 tiles of a Monolith.
- MEASUREMENT (local, 1.0.3): v5r tributes on great_wall (1,003 and 1,026 gems against the starter; v4 0 and 0) and shortcut (570 and 135; v4 0 and 0) through P_S1_BANK_REACH, which lets champions enter an asleep Monolith's zone. boss_battle and contest stay at 0 for v4, v5r, and the SDK starter; boss_battle's altar is reachable from the Monolith tile alone, so tributes there need a Monolith kill.

## 3g. D1: the top-20 draft on v5r (pre-registered 10:08; result 10:40; engine 1.0.3, 40 maps x 2 seats, release)

- Build v5r-meta: v5r with its default draft (Cassi and Issac, Stellar, Varg, Ryth, Ren) replaced by Heart of Ruin, Vesper, Marcella, Ryth, Vendetta in that order (nests as the top 3-, 4-, and 5-pick sets); role code unchanged.
- Primary: v5r-meta 48 of 80 against v5r; the pre-registered lines were 49 (supports) and 31 (falsifies). The verdict belongs to the G3 grader.
- Secondary: against v5r, gems per game 716 vs 584, deaths 12.4 vs 27.0, pace 46.6 vs 38.2. Against the SDK starter 75 of 80 (v5r 77), 764 gems a game (v5r 799), deaths 4.3 (v5r 8.1), 0 overruns. Against v4 53 of 80 (v5r 52).

## 3h. Panels for F2 (MEASUREMENT, references; admit nothing)

- random2 panel on SDK 3.0.5: 19 of 20 bots compile (centre-v2 fails on an out-of-line definition in stack.cpp). v5r against all 19 (40 maps x 2 seats, 1.0.3, release; complete 11:29): 1,284 of 1,520 (0.845). tung-clone 77, locker 76, x-mite 76, x-shawn 76, v3-quick 75, v5-ambush 75, x-hello 75, x-ifish 75, raider2 72, v5-stack 69, hunter 67, centre 61, centre-v3 61, x-foundry-v16 61, tact-on 60, v5-two 60, x-v4-14 58, tact 56, v4-amb 54 (of 80). The competitor panel (9 bots) runs next. The 18 Oct clones of top teams are pre-1.0 versions; the panel detects regressions and says little about the gap to today's top teams.
- Competitor repos (18 Oct list in random2/project/competitor-repos.md, cloned 23 Oct): 58 C++ bots compile on SDK 3.0.5 (42 distinct non-starter sources). Most changed last before 20 Oct; the latest: Not-TNB 22 Oct 21:33, DeySag 22 Oct 13:29, DE41H 21 Oct (Python), dpark2380 20 Oct 22:33. A2.19 competitor panel (9 bots): deysag-baseline, fhl-08-foundry-v16, fhl-08-squad-v21, not-tnb-stress-test-cpp, realahmedkhairi-benchmark, s0ulsyte-merged, s0ulsyte-v2, dpark2380-v7, dpark2380-v8.
- The GitHub search API is closed to these sessions; one web search on 23 Oct found no Cambridge Battlecode repo outside the 18 Oct list.

## 3i. Observation (platform visualiser, one game; OBSERVATION)

- prism best girl (rank 1) against Intercontinental Fish (rank 2), road_of_riches, match de5c7061, engine 1.0.3, 847 rounds. At round 847: gold 8,263 vs 3,368; gems tributed 1,033 vs 150; kills 32 vs 8. The winner's tribute line rises in steps from about round 100. One game; the summary fetch measures this across 500.

## 3j. S2 measures, partial (MEASUREMENT and DERIVED; laptop fetch 1, 241 top-20 vs top-20 ranked games on 1.0.3, 31 of 40 maps; data/s2/S2-MEASURES.csv)

| Medians per team-game, per 100 rounds | Top-20 winners | Top-20 losers | v5r vs starter (local) |
|---|---|---|---|
| Gems tributed | 91.2 | 42.6 | 59.3 |
| Gems from camps | 90.3 | 57.9 | 62.1 |
| Gems lost | 0.46 | 3.96 | 0.71 |
| Deaths | 0.31 | 0.78 | 0.25 |
| Gold earned | 740 | 425 | 425 |
| Items bought | 26.8 | 16.1 | 14.7 |
| Wards placed | 11.0 | 8.9 | 0 |
| Camp damage dealt | 11,029 | 7,856 | 9,111 |
| Camp damage taken per damage dealt | 0.230 | 0.258 | 0.393 |
| Camp gems per 1,000 camp damage | 7.76 | 5.95 | 6.34 |
| Rounds to 250 / 500 / 1,000 gems | 508 / 754 / 1,115 | 614 / 874 / none | 644 / 955 / 1,417 |

- v5r takes 0.39 to 0.42 camp damage per point dealt in every local set (vs starter, vs v4, and as v5r-meta), against 0.23 to 0.26 for the top 20. Issac takes 0.89 per point dealt. Per-champion figures for top teams arrive with fetch 2 (champions.csv).
- Top-20 games run 1,129 rounds (median); v5r's games against the starter run 1,734, so its per-100-round rates come from a longer, mostly uncontested game.

## 4. Baselines (MEASUREMENT, release builds, 40 maps, both seats, salt 0, sandbox)

| Set | Engine | Result | Fuel overruns (bot) | Losses |
|---|---|---|---|---|
| v4 vs SDK starter-cpp | 1.0.3 | 75/80 | 1 (starvation/B r1716) | great_wall A+B, broken_curve/A, shortcut A+B (0-0) |
| v5r vs SDK starter-cpp | 1.0.3 | 77/80 | 1 (starfish/A r1493) | road_of_riches A+B (238-727, 317-547), gold_rush/B (440-462); rerun in chat 2 with statistics (LEDGER 09:50; the repo ignores raw results files) |
| v22 vs SDK starter-cpp | 1.0.3 | 73/80 | 603 (rounds 1 and 2 on every map) | contest/A, reverse_spiral/A, broken_curve/A, gold_rush A+B, shortcut A+B |
| v5r vs v4 | 1.0.3 | 52/80 | 1 vs 2 | 0-2 on contest, gemfight, lanes, swirl, starvation, yellow_brick_road |
| v5r vs v22 | 1.0.3 | 46/80 | 0 vs 607 | per game in results/lead/h2h-v5r-v22-e103/games.csv |
| v22 vs v4 | 1.0.3 | 39/80 | 637 vs 0 | per game in results/lead/h2h-v22-v4-e103/games.csv |
| v4 vs random2 starter-cpp | 1.0.2 | 74/80 | 2 (road_of_riches/B, starvation/B) | great_wall A+B, road_of_riches/B, broken_curve/A, shortcut A+B |

Reading rule (DERIVED): at n = 80, 49/80 is two binomial SDs; the salted v4 copies' 73/126 scales to 46/80.

## 5. Conclusions

1. No existing build passes F1 on engine 1.0.3, the platform's engine. Strongest evidence against: F1 here uses the SDK 3.0.5 starter template; against random2's older starter the counts differ (v4 74/80), so the floor depends on the reference bot.
2. v5r beats v4 head to head (52/80, above the 49/80 line) and loses fewer F1 games (3 against 5). Strongest evidence against: one 80-game deterministic set per pairing; v5r loses road_of_riches to the starter on both seats with 32 deaths (B4) and 0-2 to v4 on six maps.
3. v22 fails F1 by the widest margin (73/80, 603 overrun events) and sits inside the noise band against v5r (46/80) and v4 (39/80). Strongest evidence against: v22 is MicroStella's ladder submission with an unknown rating, and most of its overruns fall in rounds 1 and 2, so their cost is concentrated in the opening.
4. Sprint and qualifier results depend on unseen maps. v5r's default plan matches v5r on the pool within noise (38/80) and edges v4's (49/80), so v5r is the strongest existing Sprint 2 build. Strongest evidence against: the proxy keeps pool geometry; unseen maps may differ in size and layout from the pool (OUT OF REACH until a sprint's maps are published; Sprint 1's maps may be retrievable from its replays).
5. v5r's farming production falls short of the top teams': against a starter that tributes 156 gems a game, v5r produces 0.76 of what a top-20 team produces against another top-20 team, and under 0.6 on twelve maps, ten of them Monolith, golem, or far-altar maps. Strongest evidence against: top-team figures include gems taken by kills, and v5r's figure comes from one deterministic 80-game set against one opponent; per-source gems for top teams wait for the summary fetch.
6. Draft: the top 20 converge on Heart of Ruin, Marcella, Ryth, Vendetta, and Vesper (1.0.3), and our draft shares two of the five. The production model gives our draft 1.05 of the core's farming rate, so any draft cost lies in fights and gold. Strongest evidence against: win share with each pick inside the top 20 is flat (0.50 to 0.58), which measures picks among teams of equal strength and leaves our rating band unmeasured.

7. The top-20 draft dropped into v5r's role code halves its deaths and raises its pace head to head (D1: 48/80 against v5r, deaths 12.4 vs 27.0, pace 46.6 vs 38.2) and costs 2 games and 35 gems a game against the starter. Strongest evidence against: v5r's role code was written for its own five champions, so D1 measures picks without matching behaviour; one deterministic set per pairing.

## 6. Open

- A merged team's record and rating (when the new team starts).
- The 1.0.3 change list (none published as of 09:03), and whether it caused the top-20 pick shift between 1.0.2 and 1.0.3 (Vesper +0.28, Vendetta +0.19, Stellar -0.26, Ren -0.22).
- Match-database range reads (laptop: download one part link in the browser and test the signed URL with a 16-byte range).
- v5r's first 100 ranked games and the rollback verdict (12:25 BST in chat 2's session if it fires; 12:35 BST in chat 1).
- F2 reference: the seven remaining random2 panel bots and the 9-bot competitor panel for v5r (running in chat 1's sandbox; the sandbox and its runs end with that chat).
- S2 metrics from the laptop fetch (gems by source, deaths, gold, items) for the top 20, UK top 8, and our losses; then the per-mechanic effects that need data (kills, deaths, altar locks) and model v2 with shards.
