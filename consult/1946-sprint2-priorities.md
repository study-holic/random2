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

Pasted by Xans on the evening of 20 Oct. Condensed below. Option numbers are translated to Prompt 1's order: 1 release, 2 fights, 3 production, 4 map-specific, 5 imitation, 6 new layer, 7 not listed, 8 none. Prompt 2 listed its options in reverse, so ChatGPT's and Copilot's numbers are translated.

### Claude (Prompt 1)
- **Gap decomposition:** alive share 77.8 % vs 94.9 % (×1.22). Pickups ×4.2 = ×1.6 more clearing and capturing time × ×2.7 more gems per such round (camp choice, clearing speed, gear, monoliths). Closing ×3.5 per alive round is work for 31 Oct.
- **Sprint 2:** the logistic expectation against rank 100 is 0.16 a game, so the bracket matches we can win are against teams near our rating.
- **Ranking:** 1 release (the draft travels inside the package; test the package with v4's draft vs the core draft) > 7 robustness (35-map sweep, fuel watchdog at about 90k, arena test of an overrun) > 5 narrowed to production queries (pickups by camp type; recalls by location and HP; gold spent and purchase order by round 200) > 3 built on 5 (siege every reachable monolith) > 2 avoidance only > 4 zero maps > 6 > 8.
- **Leave out:** ability-first (67 vs 75) and the second-death rule (73). The auction waits for a watchdog and at least 5,000 fuel of headroom.
- **Release:** freeze code Fri 04:00; submit the version with the best ladder performance rating.
- **Idea, a basketball screen:** a chased champion retreats past an awake camp, so the nearer pursuer takes the camp's hits (measured rule 1). Test in an arena first.
- **Pivotal fact:** dead share 22.2 % vs 5.1 %, which caps the gain from avoidance at ×1.22.
- **Measure:**
  - tributed gems, champion-kill deaths and pickups, paired, with v4 run in the same window; release at 2 SE or more of the paired gem difference;
  - a per-version ladder performance rating; decode ranked games;
  - self-play only as a regression gate (reject below 62/126);
  - choose per-map exceptions on one seed set and score them on another.
- **Unreliable facts flagged:** top-10 deaths 4.3 vs 6.0; 59 % vs 201/270 = 74 %; samples of 180, 90 and 91 games; a recall share of 15.8 % implies about 150-170 recalls a game; the 73 null may be harness bias; package 82 vs 70; duels exclude items; the decoders differ by 6 %.

### Copilot (Prompt 2)
- **Diagnosis:** the bot often fails to take part in the economy at all. Fix that; no rewrite.
- **Ranking:**
  - 1 release, in this order: pathing, spending, tribute rule, locks, contest siege, the auction only after a fuel cut, then map overrides;
  - 7 progress watchdog: track each champion's progress, then escalate (relax soft hazards, drop the claim, nearest feasible task, escape tile), with hysteresis against oscillation;
  - 4 map-specific: structural families, no seed-fit exceptions;
  - 3 production: assignment, travel, clearing and conversion measured separately;
  - 2 fights: disengage, focus only when committed, no blanket ability-first;
  - 5 imitation: conditional rules only (covariate shift);
  - then 6, then 8.
- **Fuel:** no production peak above about 85,000 without knowing the engine's overrun behaviour and having a fallback.
- **Idea:** model-predictive control with a safety shield. Log predicted against actual travel, clearing, banked gems and death-risk bands to check calibration.
- **Pivotal fact:** the pathing change's 1.5-1.8× gain with no map more than 1 % slower.
- **Measure:** a deterministic regression suite (zero-gem maps, cycles, stuck carriers, fuel); paired self-play with a second reference bot; a top-10 panel on fixed-round gems, conversion and deaths. Win rate last.

### ChatGPT (Prompt 2)
- **Diagnosis:** an economic throughput problem amplified by a death spiral: about 19 kills × 50 gold, or about 950 gold a game, handed to opponents.
- **Ranking:** 7 correctness and bottleneck removal > 3 production > 1 release, as a discipline > 4 map-specific > 2 fights (bounded: decline losing isolated fights, conditional abilities) > 5 > 6 > 8.
- **Changes:** reject the universal core draft (54/126). Freeze the auction: its 78 includes the pathing change, and it peaks at 97,209 fuel. Spending is the strongest ablation. Separate the lock and tribute effects. Run pathing-only on shortcut.
- **Idea, renewal-aware camp scheduling (operations research):** V = G·P(bank) / (travel + clear + capture + bank share + expected loss); vacate spawn tiles. It is falsified if spawn suppression is already negligible.
- **Pivotal fact:** the pickup gap, 270 vs 1,110-1,175.
- **Measure:** paired tributed-gem margin against a frozen v4, interleaved, with a clustered bootstrap; fuel below about 90,000.
- **Flags:** 59 % vs 74 %; deaths 4.3 vs 5.2 and 6.0; only 20 of 35 maps tested; a median round to 1,000 hides maps that never finish.

### DeepSeek (Prompt 1)
- **Diagnosis:** economy, conversion, survival and map robustness, not micro.
- **Ranking:** 7 as a prerequisite (rule checks, a frozen v4, a continuous-metric gate) > 3 production > 5 imitation of macro invariants > 4 map-specific > 1 release, as a mechanism (pathing alone first, then contest, then one macro package) > 6 > 2 > 8.
- **Locks:** our 4.0 a game may be wasted gold; only 5 of 146 top-10 locks cancelled a tribute.
- **Idea, a domestique escort:** one champion stands nearer the camp or altar to take monster aggro (rule 1), and clears locks while the carrier channels.
- **Pivotal fact:** that the 180 unranked games against the top 10 represent the bracket's top 10 (they may run test builds).
- **Measure:** no-opponent regression on 35 maps; self-play at 80/126 or more to count; a top-10 panel on continuous metrics with a pre-registered rule (for example +15 % median tributed gems, +10 points conversion, deaths not up more than 10 %).

## Claim check and classification (lead, 20 Oct)

| Claim | Who | Class | Check |
|---|---|---|---|
| No new decision layer or auction before the lock | all 4 | agrees across all four; new against Draft 4's S1 plan | auction 78 vs pathing-only 74 is inside noise; peak 97,209 (results/v5-s1.md) |
| Reliability first: 35-map sweep, stuck detection, fuel watchdog | all 4 | new | confirmed tonight: no build passes all 35 maps (v5 shortcut 0-0; pathing-only lost shortcut A 84-115, and starvation hit 100,000 fuel; results/s1g1-newmaps-vs-starter-20261020.json) |
| Ship the pathing change first | all 4 | agrees | needs the starvation fuel fix first |
| The core draft inside the package may be offsetting it: A/B the draft | Claude; all 4 doubt it | new | draft alone scored 54/126 (results/v5-s2a.md) |
| Drop ability-first and the second-death rule | 4 / 2 | agrees | 67 vs 75; 73 = null |
| Judge on paired continuous metrics, with v4 in the same window | all 4 | new for "same window" | NOT POSSIBLE: `match-groups create` has no version option, so groups always play the live submission. Use A-B-A in alternating windows |
| Self-play as a regression gate only (reject below 62) / promotion needs 80+ | Claude / DeepSeek | contradict each other | adopt 62 as the rejection gate; 80 is not required (the panel decides) |
| 59 % conversion is inconsistent with 201/270 | Claude, ChatGPT | correct | our prompt mixed samples: 59 % = 159/269 from the 90 A/B games (results/v5-s3-diag.md); 201 is from the 180-game baseline. Erratum |
| Top-10 deaths 4.3 vs 6.0 | Claude, ChatGPT | correct | 4.3 from the 180-game baseline script, 6.0 from S3's 90-game decode; reconcile (method or sample) |
| Recall share 15.8 % implies about 170 recalls a game | Claude | plausible | 5 bodies × about 1,100 rounds × 15.8 % / 5 ≈ 174; verify the decoder's label before copying recall behaviour |
| Lock value may be overstated in self-play if v4 does not clear locks | Claude | partly answered | v4 does detect and attack locks (bot.cpp:432-461); it attacked 26 of 146 top-10 locks, and one took 5 hits |
| 82 → 70 means the package regressed | Copilot | contradicts | these are two separate implementations (builds A and B); B scored 78 before its fuel stop (results/v5-s2b.md) |
| Spawn-suppression scheduling | ChatGPT | new idea, mostly falsified by our data | self-block is 0-6 champion-rounds a game except concentrated_wealth, 601 (results/v5-s1-diag.md) |
| Winnable bracket matches are near our rating: decode ranked games and compute a per-version performance rating | Claude (DeepSeek on representativeness) | new | ranked games run anyway and use no quota; the top-10-only rule applies to unranked groups |
| Logistic 0.16 vs rank 100; 0.43 appearances per map per 15 games; about 2,800 groups to Fri 04:00 | Claude | checked | 1/(1+10^(284/400)) = 0.163; 15/35 = 0.43; 56 h × 50 = 2,800 |
| Fuel ceiling for new code 85-95k | Copilot, ChatGPT, Claude | new | our gate allows 100k; adopt 90,000 for new code |

## Synthesis (lead)
- **Consensus of all four:**
  - no rewrite;
  - reliability on all 35 maps first;
  - the pathing change first;
  - ability-first and the second-death rule left out;
  - panels judged on continuous paired metrics;
  - fuel headroom.
- **What changes:** the release order (PLAN-v4.md, Draft 5).
- **Advice we can't follow:** the concurrent-control advice cannot be followed on this platform; A-B-A in alternating windows replaces it.
- **Errata for our prompt:** 59 % and 0/90 come from the 90-game sample; 201 and 4.3 come from the 180-game sample.
