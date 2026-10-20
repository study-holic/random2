# Cross-domain idea cards (scout-web, 18 Oct, written before the 22:05 BST stop)

Scope: ideas from outside cambc, each mapped onto a GameRules.md section, with a fixed falsifier. Nothing here is tested. "Panel" means `scripts/mapgate.py` (x-v4-14, x-foundry-v16 and four clones, both sides) = 12 games per map. "Baseline" means v1 on the same map, panel and engine. Team sizes and kills per game come from `intel/meta-2026-10-18.md` §2 and §3(a). Kills are pre-patch (engine v0_0_1). Camp types are not in the meta file. Where a card names camp types, they come from repo notes (`notes/contest-experiments.md`, IDEAS.md, mining/) and need a map probe before use.

DIRECT = the cited rule text supports the mechanism, and only the size of the effect is unknown. ANALOGY = a correlation from the outside domain that needs a test. PROBE = DIRECT only if a 5-minute local engine check passes; the card says which check.

## Ranking (score = expected effect 1-5 × confidence 0-1 ÷ build minutes × 100)

| Rank | Card | Effect | Conf. | Build min | Score | Type | Refines |
|---|---|---|---|---|---|---|---|
| 1 | C3 EOQ bag cap per map | 3 | 0.60 | 15 | 12.0 | DIRECT costs, ANALOGY demand | tarot #2 (bag cap) |
| 2 | C8 Tavi field shopping, zero float | 1.5 | 0.60 | 10 | 9.0 | DIRECT | none |
| 3 | C14 Race-map pacifism | 1.5 | 0.40 | 10 | 6.0 | ANALOGY | seed 14 (inverse) |
| 4 | C1 Ward camp-block (bold) | 5 | 0.35 | 30 | 5.8 | PROBE | mining/others "Wards" |
| 5 | C5 Contest goal-line zone at the bank | 4 | 0.50 | 35 | 5.7 | DIRECT | seeds 4, 1, 7 |
| 6 | C6a Fight beside the awake camp | 2 | 0.40 | 20 | 4.0 | DIRECT | seed 14 |
| 7 | C7 Emblem power-spike recall | 2 | 0.40 | 20 | 4.0 | DIRECT | tarot #3 |
| 8 | C4 Post-kill objective window | 3 | 0.35 | 30 | 3.5 | DIRECT | seeds 3, 4 |
| 9 | C12 Consolidated carrier capture | 2 | 0.35 | 20 | 3.5 | DIRECT | tarot #1 stander rule |
| 10 | C9 Time-window routing, two-stop lookahead | 3 | 0.30 | 35 | 2.6 | DIRECT | tarot #5 |
| 11 | C11 Nine-tile micro score with stutter-step | 3 | 0.30 | 35 | 2.6 | DIRECT | seed 14 |
| 12 | C6b Aggro grief (bold) | 2.5 | 0.20 | 25 | 2.0 | PROBE | none |
| 13 | C10 Ward the enemy jungle for timers (fallback to C1) | 2 | 0.30 | 30 | 2.0 | DIRECT | seed 3 |
| 14 | C2 Ward wall-off at a 1-wide choke (bold) | 3 | 0.15 | 35 | 1.3 | PROBE | none |
| 15 | C15 Timing invade on the predicted enemy chain (bold) | 3 | 0.15 | 35 | 1.3 | ANALOGY | retest EXP-020 |

C1 and C5 have the largest upside. C3 and C8 are cheap enough to try first. C1, C2 and C6b each need a one-game engine probe before any build; a failed probe closes the card at 5 minutes.

---

## C1. Ward camp-block (Dota 2 camp blocking) — PROBE, bold

