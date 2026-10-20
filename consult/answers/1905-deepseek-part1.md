# DeepSeek (DSeek) Part 1 answer, rules angle (collected 20 Oct). CONDENSED: tables flattened and wording trimmed; every claim, number, ranking and rule quote kept.

Part 1: Diagnosis, Priority, and Test Design

I've read the whole file. Below I take the rules angle as asked, quote the lines I'm relying on, and flag where the evidence is thin. Short version up front: your diagnosis is directionally right on F1/F3/F4 and partly wrong on F2; F5/F6 are real but small; the biggest missing cause is tempo (first-tribute round and bank-trip count), not target selection or item stats, and the addendum has already falsified the naive form of F3. The single highest-leverage rule interaction you are not using is the capture-timer + respawn interaction and the tribute-timing arithmetic (ceil(gems/10) plus capture-before-channel). The highest-leverage rule you are already using but haven't quantified is the monolith's 150-round respawn combined with 200 gems.

## QUESTION 1 - Is the diagnosis right?

### F1 - target scoring ignores gems; gold-only camps attract champions
Supported, but it is a symptom of two different problems, only one of which is a cause of losses.
Evidence for: "stampede: 30-42 % of our champion-rounds on gold clusters"; Appendix B s2 pilgrimage "49% at gold camps" vs top-10 32%; stampede "6% gem camps against 22%"; and the line in s2's interpretation: "pilgrimage, rushbait, stampede, eight lose it to where champions stand (gold camps, spawn, walking), not to yield per camp-round" - yield per gem-camp round is 99 vs 95 on pilgrimage. So on pilgrimage the gold-camp share is not costing yield per camp-round; it's costing presence at gem camps and at altars.
But on the maps where you actually lose: on swirl you are at gem camps 21% (opponent 25%, top-10 24%) and dead 45% - the problem is deaths and bag loss ("0.60 of collected gems reaching an altar"), not gold-camp tourism. On fragment the same: 46% at gem camps (top-10 55%), 31% dead, 0.75 reaching. So F1 is a real cause on pilgrimage/eight/rushbait/stampede and not on swirl/fragment/packs/interlock, where the cause is death and bag loss.
Share of gap: on the gold-camp family plausibly 0.1-0.2 of the 0.56 gap; on the death family ~0. Rule: "Rewards combine every monster in the group... The group respawns together after rewards are claimed, using the longest cooldown of its members. Any occupied spawn tile delays the entire group." Gold clusters have 0 gems and 35 gold - pure economy; gold camps should be taken only when no gem camp is in travel range; criterion is gems per champion-round, which is what (a) tries to compute. But (a) alone won't fix swirl.
Refuted if: shifting gold-camp rounds to gem-camp rounds on pilgrimage/stampede/eight does not move first-tribute round and tributed-per-round up. Falsifier: if you raise gem-camp share to 25% and still tribute ~0.5/round, the cause is not target selection. Measure before writing the scorer.

### F2 - monoliths never targeted
Half-right, and the half that is wrong matters. Rule: | Monolith | 20,000 | 200 | 3 | 300 | 200 | 150 | (200 gems, 150 respawn, 20,000 HP, strength 200). Appendix C: contest gem supply 2.58 gems/round with only two groups bearing gems, both monoliths; contest top-10 tributes 1.5 gems/game, yours 0.6. On contest, the monolith is the entire gem economy. Your 10->11/12 panel result and median win round 2000->1510 is real. But contest is 34 of 515 games (6.6%), top-10 only reach 0.17 gems/round there, and your 18% win rate is a symptom of a low-variance high-death map, not a big income engine.
On the monolith map you already tribute 1.59/round vs top-10 2.22; groups there are 8gem+1monolith | 392 | 300 | 24000 | 200 | 150. The "monolith siege lost ground (off there)" note is important: gem clusters are nearer and the monolith is a late-game target. F2 explains essentially none of the 0.56 gap on average.
Refuted if: contest siege win rate does not survive a 30-game real-team sample (a local panel "badly overstated v2" - s5 last bullet).

