# Consult 19:46 BST Tue 20 Oct: priorities until the Sprint 2 lock

Xans pastes Prompt 1 into DeepSeek and Claude, and Prompt 2 into ChatGPT and Copilot, each in a fresh chat, then pastes the answers back. Answers go under "Answers" below; the lead then classifies each claim (new / agrees / contradicts), checks every factual claim against docs/rules/GameRules.md, FACTS.md or the data, and records what changed in PLAN-v4.md.

Sources (lead, not pasted): top-10 baseline notes/panel-v4-top10.md; funnel, deaths, time use, gear, locks results/v5-s3-diag.md (main, 1,112) and branch claude/happy-heisenberg-k4ttd6 results/v5-s3.md (1,175, siege 333/13.7, opener 67/126); unopposed and auction results/v5-s1.md, results/v5-s1-diag.md; S2 results/v5-s2b.md, results/v5-s2a.md; second-death results/v5-w1.md (73/126); new maps results/gate-v5-20261020-183901Z-win-mapgate.json; shortcut 564/210 results/s1-shortcut-vs-starter-20261020.json; ladder preflight 19:39. Bias review: notes/critic-consult-1946.md (14 of 16 edits applied; 'slows no map' kept for the two fixes alone, worst corners +1 %; branch figures verified).

Rules for these prompts (skill /consult): facts only, no stated preference, every option given equal space, and the evidence against the current plan included.

---

## Prompt 1 (rules-heavy: DeepSeek, Claude)

We are a team in a bot-programming competition (Cambridge Battlecode 2026, engine 1.0.0). We want an independent strategic opinion on how to use the next four days. Please be concrete and critical, and tell us where our facts look unreliable.

**The game.**
- Two teams of 3 to 5 champions each, the number set by the map, play on a grid. The pool has 35 maps; the team size per map is listed below.
- **Win:** the first team to 1,000 tributed gems. At round 2,000 the tiebreaks are tributed gems, then lifetime gold, then kills.
- **Draft:** 12 champions. In each draft round both teams' picks are revealed together; the two teams may pick the same champion, a team may not pick one twice.
- **Camps** are groups of monsters: wolf (4 gems, 40 gold, attack range 3), elk (15 gems, 90 gold, range 4), monolith (20,000 HP, 200 gems, 300 gold, range 5, respawn 150 rounds), gem cluster (7 gems), gold cluster (20 gold).
- **Capture:** kill every monster of a group, then stand on one of its spawn tiles for 5 rounds with no enemy on any of its spawn tiles. Allies standing on the tiles split the reward.
- **Tribute:** stand within 1 tile of an altar and channel for ceil(bag/10) turns; the whole bag is tributed at the end. A stun, the champion's death, or an enemy lock newly placed at that altar cancels the channel. Damage does not.
- **Altar lock:** a 10-gold consumable placed within 2 tiles of an altar; it blocks the other team's tributes there and is removed by basic attacks (the rules say 3; one replay showed 5).
- **Death:** respawn after 60 rounds. The killer takes all carried gems plus 50 gold. A champion killed by a monster loses its carried gems.
- **Recall:** a 5-turn channel home, cancelled by damage, stun or death. Gold buys stat shards (30 gold) on the team's spawn tiles or while dead; 5 matching shards form an emblem.
- **Bots** run in WebAssembly with 100,000 fuel per turn. Allies share no vision; they share 8 bytes of memory per team, plus 1,024 bytes written during the draft that are read-only in the match.

**Rules we measured in engine test arenas (20 Oct; they add to or differ from the published rules):**
1. Monsters attack the nearest champion of either team, by Chebyshev distance; a tie alternates targets every round. A camp woken by one team keeps hitting a nearer champion of the other team.
2. When a camp's respawn timer reaches 0, the group waits for as long as any of its spawn tiles is occupied, by either team, and respawns the round the tile is freed. A champion next to the tiles has no effect.
3. Monolith damage does not regenerate: 380 rounds after one hit, its HP was unchanged.
4. Duels without items: Heart of Ruin beats Prism, Stellar, Willow, Tavi and Vesper every time and wins 30 % of duels against Varg, 33 % against Ryth and none against Cassi & Issac or Marcella. Prism gains +1 Focus per carried gem.

**Team size per map.** 3 a side: broken_curve, collision, fragment, gold_rush, packs, the_farm. 4: hooks, interlock, reverse_spiral, rows, rushbait_v2, shortcut, swirl. 5: the other 22.

