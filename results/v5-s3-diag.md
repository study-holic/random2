# v5 S3 diagnostics: v4 against the top 10 (20 Oct 2026)

Data: the 90 games of `data/real/v4-panel/v4top-t02..t10-A/B` (sets A and B: eight, contest, ambush, collision, corners, corridors, broken_curve, gold_rush, boss_battle, concentrated_wealth; 9 games a map, one per team; engine 1.0.0), decoded at stride 1 with `tools/top10/extract_pilot.mjs`. Side and map come from `data/unranked/matches.jsonl`. Every number below comes from `tools/cloud/metrics.py` (pushed on this branch) unless a script is named. Per-map cells are n = 9 (indicative only).

Cross-check: the script reproduces `notes/panel-v4-top10-AB.md`: gems 159 / 943, deaths 22.7 / 6.0, gems lost to champion kills 62 / 3 (both sides).

Engine set-up for the decoder in a cloud container: `cambc view <replay>` serves the visualiser assets; `PanelChrome-*.js` and `v1_0_0-*.wasm` were fetched from `http://127.0.0.1:<port>/assets/` into `data/top10/engine/` (git-ignored); set `CAMBC_ENGINE_CACHE` to that folder. `extract_pilot.mjs` needs `--stride 1` passed explicitly (without `--stride` its argument filter drops the first file).

Action kinds in the 1.0.0 action log (inferred from state changes): 4 move, 5 attack (an attack with no damage entry hits an altar lock), 6 cast, 11 recall start, 12 tribute start, 13 shop, 20 altar-lock placement.

## 1. Gem funnel (per game, means over 90 games)

| side | acquired from camps | acquired from kills | tributed | lost to champion kills | destroyed by monster deaths | carried at the end | residual |
|---|---|---|---|---|---|---|---|
| v4 | 269 | 3 | 159 | 62 | 15 | 35 | 0 |
| top 10 | 1,112 | 63 | 943 | 4 | 89 | 140 | 0 |

The residual (acquired − tributed − lost − destroyed − carried) is 0 for both sides, so the funnel balances.

ChatGPT's test (acquire about 550 and bank 499 = production; acquire about 950 = conversion): **v4 acquires 272 and banks 159 against the top 10.** Production is the main gap: 269 from camps against 1,112 (a factor of 4.1). Conversion is a second gap: v4 banks 58 % of what it acquires (159 of 272) against 80 % for the top 10 (943 of 1,175); 62 gems a game (23 % of acquisition) go to enemy champions.

Tribute channels are not the leak: 731 of v4's 769 tribute starts completed (95 %); 8 were cancelled by a lock, 12 by the carrier's death and 13 by a stun or hit (`tribute success` script, scratch). The gems are lost before the altar: carriers die to champions on the way or at the altar before they start.

Banked gems by round (mean): v4 21 / 42 / 71 / 100 / 123 at rounds 200 / 400 / 600 / 800 / 1,000, then 159 at 2,000. Top 10: 48 / 150 / 305 / 494 / 674, reaching 1,000 in 83 of 90 games (mean round 1,169). v4 never reaches 1,000. The gap opens before round 200 (21 vs 48) and widens at 0.8 gems a round against 0.1.

Per map (us / them):

| map | acq camp | acq kill | tributed | lost to champion kills | destroyed by monsters | carried at end | ability-open share |
|---|---|---|---|---|---|---|---|
| ambush | 629 / 1003 | 4 / 50 | 484 / 1012 | 50 / 4 | 70 / 15 | 28 / 23 | 0.53 / 0.78 |
| boss_battle | 250 / 2340 | 0 / 5 | 0 / 479 | 5 / 0 | 0 / 722 | 245 / 1149 | – / 1.00 |
| broken_curve | 283 / 1045 | 5 / 32 | 214 / 1020 | 27 / 5 | 39 / 27 | 8 / 26 | 0.42 / 0.82 |
| collision | 235 / 894 | 1 / 143 | 82 / 1014 | 138 / 1 | 9 / 9 | 7 / 14 | 0.75 / 0.76 |
| concentrated_wealth | 149 / 987 | 1 / 64 | 78 / 1022 | 61 / 1 | 3 / 1 | 8 / 26 | 0.26 / 0.79 |
| contest | 0 / 822 | 0 / 0 | 0 / 822 | 0 / 0 | 0 / 0 | 0 / 0 | 0.34 / 0.81 |
| corners | 358 / 958 | 0 / 76 | 239 / 1009 | 85 / 0 | 8 / 2 | 25 / 24 | 0.38 / 0.83 |
| corridors | 210 / 1155 | 8 / 60 | 139 / 1017 | 57 / 14 | 7 / 90 | 15 / 93 | 0.61 / 0.69 |
| eight | 205 / 954 | 10 / 115 | 92 / 1011 | 116 / 10 | 0 / 7 | 8 / 40 | 0.78 / 0.79 |
| gold_rush | 367 / 958 | 0 / 88 | 265 / 1024 | 82 / 0 | 12 / 19 | 7 / 4 | 0.47 / 0.77 |

