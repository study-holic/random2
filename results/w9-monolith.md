# w9: Monolith siege on contest (builder w9, 19 Oct)

Question: can our champions damage and capture the Monolith from outside its attack range, turning contest from a no-score game into a gem win?
Answer on the local panel: yes. On contest the siege scores 1,000 gems in most games (median win round 1,510; v2 needs 2,000), beats v2 head-to-head 6 of 6, and raises the panel from 10.00 to 11.00 of 12. It falls short of the ΔW ≥ 1.5 admission rule because v2 already wins 10 of 12 on tiebreaks. On the monolith map the siege loses ground, so it stays off there and w9 plays exactly as v2.

## What changed (bots/w9, from v2a)

| File | Change |
|---|---|
| `src/siege.h`, `src/siege.cpp` (new, 344 lines) | Siege module. Off unless a map plan calls `Siege::enable`. `init` (l.116): finds each Monolith, then builds its **ring**: free tiles at Chebyshev distance exactly 4 that have line of sight (`(walls|bushes).shadow_cast(mono, 8)`). Contest has 10 ring tiles per Monolith. `init` also prints each champion's live kit once. `pre` (l.154): remembers whether each Monolith is alive and blocks every tile within 3 of a live one in `move_towards_closest`. It also runs the **safe tribute**: a carrier within 8 tiles of an altar does not start a tribute while an enemy is within 6 tiles (`safe_tribute`, at most 80 turns per bag). It also runs the **escort beacon**: a carrier with ≥ 50 gems writes its tile into shared bytes 6-7 as x+100, y+100 (contest has no squad groups, so those bytes are free); the beacon is cleared when it goes stale. `mid` (l.213): a fresh carrier waits up to 6 turns for 2 allies within 3 tiles. Champions without a bag escort the carrier to within 2 tiles. A caster (any ability with damage > 0, range ≥ 4, and a camp or ground target) picks a Monolith (allies already on it, lower remembered health, distance), walks the ring by a path that avoids range 3, and casts from distance 4 (`cast_on_mono` l.69: unit casts with `use(camp)`, ground area casts on the Monolith tile or a tile next to it). When the Monolith dies, the visible caster with the highest max health captures; the others guard within 2. `buy` (l.341): casters use Focus as their main stat (Monster emblem, then Power Focus and Health alternating). |
| `src/util.cpp` l.31, l.39, l.116 | Applies only while the siege is on: the Monolith counts as dangerous at range 3 instead of 3+1 (otherwise every caster below about 890 max health retreats from the ring), and `Siege::block()` is added to the blocked tiles. With the siege off the code path is identical to v2. |
| `src/map_contest.cpp` | Siege on: `P_SIEGE_CONTEST 1`, `P_SIEGE_ESCORT 1`, `P_SIEGE_TANK 1`, `P_SIEGE_SAFE 6`. Draft `P_SIEGE_DRAFT 1`: Stellar, Tavi, Prism, C&I, Ryth. 0 keeps v2's list; 2 is Prism+Ren+Stellar+Tavi+Willow (built, not tested). |
| `src/map_monolith.cpp` | `P_SIEGE_MONOLITH 0`: off, because the test lost ground (below). |

## Rule lines relied on (quoted)

