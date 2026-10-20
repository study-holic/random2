# v4 base: cloud build and test (20 Oct 2026)

Engine 1.0.0, CLI/SDK 3.0.1, the 21-map pool, `--jobs 3`. Opponent for the head-to-heads: bots/v3b-e1, the live bot's source on the current SDK. Each head-to-head is 21 maps × 2 sides × salts 0, 1 and 2 = 126 games.

## Result
- **Gate: PASS.** `results/gate-v4-20261020-082049Z-linux.json`: 42/42 vs starter-cpp, 0 faults, max turn fuel 90,697, no fuelExhausted after round 1, mapid correct on all 21 maps.
- **Head-to-head vs v3b-e1: 86/126 = 68.3 %** (target ≥ 45 %). Deaths 18.4 a game, gem margin +180 a game. `results/v4-final-h2h.json`.
- **Still failing the kept-map preference:** corners 0/6, expanse 0/6 and lanes 1/6 lost to v3b-e1. No tested setting that passes the gate avoids it (table below).

## Corridors: cause and fix
Traced game: corridors, v4 as A vs starter-cpp. v4 lost 138-240 gems with 93 deaths. No enemy champion was within 4 tiles at most of them. Most deaths were on row y=16, 3 tiles from the shared camp row (y 19-20). Decoded with tools/top10/extract_pilot.mjs, plus the bot's own decision trace (P_ROWS_DBG).

**Cause 1: unseen camps and a recall loop.** Bush rows at y=17 and y=22 block sight. Monsters see through bushes (GameRules l.164), so at y=16 the camps 3 tiles away hit the champion for 120-240 a round while it sees none: the trace shows `nc0` and `inc0` for the visible camps and modelled incoming damage. The retreat branch then reads "no incoming damage, no enemy" and recalls (bots/v4/src/a/bot.cpp:611 `else if (!enemies_near) { me.recall(); }`). The hit cancels the recall, and the next turn recalls again until the champion dies (Vendetta: 440 → 260 → 140 → 20 → dead without moving, rounds 27-31).
- **Fix FIX_UNSEEN_HIT, `P_UNSEEN_HIT` 1** (util.h:23-31; bot.cpp:567-572, 610; bot.h:82-83). When health drops away from spawn with no enemy champion in view and zero modelled camp damage, the champion counts as under unseen fire for `P_UNSEEN_MEMORY` 3 rounds. In that window a retreat walks home instead of recalling.

**Cause 2: stand tiles out of sight.** The first fix stopped the deaths, but v4 as B then scored 0 gems. Its squad shuttled between y=22, inside the bush where the camps are visible, and y=23, its ranged stand tile at distance 3, where the prey is out of sight. So it never attacked (micro.cpp farm_squad ranged goals).
- **Fix FIX_STAND_SIGHT, `P_STAND_SIGHT` 1** (util.h:33-37; micro.cpp:195-202). A ranged champion's stand tiles are kept only if they see the prey (SDK `visible_tiles_from`, checked only when there are at most 12 candidates). With none left, the existing fallback (any tile within attack range) applies.

