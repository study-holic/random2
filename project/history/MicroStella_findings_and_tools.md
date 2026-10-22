# MicroStella — Bot Findings and Tools (18–22 Oct 2026)

Ladder submission: **v22**. Bundles: v20.zip, v21.zip, v22.zip. Work folder: `~/battlecode` (Mac) / `/home/claude/bc` (container).

## 1. Version lineage

| Version | Change | Result |
|---|---|---|
| v11 | William's candidate-v418b | ~456th |
| v16 | Draft lists, Tavi field shop, Monolith poke module, altar locks, channel interrupts, contested-group rule | Sprint 1 entry |
| v19 | #1-team roster on swirl only | Collapsed when the map pool was replaced (19 Oct 21:51) |
| v20 | Gem-weighted target scoring (0.3/gem, capped at 12 tiles; gold-only groups +12 once gold ≥ 60) | 26–16 vs v19 on the new pool |
| v21 | Reachability filter, corridor-jam pathing, `long` overflow fix in pick_monolith, gate_mode (boss_battle), one squad on big 5-pick maps, bolder camp retreat | 117/168 |
| **v22** | No Cassi & Issac on 3-pick maps; stun staggering | **129/168** |
| v23 (not shipped) | Awake-camp path avoidance; escape-scaled retreat | 125/168 — rejected |

## 2. Bugs found and fixed
1. **Unreachable camps.** Groups needing a teleport crystal were walked at forever. islands: 38 of 46 groups unreachable on foot (8 walkable, 14 one-hop); big_world 4, fracture 2, ambush 2. Fix: BFS from spawn over walls|holes at init → `GroupInfo::reachable`. islands gems@600: 0–360 → 936–1104.
2. **Corridor jam.** `move_towards_closest` treated allies as walls → squads deadlocked in 1-wide corridors. Fix: second BFS through ally tiles.
3. **`1L << 40` is 0 in wasm32** (long is 32-bit). `pick_monolith` never picked a Monolith — the v12 poke play never ran on any map. Fixed with `long long`. fracture 0–2 → 2–0.
4. **boss_battle:** the only altar (25,20) is reachable solely from the Monolith tile (24,20). Every bot scored 0. `gate_mode`: full-time poking from distance 4, no banking/escorting while the gate is shut, bags capped at 60 (tribute = ceil(gems/10) turns; any stun cancels it). 0–0 → 2–0.
5. **One squad** on 5-pick maps with both sides ≥ 40 (not mono-only, not rows): the 2+3 split fed champions one at a time into Elk stacks.
6. **Camp retreat** buffer 1.5 hits + 5% (was 4 hits + 10%): a tank used to retreat after one hit from a 3-Elk stack. Tank rotation at 50%.
7. **3-pick draft** → Tavi, Stellar, Ryth. Cassi & Issac die twice per kill (fragment: 29 deaths). fragment/collision/broken_curve → 2–0 each.
8. **Stun staggering**: no stun on a target already stunned this round (`unit::buffs().has_stun()`).

## 3. Engine facts (verified with probe bots)
- Camps do **not** regenerate; damage persists across visits.
- Hitting one camp wakes the group, but each monster attacks only champions within its own range 3 and never chases; sleep after 2 damage-free rounds.
- Elk 2400 HP / 60 dmg; Monolith 20,000 / 200; wolf 1350 / 20; clusters 500 / 0.
- Tribute = ceil(gems/10) turns; damage does not interrupt, stun/death/altar lock do.
- Tiebreak at round 2000: gems → **gold earned** → kills; side B wins full ties. Side A acts first each round (symmetric centre brawls favour A).
- Every champion exhausts fuel on turns 1–2 (init BFS + group tables); only those turns are lost.
- Earlier (lab bot): capture 5, sleep 2, respawn 60, recall 5, submit range 1, win 1000, kill gold 50, passive gold 0, spawn heals fully every round, wards cost no action, lock hits 3, TP channel 3, consumables 10g, TP onto wall/hole forbidden.

