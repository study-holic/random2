# CONTEXT (CAMBC 2026 lead chat 1, Fri 23 Oct)

Labels follow A2.11. Every result names engine, pool, build, sample, and opponents. Times are BST.

## 1. Environment (KNOWN, 23 Oct 07:52 to 08:00)

- The claude.ai sandbox runs the engine with no settings change: 2026a.battlecode.co.uk answers directly. cambc's own downloader rejects the proxy certificate for github.com, so scripts/setup_engine.sh fetches wasi-sdk 33, cmake 3.31.8, and ninja 1.12.1 with curl into cambc's cache. Two cores, 7 GB RAM; a release game takes 2 to 10 s.
- Workspaces: /home/claude/ws305 (cambc 3.0.5, SDK 3.0.5, engine 1.0.3, primary) and /home/claude/ws (cambc 3.0.4, SDK 3.0.4, engine 1.0.2).
- The platform pool fetched by `cambc init` at 07:56 holds 40 maps, byte-identical to random2/maps. Platform file names use a 00/01 prefix and hyphens (01big-world).
- github.com answers the sandbox's curl through the proxy for release downloads; a plain request to github.com/study-holic/random2 returned 403, so the attached ZIP of main is the repo source here.

## 2. Engine version

- KNOWN: cambc 3.0.5 (PyPI, 22 Oct 07:44 BST) runs engine 1.0.3. Part B treats 1.0.2 as current.
- DERIVED: 1.0.2 and 1.0.3 share design tables (13 champions, 6 camps), the 407-entry export list, and embedded strings. SDK 3.0.5 changes one cambc.h comment (debug drawings persist until the drawer's next turn).
- DERIVED: v4 against both starters gives 160 of 160 identical games on 1.0.2 and 1.0.3; v5r vs v4 scores 52/80 on both. Evidence against full equivalence: the 1.0.3 change list is unpublished in any source here, and one bot pairing cannot exercise every code path.
- KNOWN: the sandbox reproduces B4 exactly on 1.0.2 (v4 74/80 against random2's starter-cpp, same two overruns, same six losses). That older starter logs 21,170 overrun events in 80 games.
- KNOWN: the platform runs 1.0.3 with the same build hash as cambc 3.0.5 (ranked match 7ebae7cd, 23 Oct 07:42 UTC, patchId 1.0.3:fcb6c3e8...). The platform's patch-notes page lists nothing after 1.0.2.

## 2a. Event and platform (KNOWN, platform pages, 23 Oct 08:41 to 08:50)

- Sprint 2 locks Sat 24 Oct 23:59 BST (tournament 25 Oct); Sprint 3 locks Sat 31 Oct 23:59 GMT (1 Nov); International qualifiers 4 Nov; UK qualifiers 11 Nov; finals 18 Nov at the Cambridge Union, 16 teams (8 UK, 8 international). Xanarchy (team of 1) is eligible for the sprints, the UK qualifiers, and the main prize pool.
- Every sprint tournament uses unseen maps outside the active pool. Map-specific code earns nothing there; sprint builds need a held-out map test.
- One person in two teams, and code shared between teams: OPEN (public pages say teams of 1 to 4 and nothing more; source: email or Discord).
- Xanarchy: rating 1,354, ladder #563, season ranked 310-312 (08:43). Platform numbering: v1 = our v2, v2 = our v3b, v3 = our v4, v4 = our v5r. v5r uploaded 08:53 and selected for the next match; platform v3 stays the fallback until v5r compiles. Rollback: Make Live on platform v3 if v5r wins 40 or fewer of its first 100 ranked games, or 0 of its first 6 on road_of_riches.
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
- Gold bonuses (lab3 M4, M6): Vesper's +33 per cent applies to capture gold (40 -> 53) and kill gold (100 -> 133). Marcella's 2 gold per gem counts her own tributes alone (M5). A tribute of up to 10 gems completes in one turn end.
- Rewards and capture split reconfirmed (p03): gem cluster 7, gold cluster 20, wolf 4 gems and 40 gold, a 21-gem group paid 11 and 10. Ranges reconfirmed (p00 to p02): wolf 3, elk 4, Monolith 5. Targeting reconfirmed (p04 to p08): nearest champion, immediate switch, alternation between equidistant champions.

## 3a. Maps (DERIVED, data/MAPS.csv, data/MAP-GROUPS.csv, data/renders)

- Every map has a walking route between the spawns. Altars not walkable from spawn A: collision 1 of 2; islands 6 of 10; boss_battle's single altar is usable only from the Monolith tile (24,20), so no team tributes there before a Monolith dies.
- B4 map rows on 1.0.3: islands holds; boss_battle holds; elk_arena's centre group pays 72 gems (B4's 216 is the pre-1.0 value); corridors' "no tile in reach of fewer than two elk" fails (8 tiles); v4 tributes 0 gems on boss_battle, contest, and shortcut in every game (6 per map).
- The supply ceiling (gems / (respawn + 5)) ignores clear and travel time and runs from 1.1 to 14.6 gems per round across the 40 maps; S3 needs a clear-time version before it binds.
- PATCH-NOTES-1.0.md matches the engine on all 35 champion stat values, every ability and passive change, all camp values, kill gold, ward cooldown, and lock hits.

## 4. Baselines (MEASUREMENT, release builds, 40 maps, both seats, salt 0, sandbox)

| Set | Engine | Result | Fuel overruns (bot) | Losses |
|---|---|---|---|---|
| v4 vs SDK starter-cpp | 1.0.3 | 75/80 | 1 (starvation/B r1716) | great_wall A+B, broken_curve/A, shortcut A+B (0-0) |
| v5r vs SDK starter-cpp | 1.0.3 | 77/80 | 1 (starfish/A r1493) | road_of_riches A+B, gold_rush/B |
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
4. Sprint and qualifier results depend on unseen maps, so the map-id dispatch in v4 and v5r has no value there; the ladder still uses the 40-map pool. Strongest evidence against: none in hand; the unseen maps' generator and size range are unknown (OUT OF REACH until a sprint's maps are published).

## 6. Open

- B1b #9 inventory and stat caps; B1b #13 golem payout (group values KNOWN).
- One person in two teams; code shared between teams (email contact@battlecode.co.uk or Discord).
- The 1.0.3 change list (Discord).
- Match-database range reads (laptop: download one part link in the browser and test the signed URL with a 16-byte range).
- v5r's compile status and first ranked results on the platform.
- S2: metrics source check (`cambc summary` fields), the index via the public match-group API, and the per-map ranked record of v5r against v4.
