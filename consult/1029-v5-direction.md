# Consult 10:29 BST Tue 20 Oct: v4 real-team result and the v5 direction

> **Erratum (10:58):** both prompts said a tribute is interrupted by damage. Wrong: GameRules l.181 and l.229 say damage does not stop a tribute; only a stun, death or a newly placed altar lock does (recall is stopped by damage). Copilot caught it; a correction follow-up was sent to DeepSeek, Claude and ChatGPT.

Senders: DeepSeek and Claude get Prompt 1 (rules-heavy); ChatGPT and Copilot get Prompt 2 (code and data). Copilot is pasted by Xans (the browser safety check blocks the agent from sending files to Copilot). Answers are appended below with claim checks.

---

## Prompt 1 (rules-heavy: DeepSeek, Claude)

We are a team in a bot-programming competition (Cambridge Battlecode 2026, engine 1.0.0). We want an independent strategic opinion. Please be concrete and critical.

**The game.**
- Two teams of 3 to 5 champions each, set by the map, play on a grid map. 21 maps: 15 maps have 5 a side, hooks and interlock have 4, and broken_curve, collision, fragment and gold_rush have 3.
- **Win:** the first team to 1,000 tributed gems wins. At round 2,000 the tiebreaks are tributed gems, then lifetime gold, then kills.
- **Draft:** teams draft from 12 champions. Within each draft round both teams' picks are revealed together, and a team cannot pick the same champion twice.
- **Camps** are groups of monsters: wolf (4 gems, 40 gold, attack range 3), elk (15 gems, 90 gold, range 4), monolith (20,000 HP, 200 gems, 300 gold, range 5, respawn 150), gem cluster (7 gems) and gold cluster (20 gold).
- **Capture:** kill every monster in a group, then stand on one of its spawn tiles for 5 rounds with no enemy on any of its spawn tiles. Allies on the tiles split the reward.
- **Tribute:** a champion stands within 1 tile of an altar and channels for ceil(bag/10) turns. The channel tributes the whole bag, and damage or a stun interrupts it.
- **Altar lock:** a 10-gold consumable placed within 2 tiles of an altar. It blocks enemy tributes there and is removed by 3 basic attacks.
- **Death:** the champion respawns after 60 rounds. The killer takes all its carried gems plus 50 gold. A champion killed by a monster loses its gems (they are destroyed).
- **Recall:** a 5-turn channel home, interrupted by damage or stun.
- **Gold** buys stat shards (30 gold); 5 matching shards form an emblem.
- **Vision** is 8 tiles; walls and bushes block sight, but monsters see through bushes.
- **Bots:** each turn has a 100,000-fuel CPU budget (C++ compiled to WebAssembly).

**Rules measured in engine test arenas (20 Oct; these differ from or add to the published rules):**
1. Monsters attack the **nearest** champion of either team, and switch as soon as someone comes closer. A camp woken by team A keeps hitting a team-B champion who stands nearer and never attacked it.
2. A camp's respawn timer counts down normally. At zero, the group waits for as long as **any** of its spawn tiles is occupied, by either team, and respawns in the round the tile is freed. A champion standing next to the tiles has no effect, contests nothing, and gets no share of the reward.
3. Prism gains +1.0 Focus per carried gem. With 21 or more gems it beats Stellar in 80–95 % of duels, and it loses to Ren and Varg at every gem level.
4. In duels, Heart of Ruin beats Prism, Stellar, Willow, Tavi and Vesper every time. It loses to Cassi & Issac, Marcella, Varg (30 %) and Ryth (33 %).

**Our results (all on engine 1.0.0, 20 Oct).** Ladder at 09:36: our rating is 1,388 (rank 536 of 716). Rank 20 is at 1,897 and rank 10 at 1,970. Our new version v4 went live at 09:46. In 160 unranked games against 8 teams rated 1,453–1,771, with the same teams and maps for both versions:

