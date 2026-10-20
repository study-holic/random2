# Step 0: where our deaths come from under engine 1.0.0 (20 Oct 2026)

Data (all engine 1.0.0, decoded 20 Oct): v3b real-team panel, 160 games, 21 maps, 7-8 games per map (each map n<30, so per-map rows are indicative only); rank-us = 50 of our ranked games from the top-10 study (v2/v3b mixed, 04:30Z on); top10 = 191 sides of the 8 top-10 teams with 1.0.0 data (not Fish/SBC). No win-rate comparison is made here, so no rating adjustment; panel opponents are 1,450-1,770 rated, top-10 are 1,975-2,150, so death counts are not like-for-like.
Tools: tools/step0/{final,agg2,drafts}.py. Round-level and action-level decodes in data/unranked/v3b_full, data/top10/full.

## Method and its limit
- The decoder gives no killer field. Champion kill = an enemy champion action damaged the victim in the death round or the one before (exact). Otherwise "monster (near camp)" = no champion damage and a wolf/elk/monolith camp within 8 tiles (proxy: monster damage is not logged, and a dead or respawning camp still counts). "other" = neither.
- Cassi and Issac partner deaths (second body dies with the first, GameRules l.387) take the cause of the first. 692 of our 6,065 v3b deaths (11%) are partner deaths.
- Validation of the cheaper round-level rule on 158 top10 full games: champion precision 99%, recall 85%; the exact rule is used below.
- Location priority: own spawn (8 tiles of the team's round-0 positions), altar (any of the 4, 8 tiles), camp, elsewhere. The altar test is loose: altars sit near camps on many maps.

## 1. Death split (share of deaths, count)
| set | games | deaths/game | champion | monster (near camp) | other |
|---|---|---|---|---|---|
| v3b panel | 160 | 37.9 | 3,046 (50%) | 2,897 (48%) | 122 (2%) |
| our ranked (rank-us) | 50 | 36.3 | 802 (44%) | 932 (51%) | 83 (5%) |
| top10 sides | 191 | 14.2 | 1,985 (73%) | 587 (22%) | 133 (5%) |

| v3b deaths by | spawn | altar | camp | elsewhere | rounds 0-200 | 200-600 | 600+ |
|---|---|---|---|---|---|---|---|
| all (6,065) | 622 (10%) | 2,612 (43%) | 2,272 (37%) | 559 (9%) | 518 (9%) | 1,365 (23%) | 4,182 (69%) |
| top10 (2,705) | 541 (20%) | 1,623 (60%) | 336 (12%) | 205 (8%) | 317 (12%) | 772 (29%) | 1,616 (60%) |

Cause x location, v3b: monster at camp 1,701; champion at altar 1,595; monster at altar 990; champion at camp 558; champion at spawn 492; champion elsewhere 401; monster at spawn 121.
Cause x round band, v3b: champion 188 / 661 / 2,197; monster 326 / 690 / 1,881 (0-200 / 200-600 / 600+).
Gems carried at death, v3b (per game): champion kills 541 carrying >0, 230 carrying >=20, 69.5 gems lost per game; monster kills 154 carrying >0, 68 >=20, 23.2 gems destroyed per game (GameRules l.315); other 0.7. Top10 per side: champion kills 187.3 gems/game, monster 79.8. Top-10 deaths carry far more because they carry far more (0.57 gems/rnd).
Context (from lead, results/arena-1020.md): monsters hit the NEAREST champion, and an awake camp keeps hitting a nearer enemy who never attacked it.

## 2. Gems lost on our deaths vs taken from enemy deaths (per game)
| set | our deaths lost to champions | destroyed by monster/other | we take from their champion-killed deaths | all gems their dead carried | their deaths/game | share of their deaths killed by a champion |
|---|---|---|---|---|---|---|
| v3b (n=160) | 69.5 | 23.9 | 31.7 | 85.2 | 21.4 | 27% (907) |
| rank-us (n=50) | 52.2 | 16.2 | 39.8 | 53.4 | 23.8 | 36% (431) |
Net theft, v3b: -37.8 gems per game (lose 69.5, take 31.7). Their dead carried 85.2, of which we took 37%; the rest was lost to monster or unattributed deaths or to their own tributes first. Reference: tributed gems per round 0.24 us vs 0.48 them (data/unranked/v3b_tables.txt).

## 3. v3b draft (our picks in order, from sumj roster.draft; 160 games)
| maps (games each) | picks in order |
|---|---|
| ambush, elkring, expanse, lanes, monolith, big_world, boss_battle, concentrated_wealth, corridors, elk_arena, fracture, islands (7-8) | Vendetta, C&I, Varg, Ryth, Tavi |
| interlock, hooks (8) | Vendetta, C&I, Varg, Ryth |
| collision, fragment, broken_curve, gold_rush (8) | Vendetta, C&I, Stellar |
| contest (8) | Stellar, Tavi, Prism, C&I, Ryth |
| corners (8) | C&I, Stellar, Varg, Ryth, Ren |
| eight (7) | C&I, Stellar, Tavi, Ryth, Ren |

Presence over 160 games: C&I 100%, Vendetta 86%, Ryth 80%, Varg 71%, Tavi 65%, Stellar 34% (55), Ren 9% (15), Prism 5%.
Top-10 sides (191): Stellar 96%, C&I 95%, Varg 71%, Ren 69%, Ryth 69%, Tavi 31%, Vendetta 2%; 68 sides hold the full core (Stellar, C&I, Ren, Varg, Ryth). The core is used on 5-pick maps by most top-10 sides (e.g. corners 6 of 7, monolith 6 of 6).
Answer to the ChatGPT question: v3b drafts Stellar on 7 of 21 maps (3-pick maps collision, fragment, broken_curve, gold_rush as third pick; contest first; corners and eight second), so "does not draft Stellar" is wrong, but only 34% of games vs 96%. It drafts Ren only on corners and eight (9% vs 69%). On 12 maps it plays no Stellar and no Ren; on 14 of 21 maps it opens with Vendetta, which top-10 sides almost never pick (2%).
Our dead by champion type (v3b): C&I 2,815 of 6,065 (46%), Vendetta 880, Ryth 663, Stellar 639, Tavi 498, Varg 374.

## 4. The three causes that, removed, cut v3b deaths most
| rank | cause | deaths (share) | concentrated on (deaths) |
|---|---|---|---|
| 1 | Monster kill near a camp (1,701 at camp, 990 at altar, 121 spawn, rest elsewhere) | 2,897 (48%) | contest 728, corridors 619, broken_curve 552, fracture 347, gold_rush 286, boss_battle 147 (these 6 maps = 94%) |
| 2 | Champion kill at an altar | 1,595 (26%) | hooks 313, gold_rush 227, fragment 215, interlock 127, concentrated_wealth 122, eight 95 |
| 3 | Champion kill at spawn/camp/elsewhere (spawn 492, camp 558, elsewhere 401) | 1,451 (24%) | spread; not map-concentrated |
Overlapping slice: C&I partner deaths 692 (11%), concentrated on broken_curve 154, corridors 134, contest 128, fracture 79, gold_rush 76; they follow from cause 1 on those maps.
Concentration: 40 of 160 games have more than 60 deaths; those hold 3,450 of 6,065 deaths (57%), 64% of them monster deaths. Death counts per game are 94 to 120 on corridors and contest, 74-77 on gold_rush and broken_curve, below 5 on islands and expanse.
Gem stakes differ from death counts: monster deaths destroy only 23.2 gems per game (most carry nothing, 154 of 2,897 carry gems), champion deaths lose 69.5. Monster deaths cost time (60-round respawn) rather than gems.

## Interpretation (labelled; not data)
- The v5 survival module has two separable jobs: stop repeated camp deaths on 6 maps (contest, corridors, broken_curve, fracture, gold_rush, boss_battle), consistent with the camps hitting the nearest champion, and reduce carrier theft at altars on hooks, gold_rush, fragment, interlock, concentrated_wealth, eight.
- Top-10 sides die 2.7 times less per game and mostly to champions (73%); monster death is the main gap, not champion death.
- Untested: whether the near-camp deaths come from our attacks on the camp or from standing near an awake camp. Needs a /watch of one contest and one corridors loss.
