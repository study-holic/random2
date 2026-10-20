# DeepSeek (DSeek) Part 2 answer (collected 20 Oct). CONDENSED: tables flattened, wording trimmed; sections A1-A2 were read in a truncated page extract and are summarised. Section A3-A7, B and C-D were captured in full text and are kept close to verbatim in meaning.

Preface on new facts: (i) six stale local maps mean the local rejections of (a) and (d) are not yet evidence against those ideas; re-run on corrected layouts. (ii) 27-109, deaths 12.0 vs 4.8, 0-8 on collision/eight/elkring/fragment: treat as the real-team baseline; it is a "cannot survive the opening" problem. (iii) gem-per-round scoring failed for lack of a survival term ("If a camp kills you, the carried gems are lost"). (iv) "Melee champions can walk and attack in the same turn" -> disengage only before contact, or with dash/stun.

## A1 Battlecode seasons
- Directional-scoring micro (8 directions scored on target distance, ally distance, threat, cast availability) [Court for Clout 2024]: -2 to -5 deaths/game on the 0-8 maps.
- "Both ways" attack-move evaluation [SPAARK 2025 postmortem]: melee evaluates walk+attack, attack+walk, walk+cast, cast+walk; links to the 33/60 never-cast victims. Rule: "Ranged champions normally choose walking OR a basic attack".
- Draft memory as precomputed camp order/altars/rally (48-120 bytes) [4 Musketeers 2023]: faster first tribute (69 vs 86).
- Symmetry-based opponent prediction via map symmetry in the API: mirror alerts in shared memory; -1 to -2 ambush deaths.
- Do not switch targets every turn [Om Nom 2025]: store target group id (1 byte).
- Retreat hysteresis [SPAARK 2025]: recall at <35%, commit; do not recall carrying >20 gems unless enemy within 4 (14% of deaths had bags >=20).

## A2 MOBAs
- Camp blocking (inverse of Dota stacking): "Any occupied spawn tile delays the entire group." One champion-round denies a 60-round respawn. +0.05-0.1 gems/round on few-gem-group maps (fragment 6, contest 2). Falsifier: denial costs more than the denied gems.
- Jungle timers from capture round + respawn column (wolf 40, elk 60, gem 50, gold 50, monolith 150): +0.05-0.15 on high-supply maps (swirl 28.4, expanse 21.2, lanes 17.4).
- Objective trading (push opposite camp when 2+ enemies seen): avoids lone fatal fights; +0.05-0.1.
- Gold-driven recall (gold >=150 near spawn -> recall to buy emblem); top-10 first shards power 2,186 / jungle 1,931 / durability 1,765.
- Split pushing: "Extra allies do not speed capture" is a designed split-push incentive; fast champions take far camps.
- Counter-picking: Stellar in 98% of top-10 drafts, our top killer; "Cassi and Issac + Ren + Stellar 57% (n=215, +5)"; +10-30 rating if Stellar pick rate 32% -> >70%.

## A3 RTS
- Kiting/stutter-step: ranged alternate attack at range 3 / step away at range 2. -2 to -4 deaths. Falsifier: dash champions (Issac/Vendetta/Ren) close anyway.
- Focus fire with kill confirmation; Cassi & Issac are one kill removing two bodies, first focus target ("Cassi and Issac kills Cassi and Issac 368"; they are 51% of our deaths). -3 to -6 deaths.
- Concave/surround: 3 champions approach 3 different adjacent tiles; "60 deaths with no ally within 4 tiles".
- Timing attack on emblem completion ("Five matching shards automatically combine into the corresponding emblem"); Tavi-Focus 12/12 on contest is a timing attack.
- Threat/influence map: 16x16 local grid; aggregate "threat at [x,y]" and "safe altar" in the 8 bytes. -1 to -3 deaths.
- Build order as opening book in draft memory (draft phase "can see the entire map layout"); first tribute 69 vs 86; +0.1-0.2 gems/round.