Corridors vs starter-cpp: 0/2 with 54 deaths a game → 2/2 with 7.5 deaths a game (404-116 and 254-58 gems). In the final head-to-head, corridors is 5/6 vs v3b-e1. An earlier attempt (do not wait inside an awake camp's API range when no stand tile is reachable) changed nothing because the camps were not in view; it was removed.

## Ablations (one switch changed from the delivered setting; head-to-head vs v3b-e1, 126 games each)
| variant | h2h wins / 126 | win % | kept maps lost 5-6 of 6 | deaths/game | gem margin/game |
|---|---|---|---|---|---|
| base (delivered: P_DEATH_LOOP 3, P_CAMP_RANGE 2, P_FIX_MONO_RING 1, P_MONO_ZONE_ALWAYS 1, corridors fix on) | 85/126 | 67.5 | corners, expanse, lanes | 18.5 | +172 |
| death0 | 75/126 | 59.5 | lanes | 23.8 | +96 |
| death1 | 85/126 | 67.5 | none | 21.6 | +124 |
| range1 | 73/126 | 57.9 | elkring, expanse, monolith | 18.9 | +119 |
| ring0 | 82/126 | 65.1 | corners, expanse, lanes | 20.3 | +169 |
| zone0 | 80/126 | 63.5 | corners, expanse, lanes | 20.7 | +179 |
| corr0 | 72/126 | 57.1 | corners, expanse, lanes | 24.8 | +103 |


"corr0" turns off both corridors switches. Gate check of the only variant with no kept map lost 5-6 of 6: P_DEATH_LOOP 1 **FAILS** (40/42; lost broken_curve/B and gold_rush/A; `results/gate-v4-death1-20261020-081710Z-linux.json`).

## Chosen setting
Rule: pass the gate; then the highest head-to-head win rate with no kept map lost 5-6 of 6; then the fewest deaths per game. No passing tested setting meets the kept-map condition. The delivered setting passes the gate and has the highest head-to-head rate (67.5 %, tied only with death1, which fails the gate). It also has the fewest deaths (18.5 a game). **Chosen: delivered setting**, now in bots/v4: P_DEATH_LOOP 3, P_CAMP_RANGE 2, P_FIX_MONO_RING 1, P_MONO_ZONE_ALWAYS 1, P_UNSEEN_HIT 1, P_STAND_SIGHT 1, plus P_FIGHT_CACHE 1 (below).

## Fuel
Profiled with `fuel_remaining()` around `farm()`. The peak was in `farm_solo` → `fight_any`: `cast_on_camp` recomputed every ability's `valid_tiles()` for each camp within 4 tiles. **FIX_FIGHT_CACHE, `P_FIGHT_CACHE` 1** (micro.cpp:33-75; `enemy_champion_near` exported, util.cpp:95, util.h) queries them once per call. Decisions are unchanged: the gate after the change matched the gate before it game for game on win, round, gems, opponent gems and deaths (0 differences in 42 games).
- `farm()` peak: 76,492 → 47,858 (concentrated_wealth B; lanes, islands, gold_rush, elk_arena and big_world all ≤ 46k).
- Gate turn maximum: 95,321 → 90,697. The remaining peaks (lanes B 90.7k, islands B 90.3k) are outside `farm()`. They are under the 100,000 gate limit but above the 80,000 guideline; not addressed.

## Final head-to-head per map (v4 vs v3b-e1; 6 games a map)
| map | wins/6 | deaths/game | gem margin/game |
|---|---|---|---|
| ambush (kept) | 4/6 | 10.8 | +52 |
| big_world | 5/6 | 6.8 | +202 |
| boss_battle | 6/6 | 4.3 | +0 |
| broken_curve | 6/6 | 20.3 | +290 |
| collision (kept) | 2/6 | 18.2 | +44 |
| concentrated_wealth | 2/6 | 34.2 | +130 |
| contest (kept) | 6/6 | 9.3 | +67 |
| corners (kept) | 0/6 | 18.5 | -206 |
| corridors | 5/6 | 9.7 | +186 |
| eight (kept) | 4/6 | 28.2 | +197 |
| elk_arena | 5/6 | 38.3 | +84 |
| elkring (kept) | 5/6 | 9.2 | +147 |
| expanse (kept) | 0/6 | 4.0 | -94 |
| fracture | 6/6 | 23.8 | +155 |
| fragment (kept) | 5/6 | 41.7 | +250 |
| gold_rush | 6/6 | 27.8 | +567 |
| hooks | 5/6 | 36.0 | +535 |
| interlock (kept) | 4/6 | 16.5 | +219 |
| islands | 6/6 | 2.7 | +1010 |
| lanes (kept) | 1/6 | 13.2 | -112 |
| monolith (kept) | 3/6 | 12.3 | +66 |
| **total** | **86/126 (68.3 %)** | **18.4** | **+180** |

0 faults, 0 errors.

## Still failing / open
- Kept maps lost to v3b-e1: corners 0/6 (gems -206 a game), expanse 0/6 (-94), lanes 1/6 (-112). collision 2/6 and concentrated_wealth 2/6 are weak. None of the ablated switches fixes them: corners, expanse and lanes are lost under every setting except death1, which fails the gate. The cause is untraced; likely candidates are the new exact map ids and plans on the changed layouts (v4-base-release.md fix 1).
- Max turn fuel 90.7k outside `farm()` (lanes, islands).
- Not run: /watch of a loss (no browser here), panel21.