| | previous version | v4 |
|---|---|---|
| W-L | 29-131 | 37-123 (23 %) |
| Gems per game, us / them | 383 / 781 | 499 / 782 |
| Deaths per game, us | 37.9 | 23.0 |
| Gems lost to champion kills, us / them | 69 / 31 | 74 / 29 |

Per map, v4 (8 games each, W-L, gems us/them, our deaths per game):
- contest: 0-8, 0/325, 51.5. Neither version has ever scored a gem there; the gem sources on that map are the monolith and kills.
- fragment: 0-8, 236/882, 53.6.
- hooks: 0-8, 148/984, 55.6, with 5 tributes a game.
- gold_rush: 0-8, 432/898, 45.4.
- Near even: corners 3-5, 720/860, 10.6; expanse 5-3, 922/757, 2.2; lanes 2-6, 787/933, 11.0; monolith 1-6, 756/995, 15.6; islands 4-3, 910/570, 7.4.


**v4 against the top 10** (unranked, 20 Oct 10:00–10:30; teams ranked 2–10, rank 1 had no eligible bot; 10 of the 20 maps, 90 games): **1-89**. The one win is a 0-0 tiebreak on boss_battle.

| | v4 | Top-10 teams |
|---|---|---|
| Gems per game | 159 | 943 |
| Deaths per game | 22.7 | 6.0 |
| Gems lost to champion kills per game | 62 | 3 |

Per map (9 games each; gems us/them, deaths us/them, tributes us/them):

| Map | Gems | Deaths | Tributes |
|---|---|---|---|
| ambush | 484/1012 | 13.8/4.6 | 12.0/23.3 |
| corners | 239/1009 | 12.1/1.0 | 20.8/44.1 |
| collision | 82/1014 | 24.8/0.9 | 6.4/38.7 |
| eight | 92/1011 | 25.7/2.9 | 3.6/28.3 |
| contest | 0/822 | 48.7/7.3 | 0/4.1 |
| gold_rush | 265/1024 | 39.8/10.4 | 18.7/52.7 |
| broken_curve | 214/1020 | 17.8/4.6 | 7.4/39.9 |
| concentrated_wealth | 78/1022 | 25.4/1.8 | 5.8/34.4 |
| corridors | 139/1017 | 17.7/14.1 | 6.3/18.4 |
| boss_battle | 0/479 | 1.2/12.7 | 0/4.1 |

Each cell has fewer than 30 games.

**Where our deaths came from** (previous version, 160 games):
- 48 % to monsters near camps or altars, and 50 % to champions.
- Top-10 teams: 22 % to monsters and 73 % to champions, at 14.2 deaths per game.
- 43 % of our deaths were within 8 tiles of an altar.

**Top-10 teams compared with us** (ranked games):
- Fights opened with an ability: 85 % vs 35–43 %.
- Gear points per champion at round 200: 3.8 vs 1.7.
- Gems per tribute: 41 vs 24–30.
- Altar locks per game: 6.4 vs 0.
- Drafts: Stellar 96 %, Cassi & Issac 95 %, Ren 72 %, Varg 70 % and Ryth 67 %; Vendetta 3 % and Heart of Ruin 0 %. Our v4 drafts Vendetta first on 14 of 21 maps.

**Local self-play:** v4 beat the previous version's code in 86 of 126 games (68 %). Self-play and the real panel disagree on corners, which went 0/6 in self-play and 3-5 in both real panels.

**Our pre-registered rule:** "if v4 wins under 30 % against the panel, the gap is structural: the economy and routing module goes first and fight micro waits." v4 won 23 %.

