# Engine 1.0.0 (20 Oct): differences from engine 0.0.3 (SDK 2.2.5)

Sources: SDK diff (`archive/sdk-0.0.3/sdk/{cpp,rust}` vs `sdk/`), plus the engine's own design tables read from the cached engine WASMs (v0_0_3 vs v1_0_0) with `tools/maps21/designs.mjs` -> `data/designs-0.0.3.json`, `data/designs-1.0.0.json` (`node tools/maps21/showdesigns.js` prints them). The site doc (`GameRules.md`, 20 Oct) LAGS the engine: it still has every OLD number below and no Heart of Ruin or golem. Tag: [SDK] = in SDK headers/constants; [ENG] = read from the engine design table only (not stated in the SDK text or the site doc); [SITE] = site doc already agrees.

## 1. Constants (SDK, C++ and Rust identical)

| Constant | 0.0.3 | 1.0.0 | Site doc |
|---|---|---|---|
| prism_focus_per_gem | 0.2 | 1.0 | still 0.2 |
| vesper_vale_bonus_gold_percent | 20 | 33 | still 20 (Treasure emblem 20 + 20 = 40 in doc) |
| marcella_tribute_gold_per_gem (new) | - | 2 | absent |
| heart_of_ruin_missing_health_damage_max_bonus (new) | - | 0.5 | absent |

Gem cluster and gold cluster comments: 500 -> 120 health [SDK, ENG confirms]. All other constants (item costs, respawn, tribute, etc.) are unchanged.

## 2. Base stats rebalance [ENG only; the SDK text does not mention it, site doc has the old values]

Format base (growth per shard). Only changed fields are listed; unchanged champions/fields omitted.

| Champion | Change 0.0.3 -> 1.0.0 |
|---|---|
| Ryth | Strength 50 -> 45 |
| Vesper Vale | Strength 45 -> 50 |
| Vendetta | Strength growth 5 -> 6 |
| Cassi | Health 700 -> 600 (growth 71.25 -> 65); Strength growth 3.5 -> 3; Focus growth 15 -> 13 |
| Issac | Health growth 95 -> 80; Strength growth 4.75 -> 3 |
| Stellar | Strength 35 -> 30; Focus 100 -> 85 (growth 10 -> 9) |
| Prism | Health growth 75 -> 60; Strength growth 3.5 -> 2.5; Focus growth 10 -> 8 |
| Marcella | Focus 50 -> 60 (growth 10 -> 15) |
| Willow | Strength 45 -> 55; Focus 105 -> 120 |
| Ren | Strength 40 -> 35; Focus growth 10 -> 9 |
| Varg | Strength 50 -> 40 |
| Tavi | Strength 35 -> 45; Slot 1 stun modifier 2.2 -> 1.4 x Focus |

Ranged/melee flags, vision 8, move 1, camp damage multiplier 1.0 and all other cooldowns (8 for every ability): unchanged.

## 3. Heart of Ruin (champion_draft id 11, champion_type id 12) [SDK enums; kit numbers ENG; passive max SDK]

Melee (range 1, move 1, vision 8). Health 1000 (+100 per shard), Strength 40 (+5), Focus 100 (+15), camp damage multiplier 1.0.
- Passive `missing_health_damage` (passive id 10): up to +50% to ALL damage, linear with missing health (0% at full health, 50% at 0). Counted in `ability damage` queries (outgoing bonus). The legacy id 9 (omnivamp) stays reserved.
- Slot 0 (shield, self): range 4 field (self-target), modifier 1.5 x Focus, 5 turns, cooldown 8 (the visualiser tooltip reuses Varg's shield text; 150 at Focus 100, versus Varg's 2.5 x Focus).
- Slot 1 (dash with landing effect): range 2 (diagonal counts one, may cross holes along a legal dash path), landing radius 1 (Chebyshev, all 8 neighbours), damage 1.75 x Focus (before bonuses and mitigation), stun 1 turn, cooldown 8. Landing effects hit enemies and camps only after a successful dash.
- Basic attack: melee, normal. No attack reset.
The 0.0.3 engine table returned an unreleased placeholder for id 12 (aoe 0.9 x Focus, stun 1.0 x Focus, passive id 9); it was not selectable.

