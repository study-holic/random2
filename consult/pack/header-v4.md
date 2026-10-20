# Team Xanarchy, Cambridge Battlecode 2026: context for an outside reviewer (v4, 19 Oct 2026 19:00 BST)

Written by the team's lead agent. Self-contained: our situation and measurements first, then appendices: (B) decoded ladder behaviour data, (C) per-map camp economics, (A) the official game rules in full, and a signature index of the C++ Bot API. Everything is a fact or a measurement unless labelled "hypothesis".

## 1. Situation

- Cambridge Battlecode 2026: each team writes a bot (ours is C++ compiled to WebAssembly). Ranked ladder plus tournaments. **Sprint 2 submission lock: Sat 24 Oct 23:59 BST** (5 days). Team Xanarchy = one person (Xans) plus AI coding agents; 14 CPU cores locally (about 110 local games a minute), plus 50 unranked match groups (250 games) an hour against any real team on the platform.
- Our bot "v2" went live 23:33 BST 18 Oct. Ladder: 515 ranked games since, 235 wins (46 %), rating 1,500 → 1,417 (rank 499 of about 1,050). Rank 200 ≈ 1,600, rank 100 ≈ 1,686, rank 50 ≈ 1,771, rank 1 ≈ 2,315.
- Every team plays the same 17 maps. Team size per map is 3, 4 or 5 champions.

## 2. The game in brief (full rules in Appendix A)

- **Draft**: 11 champions (Ryth, Vesper Vale, Vendetta, Cassi & Issac = one pick/two bodies that die together, Stellar, Prism, Marcella, Willow, Ren, Varg, Tavi); picks alternate and are revealed; the map is known during the draft.
- **Objective**: first team to tribute 1,000 gems wins; otherwise most tributed gems at round 2,000 (tiebreaks: lifetime gold, kills, Team B).
- **Camps**: wolves (1,350 HP, 20 str, 90 gold, 22 gems, 40-round respawn), elks (2,400, 60, 160, 36, 60), gem clusters (500, 0, 0, 24, 50), gold clusters (500, 0, 35 gold, **0 gems**, 50), monolith (20,000, 200, 300, 200, 150; does not regenerate). Monsters hit at range 3 (Chebyshev), never chase, attack the nearest champion until undamaged for 2 turns. A cardinally connected group is captured by killing all monsters then occupying a spawn tile uncontested for 5 rounds; rewards split among allies present; "any occupied spawn tile delays the entire group" respawn.
- **Tribute**: channel of max(1, ceil(gems/10)) turns at an altar. Damage does not interrupt it; stun, death or a new altar lock does. Altar lock: 10 gold, one carried at a time, placed within 2 tiles of an altar, blocks enemy tributes there, 3 basic attacks clear it.
- **Combat**: melee range 1, ranged 3; ranged champions choose walking OR a basic attack each turn (attack-reset abilities restore the attack); one ability cast per turn, 8-turn cooldowns; abilities scale with Focus; stun interrupts channels; recall is interrupted by damage. Kill: the killer takes all carried gems + 50 gold; respawn after 60 rounds.
- **Items**: gold buys shards/emblems (Strength, Focus, Health in several families: monster, power, durability, treasure, jungle, home, bruiser) on spawn tiles or while dead; Tavi may buy shards anywhere (Health bought in the field raises max HP without healing).
- **Bots**: each champion is its own WebAssembly instance (its memory persists across its own turns for the whole match). Team memory: **8 bytes** shared all match, plus **1,024 bytes of draft memory** written during the draft (the drafter sees every enemy pick) and read-only for champions afterwards. Vision is per champion (8 tiles, plus its own ward); **teammates' vision is not shared**. Fuel: about 100,000 instructions a turn (2× first turn, 10× first draft turn). Deterministic engine. No persistence between matches; no opponent-identity API.

## 3. Our bot (v2)

About 2,000 lines of inherited C++ plus per-map hooks. Turn flow: shop → read view/enemies/allies → retreat/recall by health → enemy response → target choice (squads with a claim in shared memory) → farming micro (casts any ready damage/stun ability) → bank at about 36 gems to the nearest altar. Per-map files can override draft, targets and buying.

Diagnosed faults (each with file:line in our code; these are hypotheses about cause, with the evidence):

