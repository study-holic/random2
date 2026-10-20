# Arena tests on engine 1.0.0 (20 Oct 2026, cloud runner)

CLI cambc 3.0.1, engine 1.0.0, SDK 3.0.1 (the cloud parity test, `results/cloud-parity.md`, showed this container matches the laptop game for game). Maps written with the engine's own MapBuilder (`tools/arena/mb1.mjs`, the map editor's v1_0_0 WASM). The engine is deterministic: a repeated run of the same map and bots is the same game, so "runs" below are distinct setups (both teams mirrored, jittered start positions, map salts), not reseeds.

## Tables

### 1. Camp rewards and damage ranges

| Camp | Reward measured (one capturer) | ENGINE-1.0.md | Damage taken by Chebyshev distance (hits / own turns at that distance while awake) | First-damage range | ENGINE-1.0.md / FACTS |
|---|---|---|---|---|---|
| Wolf | 4 gems, 40 gold | 4 / 40 | d3 6/22 (20 each) · d4 0/18 · d5 0/14 · d6+ 0 | **3** | 3 ✓ |
| Elk | 15 gems, 90 gold | 15 / 90 | d3 6/22 · d4 6/18 (60 each) · d5 0/14 | **4** | 4 ✓ |
| Monolith | not killed (20,000 HP) | 200 / 300 | d3 4/20 · d4 2/18 · d5 2/16 (200 each) · d6 0 | **5** | 5 ✓ |
| Gem cluster | 7 gems | 7 | str 0 | – | ✓ |
| Gold cluster | 20 gold (26 to Vesper: +33 % passive, floored) | 20 | str 0 | – | ✓ |

| Split test | Present on the region at completion | Paid |
|---|---|---|
| Group of 3 gem clusters (21 gems) | 2 allies on two of its tiles | 11 + 10 (remainder to the lower id) |
| Group of 3 gold clusters (60 gold) | 3 allies | 20 + 20 + 20 (Vesper 26) |
| Single gem cluster | capturer on the tile, ally standing adjacent | 7 + 0 (adjacent does not count) |

### 2. Monster targeting (one elk at the centre, one Vesper per team; per-round HP log)

| Setup | Who the elk hit | Reading |
|---|---|---|
| A attacks from d3, B passive at d2 | B every round (14 hits) until B died, then A | **nearest**, not the attacker |
| A attacks from d2, B passive at d3 | A until A died, then B | nearest |
| Both attack from d3 | alternated A, B, A, B … every round | tie: alternates (consistent with "last to damage it", since the starting team swaps each round) |
| A attacks once from d3 and walks to d6; B passive at d2 | B every round until B died (14 hits); A never | **aggro as a weapon works**: a group woken by us keeps hitting an enemy standing nearer |
| A attacks from d3; B walks in to d4, d3, d2 | A while B at d4; A at the tie d3 (only A attacking); B from the round B reached d2 | switches to the nearer champion at once |

Asleep camps never attacked: 4 rounds at d3 of the wolf, elk and monolith (and d2 in the targeting maps) before the first hit, 0 damage. A group went back to sleep 2 rounds after the last in-range round (wolf r34 after r32; elk r35 after r33; monolith r36 after r34).

### 3. Respawn delay (single gem cluster, respawn 50)

| Setup | Capture round | Occupier leaves | Respawn | Timer while occupied |
|---|---|---|---|---|
| Control: capturer steps off | 19 | 20 | **69** (= capture + 50) | – |
| Own champion stays on the tile 100 turns | 88 | 189 | **189** | counted 50 → 0 by 138, then held at 0 |
| Own champion on an adjacent tile 100 turns | 208 | – | **258** (= + 50) | adjacent: no effect |
| Enemy stands on the tile after our capture | 41 | 148 | **148** | counted to 0 by 91, held at 0 |

Same results on both sides of each map (mirrored scripts).

### 4. Capture contest