- **Source:** Dota 2 players block a neutral camp by putting a ward in its spawn box: "if the camp's spawn box is empty at the minute mark new neutral creeps will spawn". https://gamersdecide.com/articles/dota-2-how-to-block-neutral-camps ; glossary https://winio.ai/glossary/dota2/blocking
- **cambc mechanic:** Camps & capture says "Any occupied spawn tile delays the entire group." BotAPICpp.md `camp_group_respawning` says "zero means respawning is currently blocked by occupied spawn tiles." Vision & wards says wards are permanent, do not expire or take damage, and one per champion is active. ELTheo77 found that "a ward on a capture tile stalled a reward 60 rounds" (mining/others.md, "Wards"). The open question is whether a ward counts as an occupant for respawn. If it does, the enemy cannot remove our ward, so the block lasts until we move it.
- **Probe (5 min):** one local game. After capturing an own-side gold cluster (50-round respawn), place a ward on one of its spawn tiles and read `group.state()` each round. PASS if `camp_group_respawning.timer` stays at 0 for 20 or more rounds past the expected respawn.
- **Maps:** all. Largest on 3-champion maps where each gem group is a large share of income: collision, expanse, fragment, packs (3 picks). Also rows (4 picks, race, 0.4 kills a game), where the enemy only farms. Targets are the enemy-side gem groups nearest the enemy spawn, which are the first stops of the foundry-style opening chain (tarot #1).
- **Hook:** `Plan::turn(Phase::mid)`. When a champion is within ward range of an original spawn tile of an enemy-half gem group whose state is `camp_group_respawning`, it places its ward there and keeps it. Allow at most 1 or 2 champions to do this (the ones v1 uses wards least), and never on our own groups' tiles.
- **Expected effect:** a 2-wolf group yields 44 gems per cycle of about 60 rounds (40-round respawn plus clear and capture), about 0.7 gems a round. Blocking it from round 150 to 600 denies up to about 300 enemy gems. If the enemy re-routes and gets two thirds of that back elsewhere, the net is about 100 gems per block, about a 10% slower enemy race on a 3-champion map.
- **Falsifier:** the probe fails, which closes the card. If it passes: on collision, packs and rows (12 panel games each), enemy median submitted gems at round 500 fall by less than 8%, or wins fail to reach baseline + 1 on at least 2 of the 3 maps.
- **Build:** 5 probe + 25 rule = 30 min.
- **Risk:** the panel opponents may do this to us. Also check that v1 never wards its own spawn tiles.

## C2. Ward wall-off at a 1-wide choke (StarCraft walling) — PROBE, bold

- **Source:** StarCraft walling uses buildings "to narrow a choke ... or to block it off completely". https://liquipedia.net/starcraft2/Walling
- **cambc mechanic:** Maps & movement says landing cannot be on an occupied tile, and an unseen occupant makes a step return false. mining/others.md ("Wards") says wards "block tiles". A permanent enemy-proof ward on a 1-tile corridor on the enemy's shortest spawn-to-altar path would force a detour on every trip, if the engine treats a ward as an occupant for movement.
- **Probe (5 min):** in a local game, put an ally ward on a tile and have an enemy-side bot try `move_absolute` onto it. PASS if the move throws or returns false. Also check whether our own champions can still pass (`invalid_action_place_ward_ally_ward_present` exists, so the API tracks ward occupancy).
- **Maps:** corridor maps where `Plan::init` finds a 1-wide articulation tile on the enemy's spawn-to-nearest-altar path that is not on ours. Candidates by name are lanes, interlock, swirl and rushbait_v2; the geometry is unchecked.
- **Hook:** `Plan::init` (BFS articulation scan, cached), then `Plan::turn(Phase::mid)` for one champion to place the ward when passing.
- **Expected effect:** k extra rounds per enemy bank trip (k = detour length). With 25 enemy trips a game and k = 6, that is about 150 enemy champion-rounds lost.
- **Falsifier:** the probe fails, or no qualifying choke exists on any map, which closes the card. Otherwise, on the 2 best maps (12 panel games each), enemy rounds to 1,000 rise by less than 5%.
- **Build:** 5 probe + 30 = 35 min.

## C3. EOQ bag cap per map (Harris 1913 economic order quantity) — DIRECT costs, ANALOGY demand

- **Source:** EOQ, Q* = sqrt(2DS/H). It minimises ordering cost D·S/Q plus holding cost H·Q/2. https://corporatefinanceinstitute.com/resources/accounting/what-is-eoq-formula/ ; https://en.wikipedia.org/wiki/Economic_order_quantity
- **cambc mechanic:** the order cost S is the altar round trip, plus ceil(g/10) tribute ticks (Gems & altars, "Tribute at an altar"). The holding cost H is the death hazard: "Carried gems can be stolen on death" (Gems & altars), and the killer receives all carried gems (Death, recall & recovery). That is a double swing, so H = 2 × hazard per carried gem per round. With D = gem income per champion-round and S = D × T (gems not farmed during a T-round trip), Q* = D × sqrt(2T / h).
- **Numbers.** Here n = picks, R = top-50 rounds to 1,000, k = pre-patch kills a game, T = 20 assumed, D = 1000/(nR), and h = k/(nR) (deaths per champion-round, doubled for the swing):

| Map | n | R | k | D | h | Q* |
|---|---|---|---|---|---|---|
| rows | 4 | 386 | 0.4 | 0.65 | 0.00026 | 255 |
| monolith | 5 | 447 | 5.3 | 0.45 | 0.0024 | 58 |
| ambush | 5 | 594 | 4.3 | 0.34 | 0.0014 | 56 |
| rushbait_v2 | 4 | 660 | 5.3 | 0.38 | 0.0020 | 53 |
| elkring | 5 | 577 | 14.7 | 0.35 | 0.0051 | 31 |
| fragment | 3 | 772 | 19.2 | 0.43 | 0.0083 | 30 |
| swirl | 4 | 600 | 26.0 | 0.42 | 0.0108 | 25 |
| pilgrimage | 5 | 844 | 15.2 | 0.24 | 0.0036 | 25 |
| eight | 5 | 640 | 23.1 | 0.31 | 0.0072 | 23 |
| stampede | 5 | 686 | 22.5 | 0.29 | 0.0066 | 23 |

  Cross-check: Mite's measured median bags (mining/others.md) are elkring 36 and rows 132, with collision, interlock and lanes at 22 to 24. The ordering matches; rows is capped by income, not by the formula. Maps without pre-patch kills (collision, corners, expanse, interlock, lanes, packs) need h from our own replays (`deaths.py`).
- **Maps:** all except contest, which has no gem camps.
- **Hook:** `Plan::init` sets the bag cap tunable in `params.h` from the table, with T replaced by the BFS distance from our camp centroid to the nearest altar × 2. The hard ceilings (45-gem bank and round-2,000 overrun) stay.
- **Expected effect:** rows has fewer bank trips, about 3% fewer rounds to 1,000. Contact maps lose fewer large bags: fewer deaths carrying 30+ gems.
- **Falsifier:** on rows, elkring and eight (12 panel games each), gems per 100 rounds fail to beat baseline by 3% on at least 2 of the 3 maps, and deaths carrying 30+ gems do not fall on eight.
- **Build:** 15 min (a table plus one formula; no new behaviour if v1 already reads a cap tunable).

## C4. Post-kill objective window (LoL "convert kills into objectives") — DIRECT

- **Source:** "calling Baron" when "your team has a numbers advantage ... (i.e. 2 enemies are dead) such that you cannot lose even if they all-in'd you". https://www.mobafire.com/league-of-legends/question/quot-calling-quot-baron-233 . Pro recaps describe teams that "leveraged each of its kills into an objective": https://www.rotowire.com/esports/article/lol-recap-eu-week-2-32207
- **cambc mechanic:** a dead champion sits out 60 owner turn ends (Death, recall & recovery). A capture needs the region held uncontested for 5 rounds (Camps & capture). The killer knows the kill and allies do not (Vision is not shared). The killer writes "enemies dead, earliest respawn round / 8" into one free byte of the 8-byte shared memory (Shared memory).
- **Maps:** 5-champion contact maps (eight, stampede, pilgrimage, elkring; 15 to 23 kills a game), and contest and monolith for monolith finishes.
- **Hook:** `Plan::turn(Phase::mid)` macro mode. While at least 2 enemies are dead and the window has 25 or more rounds left, the team commits to the best contested objective in reach: an enemy-half Elk or gem group, a reward-available group, or a monolith with 12,000 or less health in vision. Five champions deal about 350 a round (5 × ~45 basic plus spells at 2×Focus every 8 turns), which is too slow for a fresh 20,000-health monolith.
- **Expected effect:** one extra contested objective per 2 enemy deaths. On eight that is about 36 to 72 gems a game.
- **Falsifier:** on eight, stampede and contest (12 panel games each), the mode fires less than once a game (log it), or wins fail to reach baseline + 1 on 2 of the 3 maps.
- **Build:** 30 min (shared-memory byte, mode, objective pick). First check v1's shared-memory layout for a free byte.

## C5. Contest goal-line zone at the bank (zone defence; punish the team that took Baron) — DIRECT

- **Source:** a zone defence protects the scoring area and suits teams facing a few dangerous attackers, but it gives up ball pressure. https://www.coachesclipboard.net/youth/ZoneDefense.html ; https://www.masterclass.com/articles/man-to-man-vs-zone-defense . In LoL, the team that just took Baron is often low and gets fought right after; Baron steals and fights are decided by vision and crowd control held in reserve: https://mobalytics.gg/blog/4-tips-secure-baron-league-of-legends/
- **cambc mechanic:** contest's only gems are 2 monoliths (200 each), and its only altars are adjacent at the centre, (25,24) and (24,25) (notes/contest-experiments.md). Every enemy gem must walk to one tile pair. Tools at that pair:
  - A new lock cancels a tribute, and stun or death cancels it too. Damage does not (Gems & altars, Altar locks).
  - A kill hands us the whole carried bag (Death, recall & recovery). We would bank it at once, standing next to the altar: 40 gems is 4 ticks.
  - The monolith hits for 200 (Camps & capture, monster table), so the capturers arrive damaged.
  - The 4 central gold groups worth 105 gold each sit by the altars, so the zone team farms gold while it waits, which feeds the 0-0 gold tiebreak (How to win).
- **Plan:** do not chip monoliths, which inverts seed 4's chip plan. Every champion buys an altar lock. Two champions hold within 2 of the altar pair and farm the centre gold; the others farm the 105-gold groups. On sight of an enemy carrying gems: lock if it starts a tribute, stun, focus it, then tribute the stolen bag.
- **Maps:** contest only (5 picks, 56.3 kills a game; top-50 winners need 1,602 rounds, which shows top teams do kill monoliths).
- **Hook:** `map_contest.cpp`, using `Plan::buy` (lock), `Plan::turn(Phase::pre)` (carrier response) and `Plan::target` (zone and gold farm).
- **Expected effect:** enemy conversions of monolith captures drop from about 1 per 200-gem capture to under 0.5. Each denied capture whose carrier dies swings up to 400 gems.
- **Falsifier:** the panel opponents do not chip monoliths themselves (v1 vs foundry-v16 ended 0/0), so the test opponent must be the chip bot `candidates/map_contest_v2.cpp` (ct3) plus foundry-v16. Over 12 games on contest, falsified if the enemy tributes 200 or more in more than 3 games, or if wins fall below v1's baseline against the same pair.
- **Build:** 35 min.

## C6a. Fight beside the awake camp (Dota pulling: let the neutrals hit them) — DIRECT

- **Source:** Dota pulling draws neutral-camp aggro so that neutrals fight for you. https://liquipedia.net/dota2/Pulling
- **cambc mechanic:** Camps & capture says an awake monster attacks the nearest champion every turn; all monsters have range 3 and do not chase. A camp kill destroys the victim's carried gems (Death). mining/others.md (aggro) notes that ties go to the lowest handle.
- **Rule:** when an enemy champion stands within 3 of an awake monster and strictly nearer to it than any of our champions, engage that enemy from tiles at distance 4 or more from every awake monster: spells at range 4, or ranged basics at 3 from the enemy. Do not engage when the nearest-champion test would pick us.
- **Maps:** Elk maps (elkring, eight, stampede, ambush; Elk deals 60 a round), monolith (200 a round), and contact maps generally.
- **Hook:** `Plan::turn(Phase::pre)`, as a tile filter on engagement.
- **Expected effect:** the camp adds 20 to 200 damage a round to our side of the fight. On an Elk that is about +30% team damage in contested clears.
- **Falsifier:** on elkring, eight and stampede (12 panel games each), the trigger fires less than once a game, or K/D fails to rise by 0.2 on at least 2 of the 3 maps.
- **Build:** 20 min.

## C6b. Aggro grief (wake a sleeping camp next to an enemy) — PROBE, bold

- **Source:** same as C6a (https://liquipedia.net/dota2/Pulling).
- **cambc mechanic:** damage wakes the group (Camps & capture). The woken group targets the nearest champion. A ranged spell at range 4 (Stellar, Cassi, Tavi, Prism, Marcella) hits from outside the monsters' range 3. A camp kill destroys the victim's carried gems.
- **Probe (5 min):** check that a camp woken by our range-4 hit attacks a nearer enemy and not us. Also check which wake rule holds: GameRules gives both "does not take damage for 2 turns" and "two rounds without an in-range champion".
- **Rule:** when a sleeping Elk, wolf group or monolith has an enemy carrier within 3 tiles, nearer than any ally, a range-4 caster hits it once.
- **Maps:** contest and monolith (monolith deals 200 a hit; a 700-health carrier dies in 4 hits), Elk maps.
- **Hook:** `Plan::turn(Phase::pre)`.
- **Expected effect:** 0.5 to 1 extra enemy death a game to camps, with gems destroyed. This denies their bag but does not give it to us.
- **Falsifier:** the probe fails. Otherwise, over 12 panel games on contest and elkring, enemy camp deaths fail to rise by 0.5 a game.
- **Build:** 5 + 20 = 25 min.

## C7. Emblem power-spike recall (LoL recall timing) — DIRECT

- **Source:** recall when you can afford your next item, and not while the team contests an objective. https://mobalytics.gg/blog/lol-5-best-recall-timers/ ; https://metabot.gg/en/league/guides/when-to-recall-back-timing-economy
- **cambc mechanic:** five shards combine into an emblem, and "Only the completed emblem grants its passive" (Shards and emblems). Jungle gives 30% less camp damage, Monster 10% more damage to camps, Home a 3-turn recall. Buying Health on spawn heals by the amount gained (Extra item rules). Recall takes 5 turns and damage cancels it (Recalling).
- **Rule:** recall early, even at high health, if all of these hold: gold covers the rest of the next emblem (150 − 30 × shards already in that family); no enemy within 5 and no awake camp within 3; and the bag is under half the cap, or the champion has just tributed. Suppress the rule while the team is in C4's window.
- **Maps:** farm maps where camp damage forces pitstops: rows, packs, lanes, corners, elkring, monolith.
- **Hook:** `Plan::turn(Phase::mid)` plus `Plan::buy`.
- **Expected effect:** the first emblem completes 30 to 60 rounds earlier, giving fewer heal trips and about 2% fewer rounds to 1,000.
- **Falsifier:** on rows, packs and elkring (12 panel games each), the median first-emblem round is not 30 or more rounds earlier, or rounds to 1,000 do not fall on at least 2 of the 3 maps.
- **Build:** 20 min. Check v1's buy queue first: if it is Power-first (as top teams are, per tarot), the spike is the 6-shard Power completion.

## C8. Tavi field shopping, zero float (StarCraft macro) — DIRECT

- **Source:** in StarCraft, "spend your money as soon as possible, unless ... waiting for a specific tech". https://liquipedia.net/starcraft2/Macro
- **cambc mechanic:** Tavi's passive lets him buy shards anywhere; Health bought in the field raises maximum health without healing (Tavi; Extra item rules). Other champions shop on spawn or while dead (Items & gold).
- **Rule:** Tavi buys a Strength or Focus shard (per v1's queue) whenever gold reaches 30, anywhere. Every champion on spawn spends down to under 30 gold, except a 10-gold lock reserve where C5 applies.
- **Maps:** maps where v1 drafts Tavi. Ladder drafts with Tavi lead on contest (63% pick share), stampede (61%), packs (C&I + Stellar + Tavi at 72% top 50) and corners (55%).
- **Hook:** `Plan::buy` (the hook runs at the start of `buy_items`; check that v1 calls it off spawn for Tavi).
- **Expected effect:** small: Tavi's mean unspent gold falls by 100 or more, and gems per 100 rounds rise by about 1%.
- **Falsifier:** if v1 already field-buys for Tavi (grep `buy_items`), the card closes at 0 minutes. Otherwise, on packs and stampede (12 panel games each), Tavi's mean unspent gold (sampled every 25 rounds) does not fall by 50 or more, or rounds to 1,000 fail to beat baseline.
- **Build:** 10 min.

## C9. Time-window routing with two-stop lookahead (VRPTW, Solomon 1987 insertion) — DIRECT

- **Source:** in vehicle routing with time windows, insertion heuristics that charge waiting time "consistently gave very good results" (Solomon, Operations Research 35(2), 1987). https://ideas.repec.org/a/inm/oropre/v35y1987i2p254-265.html
- **cambc mechanic:** respawns take 40, 60, 50, 50 and 150 rounds (Camps & capture table), and `camp_group_respawning.timer` can be read while an original spawn tile is in vision (BotAPICpp.md). A camp is a customer with ready time r. Service starts at max(arrival, r).
- **Rule:** in `Plan::target`, score candidate pairs (a then b) as (gems_a + gems_b) / (max(travel_a, r_a) + clear_a + max(travel_ab, r_b − t_a) + clear_b), and take a. This replaces v1's one-stop choice when the pair score beats it by 10% or more.
- **Maps:** where the old bot is slow against foundry: swirl (v4.14 1,545 rounds vs foundry-v16 785), rushbait_v2, pilgrimage, fragment.
- **Hook:** `Plan::target`.
- **Expected effect:** the idle share (`timeuse.py`) falls by 5 points, and rounds to 1,000 fall by about 5% on swirl.
- **Falsifier:** on swirl and rushbait_v2 (12 panel games each), the idle share does not fall by 5 points, or rounds to 1,000 fall by less than 3%.
- **Build:** 35 min. This refines tarot #5 (respawn wait vs remote work) with explicit waiting cost and lookahead.

## C10. Ward the enemy jungle for timers (LoL timer keeping, counter-jungling) — DIRECT

- **Source:** track objective and camp timers, and invade when the enemy jungler is known to be elsewhere. https://dignitas.gg/articles/blogs/League-of-Legends/6991/the-importance-of-keeping-and-maintaining-timers-in-league-of-legends ; https://www.gameskinny.com/tips/league-of-legends-counter-jungle-guide/
- **cambc mechanic:** a ward gives permanent 6-tile vision to its owner (Vision & wards). A group's respawn timer can be read while an original spawn tile is in that vision (BotAPICpp.md). An enemy on a capture region resets the capture (Camps & capture).
- **Rule:** one invader keeps its ward on the enemy's best near-spawn gem group. When the timer is at or below its travel time, and no enemy is seen within 8, it walks there and clears. If the enemy arrives during the clear, it stands on the region to deny the capture (seed 3).
- **Maps:** 5-champion maps where one body can be spared: lanes, corners, monolith, ambush.
- **Hook:** `Plan::turn(Phase::mid)` plus `Plan::target`.
- **Expected effect:** one extra enemy-side group capture every ~100 rounds.
- **Falsifier:** on lanes and corners (12 panel games each), the invader captures fewer than 2 enemy-side groups a game, or wins fall below baseline.
- **Build:** 30 min. Use this card only if C1's probe fails, since a blocked camp is worth more than a timed one.

## C11. Nine-tile micro score with stutter-step (Battlecode attack micro; StarCraft kiting) — DIRECT

- **Source:** MIT Battlecode lecture: "one of the most important things to spend your time on is attack micro. If you can win every engagement you will win the game." https://ocw.mit.edu/courses/6-370-the-battlecode-programming-competition-january-iap-2013/bf23daf8bea14b70f0ca330bb4d5b8e2_tbsYFzmk_24.pdf . Cout for Clout 2024 has an "attack micro" section (table of contents seen; body not read): https://battlecode.org/assets/files/postmortem-2024-cout-for-clout.pdf . Kiting, where ranged units pull back as melee closes: https://liquipedia.net/starcraft2/Kite
- **cambc mechanic** (A champion turn; Combat & abilities):
  - Ranged champions cannot move and basic attack in one turn.
  - Melee threat radius is 2 (move 1, range 1), or 4 with a dash ready (Issac, Vendetta, Ren, Vesper). Ranged threat is 3 for basics and 4 for spells.
  - A walking-compatible attack reset lets Tavi, Vesper and Willow walk, reset and attack in one turn (mining/others.md engine facts).
- **Rule:** score each of the 9 tiles as W1 × (a target is attackable from here this turn) − Σ over enemies of expected damage next turn if the tile is inside their threat radius − W3 × (within 3 of an awake camp and nearest to it). Rangers with a ready walking reset step out to range 3 from melee, then reset and attack.
- **Maps:** contact maps: swirl (26 kills a game), eight, stampede, fragment.
- **Hook:** `Plan::turn(Phase::pre)`, the fight branch only.
- **Expected effect:** about 20% less damage taken per kill.
- **Falsifier:** on swirl, eight and stampede (12 panel games each), K/D does not rise by 0.2 on at least 2 maps, or wins fall below baseline on any.
- **Build:** 35 min. This refines seed 14: the fight evaluator decides whether to engage, and this card decides the tile.

## C12. Consolidated carrier capture (freight consolidation) — DIRECT

- **Source:** consolidation spreads a trip's fixed cost over more freight and cuts the number of trips. https://racklify.com/encyclopedia/what-freight-consolidation-is-and-how-it-cuts-shipping-cost/
- **cambc mechanic:** Camps & capture says rewards are split among allies present at completion, extra allies do not speed capture, and the lowest ID gets the remainder. So who stands on the region decides who carries the gems.
- **Rule:** when a reward is available, only the designated carrier stands on the region and the others move on. The carrier is the ally with the largest bag still below the C3 cap, with ties going to the ally nearest an altar. Tarot's stander rule picks the nearest ally instead.
- **Maps:** low-hazard maps where large bags are cheap (C3 Q* of 50 or more): rows, monolith, ambush, rushbait_v2.
- **Hook:** `Plan::target`.
- **Expected effect:** about 20% fewer tribute trips per game at the same income, and about 2% fewer rounds to 1,000.
- **Falsifier:** on rows and monolith (12 panel games each), the tribute count per game falls by less than 20%, or rounds to 1,000 fail to beat baseline.
- **Build:** 20 min. First check what v1 does when 2 allies reach the same reward.

## C14. Race-map pacifism (Battlecode macro vs micro) — ANALOGY

- **Source:** a Battlecode 2019 postmortem observes that early combat units slow the economy a lot. This is from a search snippet; the PDF was not read: https://battlecode.org/assets/files/postmortem-2019-oak.pdf . Counterweight: the MIT micro lecture in C11.
- **cambc mechanic:** the game is a race to 1,000 tributed gems (How to win). Time spent chasing is time not farming. A champion who chases and dies loses 60 rounds (Death).
- **Rule:** on race and mixed maps, drop chases and hunts. Fight only when hit, or when the target is a carrier with 40 or more gems that is lethal in range this turn.
- **Maps:** rows (0.4 kills a game, race), rushbait_v2 (5.3, mixed; the name invites a rush), monolith (5.3), ambush (4.3).
- **Hook:** `Plan::turn(Phase::pre)`, gating v1's engage branch. The builder finds that branch; no tunable is assumed.
- **Expected effect:** about 2 to 3% fewer rounds to 1,000 on rows.
- **Falsifier:** on rows and rushbait_v2 (12 panel games each), median rounds to 1,000 do not fall by 2%, or wins fall below baseline on either map.
- **Build:** 10 min.

## C15. Timing invade on the predicted enemy chain (StarCraft timing attack; LoL level-1 invade) — ANALOGY, bold

- **Source:** a timing attack is planned so that power peaks as the army moves out. https://liquipedia.net/starcraft/Build_order . Invading the enemy jungle to "take their creeps or ... kill the opposing jungle champion while they're clearing": https://www.gameskinny.com/tips/league-of-legends-counter-jungle-guide/
- **cambc mechanic:**
  - foundry-v16 and its clones derive their opening chain deterministically from static map data (tarot #1), so we can compute the mirror chain from their spawn in `Plan::init` (draft fuel ×10).
  - An awake camp hits the nearest champion (Camps & capture), so they tank it while clearing.
  - An enemy on the region resets their capture.
  - Before their first shop, nobody has shards.
- **Rule:** on maps where our spawn is within about 25 steps of their second chain stop, 2 or 3 champions arrive there as the group nears death. They contest the capture and attack the damaged enemy.
- **Maps:** ambush and rushbait_v2 (4 or 5 picks, mixed). The retest record EXP-020 found that a wipe works (5 kills, 0 deaths by round 73) but games ran 100 to 150 rounds longer; this card must beat that.
- **Hook:** `Plan::init` plus `Plan::turn(Phase::mid)` for rounds 1 to 120 only.
- **Expected effect:** about +2 wins of 12 on ambush, if the rejoin cost stays under 50 rounds.
- **Falsifier:** on ambush and rushbait_v2 (12 panel games each), wins fail to reach baseline + 2 on either map, or rounds to 1,000 rise by more than 50.
- **Build:** 35 min.

---

## Notes from the search (not cards)

- **Earlier Cambridge seasons:** I found no public postmortems. battlecode.cam redirects to https://2026a.battlecode.co.uk/ (title only). The Cambridge finals page https://luma.com/a1jsj4wq describes "weeks of ladder play and qualifiers", then a live 16-team double-elimination bracket at the Cambridge Union. That bears on PLAN.md open decision 1 (ladder vs tournament standings), but the page gives no year and says nothing about Sprint 1.
- **MIT postmortem PDFs** do not render through WebFetch here (binary), and the local Read tool lacks pdftoppm. The C11 and C14 citations rest on tables of contents and search snippets.
- **Rule observation, no outside source** (for IDEAS.md if wanted): the tribute length is fixed by the bag at the start, and completion banks the whole bag, including gems captured since, because captures resolve before channels (Gems & altars, timing details). If any capture tile lies within Chebyshev 1 of an altar, a carrier holding 1 to 10 gems can start a 1-tick tribute the turn before its capture completes and bank the full reward in that tick. `Plan::init` can scan for such tiles in 5 minutes. If none exist, the idea is closed.