## A4 Team sports
- Zone marking at camps (no shared vision makes man-marking impossible); fight only with local advantage. -2 to -4 deaths.
- Pressing trigger: enemy stationary 2 rounds is probably recalling; "Damage or stun interrupts" recall, so a basic attack cancels it (tribute is not interrupted by damage). Cancel 1-2 recalls/game.
- Set play: Cassi stun (range 4) -> Tavi 2.2xFocus (range 4) -> melee steps in; needs priority ordering ("set_priority takes effect from the next round"). +1-2 kills/game.

## A5 Operations research
- Prize-collecting TSP/orienteering = option (a) with a survival term ("if enemy within 4 tiles of path, profit = 0"): +0.1-0.2 gems/round on 4-5 maps.
- VRPTW: arrive 1 round after respawn (capture 55 -> respawn 115 -> arrive 116); +0.05.
- Altar queueing: two champions with 15 and 25 gems take 2 and 3 ticks; second should go elsewhere; "altar X busy until Y" in 2 bytes; +0.02-0.05.

## A6 Game theory
- Draft is simultaneous-with-revealed-history, backward induction does not apply cleanly; dominant action is Stellar first; do not overthink the counter table ("fires on 0-0.1% of drafts").
- Mixed strategies ~0 value (deterministic engine, no persistence). Exploitation vs safety: safe policy (bag safety, cadence) first; +0.1 gems/round, -2 to -5 deaths.

## A7 ML / adaptive
- Conditional policies in draft memory keyed on enemy draft (80 bytes): +0.05 gems/round.
- 1-variable Kalman/Bayes estimate of enemy damage per hit; avoid champions with estimate >50.
- Do not use bandits in-match. Logistic fight-outcome model (10 weights, 40 bytes) with HP ratio, champion count, matchup, ally distance, altar distance: -2 to -4 deaths.