| Setup | Result |
|---|---|
| A on a tile of a 3-cluster group from round 42; enemy B on another tile of it rounds 44-54 | B left during its turn 54, so the region was uncontested from the round-55 phase; reward (21 gems, all to A) paid at round 60. Every capture in these tests paid 5 rounds after the first uncontested occupied phase (rewards test 94 → 99; adjacent test 36 → 41; this test 55 → 60), so **the enemy reset the timer**; B got nothing. |
| Enemy adjacent to (not on) the tile for the whole capture | Paid on time (on tile 36, paid 41): **adjacent enemies do not contest**; the region is the spawn tiles |

### 5. Prism with carried gems, 1v1 (arena-duel, fight to the death; both orders)

Prism's Focus measured at the duel start: 80 / 101 / 122 with 0 / 21 / 42 carried gems = **+1.0 Focus per gem** (gems come in 7s, so the 20 and 40 levels are 21 and 42).

| Prism vs | 0 gems | 21 gems | 42 gems |
|---|---|---|---|
| Stellar | 0-160 (0 %) | **115-28 (80 %)** | **145-7 (95 %)** |
| Ren | 0-160 (0 %) | 0-104 (0 %) | 0-88 (0 %) |
| Varg | 0-145 (0 %) | 0-96 (0 %) | 0-80 (0 %) |

Order split (Prism on A / on B), 42 gems vs Stellar: 76-2 / 69-5; 21 gems: 56-15 / 59-13; all other cells 0 wins on both sides.

| Damage per cast (immediate health drop of the target) | 0 gems | 21 gems | 42 gems |
|---|---|---|---|
| Slot 0 stun + damage (n 990 / 751 / 716) | 120 (= 1.5 × 80, every cast) | 151 (= 1.5 × 101) | 183 (= 1.5 × 122) |
| Slot 1 area (n 787 / 629 / 595), median | 80 | 101 | 90 |

Area casts deal exactly 1.0 × Focus to an unshielded target. Lower readings are Varg's shield absorbing (all Varg casts but 15) or a finishing blow on a low target; 3 of 2,011 readings exceed 1.0 × Focus (unexplained).

With the lab's retreat policy instead (P_ADAPT without P_NORETREAT, `results/arena-1020-duels-adapt.json`): vs Stellar 5 % / 0 % / 42 %, vs Ren 0 % everywhere, vs Varg 773 of 774 duels end with Prism escaping at 100-180 HP while Varg has 590-770.

### 6. Heart of Ruin

1v1, fight to the death, both orders, about 76 duels each:

| HoR vs | W-L | HoR on A / on B |
|---|---|---|
| Prism (0 gems) | 80-0 (100 %) | 40-0 / 40-0 |
| Stellar | 80-0 (100 %) | 40-0 / 40-0 |
| Willow | 80-0 (100 %) | 40-0 / 40-0 |
| Tavi | 76-0 (100 %) | 38-0 / 38-0 |
| Vesper | 76-0 (100 %) | 38-0 / 38-0 |
| Vendetta | 63-13 (83 %) | 32-6 / 31-7 |
| Ren | 48-24 (67 %) | 26-10 / 22-14 |
| Ryth | 24-48 (33 %) | 10-26 / 14-22 |
| Varg | 18-42 (30 %) | 9-21 / 9-21 |
| Cassi & Issac | 0-76 (0 %) | 0-38 / 0-38 |
| Marcella | 0-72 (0 %) | 0-36 / 0-36 |

2v2, HoR + Stellar vs Ren + Varg, one corridor, fight to the death: **24-34 (41 %)** over 58 team fights; HoR's team on A 8-10, on B 16-24. With the retreat policy most duels end in escapes (C&I 36 of 61, Ryth 27 of 79), so the table above uses the decisive policy.

## Setups