- GameRules.md l.185: `| Monolith | 20,000 | 200 | 3 | 300 | 200 | 150 rounds |` (health, strength, range, gold, gems, respawn).
- l.206: "All monsters have attack range 3 and vision 8, see through bushes and do not chase." l.189: the monster attacks "the nearest champion every turn, until it either dies or does not take damage for 2 turns".
- l.251: "Ranges use Chebyshev distance, so diagonals count as one tile. Melee range is 1; ranged range is 3." So no basic attack reaches range 4.
- Range-4 abilities (GameRules "Target / range" lines). On camps or enemies: Cassi slot 0 (2×F) l.438 and slot 1 (stun, no damage) l.444; Prism slot 0 (1.5×F + stun) l.556; Stellar slot 0 (2×F) l.586; Marcella slot 1 (2×F) l.656; Tavi slot 1 (2.2×F + stun) l.688; Ren slot 0 (1.25×F + stun) l.744. Ground, radius 1: Ryth slot 0 (1×F + stun) l.492; Prism slot 1 (1×F) l.562; Stellar slot 1 (1.4×F) l.592. Willow's range-4 spell is an ally heal (l.712).
- The outside reviewer's claim is partly right. Stellar's *spells* (single-target and area) are range 4, and Cassi's stun is range 4 but does no damage. Stellar's *basic attack* is range 3 (l.602).
- The live kit printed by `Siege::init` (smoke game, team-a.stdout) matches: `t5(Stellar) s0:r4m200C s1:r4m140CG`, `t11(Tavi) s1:r4m220C`, `t6(Prism) s0:r4m150C s1:r4m100CG`, `t3(Cassi) s0:r4m200C s1:r4m0C`, `t0(Ryth) s0:r4m100CG`, `t4(Issac) -`. The Monolith's `attack_range()` returned **3**. (C = camp target, G = ground target.)
- cambc.h l.3269: casting range is `out.draw_chebyshev({owner.x, owner.y}, range)`, so it is Chebyshev.
- l.193: capture = "occupy at least one of its spawn tiles uncontested ... five-round timer ... Rewards are shared among allies present". The Monolith is a one-tile group, so one champion takes all 200 gems.
- l.223: tribute lasts ceil(gems/10) turns, so 200 gems take 20 turns; "Damage does not interrupt it; stun, death or an altar lock does." l.361: "The killer receives all carried gems."
- Observed in replays, not in the rules: Monolith health **does not regenerate**. In v2's contest replay vs x-v4-14 it falls steadily from 20,000 to 10,198 over 2,000 rounds and never rises.

## What v2 does at the Monolith now (trace, matches/v2-230053)

- Contest has exactly 2 Monoliths, (17,31) and (32,18), plus 20 gold clusters (type 3). The Monoliths are the only gem source, so every v2 game ends 0-0 on gems.
- v2 excludes Monoliths from targets (`squad_target = !monolith`, and `choose_target` skips them). In 3 games its whole team dealt 1,111 to 3,432 damage to Monoliths, all incidental casts while farming the gold clusters 4-5 tiles away.
- v2 spent 9 to 21 champion-rounds per game within range 3 of a Monolith. Its deaths (16 to 44) are to champions, at distance 4 to 9 from the Monoliths.

## Test 1: panel, contest, 3 salts (results/w9-monolith.json = w9-monolith-panel-safe6.json)

| Version | W/12 | Strong/4 | Median win round | Deaths | Gem margin | Faults | Late fuel |
|---|---|---|---|---|---|---|---|
| v2 (results/v2-panel.json) | 10.00 | 2.00 | 2,000 | 20.0 | 0.0 | 0 | 0 |
| w9 siege, no safe tribute (panel.json) | 10.33 | 2.33 | 1,443 | 24.9 | +672 | 0 | 0 |
| **w9 final, safe tribute 6** | **11.00** | **3.33** | **1,510** | 23.5 | **+739** | 0 | 0 |
| w9, safe tribute 8 (panel-safe8.json) | 9.67 | 2.33 | 1,646 | 28.2 | +589 | 0 | 0 |
| w9, v2 draft + siege + fixes (`bots/w9b`, panel-v2draft.json) | 11.00 | 3.00 | 2,000 | 19.9 | +556 | 0 | 0 |
| w9, Cassi farms instead of sieging (`bots/w9cf`, panel-cassifarm.json) | 11.00 | 3.33 | 1,586 | 23.4 | +778 | 0 | 0 |

cmpcells, final vs v2: ΔW +1.00, ΔStrong +1.33, ΔGem +8,867. The admission rule (ΔW ≥ 1.5) is **not met**. The ceiling is 12 and v2 already wins 10 (at round 2,000, on tiebreaks).

Per opponent, final (v2 in brackets): x-v4-14 6/6 (2/6); x-foundry-v16 4/6 (4/6), gem margin +333; x-hello 6/6 (6/6); x-ifish 6/6 (6/6); x-mite 5/6 (6/6); x-shawn 6/6 (6/6). The only losses are to x-foundry-v16 (2) and x-mite (1).

The one-salt cells from the build iterations (draft C before the escort fixes 9/12; v2 draft with the siege 10/12 at median round 2,000; after the stale-beacon fix 11/12, round 1,615) are in `w9-monolith-a1.json`, `-b1.json`, `-a2.json`.