## 4. Dash structure change [SDK]

`ability_dash` gains `radius`, `modifier`, `duration` (was range only). Existing dashes (Vesper, Vendetta, Issac, Ren) have radius 0, modifier 0, duration 0 in the engine table: no landing damage or stun. Only Heart of Ruin uses the landing effect. `champion::ability_damage` now includes dash landing damage. Implication for the duel lab: a dash is a gap-closer plus a 1.75 x Focus area stun for HoR only.

## 5. Golem camp (camp_type id 5) [SDK enum; stats ENG]

5,000 health, strength 120, attack range 4, vision 8, move 1, no growth. Rewards (gold, gems, respawn) are not exposed in the SDK; they are not in the site doc. No bundled map contains a golem (all 21 maps scanned), so golems are not on the ladder pool today. `camp::is_golem()` added.

## 6. Other camp changes [SDK + ENG]

| Camp | 0.0.3 | 1.0.0 |
|---|---|---|
| Gem cluster | 500 HP | 120 HP |
| Gold cluster | 500 HP | 120 HP |
| Elk | range 3 | range 4 (HP 2400, strength 60 same) |
| Monolith | range 3 | range 5 (20,000 HP, strength 200 same) |
| Wolf | 1350 HP, strength 20, range 3 | unchanged |

Gem and gold clusters die in about a quarter of the old time. `notes/map-economics.md` per-group HP and the "gems per 1,000 HP" columns use the old 500.

Per-camp rewards [ENG: read from the group data in the official map files, `data/maps21.json`; the site table is stale]:

| Camp | Gems | Gold | Respawn | Site doc says |
|---|---|---|---|---|
| Wolf | 4 | 40 | 40 | 22 gems, 90 gold |
| Elk | 15 | 90 | 60 | 36 gems, 160 gold |
| Gem cluster | 7 | 0 | 50 | 24 gems |
| Gold cluster | 0 | 20 | 50 | 35 gold |
| Monolith | 200 | 300 | 150 | same |

Some groups on the maps carry a x2 reward multiplier (group totals in `notes/map-economics.md`). Verify with a replay (tribute or capture event) before relying on single figures.

## 7. API additions [SDK]

- `awake_camp_tiles()`, `asleep_camp_tiles()` (+ `_into(bitboard&)` forms): bitboards of visible awake/asleep camps; dead camps excluded.
- `in_vision(vec2)`: current self-vision (including ward vision) whether or not the tile is occupied; out-of-bounds throws `api_bad_position`.
- `ward::owner()`: owning champion captured while the ward was visible.
- Pathing helper (`move toward` / bitboard path code): the target is now always treated as free for pathfinding (line ~5202: `blocked.set(target,false)` replaces `if blocked.is_set(target) return false`). Entering a known blocked target now throws instead of returning false. Audit any code calling the SDK path-move on a possibly-blocked tile.
- Doc-only: `attributes`/dash metadata comments clarified; `champion_designs` array is 13 long (index = champion_type).

## 8. Enums [SDK]

- `champion_draft`: heart_of_ruin = 11 (12 champions; Cassi and Issac share one draft slot 3). `champion_type`: heart_of_ruin = 12 (13 bodies).
- `camp_type`: golem = 5 (map size 6).
- `passive_type`: missing_health_damage = 10; stun_immunity now also "gains gold per gem tributed" (Marcella).

## 9. Marcella and Vesper behaviour [SDK]

Marcella: stun immunity plus 2 gold per gem tributed (new). Vesper: 33% bonus gold (was 20%).

## 10. Not interpretable from the SDK

Golem rewards, respawn and behaviour (aggro/leash); whether the 1.0 engine changed round limits, tribute or combat formulas beyond the constants (the diff shows none); items and consumables (the SDK item table text is unchanged, but a numeric diff of item stats was not run).