- **Probe bot** `bots/arena-probe` runs a step script per champion (go to, wait, hit once, kill a group, wait for a group state, attack in place) and logs per own turn: position, health, gems, gold; camp health and awake state; group state and respawn timer. Maps `maps-arena/p00..p13` (`tools/arena/mkprobe.mjs`): 31 high, width 31 + 2 × scenario (the bot reads its scenario from the width), rotational symmetry, A spawn x 0-3, y 0-3. Damage seen at the start of turn r was dealt in round r's camp phase with the champion where its line says.
  - Ranges (p00-p02): a Vesper (ranged 3, 900 HP) walks to distance 3 of a lone camp in steps of 2-3 turns, hits it once, backs off one tile a turn to d8, waits 6 turns, then repeats with 2 turns per tile. Both teams run it on their own copy: 2 sides × 2 cycles.
  - Rewards (p03): 5 champions (Vesper, Stellar, Prism, Willow, Tavi) kill one group at a time; named champions stand on its tiles; the others wait off the region. Both sides.
  - Targeting (p04-p08): elk on the centre tile (self-mirrored), one Vesper per team, positions in the table; scripts start at round 40.
  - Respawn (p09 own, both sides; p10 enemy) and contest (p11 enemy on the region, p12 enemy adjacent, p13 ally adjacent).