## B Micro and matchups
(i) Per kit: Cassi stun first then damage next turn, stun melee at range 2; Issac 3 basic attacks in 2 rounds with reset; Cassi & Issac move as a pair, Cassi behind Issac; Ryth AoE stun radius 1 and Health shards give Strength (passive); Vesper +20% gold, Treasure emblem; Prism "gains 0.2 Focus per carried gem" (carry 10-20 gems while fighting); Stellar cast on cooldown, never basic attack, no walking and casting; Vendetta kill a camp monster last to reset, then dash into enemy; Marcella "Immune to stuns" (counter to Cassi/Tavi/Ryth) but -0.15 casting from range 4 per our duel lab; Tavi buys shards from anywhere, assassin build not camp clearer; Willow heal before the burst (315 HP at base); Ren wants to be attacked (retaliation 0.2xFocus); Varg splash and shield (2.5xFocus) cast before walking in, keep Cassi & Issac >1 tile apart vs Varg.
Pairings: Stellar vs Cassi (whoever casts first wins; Cassi holds stun for Stellar's cast turn); Stellar vs Tavi (Tavi needs 5 Focus shards first); Varg vs Cassi & Issac (splash; never clump); Vendetta vs Stellar (dash 2 + walk 1 closes 3 tiles; Stellar must not be within 4 of a dash-ready Vendetta); Ryth vs 2+ enemies; Marcella vs stun drafts.
(ii) Weights from our data: Stellar top killer on 7 maps; Cassi & Issac 51% of our deaths; killer distribution Stellar 21%, C&I 18%, Varg 16%, Ryth 13%, Tavi 11%, Ren 9%.
(iii) 8-byte layout: bytes 0-1 objective (x,y); 2 threat flag; 3 bag-safety flag (a teammate carries >=20); 4-5 last enemy cluster (x,y); 6 round of last update mod 256; 7 tribute cadence flag (0 bank to 10, 1 to 20, 2 to 36). Draft memory: map id -> opening book pointer, enemy list, per-enemy conditionals, rally points, camp priority order. Each champion decision tree: escort if bag flag; casters hold at range 4 on threat; melee with dash engages; else farm; override recall if HP <35% or bag >=20 with enemy within 4. Do not put respawn timers in shared memory (derivable).
(iv) Experiment design: Tier 1 duel arena 15x15, 121 ordered matchups x ~6 geometries x 2 policies x 100 games = 145,200 games (~22 h at 110 games/min); Tier 2 squad arena 4,000 games; Tier 3 real-map shadow duels 30 games per policy; Tier 4 30 real-team games. Pre-registered falsifiers: kiting <+5 points at range 3; stun-before-cast <+8 points vs cast first; dash-engage <+5; pre-emptive heal <+5; Varg splash target <+5. Randomise who acts first ("Teams alternate: the team that starts swaps every round"; "Priority, then draft order"); priority itself is a separate micro row. Spend 60% of duels on Stellar-vs-C&I, Varg-vs-C&I, C&I mirror.

## C New strategies (C1-C15; full text in this chat; one line each)
- C1 capture-and-tribute in one round ("Captures resolve before channels"); DeepSeek itself unsure whether channel length is snapshotted at start; needs a gem group adjacent to an altar; falsify by 10-game replay audit.
- C2 one-round respawn block: stand on a gem-camp spawn tile ("Any occupied spawn tile delays the entire group"); cap to maps where enemy camps are far (eight, fragment, stampede).
- C3 Tavi field-buy Health before recall ("Health bought away from spawn raises maximum health without healing"): 5 shards = 375 max HP for 150 gold realised on next recall; falsify by logging max/current HP across a field buy and recall.
- C4 priority-ordered set play (Cassi priority above Tavi so stun lands first); falsify by logging cast order in 20 replays.
- C5 recall-cancel press (any basic attack cancels recall).
- C6 defensive altar lock on own altar while channelling a big bag ("An altar lock blocks opposing tributes but allows its owner's team to tribute"); untested (0 lock placements).
- C7 bait the camp: position so the enemy is the nearest champion to an awake camp ("attack the nearest champion until undamaged for 2 turns"); free elk damage.
- C8 Prism gem-carry spike (0.2 Focus per gem); practical cap 20-30 gems, marginal.
- C9 Vesper gold specialist on gold-heavy maps (pilgrimage, stampede, ambush; +20% gold, Treasure emblem to 40%).
- C10 carry exactly 1 gem to a locked altar: "Attempts to tribute against a locked altar are implicitly converted to auto attacks against the altar lock."
- C11 Marcella anti-stun counter-pick ("Immune to stuns").
- C12 8-byte beacon for enemy Stellar position.
- C13 respawn-window push scheduling (60-round respawn); low value on short maps.
- C14 two-altar cross routing (altar positions not in the file; gap).
- C15 per-champion tribute cadence: bank 10 for melee, 20 for casters, 36 only when altar >15 tiles away.
Rejected by DeepSeek as unsound: tribute-in-fight bait; draft memory as a lookup of enemy behaviour.

## D Ranking (rating pts, days)
1 C15/M1 cadence + altar-first routing (+60-100, 1.0 d); 2 8-byte protocol + turn-flow reorder (+50-90, 1.0); 3 C2 spawn-tile block (+40-80, 0.5, caveat denial champion dies); 4 both-ways micro + kiting (+30-60, 1.0); 5 objective trading + C12 beacon (+30-60, 1.0); 6 C7 bait the camp (+20-50, 0.5); 7 opening book in draft memory (+20-50, 1.0); 8 C11 + Stellar-first draft (+20-40, 0.5); 9 orienteering with survival (+20-50, 1.5); 10 C9 Vesper (+10-30, 0.5); 11 C5 recall-cancel; 12 jungle timers; 13 C3; 14 C6; 15 C13.
Not ranked: monolith siege beyond contest, damage-only builds, bundled engagement rules, offensive locks, bandits/mixed strategies.
Plan: Day 1 cadence + shared memory/turn-flow; Day 2 respawn block, both-ways micro, draft table; Day 3 beacon, camp-edge, opening book; Day 4 gate results; Day 5 hardening.
Fact check flag: if the 27-109 set was mostly on the stale maps, death items move up and cadence moves down.