## 4. Map facts (new 21-map pool)
- islands: 8 walkable groups (624 gems), 14 one-hop (392), 24 need two hops; altars 4 walkable + 4 one-hop.
- big_world: 100×100, 6 altars; 4 free gem-cluster groups (192) need one hop.
- elk_arena: one gem group (216) at centre, 28 gold-only groups — holding the centre decides it.
- corridors: camp row y=19–20; rows 16–18 inside every camp's reach; group 8 = four Elks in a 2×2 block. 23 of 30 deaths were to camps.
- boss_battle: wall across the map; gap at x=22–25 holds two Monoliths (400 gems) and the altar; ~760 gems per side.
- monolith (00): 392-gem Monolith corners, Elks present → not the rows map.
- 3 picks: fragment, gold_rush, collision, broken_curve. 4 picks: hooks, interlock.

## 5. Tested and rejected (168-game bench unless noted)
- Six-body (Cassi & Issac) roster on every 5-pick map: 49/84 vs 53/84.
- Monster shards ×10 before Power: 6–10.
- Retreat-twice-then-shelve; gather-before-engage at Elk groups: worse.
- `P_THREAT_LAYER 1`: worse (again). Ranged kite, engage-HP floor, focus rule 0, chase 0/4, flee 1/3, save-CC: 85–118, none above baseline 117.
- Conditional (one-squad-only) bold retreat: 101. Ranged poke-from-4: 99. Awake-camp avoidance / escape-scaled retreat: 122–125 vs v22's 129.
- P_ROWS_CMD 0 on the monolith map: identical (not a rows map).

## 6. What beat us on the ladder
- Blame the Altar 4–1: fracture (Elk-stack bounce, fixed), hooks (Cassi & Issac fed), corridors (brawl), elkring.
- DIET CODE 5–0 unranked: pure pace (big_world 1004 vs 488 at r561, zero deaths); boss_battle by r1927. Roster Cassi & Issac, Stellar, Ren, Ryth, Varg.

## 7. Tools
| Tool | Use |
|---|---|
| `pace.py <bot> [opp] [maps...]` | Both-side runs; gems at round 300/600, first tribute, deaths, winner. Caches per bot name — delete `replays/pace_<bot>_vs_<opp>` after changing a bot. |
| `evalall.sh <bot>` | 168-game benchmark: 21 maps × 2 sides × v19/v20/v16/v13. The only number to trust; 16-game samples lied twice. |
| `addtrace.py <bot>` + `trace.sh <A> <B> <map> <out>` | Inject a per-turn state trace (`T r<round> a<idx> ... tg hold bank retr hp bag`) and extract it from the replay bundle. |
| `summarize.py replays/<dir>` | Results table from `--statistics` JSONs. |
| `bots/probe` | Dumps map dims, altars, spawns, groups with walking distance, ASCII map (8 rows per turn). |
| `bots/elkprobe` | Monster wake/regen test. `bots/lab`: engine constants. |
| `reach.py <probe stdout>` | Walk vs one-teleport reachability of groups and altars. |
| Replay viewer | `cambc view <replay> --port 7026` + Playwright (`/opt/pw-browsers/chromium`, swiftshader); round slider is `input[type=range]`; ~2.5 rounds/s load. |
| Match data | `cambc matches summary <id> --json -o f.json`, `cambc matches show <id> --json` (needs login; run on the Mac). |

## 8. Next steps
1. Work from match-group zips of real v22 losses, not self-play.
2. corridors (0–2): plan for the 2×2 four-Elk block; wait for monsters to sleep before transiting the camp row.
3. hooks / gold_rush as side B (first-mover brawls).
4. Teleport crystals on islands (+392 gems) and big_world (+192).
5. monolith map: ~70 gems behind; the 392-gem corners are now pokeable.
6. Draft-turn precompute into draft memory (turn-1 fuel).