On collision and eight v4 loses more gems to champion kills than it tributes (138 vs 82; 116 vs 92).

Self-play for comparison (local, engine 1.0.0, `results/s3-v4-vs-v3b-e1-s0.json`: v4 vs v3b-e1, 21 maps × 2 sides, salt 0, 42 games; mapgate 32/42 wins counting the gold tiebreak):

| v4 vs | games | acquired from camps | from kills | tributed | lost to champion kills | destroyed by monsters | carried at end | deaths (champion / monster / other) |
|---|---|---|---|---|---|---|---|---|
| v3b-e1 (local) | 42 | 791 | 60 | 744 | 66 | 14 | 30 | 16.8 (12.9 / 3.7 / 0.2) |
| top 10 (real) | 90 | 269 | 3 | 159 | 62 | 15 | 35 | 22.7 (18.5 / 4.1 / 0.1) |

**Reading:** against v3b-e1 v4 acquires 851 and banks 744 (87 %), so by ChatGPT's test the self-play problem would look like neither. Against the top 10 the same bot acquires 272: camp production falls by two thirds (791 to 269) while the gems lost to champion kills stay at about 62-66 a game. The loss to the top 10 is production first (they take the camps and v4 spends 22 % of its time dead), conversion second. Self-play does not show it; this is the same warning as the 68 % local vs 23 % real-team result.

## 2. v4's deaths against the top 10, by cause (step0 method)

Method (notes/step0-deaths.md): champion kill = an enemy champion action damaged the victim in the death round or the one before; monster = otherwise, with a wolf, elk or monolith camp within 8 tiles (camps mirrored by the map symmetry; `data/maps21.json` lists one half); other = neither. Cassi and Issac partner deaths take the first's cause.

| | deaths | per game | champion | monster | other |
|---|---|---|---|---|---|
| v4 | 2,042 | 22.7 | 1,667 (82 %) | 365 (18 %) | 10 (0 %) |
| v4 excluding contest | 1,604 | 19.8 | 77 % | 23 % | 0 % |
| top 10 | 542 | 6.0 | 47 (9 %) | 477 (88 %) | 18 (3 %) |

**Claude's deciding fact holds: 82 % of v4's deaths against the top 10 are champion kills (77 % without contest). Fights rise in priority.** This differs from the v3b mid-ladder panel (50 % champion, 48 % monster): against the top 10 the camps are no longer the main killer.

By location (champion kills): altar 722, camp 425, spawn 262, elsewhere 258. Monster deaths: camp 182, altar 100, spawn 81.
By round band (champion / monster): 0-200: 166 / 78; 200-600: 534 / 150; 600+: 967 / 137.
Victims: C&I 1,000 bodies (49 %; partner deaths included), Vendetta 310, Ryth 184, Stellar 174, Tavi 173, Varg 146, Ren 55.
Gems carried at champion deaths: 5,599 (62 a game), 368 of the 1,667 deaths carried gems.
Deaths per game by map: contest 48.7 (all champion), gold_rush 39.8 (30.2 champion), eight 25.7, concentrated_wealth 25.4, collision 24.8, corridors 17.7 (9.0 monster), broken_curve 17.8 (10.4 monster), ambush 13.8, corners 12.1, boss_battle 1.2.

v4 kills little: 9 % of the top-10 deaths are champion kills by v4 (47 in 90 games); v4 takes 3 gems a game from kills against their 63.

## 3. Lock awareness

Code (bots/v4/src/a/bot.cpp):
- Detection: only inside `Bot::bank()` (l.432-435). `vision::altar_locks(team::enemy)` marks a visible locked altar for 40 rounds (`lock_until`); `choose_altar` adds 3 to its score (l.58).
- At a locked altar in submit range: attack the lock with basic attacks; with an enemy within 4 tiles, only after 3 turns of waiting (l.446-459). Nothing else changes: the champion stays at the altar.
- At an unlocked altar in submit range: start the tribute at once, "whatever the enemies" (l.463-467; EXP-024/025: damage does not interrupt a tribute). Neither lock risk nor kill risk is checked, and a lock placed after the start cancels the channel (FACTS, Tribute line).
- v4 never buys or carries a lock (no `buy(consumable_type...)` call anywhere in src/a), so it never locks an enemy altar and has no interdiction of enemy tributes: it does not read `is_submitting_gems()` on enemies.