### F3 - item builds alternate Health with the main stat
Evidence supports "our builds are bad", not the specific diagnosis, and the addendum has falsified the fix. Evidence: "an opponent bought no Health and dealt 1.3-1.5x our damage per basic hit"; Tavi stacking Focus averaged 1,512 damage/cast; s5: top-10 first shards are "power 2,186, jungle 1,931, durability 1,765... stat focus 3,209, strength 2,549, health 2,322"; yours "monster 1,340 and power 1,200 only."
The addendum is decisive against the straightforward version: damage-only builds lost head-to-head 38%/43% and deaths rose because "the local opponents burst us down." Confound: local opponents are not the teams you lose to on the ladder. Both can be true if ladder teams buy Durability (| Durability | 5 shards of the chosen stat | 10% less incoming damage. |) and Jungle (30% less camp damage) rather than health shards. The right reading of F3 is "buy durability/jungle emblems and power shards of the damage stat, not health shards" - not "buy no defensive stats". Tavi Focus is a fight build: Focus on contest (monolith is a 20,000 HP target killed with casters from range 4), Strength or mixed where Tavi must clear.
Share of gap: 0.05-0.15 gems/round overall, more on swirl/fragment/packs. Not the tempo gap.
Refuted if: a real-team 30-game test of durability-first builds (5 durability shards = one emblem) does not improve deaths and win rate together. Pre-register: admit only if both improve by >=1 SD on the same map family. Do not judge by head-to-head vs v2 alone.

### F4 - lone even fights; flee only from 2+
Strongly supported as a cause of deaths; weakly as a cause of lost gems. Evidence: "79 deaths in 4 decoded losses: 60 with no ally within 4 tiles, 74 at >= 50 % health, 78 carrying no gems"; "in those 60 lone fatal fights we dealt 6,893 and took 60,065; 33 of the 60 victims never cast." Rule: "If you are a ranged champion, you may NOT move, and basic attack on the same turn" and "Melee champions can walk and attack in the same turn." 33/60 "never cast" is a fuel/ordering problem as much as policy - an engine-order bug (target choice before cast?). "78 carrying no gems" means most fatal lone fights are not bag-loss fights; they cost 60 rounds of respawn. 13% dead vs top-10 9%; shortfalls in s4 are 0.46-0.76 on death maps; large share on swirl (45% dead vs 23%) and fragment (31% vs 18%).
Share of gap: large on swirl/fragment/packs/interlock (0.1-0.2 of the 0.56 there); small elsewhere. 41 deaths/game on swirl is the worst number.
Refuted if: the fight-outcome model at real-team scale does not reduce deaths on swirl and fragment without reducing tributed-per-round. The earlier "engagement rules" experiment (rejected 28/102) bundled several changes; the reported failure ("a disengaging Varg was chased 10 rounds and killed") is a retreat-execution bug, not a refutation.

### F5 - retreat formula counts every elk within 4 tiles
Real bug, negligible cause. Rule: "Monsters hit at range 3 (Chebyshev), never chase, attack the nearest champion until undamaged for 2 turns." An elk at distance 4 cannot hit you. Recall 25.9/game vs top-10 28.0, so no overall over-recalling. Worth 10 lines, not a day. Share ~0. Refuted if the fix moves recalls/game on elkring (21.7 vs 32.3) by >2.

### F6 - dashes and Willow's heal never cast
Supported (code audit), small. "Melee champions can walk and attack in the same turn." Vendetta: "When Vendetta gets a kill, she resets all her ability cooldowns... applies to killing camp monsters too." Willow heal: "Restores up to 3 x Focus health." Share ~0.02-0.05. Fix in the same pass as F4. Refuted if dash/reset/heal casts increase deaths; criterion: deaths down AND camp clear time down on the same map.

## Causes not listed
- M1 Tempo / first-tribute round: s5 "first tribute round 69 against our 86"; s4 shortfalls "first tribute 0.25" stampede, 0.12 swirl, 0.61 rows, 0.65 expanse, 0.69 collision. Bank-trip problem. Rule: tribute duration max(1, ceil(gems/10)) and "Captures resolve before channels, so gems captured this round are included." Arrive with the gems needed for the next tick; "bank at ~36" is the inverse policy (fewer, larger tributes), loses capture-then-tribute and loses bag gems on death (14% of deaths carry >=20 gems). Highest-leverage, cheap change: a policy constant, testable in a day.
- M2 Bank-trip count: tributed per collected 0.85 vs 0.89; swirl 0.60 vs 0.90; 869 deaths (14%) had bag >=20. Rule "if carrying >=15 and no altar within X, path to altar and do not initiate fights."
- M3 Tribute-count gap on eight/fragment/packs/pilgrimage/swirl is trip frequency and altar share, not yield: eight 8.5 vs 21.0 tributes, bags 66 vs 48; pilgrimage 7.3 vs 18.1; swirl 4.7 vs 21.8. Altar share: you swirl 17%, fragment 6%, eight 6%, pilgrimage 4% vs opponent 25/12/12/4. Fix: path-to-altar when bag >=10.
- M4 Damage gap in lone fights is micro ordering: 33/60 never cast. Rule: "Each turn normally allows one ordinary move, one basic attack, and one successful ability cast across both slots." Reorder enemy-cast check before camp cast (20 lines).
- M5 Stellar draft gap: "Stellar is the top killer on 7 of the 11 maps with 200 or more of our deaths"; "98% of top-10 drafts contain Stellar (ours 32%)"; "Cassi and Issac + Ren + Stellar 57% (n=215, +5)" vs "with Tavi and Varg 39% (100, -8)". Per-map table with a 30-game real-team gate (s6 warns draft changes measured small).
- M6 8-byte shared memory unused: rule "Each team has 8 bytes of shared memory... plus 1,024 bytes of draft memory... read-only for champions afterwards." Put per-map routing computed at draft (full map visible) in draft memory.