| # | Fault | Evidence |
|---|---|---|
| F1 | Target value ignores gems: score = distance + wait − 0.05 × gems (20 gems ≈ 1 tile); gold-only groups (0 gems) are ordinary targets | stampede: 30-42 % of our champion-rounds on gold clusters; Appendix B §2: pilgrimage 49 % of our champion-rounds at gold camps vs 32 % for top-10 teams; stampede gem-camp share 6 % vs 22 % |
| F2 | Monoliths never targeted (contest's only gems are 2 monoliths) | contest: 0.06 gems/round, 48.7 deaths/game |
| F3 | Item builds alternate Health with the main stat | decoded ladder games: an opponent bought no Health and dealt 1.3-1.5× our damage per basic hit with the same champions; another opponent's Tavi stacked Focus and averaged 1,512 damage per damaging cast (range 4) with 23 kills in one game; our Tavi buys Strength |
| F4 | Lone even fights: champions fight any 1v1; flee only from 2+ enemies | 79 deaths in 4 decoded losses: 60 with no ally within 4 tiles, 74 at ≥ 50 % health, 78 carrying no gems; in one loss we attacked first in all 10 fatal fights. In those 60 lone fatal fights we dealt 6,893 damage and took 60,065; 33 of the 60 victims never cast. An opponent's Tavi held 32 Power Focus emblems and killed a full-health 2,400-HP Ryth in one cast from 4 tiles; our Tavi's spell stayed at 220 damage all game (v2 buys Strength for Tavi) |
| F5 | Retreat formula counts every elk within 4 tiles as hitting us | champions recall from a 6-elk group at 90 % health |
| F6 | Dashes and Willow's heal are never cast | code audit + replays |

## 4. Experiments since 01:00 today (local, against v2 head-to-head and a 6-bot panel of our older versions; 3 seeds × 2 sides; a no-op change flips about 7 % of results)

| Change | Result |
|---|---|
| Monolith siege from range 4 (casters ring the monolith at Chebyshev 4, escort carrier, safe tribute) on contest | contest panel 10 → 11 of 12; head-to-head vs v2 6/6; median win round 2,000 → 1,510. On the monolith map it lost ground (off there) |
| Retreat fix (count only camps where we are the nearest champion) + gold-cluster rule fix, stampede | 11/12 (v2 9.67), deaths 9.5; head-to-head vs v2 1/2 (salt 0 only); adding a shop trip + Tavi field buys: median win round 1,703 → 1,460 but 0/2 vs v2 |
| Altar locks: buy one, drop on enemy altars we pass | vs v2 50/102, 9 of 17 maps positive, +31 gem margin: not admitted |
| Interrupt enemy tributes (stun/lock on a visible channelling enemy) | rejected (corners −195 gems); we see only 27-34 % of enemy tributes |
| Tavi buys shards in the field | gold held fell on all 9 Tavi maps; median win round down on 6/8 maps; net wins +0.33 to +2.00 of 204: not admitted |
| Data drafts per map (best ladder draft per map from 9,245 games) | admitted on eight (h2h 6/6) and stampede (5/6); across all 17 maps −1.00 |
| Counter-draft table from ladder draft data (win excess by revealed enemy picks) | fires on 0-0.1 % of drafts; every candidate swap reverses sign between halves of the data |
| Escape dash, death-loop breaker (earlier) | deaths fell; wins did not move enough to admit |
| Engagement rules: step back to allies when outnumbered-or-even, avoid areas of recent enemy sightings, move as one group (contest) | rejected: head-to-head vs v2 28/102 on all maps; slower on 16 of 17 maps; a disengaging Varg was chased 10 rounds and killed by a melee champion (melee can walk and hit in one turn) |
| Tavi all-in Power Focus with field buying | head-to-head vs v2 54/102; monolith map 6/6 |

In progress tonight: gems-per-round target scoring (F1), damage-only builds and Focus stacking for casters (F3), a duel lab measuring every champion-vs-champion matchup under different micro policies, and a fight-outcome model trained on 1,923 top-team replays.

## 5. Real-team evidence (Appendix B has the full tables)

- Gems tributed per round (us / our opponent / top-10 mutual games): all maps 0.98 / 1.08 / 1.54. Worst gaps: swirl 0.29 / 0.94 / 1.21; eight 0.84 / 1.58 / 1.23; fragment 0.55 / 1.13 / 0.99; packs 1.01 / 1.56 / 1.78; pilgrimage 0.51 / 0.86 / 1.29; stampede 0.42 / 0.59 / 0.99; contest 0.06 / 0.12 / 0.17.
- We out-earn opponents on ambush, elkring, expanse, lanes, rows. On our five weakest maps we make far fewer tributes than the opponent with equal or larger bags (eight 8.5 vs 21.0 tributes a game, bags 66 vs 48).
- Deaths per game, ours vs opponent: contest 48.7 vs 30.0, swirl 41.1 vs 21.4, fragment 18.8 vs 10.4, stampede 18.7 vs 12.9, packs 10.6 vs 4.4.
- Top-10 teams spend 45 % of champion-rounds at gem camps (we 36 %) and 8 % at gold-only camps (we 14 %).
- Our local test panel badly overstated v2 (151.7 of 204 local wins; 46 % on the ladder against teams near 1,420-1,500).

## 6. Evidence against our current lean

- Draft changes measured small (−1.00 to +0.67 wins of 204 across maps), and the counter-draft table from ladder data found no stable signal; but the data stores drafts unordered and is dominated by mid-ladder teams.
- Denial (locks, interrupts) measured neutral or negative locally, while a former teammate on a different code base reports locks/interrupts helped them on the ladder.
- Local head-to-head vs v2 can be uninformative: on some maps v2 and our old versions play move for move alike.
- Fight-avoidance changes cut deaths but have not yet moved wins (escape dash, death-loop breaker).