**Question.** What should the next 2.5 days of work go to, to reach rank 20 by Friday? Options, in no order:
- **A. Fight module:** ability-first openers, focus targets, interrupting tribute and recall channels, protecting gem carriers.
- **B. Economy and routing module:** per-map route tables computed offline, one capturer per camp group, routing around respawn timers.
- **C. Draft overhaul** toward the top-10 core.
- **D. Denial:** holding enemy camp spawn tiles to stop respawns, and using monster aggro against enemies (arena rules 1 and 2).
- **E. Map-specific plans for the four contact maps** (for example avoidance, turtling or a gold tiebreak).
- **F. Gear curve and early recall.**
- **G.** An option not listed.
- **H.** None of these.

**Please:**
1. Rank the options, with reasons tied to the rules and numbers above.
2. Name the most likely failure mode of each.
3. Propose one idea from another game or competition (a MOBA, an RTS, a sport, or an earlier Battlecode season) that maps onto one of these mechanics. Name the mechanic.
4. Say which single fact above, if wrong, would most change your answer.
5. Say whether there is a game-changing macro or micro idea that these rules make possible and that the numbers suggest top teams already use or miss.

**Constraints:**
- About 60 hours to the main release (Friday 14:00 BST); the competition locks Saturday 23:59 BST.
- Three parallel cloud build sessions of 4 CPUs each. Local games take about 1 second each and are deterministic.
- 50 unranked match groups (5 games each) an hour, played only by our live bot and only against top-10 teams.
- A 5-0 ranked series between equally rated teams moves rating by about +16 (Elo, K = 32 per series), and teams play about 6 series an hour.

---

## Prompt 2 (code and data: ChatGPT, Copilot)

We build a C++ bot for Cambridge Battlecode 2026 (engine 1.0.0) and want an engineering-and-data opinion on what to build next. Please be concrete and critical.

**The game in brief.**
- Teams of 3 to 5 champions per map, drafted from 12 champions.
- Champions kill monster camps for gems and gold: wolf (4 gems, range 3), elk (15 gems, range 4), monolith (200 gems, range 5) and clusters.
- A camp is captured by holding one of its spawn tiles for 5 rounds with no enemy on them.
- Gems are carried until they are tributed: a channel next to an altar that lasts ceil(bag/10) turns and is interrupted by damage or a stun.
- The first team to 1,000 tributed gems wins; at round 2,000 tributed gems then lifetime gold decide.
- Killing a champion takes its carried gems; a monster kill destroys them.
- Respawn takes 60 rounds. Recall is a 5-turn channel home.
- Gold buys shards: 5 shards make an emblem, about 150 gold.
- Vision is 8 tiles and bushes block it, but monsters see through bushes.
- 100,000 CPU fuel per turn (about 1 ms of work). The first play turn has 200,000 and the draft turn 1,000,000.

**Measured engine behaviour (test arenas):**
- Monsters attack the nearest champion of either team.
- Any occupied spawn tile holds a group's respawn indefinitely.
- Prism gains +1 Focus per carried gem.
- Heart of Ruin wins duels against squishy champions and loses to Cassi & Issac, Marcella, Varg and Ryth.

**Our bot (v4).** It descends from a rule-based bot written for an older engine. One controller runs per turn, in phases:
1. read the view, enemies and allies;
2. retreat when health is low;
3. respond to enemies;
4. choose a target camp or group from scores;
5. move along a path that avoids monster ranges;
6. capture, tribute, recall and buy.

Other parts of the design:
- Each map has a plan file with hooks for draft, init, pre-turn, mid-turn, target override and buy.
- Draft picks come from a static table per map.
- Behaviour switches are compile-time flags.
- The fuel peak is 90,700 a turn, and 186,800 on the first turn.
- One `camp_groups()` call costs 12,000–40,000 fuel, so it is cached.
- v4's changes since the previous version were all fixes:
  - exact map identification;
  - staged initialisation;
  - skipping unreachable groups;
  - monolith range 5 in paths;
  - when hit by an unseen camp behind a bush, walking home instead of looping recalls;
  - for ranged champions, standing only on tiles that can see the target.

**Data (engine 1.0.0, 20 Oct).** Ladder: we are 1,388 (rank 536 of 716); rank 20 is 1,897. In 160 paired unranked games against 8 teams rated 1,453–1,771, with the same teams and maps for both versions:

| | previous | v4 |
|---|---|---|
| W-L | 29-131 | 37-123 (23 %) |
| Gems per game, us / them | 383 / 781 | 499 / 782 |
| Deaths per game, us | 37.9 | 23.0 |
| Tributes per game | 19.3 | 25.3 |
| Gems lost to champion kills, us / them | 69 / 31 | 74 / 29 |

Games that flipped from the previous version to v4: 15 loss→win and 7 win→loss (McNemar p = 0.13).

Per map, v4 (W-L, gems us/them, our deaths per game):

| Map | W-L | Gems us / them | Our deaths |
|---|---|---|---|
| contest | 0-8 | 0/325 | 51.5 (no gem ever scored by either version) |
| fragment | 0-8 | 236/882 | 53.6 |
| hooks | 0-8 | 148/984 | 55.6 |
| gold_rush | 0-8 | 432/898 | 45.4 |
| corners | 3-5 | 720/860 | 10.6 |
| expanse | 5-3 | 922/757 | 2.2 |
| lanes | 2-6 | 787/933 | 11.0 |
| monolith | 1-6 | 756/995 | 15.6 |
| islands | 4-3 | 910/570 | 7.4 |

**v4 against the top 10** (unranked, 20 Oct 10:00–10:30; teams ranked 2–10, rank 1 had no eligible bot; 10 of the 20 maps, 90 games): **1-89**. The one win is a 0-0 tiebreak on boss_battle.

| | v4 | Top-10 teams |
|---|---|---|
| Gems per game | 159 | 943 |
| Deaths per game | 22.7 | 6.0 |
| Gems lost to champion kills per game | 62 | 3 |

Per map (9 games each; gems us/them, deaths us/them, tributes us/them):

| Map | Gems | Deaths | Tributes |
|---|---|---|---|
| ambush | 484/1012 | 13.8/4.6 | 12.0/23.3 |
| corners | 239/1009 | 12.1/1.0 | 20.8/44.1 |
| collision | 82/1014 | 24.8/0.9 | 6.4/38.7 |
| eight | 92/1011 | 25.7/2.9 | 3.6/28.3 |
| contest | 0/822 | 48.7/7.3 | 0/4.1 |
| gold_rush | 265/1024 | 39.8/10.4 | 18.7/52.7 |
| broken_curve | 214/1020 | 17.8/4.6 | 7.4/39.9 |
| concentrated_wealth | 78/1022 | 25.4/1.8 | 5.8/34.4 |
| corridors | 139/1017 | 17.7/14.1 | 6.3/18.4 |
| boss_battle | 0/479 | 1.2/12.7 | 0/4.1 |

Each cell has fewer than 30 games.

**Death causes** (previous version):
- Ours: 48 % to monsters and 50 % to champions; 43 % within 8 tiles of an altar.
- Top-10 sides: 22 % to monsters and 73 % to champions, at 14.2 deaths per game.

**Top-10 vs us** (ranked games):
- Ability-opened fights: 85 % vs 35–43 %.
- Gear at round 200: 3.8 vs 1.7.
- Gems per tribute: 41 vs 24–30.
- Altar locks per game: 6.4 vs 0.
- Draft core: Stellar, Cassi & Issac, Ren, Varg, Ryth. v4 opens with Vendetta on 14 of 21 maps; top-10 teams pick Vendetta 3 % of the time.

**Self-play:** v4 beat the previous code in 86 of 126 local games (68 %), but real games improved far less.

**A rule we set before the panel:** "under 30 % means the gap is structural: economy and routing first, fight micro waits." v4 won 23 %.