**Ladder (20 Oct).** Our rating 1,388 (rank 536 of 716) at 09:36, and 1,429 (rank 481 of 720) at 19:39 after 258 ranked games. At 19:39, rank 10 was 1,991, rank 20 1,919, rank 50 1,795 and rank 100 1,713. On 18 Oct a 5-game series near 1,500 moved rating by 32 × (wins ÷ games − expectation), and teams played about 6 ranked series an hour.

**Our live bot (v4, live since 09:46 on 20 Oct) against the top 10.** 180 unranked games against the teams ranked 2-10, 20 maps, 9 games a map:
- W-L 2-178. Tributed gems a game 201 vs 976; tributes a game 11.6 vs 33.6; deaths a game 23.4 vs 4.3; gems lost to champion kills 58 vs 3.
- Gems picked up from camps a game: about 270 for us, about 1,110 to 1,175 for them (two decoders). We tribute 59 % of what we pick up, they about 85 %.
- 81 % of our deaths in these games are champion kills. The top 10 die 6.0 times a game, 5.2 of them to monsters.
- Share of champion-rounds, us vs them: dead 22.2 % vs 5.1 %; walking 48.5 % vs 46.3 %; clearing camps 10.2 % vs 17.1 %; capturing 4.0 % vs 5.3 %; fighting champions 0.8 % vs 1.4 %; tributing 0.4 % vs 2.2 %; recalling 4.6 % vs 15.8 %; idle 9.4 % vs 6.8 %.
- Gear points per champion at round 200: 1.9 vs 5.3. Fights opened a game 3.7 vs 19.6; share of fights opened with an ability 47 % vs 79 %.
- Games reaching 1,000 tributed gems: 0 of 90 for us, 83 of 90 for them (median round 1,086).
- The top 10 placed 1.6 altar locks a game; 5 of their 146 locks cancelled one of our tributes in progress.
- Contest (its only gem sources are two monoliths, 200 gems each): we score 0 and die 48.7 times a game; they bank about 4 monoliths a game.
- Top-10 draft frequency: Stellar 96 %, Cassi & Issac 95 %, Ren 72 %, Varg 70 %, Ryth 67 %, Vendetta 3 %. Our live bot opens with Vendetta on 14 of 21 old maps.
- One of the top 3 teams buys no locks.

**Our bot with no opponent** (against a bot that passes every turn, old 21 maps): it reaches 1,000 gems within 2,000 rounds on 12 of 21 maps; 0 on contest and boss_battle; under 400 on broken_curve, corridors, fracture, gold_rush and elk_arena. Traces show two behaviours on these maps: carriers stop 2 tiles from an altar that lies within 5 tiles of a sleeping monolith (boss_battle, broken_curve, fracture, gold_rush), and a champion boxed in by spawn tiles stands still for 900 rounds (concentrated_wealth). A pathing change that removes both raises unopposed gems about 1.5 to 1.8 times on broken_curve, fracture and gold_rush and by 12 % on concentrated_wealth; no map is more than 1 % slower. Contest and boss_battle stay at 0 (boss_battle's only altar can be reached only from a monolith's spawn tile).

**The 14 maps added to the pool on 20 Oct** (ranked games already use them). Our candidate bot against the starter template bot, one game per side, tributed gems side A / side B: shortcut 0 / 0 (0-0 gems after 2,000 rounds, lost on gold both times); stampede 80 / 0; pilgrimage 243 / 298; reverse_spiral 385 / 372; starvation 400 / 404; rushbait_v2 444 / 1,007; split 759 / 807 with 45 / 36 deaths; packs 974 / 1,026; quadrants, rows, starfish, swirl, the_farm and yellow_brick_road reached 1,000 on both sides (swirl with 36 / 37 deaths, quadrants 8 / 30, yellow_brick_road 18 / 18). Our task-allocation build (row 4 of the table below, which includes the pathing change) scores 564 and 210 gems on shortcut against the same bot, with 28 and 13 deaths.

**Changes we built and measured today, in self-play against v4** (126 games: 21 maps × 2 sides × 3 seeds). Salted copies of v4 score 73 of 126 against v4 in the same harness (an even split would be 63). Earlier this month a version scored 68 % in self-play and 23 % against real teams.