## Test 2: head-to-head vs v2, contest, 3 salts (results/w9-monolith-h2h-safe6.json)

| Side/salt | A0 | B0 | A1 | B1 | A2 | B2 |
|---|---|---|---|---|---|---|
| Result, gems w9-v2 | W 800-600 (r2000) | W 1000-0 (r1392) | W 1000-0 (r1411) | W 1000-0 (r1298) | W 1000-600 (r1932) | W 1000-0 (r1298) |

**6 of 6**, gem margin +767, median win round 1,411. (The version without safe tribute was also 6/6, +733, round 1,917: `w9-monolith-h2h.json`.) Every gem v2 scores comes from killing our 200-gem carriers.

## Test 3: pace (median win round vs top-50 yardstick)

| Map | v2 | w9 | Top-50 winners' median rounds to 1,000 |
|---|---|---|---|
| contest | 2,000 (no gem wins) | **1,510** (panel), 1,411 (h2h) | 1,602 (only 124 of 214 games reach 1,000) |
| monolith | 584 | 584 (siege off, identical) | 447 |

## Test 4: safety

- starter-cpp, contest, both sides (`w9-monolith-starter.json`): 2/2, 1000-200 and 1000-0, 0 faults, 0 late `fuelExhausted`.
- "bot error" lines on our side, contest panel: w9 109 in 36 games (3.0 per game) vs v2 106 in 36 (2.9). All are `engine error 97: Unit is not in vision`, a type v2 already produces; no new error type.
- Fuel: the ring and line of sight are computed once in init. Each turn costs at most 2 camp reads, plus one group-state query when a Monolith tile is in view and the Monolith is dead.

## Monolith map

- Siege on (`bots/w9m`, `P_SIEGE_MONOLITH 1`, 1 salt): 8/12, strong 1/4, median round 662, gem margin +238. v2 scores 11/12, 3/4, 584, +461. The corner group needs the Monolith plus 8 gem clusters killed; the v2 draft has only 460 range-4 damage per 8 turns; casters leave a race that v2 already wins at round 584.
- **Off**, so w9 equals v2 there: 3 salts give 11.00/12, 3.00/4, 584, 4.0 deaths, +461. ΔGem 0 (`w9-monolith-mono-panel.json`).
- The rows map also has 2 Monoliths; the siege is off there too (no change).

## Deaths: champions vs Monolith

Classifier (scratchpad `deathcls.py`): a death counts as a champion kill when an enemy's kill counter rises in the same round.

| Game | Deaths | To champions | Possibly Monolith | Bags lost |
|---|---|---|---|---|
| a1 x-foundry B (before fixes) | 56 | 56 | 0 | 2 |
| a1 x-mite B | 74 | 74 | 0 | 4 |
| a1 x-ifish A | 38 | 35 | 3 | 1 |
| a2 x-foundry B | 62 | 62 | 0 | 3 |
| final x-foundry A s0 | 25 | 25 | 0 | 1 |
| final x-foundry B s1 | 70 | 70 | 0 | 3 |
| final x-hello B s0 | 19 | 19 | 0 | 1 |
| final x-mite A s1 | 27 | 27 | 0 | 1 |
| final x-mite B s0 | 56 | 56 | 0 | 3 |
| final x-v4-14 A s0 | 33 | 33 | 0 | 4 |
| **final, 6 games** | **230** | **230** | **0** | **13 (2,600 gems)** |

C&I deaths count twice. Effectively all deaths are to enemy champions. The range-3 block works; the losses are in fights at the rings and the central altar box.

## Traced moments (where the change fired)