- **Duel bot** `bots/arena-duel` = `bots/w14t` (the w14 lab duel bot) with: Heart of Ruin added (draft id 11; shield when the foe is within 4 or below 60 %, dash-in to land its stun), camp tiles blocking its path field, a 2-bit carried-gem target (Prism farms the six gem clusters at its corridor end before each duel and tributes any excess at an altar beside its spawn; opponents tribute everything, so Prism always starts at exactly 0, 21 or 42), a cast log line, and a 3-bit map salt in its start jitter. Policy for both teams: P_ADAPT (the lab's per-champion table) + P_NORETREAT. Arenas: `tools/arena/mkduel.mjs` (the w14 50 × 65 corridor layout built from scratch), one champion per team (two in the 2v2), driver `tools/arena/duels.py` (122 games, 74 s on 3 jobs).

## Rule lines

| Rule | Verdict |
|---|---|
| ENGINE-1.0.md l.70-74 rewards: wolf 4/40, elk 15/90, gem cluster 7, gold cluster 20 | **confirmed**; the SDK `camp_group` gold/gem values on these maps carry the same figures |
| ENGINE-1.0.md l.60-62, FACTS l.12-16: ranges wolf 3, elk 4, monolith 5 | **confirmed** |
| GameRules l.164 "All monsters have attack range 3 … do not chase" | **contradicted** for elk (4) and monolith (5); no camp moved in any test (consistent with "do not chase") |
| GameRules l.165 "Damage wakes the group; two rounds without an in-range champion let it sleep" | **confirmed**: no attack while asleep even at d2-3; sleep 2 rounds after the last in-range round |
| GameRules l.162 "Allies on the region split the reward; the lowest ID gets the remainder" | **confirmed** (21 → 11 + 10; 60 → 20 × 3); the region is the spawn tiles: an adjacent ally gets nothing |
| GameRules l.163 "Empty or contested regions reset it" | **confirmed** for an enemy on a spawn tile; an adjacent enemy does not contest |
| GameRules l.139 "Any occupied spawn tile delays the entire group" | **confirmed, and quantified**: the timer runs normally while the tile is occupied; at 0 the group waits for as long as any spawn tile is occupied and respawns in the round it is freed. Delay = max(0, leave round − (capture + respawn)). Enemy occupation counts the same |
| No rule line on monster targeting (the site doc is silent) | measured: nearest champion; ties alternate; a woken group hits a nearer enemy who never attacked it |
| FACTS l.47 Prism: stun + damage 1.5 × Focus, area 1.0 × Focus, +1.0 Focus per carried gem | **confirmed** to the point (120/151/183 and 80/101/122) |

## Caveats

- Duel win rates measure these bots: w14t's fight policy, open corridors, no items, no camps nearby. A 100-0 or 0-100 cell means the outcome is set by stats and policy with little room for start positions to change it. It is not a ladder win rate. The many identical results show that the effective number of distinct duels is smaller than the count.
- 48 of 122 duel games exhausted the 200,000-fuel first gameplay turn (init); the champion idles on its spawn for round 1. 6 probe turns out of about 3,000 hit 100,000 fuel and lost that turn, delaying a script step by one round at most. No measurement above depends on a lost turn.
- Raw data: `results/arena-1020-probe.json` (tests 1-4: range rows (round, distance, damage), targeting timelines, event logs), `results/arena-1020-duels.json` (decisive policy), `results/arena-1020-duels-adapt.json` (retreat policy).

## Added 20 Oct (w1 session): Monolith regeneration and the "nearest" metric
| Scenario | Setup | Result |
|---|---|---|
| p14 mono_regen | Vesper hits a Monolith once from d3, backs off to d8 (in sight) for 380 rounds | Monolith 19,950 HP unchanged (**no regeneration**); the champion's 300 HP also unchanged away from spawn |
| p15 metric | elk; A diagonal at Chebyshev 3 (Euclid² 18) attacks, B straight at Chebyshev 4 (Euclid² 16) passive | A hit every round, B never: **nearest = Chebyshev** |
| p16 metric_ctl | elk; A straight at Chebyshev 3 (Euclid² 9) attacks, B diagonal at Chebyshev 3 (Euclid² 18) passive | hits alternate A, B every round: a Chebyshev tie alternates whatever the Euclidean distance |

## Arena 7: past the per-turn fuel limit (sweep35 task 2, 20 Oct evening, cloud)
Probe: `bots/fuelprobe` (drafts like bots/idle) vs bots/idle on elkring, `cambc run --release`. Every champion takes one step each turn. On the burn round it steps first, then runs a loop of BURN_FUEL units (calibrated at 0.996-0.998 fuel a unit), then tries a second step and prints `AFTER` with the round it reached. Per-turn fuel is read from `team-a.fuel` in the bundle; fuelExhausted events come from `--statistics`.

| burn (round) | limit | over by | first action of the turn | fuelExhausted events | turns recorded at the full limit | `AFTER` printed at | champion frozen |
|---|---|---|---|---|---|---|---|
| 101,000 (r50) | 100,000 | ~1k | kept (the step shows in the next snapshot) | – | 11 | round 61 | rounds 50-60 |
| 110,000 (r50) | 100,000 | ~10k | kept | 12 per champion, rounds 50-61 | 12 | round 62 | rounds 50-61 |
| 150,000 (r50) | 100,000 | ~50k | kept | – | 16 | round 66 | rounds 50-65 |
| 175,000 (r1; 21k already used by init) | 200,000 | ~-4k + init | – | – | ≥ 9 of 10 played | not by round 10 | yes |

**Result.**
- **The turn is not cut short with its actions lost.** Actions taken before the limit are applied.
- **The bot is not removed.** No botFault is raised, and the game runs to round 2,000.
- **Execution is suspended mid-code and resumed later from the same point.** The second step and the print run, but only after a freeze.
- **Freeze length:** the champion takes no action for about 10 rounds plus about 1 round per 10,000 fuel of overrun. Fit: 1k over gives 11 rounds, 10k gives 12, 50k gives 16. Every frozen turn is recorded at the full 100,000 and raises one fuelExhausted event.
- **Later turns are delayed, not lost:** the bot resumes with its stale view of the world. One overrun of any size costs that champion about 10 rounds standing still: it does not move, attack or channel. It also keeps standing on a camp tile or spawn, and is exposed to camps and enemies.
- **Implication:** the fuel stop must leave margin. The 97.7k peak of S2 and the 100,000 turn of s1g1 on starvation are each about 10 frozen rounds when they tip over.