## QUESTION 2 - order (by rating points per day, 5 days)
Tier 1: (i) tribute cadence + altar-first routing + bag safety (+40-80 rating, 1-1.5 d); (i) turn-flow reorder: enemy cast before camp cast, dash/reset/heal (+20-50, 0.5-1 d); (g) draft table, Stellar in (+10-30, 0.5 d, per-map, 30-game gate).
Tier 2: (a) gems-per-round scoring (+20-40, 1-2 d; add survival term); (b) monolith siege contest only (+5-15, 0.5 d); (h) per-map modes in draft memory (+10-25, 1 d).
Tier 3: (c) item builds as durability/jungle emblems and power shards, Tavi-Focus on contest only (0 to +20, high variance).
Tier 4: (e) tribute timing against enemy positions; (d) engagement rules only as "don't initiate outnumbered fights" (0 to +15, risky).
Tier 5: (f) denial: data against locks ("4 of 10 top teams use them, Fish does not"; locks/interrupts neutral or negative locally); spawn-tile blocking only with spare capacity. (j) none for (f).

## Failure modes and drop rules (pre-registered)
- (i) cadence: more trips -> more deaths. Drop if deaths/game up >=2 on any of five target maps OR tributed/round down.
- (i) turn-flow: drop if deaths not down >=1 on swirl+fragment combined.
- (g) draft: drop if <55% on 30 real-team games per map, or opponents take Stellar more than us.
- (a) scoring: drop if tributed/round not up >=0.05 on target maps.
- (b) contest: drop if <55% on 30 real-team contest games.
- (c) builds: drop if deaths up OR camp-clear time up >=10%.
- (d) engagement: drop if slower on >2 maps OR deaths not down.
- (f) denial: drop if no >2-point win excess on 60 real-team games.
Meta-rule: local panel overstated once (151.7/204 local vs 46% ladder); every option needs a 30-game real-team gate.

## Testing
Real-team gate: 30 games per single decision (about 15 min); 7% no-op flip means a 60->65% change needs 200+ games for significance unless paired seeds are used (cuts n about 4x). Rule: admit on paired local (>=200 paired games, win and deaths both >=1 SD) AND >=30 real-team games with same-sign delta. One change per batch. Budget ~12,500 real-team games over 5 days; ~15-20 gated decisions affordable. Build conflict: ladder direction wins; re-run the local test vs a clone of a top-10 team; otherwise ship only Tavi-Focus on contest and durability-first on swirl/fragment.

## Which fact, if wrong, changes the ranking
- If the 46% is mostly v2 and later uploads are better, Tier 1 gain drops but order holds.
- If contest share is larger (tournament), (b) moves up two tiers.
- If top-10 reference (18 Oct 13:20-22:40) is stale, ratios are biased; direction holds.
- If no-op flip is much higher on small-n maps, 30-game gate is insufficient (60 needed).
- Most important: if the altar-share gap is caused by enemy locks/tribute-blocking rather than routing ("ours 0 lock placements"; s8 "not done"), then (f) moves up. Measure first tonight.
- Rules not yet exploited (flags): "Any occupied spawn tile delays the entire group" (1-round stand delays a 60-round respawn); "Captures resolve before channels"; "Tavi may buy shards anywhere... Health bought in the field raises max HP without healing"; "Recall arrives at turn start, so staying on spawn heals at that same turn's end"; "set_priority takes effect from the next round" (priority manipulation).
- Unsure about: exact share F1 vs M3; whether enemy locks cause the low altar share.