| Change | Wins of 126 | Deaths a game, us / v4 | Other measures |
|---|---|---|---|
| Draft the top-10 core champions on every map (no other change) | 54 | 21.1 / 21.3 | gem margin −21 a game |
| Core draft + locks + tribute rule (do not start when an enemy that can stun, lock or kill is in reach) + spend gold on shards at every visit | 70 | 17.0 / 22.9 | gem margin +132; gear at round 200 2.71 vs 2.25; 4.0 locks a game |
| Same package, earlier build; one part removed at a time | 82 full; 61 without spending; 68 without locks; 78 without the tribute rule | 17.4 / 23.5 | |
| Task allocation by sequential auction with time windows, plus a path cost near awake camps, plus the pathing change above | 78 | 20.4 / 21.9 | unopposed: median map 1.3 % faster, corners 15 % slower, 15 of 21 maps reach 1,000 (12 before) |
| The pathing change alone | 74 | 20.3 / 21.9 | |
| Contest-only siege (two shard-bought tanks at the monolith, others behind, recall in place when hurt) | 75 (v4's salted copies: 73) | contest 13.7 / 43.7 | on contest: 333 gems a game vs v4 (v4 0), 6 of 6 won |
| Abilities before basic attacks when a fight starts | 67 (75 without) | 20.0 / 21.9 | share of fights opened with an ability 0.48 → 0.63 |
| A champion avoids a camp group only after a second death there (was the first) | 73 | 22.1 / 21.7 | |

None of these has played real teams yet. Noise we have seen: builds of the same package ranged from 71 to 78 wins of 126; one map-side can swing 2× in gems between seeds; per-map exceptions in one build were chosen on the same games they were scored on.

**The question.** With four days to the lock, how should we spend the effort? Options we see, in no order:
1. **Release the changes measured today, in sequence**, each judged against the top 10 in unranked games.
2. **Champion-versus-champion play:** when to engage, retreat and recall, focus fire, opening with abilities, spacing from enemy groups.
3. **Camp production on every map:** which groups each champion takes, travel, clearing speed, when to bank.
4. **Map-specific work** on the maps where our gems are lowest, including the new ones.
5. **Imitation of the top teams** from their decoded replays: routes, recall timing, purchases and drafts, measured and copied.
6. **A new decision layer** that plans for the team as a whole instead of each champion choosing alone.
7. An option not listed.
8. None of these.

**Requests.**
1. Rank the options and give your reasons.
2. Name the most likely failure mode of each.
3. Propose one idea from another game, sport or competition that maps onto a mechanic above, and say which rule it uses.
4. Say which fact above, if it were wrong, would change your ranking.
5. Say how you would tell, within these four days, whether a change helps.

**Constraints.** The Sprint 2 bracket uses the bot submitted at Sat 24 Oct 23:59 BST: single elimination, 15 games per pairing. Further sprints lock 31 Oct, with qualifiers 4 and 11 Nov. We want the last large change live by Fri 23 Oct 04:00 Compute: three to six parallel build-and-test sessions, each able to run 126 self-play games in about 3 minutes; unranked games only against the current top 10, at most 50 groups of 5 games an hour, and a 36-group panel takes about 2 hours to finish. Our builds peak at 90,697 to 97,728 of the 100,000 fuel per turn.

---

## Prompt 2 (code and data: ChatGPT, Copilot)

We are a team in a bot-programming competition (Cambridge Battlecode 2026, engine 1.0.0). We want an independent opinion on where to spend the next four days of engineering, from the code structure and the data below. Please be concrete and critical, and tell us where our data or measurement look unreliable.

**The game in brief.** Two teams of 3-5 champions (set by the map; 35 maps) farm monster camps for gems and gold and tribute gems at altars; first to 1,000 tributed gems wins, else at round 2,000 tributed gems, then lifetime gold, then kills decide. Capture = kill a group, then hold one of its spawn tiles 5 rounds unopposed. Tribute = channel ceil(bag/10) turns within 1 tile of an altar; stun, death or a newly placed enemy lock cancels it, damage does not. A lock (10 gold) within 2 tiles of an altar blocks enemy tributes until attacked off. Death: 60-round respawn; the killer takes the bag plus 50 gold; a monster kill destroys the bag. Recall: 5-turn channel home, cancelled by damage. Gold buys shards (30) at spawn or while dead. Monsters attack the nearest champion of either team (Chebyshev; measured), and an occupied spawn tile holds a group's respawn (measured). Monolith: 20,000 HP, 200 gems, range 5, respawn 150, does not regenerate (measured).

**Our code (C++ to WebAssembly, 100,000 fuel per turn; our peaks are 90,000-97,000).**
- One process per champion; no shared vision. Shared state: 8 bytes per team, visible to allies at once; we use bytes 0-5 as each champion's claimed camp group, 6-7 for squad claims. 1,024 draft-time bytes are read-only in the match.
- Each turn: build a view (camps within 4 tiles, wider for the target), read enemies and allies, an enemy response (fight or flee by a danger score), then target choice: each champion scores camp groups independently (distance, wait for respawn, value, contested-ness), heavy groups go through a squad claim, then banking (walk to an altar when the bag passes a size or age), then shopping at spawn.
- Per-map plan hooks (draft list, pre-turn interrupts, target override, buy override); 21 maps have exact identification; the 14 new maps are not identified yet.
- Test harness: our bot against a panel, all maps, both sides, seeds 0-2. Games are deterministic per seed. Salted copies of our bot score 73 of 126 against it (an even split would be 63). A replay decoder gives per-round positions, actions, gems, deaths and causes for our games and for unranked games against real teams (180 such games are decoded).

**Results against the top 10** (ranks 2-10, 180 unranked games, 20 maps, 20 Oct). Us / them per game:

| map | W-L | tributed gems | deaths | gems lost to kills |
|---|---|---|---|---|
| ambush | 0-9 | 484 / 1012 | 13.8 / 4.6 | 50 / 4 |
| collision | 0-9 | 82 / 1014 | 24.8 / 0.9 | 142 / 1 |
| contest | 0-9 | 0 / 822 | 48.7 / 7.3 | 0 / 0 |
| corners | 0-9 | 239 / 1009 | 12.1 / 1.0 | 76 / 0 |
| eight | 0-9 | 92 / 1011 | 25.7 / 2.9 | 115 / 10 |
| elkring | 0-9 | 254 / 1008 | 18.6 / 1.9 | 161 / 2 |
| expanse | 0-9 | 622 / 1014 | 3.2 / 1.6 | 13 / 0 |
| fragment | 0-9 | 32 / 1009 | 52.7 / 1.9 | 36 / 7 |
| interlock | 0-9 | 175 / 1017 | 25.2 / 2.0 | 73 / 0 |
| lanes | 0-9 | 392 / 1009 | 16.6 / 3.4 | 54 / 5 |
| monolith | 0-9 | 324 / 1029 | 22.1 / 1.3 | 107 / 6 |
| boss_battle | 1-8 | 0 / 479 | 1.2 / 12.7 | 5 / 0 |
| broken_curve | 0-9 | 214 / 1020 | 17.8 / 4.6 | 27 / 5 |
| concentrated_wealth | 0-9 | 78 / 1022 | 25.4 / 1.8 | 64 / 1 |
| corridors | 0-9 | 139 / 1017 | 17.7 / 14.1 | 57 / 8 |
| elk_arena | 0-9 | 10 / 1010 | 23.2 / 2.8 | 0 / 2 |
| fracture | 0-9 | 156 / 1022 | 16.8 / 5.8 | 42 / 6 |
| gold_rush | 0-9 | 265 / 1024 | 39.8 / 10.4 | 87 / 0 |
| hooks | 0-9 | 2 / 1007 | 55.1 / 4.0 | 26 / 4 |
| islands | 1-8 | 461 / 958 | 7.9 / 1.7 | 30 / 0 |
| all | 2-178 | 201 / 976 | 23.4 / 4.3 | 58 / 3 |

Further measures in the same games, us / them: gems picked up from camps about 270 / about 1,110-1,175 (two decoders); share tributed 59 % / about 85 %; 81 % of our deaths are champion kills (theirs: 5.2 of 6.0 deaths are monsters); time dead 22.2 % / 5.1 %, clearing camps 10.2 % / 17.1 %, recalling 4.6 % / 15.8 %, idle 9.4 % / 6.8 %, walking 48.5 % / 46.3 %; gear points per champion at round 200 1.9 / 5.3; fights opened 3.7 / 19.6 a game, 47 % / 79 % opened with an ability; locks placed 0 / 1.6 a game (5 of their 146 cancelled one of our tributes). Their drafts: Stellar 96 %, Cassi & Issac 95 %, Ren 72 %, Varg 70 %, Ryth 67 %; ours open with Vendetta on 14 of 21 maps. On contest they bank about 4 monoliths a game.

**Our bot with no opponent** (old 21 maps): reaches 1,000 gems within 2,000 rounds on 12; 0 on contest and boss_battle; under 400 on broken_curve, corridors, fracture, gold_rush, elk_arena. Traces show carriers standing 2 tiles from an altar that lies within 5 tiles of a sleeping monolith (pathing treats the monolith's range as a wall), and a champion inside a 5×5 block of spawn tiles unable to step out (spawn tiles are walls to its pathing). A pathing change that removes both raises unopposed gems 1.5-1.8 times on broken_curve, fracture and gold_rush and 12 % on concentrated_wealth; no map is more than 1 % slower. Contest and boss_battle stay at 0 (boss_battle's only altar is reachable only from a monolith spawn tile).

**New maps** (pool 21 → 35 on 20 Oct; ranked games use them). Our candidate vs the starter bot, one game per side, tributed gems side A / side B: shortcut 0 / 0 (0-0 gems after 2,000 rounds, lost on gold both times); stampede 80 / 0; pilgrimage 243 / 298; reverse_spiral 385 / 372; starvation 400 / 404; rushbait_v2 444 / 1,007; split 759 / 807 with 45 / 36 deaths; packs 974 / 1,026; quadrants, rows, starfish, swirl, the_farm and yellow_brick_road reached 1,000 on both sides (swirl with 36 / 37 deaths, quadrants 8 / 30, yellow_brick_road 18 / 18). Our task-allocation build (row 4 of the table below, which includes the pathing change) scores 564 and 210 gems on shortcut against the same bot, with 28 and 13 deaths.

**Changes built and measured today** (self-play vs v4, 126 games; v4's own salted copies score 73). A version this month scored 68 % in self-play and 23 % against real teams.

| Change | Wins / 126 | Deaths us / v4 | Other |
|---|---|---|---|
| Top-10 core draft on every map only | 54 | 21.1 / 21.3 | gem margin −21 |
| Core draft + locks + tribute rule + spend gold on shards every visit | 70 | 17.0 / 22.9 | gem margin +132, gear r200 2.71 vs 2.25 |
| Earlier build of that package, one part off at a time | 82 full; 61 no spending; 68 no locks; 78 no tribute rule | 17.4 / 23.5 | |
| Sequential-auction task allocation with time windows + path cost near awake camps + the pathing change | 78 | 20.4 / 21.9 | unopposed median map −1.3 % rounds to 1,000; corners +15 %; peak fuel 97,209 |
| The pathing change only | 74 | 20.3 / 21.9 | |
| Contest-only siege | 75 | contest 13.7 / 43.7 | contest 333 gems vs 0, 6/6 |
| Ability-first fight opening | 67 (75 without) | 20.0 / 21.9 | ability-opened share 0.48 → 0.63 |
| Avoid a camp group after the second death there, not the first | 73 | 22.1 / 21.7 | |

None of these has played real teams yet. Noise we have seen: builds of the same package ranged from 71 to 78 wins of 126; one map-side can swing 2× in gems between seeds; per-map exceptions in one build were chosen on the same games they were scored on. Ladder 20 Oct 19:39: we are 1,429 (rank 481 of 720); rank 20 is 1,919, rank 10 1,991.

**The question.** With four days to the lock, where should the engineering go? Options we see, in no order:
1. **A new team-level decision layer** in place of per-champion choice.
2. **Imitation of the top teams** from decoded replays: routes, recall timing, purchases, drafts.
3. **Map-specific work** on the lowest-gem maps, including the new ones.
4. **Camp production on every map:** group choice, travel, clearing speed, banking.
5. **Champion-versus-champion play:** engage, retreat and recall rules, focus fire, ability-first openings, spacing.
6. **Release the measured changes in sequence**, each judged against the top 10 in unranked games.
7. An option not listed.
8. None of these.

**Requests.**
1. Rank the options with reasons, using the numbers above.
2. Name the most likely failure mode of each, of any kind.
3. Propose one idea from another game, competition or engineering field that maps onto a mechanic above, and the data that would test it.
4. Say which fact above, if it were wrong, would change your ranking.
5. Say what measurement you would trust, within four days, to decide whether a change helps.

**Constraints.** The Sprint 2 bracket uses the bot submitted at Sat 24 Oct 23:59 BST (single elimination, 15 games per pairing); later locks 31 Oct, qualifiers 4 and 11 Nov. We want the last large change live by Fri 23 Oct 04:00. Compute: three to six parallel build-and-test sessions, each running 126 self-play games in about 3 minutes; unranked games only against the current top 10, at most 50 groups of 5 an hour, a 36-group panel takes about 2 hours.

---

## Answers