**Question.** Which build gives the most rating by Friday? Options, in no order:
- **A. Fight module:** ability-first openers, focus targets, interrupting channels, protecting carriers.
- **B. Economy and routing:** offline per-map route tables, one capturer per group, respawn-window routing.
- **C. Draft overhaul** toward the top-10 core.
- **D. Denial:** holding enemy spawn tiles; using monster aggro against enemies.
- **E. Map-specific plans for the four contact maps.**
- **F. Gear curve and early recall.**
- **G.** An option not listed.
- **H.** None of these.

**Please:**
1. Rank the options, with an estimate of the rating or win-rate effect of each and the data that supports it.
2. Name the most likely failure mode or implementation trap of each, in this kind of rule-based C++ bot.
3. Propose one idea from another game or competition (a MOBA, an RTS, robotics, vehicle routing, or an earlier Battlecode season) that maps onto one of these mechanics, with how to implement it within the fuel budget.
4. Say which single number above, if wrong, would most change your ranking.
5. Say how you would test the change. We have deterministic local self-play at about 1 second a game, and real games only against the top 10 at 50 groups an hour, played by the live bot only. Self-play has already disagreed with real results once.

**Constraints:**
- About 60 hours to the main release (Friday 14:00 BST); the competition locks Saturday 23:59 BST.
- Three parallel cloud build sessions of 4 CPUs each.
- About 6 ranked series an hour; a 5-0 series between equally rated teams moves rating by about +16 (Elo, K = 32 per series).

---

## Answers

### Copilot (10:58; full text consult/answers/1029-copilot.md)
Top: **G, a "safe tempo" package**: draft core lookup, recall and shop at gold and risk thresholds, one capturer per group with fixed opening routes, banking carriers before contact, and an ability before committing. If it must pick one listed option: B. It calls the pre-registered rule correct (structural), and says "economy first" must not become a 40-hour optimiser. Ranking: G > B > F > C > A > E > D > H.

Claim checks:
| Claim | Class | Check |
|---|---|---|
| Damage does not interrupt a tribute; stun, death or an altar lock does | **contradicts our prompt** | **TRUE**: GameRules l.181, l.229. Our prompt was wrong; FACTS.md and the w2 brief are corrected |
| Top-10 teams capture ~15 groups by round 300 vs our 10-11 | new (to this consult) | TRUE: notes/top10-2026-10-20.md l.38 (15.0 vs 11.1/9.7) |
| Extra allies do not speed a 5-round capture | agrees | TRUE: GameRules l.134; arena: allies on tiles split the reward |
| Shopping while dead | agrees | TRUE: FACTS l.32 |
| Eight shared bytes for focus broadcast | new | TRUE: FACTS l.39 (8 bytes shared memory) |
| Gear 3.8 vs 1.7, 41 vs 24-30 gems per tribute | agrees | TRUE: our prompt, notes/top10 |
| E's ceiling: four maps 0 % -> 25 % = +4.8 pts | new | arithmetic TRUE if maps are uniform (4/21 × 25 %) |
| Self-play is a rejection gate only, not an estimator | agrees with our data | the 68 % self-play vs 23 % real gap |
| Elo effects per option | opinion | unverifiable estimates |

### DeepSeek (DeepThink; consult/answers/1029-deepseek.md)
Ranking: B > A (narrow) > C > G (altar locks, channel safety, recall thresholds) > F > D > E > H. C and G are quick wins in the first 6 h. Outside idea: MOBA creep-aggro pulling (rule 1). Game-changer: spawn-tile freezing of high-value camps. Follow-up: B stays first, G rises, D drops (monster damage no longer interrupts a tribute).

