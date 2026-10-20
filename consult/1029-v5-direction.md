# Consult 10:29 BST Tue 20 Oct: v4 real-team result and the v5 direction

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

(appended when they come back)