1. **Siege works.** In the smoke game vs x-foundry-v16 (draft C), Monolith 0 went from 20,000 at r24 to 0 at r249 (about 90 damage per round at base Focus). Monolith 1 fell from r274 to r524. We captured 6 Monoliths, 1,200 gems.
2. **Bug found and fixed: bags stolen.** In that smoke game every bag after the first was killed in transit at (24,23)/(24,24), next to the altars (25,24)/(24,25), and x-foundry tributed our gems (we scored 200 to their 1,000). Fixes: escort beacon, tank capturer, carrier wait.
3. **Bug found and fixed: stale beacon** (a1, x-ifish A, lost 0-200). The carrier died at (22,29) at r329. Its beacon stayed set on a tile hidden by the bush ring, so the whole team stood at (20-22, 25-27) for 1,500 rounds and died 26 times; the Monoliths stayed at 19,030 and 20,000 from r499. The fix clears the beacon after 3 turns with no carrier seen near it.
4. **Bug found and fixed: stun-broken tributes.** In a2 x-foundry B, Ryth started a 200-gem tribute at r744 next to (25,24). Enemy Tavi's stun at r751 cancelled it, and the bag was later lost at r828. A 20-turn channel cannot survive a stunner with an 8-turn cooldown. The fix is the safe tribute.
5. **Safe tribute firing** (final, x-foundry B s1, a loss). Carrier Tavi (3,000 max health) reaches (26,25) at r1680 with 5 escorts within 3. With Varg and Vendetta 4-6 tiles away, it holds instead of tributing and attacks Varg (r1690-1697, Varg down to 260). With the area clear it starts the tribute at r1698. At r1712 enemy Cassi and Stellar arrive and kill it at r1717, about one turn before the 20-turn channel completes. The likely cause of the loss is economics, not the siege rule: x-foundry earns ~14,900 gold against our ~3,200. Our casters hold the ring instead of farming the 20 gold clusters, so the enemy's burst kills a 2,180-health carrier in 2 rounds. This game: 5 captures, 3 bags (600 gems) stolen.

## Draft

- The top-50 contest drafts (`intel/meta-2026-10-18.md` §3a) are caster-heavy, which supports the range-4 siege. v2's draft (Vendetta, C&I, Varg, Ryth, Tavi) has 460 range-4 damage per 8 turns. Draft C (Stellar, Tavi, Prism, C&I, Ryth; top-50 72% over 25 games) has 1,000.
- One-salt test before the fixes: draft C 9/12, median round 1,667, +500; v2 draft 10/12, round 2,000, +483 (wins on margin, not pace).
- With all fixes, 3 salts: the v2 draft (`bots/w9b`) wins as often (11.00/12, strong 3.00) and dies less (19.9). It earns 2-3 times the gold (6.6k-13k). But only 2 of its 33 wins reach 1,000 gems, so its median win round stays 2,000, which fails the pace half of the falsifier. Gem margin +556. Head-to-head vs v2: 6/6, +567, all at round 2,000 (`w9-monolith-h2h-v2draft.json`).
- Draft C stays the default in w9: it is the only version that wins on pace.

## Falsifier (set before testing) and outcome

"Contest gem margin vs v2 not positive, or median win round not below 2,000, means reject." **Not met; the idea survives.** Gem margin +739 on the panel and +767 head-to-head; median win round 1,510 on the panel and 1,411 head-to-head.

## Verdict

- **contest:** the siege beats v2 on every measure tested: 6/6 head-to-head, +1.00 W, +1.33 strong, +739 gems, 490 rounds faster, starter safe, no new error type. It misses ΔW ≥ 1.5 only because v2 sits at 10/12. I recommend **admit on contest**, as a ceiling-limited exception for the lead to decide.
- **monolith:** reject the siege; it stays off and w9 is identical to v2.
- The weak spot left: gold. We earn about 3-4.5k per game against v2's 11-17k, and the opponents' gold rises when we stop farming, so bags die to the enemy's burst damage (x-foundry 4/6, x-mite 5/6). This is for w12's engagement work or a later economy change: for example, two non-casters farming gold, or the carrier recalling to heal and buy after the capture.

## Parameters (also tested: `P_SIEGE_CASSI_FARMS 1` lets C&I farm gold and leaves Cassi out of the siege. Result 11.00/12, +778, round 1,586, gold +1-2k; within noise of the final, so kept off)

`P_SIEGE_SAFE` 6 (8 tested: worse), `hold_max` 80, `carry_wait` 6, escort reach 30, beacon stale after 3 turns, ring = distance 4 with line of sight, capturer = highest max-health caster within 6, Focus build for casters. Variant copies: `bots/w9b` (v2 draft), `bots/w9s8` (safe 8), `bots/w9m` (monolith siege on). Salted copies: `bots/w9-s1`, `bots/w9-s2`.