### Claude (Opus; consult/answers/1029-claude.md)
Ranking: B > C > G (altar control) > F > A > D > E > H, unchanged after the correction.
- Release C, G and the gold-spending half of F first.
- Bound: v4 cut 14.9 deaths and gained 116 gems, at most 8 gems per death avoided. So matching top-10 deaths still leaves about 350 vs 880, and the remaining gap is farming rate.
- Diagnostics before session 1: (a) v4's monster/champion death split vs the top 10 (if about 80 % champion, A moves to third); (b) v4 unopposed, the round at which it reaches 1,000 gems, vs the top-10 finishing round.
- Never wait on a spawn tile (rule 2 holds our own respawns).
- Tank swap (WoW raids) on nearest targeting.
- Contest: the top 10 tribute monoliths as 200-gem bags (822 gems / 4.1 tributes); a 20-turn channel is cancelled by a lock, a stun or a kill.
- Release each passing build at once; the last substantive build should be live by Fri 04:00.
- Rank 20 is out of reach; about 1,600 is realistic.

### ChatGPT (High reasoning; consult/answers/1029-chatgpt.md, condensed copy)
Ranking: B > G (stateful objective controller, commitment hysteresis) > E > C > A > F > D > H. After the correction, A moves above C.
- Outside idea: multi-robot task allocation with time windows by auctions (Nunes and Gini, AAAI 2015). Champions are heterogeneous robots and camps are jobs with availability windows; bids come from travel, capability, risk and availability; greedy award with marginal team value recomputed after each award.
- Audit the 499 gems first: acquired vs banked funnel.
- Fuel headroom is only about 9k, so the planner must replace work, with offline tables.
- Testing: a funnel per game, scripted adversaries, one integrated candidate vs v4 (A-B-A).

## Synthesis (lead, 11:20)
| Point | DeepSeek | Claude | ChatGPT | Copilot |
|---|---|---|---|---|
| First build | B | B | B+G controller | G package (B core) |
| Pre-registered rule (economy first) | honour | honour (with a bound) | reasonable heuristic | correct |
| Draft core (C) early | quick win | release first | cheap, test substitutions | in package |
| Altar locks / tribute gate | G, rises | G, third | channel-threat evaluator | carrier safety |
| Fights (A) | narrow, second | fifth | rises above C | narrow subset |
| Denial (D) | experiment | monolith tile only | narrow test | late |
| Contact-map plans (E) | low | low | third | low |
| Top 20 by Friday | unlikely | out of reach (about 1,600) | extremely ambitious | – |

**Contradicts the lead:** at 10:10 the lead overrode the pre-registered rule and kept fights first. All four consultants say economy first, with fights narrow and later. The lead reverses that decision (PLAN-v4).

Claim checks:
| Claim | Class | Check |
|---|---|---|
| Monster damage interrupts tribute (DeepSeek main answer) | contradicts rules | FALSE (GameRules l.181/229); DeepSeek corrected it in the follow-up |
| Locks cancel tribute channels; 3 basic attacks remove them; 10 gold | agrees | TRUE (GameRules l.193-199) |
| Rule 2: waiting on our own spawn tile holds our respawn | new | TRUE by arena test 3 (own occupation holds it) |
| Contest: top-10 822 gems from 4.1 tributes, about 200 per tribute | new | arithmetic TRUE from our table; 20-turn channel = ceil(200/10) TRUE (l.181) |
| A rank-20 bot takes 35-40 % from ranks 2-10 | new | about 30-40 % by Elo (1,897 vs 1,970-2,096): roughly TRUE |
| 55 series (about 9 h) to close a 500-point gap | new | plausible with K=32 per series and 6.1 series/h (CLAUDE.md); not verified |
| At most 8 gems per avoided death | new | arithmetic on our numbers (116/14.9 = 7.8) TRUE; causal bound is an assumption |
| Nunes and Gini 2015, AAAI, auctions with temporal constraints | new | real paper (TeSSI); not re-read |
| Fuel headroom about 9k at peak | agrees | TRUE (90,697 of 100,000; 186,807 of 200,000) |
| Allies split the reward; 4 allies x 50 gems on a monolith | new | split TRUE (arena 4), but contest's monolith group has ONE spawn tile (data/maps21.json), so no split there; possible on monolith (9 tiles), gold_rush (3), corridors (3), big_world (5), boss_battle (2 monoliths) |