Measured: the top 10 placed 146 locks in the 90 games (1.6 a game; v4 0). 41 were placed with a v4 champion within 2 tiles of the altar in the next 40 rounds; 42 fell inside a v4 tribute window. Only 8 of v4's 769 tribute starts were cancelled by a lock, so the lock's main effect is to hold v4's carrier at the altar (clearing the lock costs turns) while enemies arrive.

Traced case (eight vs Mite, `v4top-t04-A`, game 1, altar (20,15)): v4's Cassi (body 3) stands at (19,14) with 37 gems and 585 health. Round 278: Mite's Cassi, 4 tiles away, casts on it for 426 (stun). Round 279: v4 starts a 4-turn tribute at 99 health with that enemy 3 tiles away. Round 280: Mite places an altar lock on (20,15) (action kind 20), cancelling the tribute, and attacks for 33. Round 281: hit for 26. Round 282: v4's Cassi and Issac are dead; Mite's Cassi now carries the 37 gems and tributes them with its Stellar at the same altar 24 rounds later. Earlier in the same game (rounds 150-156) a v4 carrier with 11 gems spent 7 turns attacking a lock at this altar before it could tribute at round 157.

## 4. Time use (share of champion-rounds; dead first, then tribute channel, recall channel, champion damage dealt or taken, camp damage, standing on a camp tile, moving, near own spawn, else idle)

| | walk | dead | clear | capture | recall | tribute | fight | spawn idle | idle |
|---|---|---|---|---|---|---|---|---|---|
| v4 | 47.2 % | 22.1 % | 10.1 % | 4.5 % | 4.8 % | 0.5 % | 1.7 % | 5.4 % | 3.6 % |
| top 10 | 44.1 % | 5.1 % | 16.9 % | 6.1 % | 16.4 % | 2.0 % | 1.7 % | 0.1 % | 7.4 % |

v4 spends 22 % of its champion-rounds dead (top 10: 5 %) and 5 % recalling (top 10: 16 %): the top 10 recall to heal where v4 dies. Clearing time is 10 % against 17 %. Dead time per map for v4: gold_rush 37 %, collision 36 %, contest 33 %, eight 31 %.

## 5. Ability-first share (for part 3)

Fights this side started (champion-on-champion damage clustered by 5-round gaps within 8 tiles) whose first damaging action was an ability cast: v4 0.50 (3.6 fights started a game), top 10 0.78 (18.4 a game). Share of all fights in which the side's own first damaging action was a cast: v4 0.47, top 10 0.73. (The brief's 85 % and 35-43 % come from a different fight extractor; this one is what metrics.py reports.)

## 6. Contest

| | v4 | top 10 |
|---|---|---|
| gems | 0 | 822 (tributes of exactly 200, 4.1 a game) |
| monolith damage dealt | 0 in 7 of 9 games; 110 and 264 (of 20,000) in the other two | all of it |
| deaths | 48.7 (47.9 champion) | 7.3 (6.0 near a camp) |
| dead time | 33 % | 5 % |

Contest has two monoliths, (17,31) and (32,18) (decoded camp list; one group each, one spawn tile each, 20,000 health, 200 gems). Both altars sit at the centre, (25,24) and (24,25), adjacent to each other and 7 tiles from each monolith. v4's siege mode is off on contest (`P_SIEGE_CONTEST 0`, engine 1.0 monolith range 5 > cast range 4), so v4 farms gold clusters and dies to the team holding the centre.

How the top 10 kill a monolith (`mono.py`, scratch; Pantheon, Sponge, procless): hitters stand at distance 1 (234 of 273 hits in Pantheon's first siege; 137 of 182 in Sponge's), with the ranged out of range; the monolith keeps hitting one tank (126 a hit through Jungle + Durability emblems) until that tank steps to distance 2 and recalls in place, then the next tank takes over (Pantheon rounds 214-249). First sieges last 190-424 rounds with leave-and-return (monolith health does not regenerate), 0-4 deaths; later sieges with gear last 17-34 rounds at 600-1,200 damage a round. Tributes: 200 gems each at a centre altar, a 20-turn channel, none interrupted by v4.
