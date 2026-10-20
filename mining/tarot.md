# Tarot Club (FHL-08) mining: foundry-v16 and team notes

Root: `/home/claude/cambc-2026/top100-intel/data/repos/FHL-08__Tarot-Club-Cambc2026`. `V16` = `bots/foundry-v16/src`. `bots/foundry/src` (main) is newer than v16 with no VERSION.txt or eval numbers (marked WIP below).
Their evaluation: `tools/replay/local_eval.sh`, 18 games per comparison (9 maps x 2 sides). v16 VERSION.txt: 12-6 vs platform-v17 (baseline 7-11), 10-8 vs foundry-v15 (baseline 7-11), gems lost on death roughly halved. v15: 13-5 vs foundry. v14: 12/18 vs Ming v12, 10/18 vs v11.
foundry-v16 contains no map fingerprint and no per-map branch (grep for width/height checks finds geometry only). Every map runs the same code; map speed comes from the generic mechanisms below.

## Top 5 (most likely to win games for us)

**1. Grouped opening chain, then wedge sectors.** For rounds 1 to 90 the whole team walks one shared chain of gem groups (elks and monoliths excluded), each stop picked greedily by gems / (hard-terrain travel from the previous stop + group hp / team basic dps). Each body derives the same chain from static data, so no comms are needed. On a reward tile only the ally nearest the free tiles stands; the others move on to the next open stop. When the chain ends (or round 90), each unit owns an angular wedge around our spawn centroid (wedge 0 faces the enemy) for 150 rounds; out-of-wedge gem camps pay +6 cost.
Evidence: `V16/exec.cpp:343-428` (opening_stop), `:1530-1601` (chain execution, stander rule), `:1604-1609` (sector window), `:477-520` (wedges), `:1446` (+6). Round-1 fast path walks straight to stop 0 inside the 2x budget: `:1056-1085`. No isolated test numbers in the repo.
Maps: all; largest effect where a few gem groups near spawn are too strong for one body (collision, wolf-heavy).
Port: per-map tunables `opening_rounds` (90), `sector_rounds` (150), `wedge_penalty` (6), plus an opening override list of group ids for maps where the greedy chain is wrong. Implement the chain as a pure function of map data and draft record.

**2. Small, growing bag cap with a death ceiling.** Bank when bag >= clamp(10 + round/60 - min(10, (enemy_banked - our_banked)/25), 10, 40). Separate hard rules: bank at 45 gems regardless; bank when channel ticks + walk would overrun round 2,000; a winning bag skips every tactic and banks. The plan adds per-role divert caps (FARMER 40, CARRY 48, ESCORT 40, FLEX 32). A bag of 10 or fewer is a 1-tick channel.
Evidence: `V16/econ.cpp:98-106`, `:242-264`; `V16/planner.cpp:178`; `V16/exec.cpp:1101-1110`, `:1271-1283`. WIP main lowers the ceiling to 25 (`bots/foundry/src/econ.cpp:102-114`, issue #108: kills turn a 40 bag into a double swing). Measured top-team cap from replays: 44 (`bots/clones/*/src/clone_config.h:25`).
Maps: all.
Port: per-map `cap_base`, `cap_per_60_rounds`, `cap_max`, `behind_tighten`, `pinata_cap`; a plan can raise the cap on safe maps.

**3. Heal-to-need pitstop and recall discipline.** Retreat below 0.55 max hp, or below 0.75 with an enemy champion within 3. At spawn, leave once hp >= min(0.9 max, 0.55 max + remaining camp dps x turns-to-clear x 1.25), priced on the camp just fled (live hp/dps snapshot). Recall only when no damaging camp is within its range and no enemy within 5; else walk home; 3 cancelled recalls force walking. Retreat stays sticky until 0.75 off spawn. Three contested failed sorties on one group (no camp death and no real damage dealt) blacklist it for 200 rounds.
Evidence: `V16/exec.cpp:62`, `:1117-1262`, `:1221-1240`; `farm_damage` flag `:166`. Part of the v16 package above.
Maps: wolf-heavy maps (collision, rows), where sorties repeat.
Port: tunables `retreat_floor`, `threat_floor`, `heal_margin` (1.25), `recall_clear_radius` (5); per-map override of the heal formula.

**4. Target choice: reward first, unfit gate, sortie pricing, hysteresis.** Candidates are route stops, the modulo partition (g % allies == slot), and unowned gem camps within 6 tiles with no ally within 2. A camp in reward_available state outranks any alive camp at any distance (crossfire kills give free captures); contested or ally-covered rewards are skipped. A camp whose dps x (hp / my_dps) >= 2 x my max hp is skipped unless reward or no fit gem camp exists. Cost = Chebyshev distance + 60 x sorties (sorties = dps x clear turns / 0.65 max hp). Gem camps always beat gold camps. The active target holds unless a rival is a reward against an alive belief or 8 cheaper.
Evidence: `V16/exec.cpp:259-277` (mine), `:303-313` (unfit), `:1407-1457` (consider), `:1484-1517`, `:1655-1674`. Planner drops gold-first routes after rows produced 1,366 gold and 0 gems (`V16/planner.cpp:232-239`).
Maps: all; gold-rich maps (rows) most.
Port: target scorer with per-map `sortie_weight` (60), `hysteresis` (8), `opportunity_range` (6), `unfit_multiple` (2.0).

**5. Respawn wait vs remote work (v16 headline change).** With nothing actionable, scan all alive gem camps map-wide. Take the remote one unless an owned camp respawns within travel + 12 rounds and its local time (max(travel, wait) + clear) beats remote time. With no target, haul to the 1-tile ring around the soonest owned respawn (never onto its tiles, since occupied tiles delay respawn) and Wait; overdue beliefs (unseen 24+ rounds) get a re-inspection walk; finally step off own tiles.
Evidence: `V16/exec.cpp:86-90` (worth_waiting), `:1627-1652`, `:1709-1771`, `:69` (stale_rounds). VERSION.txt credits this package with 7-11 to 12-6 vs platform-v17.
Maps: maps with long respawn timers or sparse gem camps; plausible main driver on swirl (SPECULATION).
Port: per-map `wait_slack` (12), `stale_rounds` (24), and a flag to force remote work.

## Collision and swirl

- Collision (41x41, 3 picks): foundry's fixed order gives Cassi & Issac, Varg, Stellar = 4 bodies (`V16/draft.cpp:22-34`, picks ignore the enemy). C&I share one merged route so Issac (range 1) takes nearest-aggro while Cassi farms from range 3 (`V16/planner.cpp:290-310`). All four bodies run the opening chain together on wolf groups for up to 90 rounds, then split by wedge. Top teams on collision (measured, `bots/clones/*/src/clone_config.h:47-48`): hello C&I+Stellar+Ren; ifish Ren+Stellar+C&I; mite Varg+Ryth+C&I; shawn C&I+Stellar+Ren. Every one takes C&I. Check whether v4.14 does.
- Swirl: absent from the repo (no map file, no table entry). foundry plays it with the generic stack. SPECULATION: the gap comes from ideas 1, 4 and 5 (team clears early, reward-first, remote fallback) and from 4-body C&I drafts if swirl has 3 picks. Verify on a local swirl replay by comparing time-idle and time-at-spawn shares (tools/replay/timeuse.py).

## Draft and economy

- Pick order C&I, Varg, Stellar, Vendetta, Tavi, Ren, Ryth, ... truncated to pick count; plan compiled for the projected roster before pick() and only when fuel >= 350k / 240k remains (`V16/draft.cpp:22-34`, `:82-131`). Port: per-map draft override.
- Build queues keyed on champion type, power shard in the scaling stat first, open-ended tail (`V16/econ.cpp:25-73`; issue #34: top teams put 60 to 68 per cent of shards into Power). WIP main adds bulk tail buys (4 per visit, 20g reserve) and skips spending while carrying 10+ gems (`bots/foundry/src/econ.cpp:242-263`; issue #102: top 4 buy 100+ items per game).
- Measured itemisation per team (team totals per game, buy order): hello power_focus 2, power_str 1, durability_hp 4, power_hp 2, jungle_str 6, bruiser_hp 10; ifish power_str 2, power_focus 2, durability_hp 4, bruiser_hp 10, jungle_str 6, jungle_focus 7; mite power_str 1, bruiser_hp 10, jungle_str 6, power_focus 2, jungle_focus 6, durability_str 3, durability_focus 4; shawn power_str 4, power_hp 6, jungle_hp 17, treasure_str 11 (`bots/clones/*/src/clone_config.h:87-95`).
- Consumables: a body whose route passes within 3 of an enemy-side altar buys altar lock and teleport crystal on its first shop, ahead of shards (`V16/econ.cpp:130-213`).
- Planner: candidate camps must be soloable by some kit (dps x clear < 2 x hp), monoliths and unreachable camps dropped, EDD order (slowest clears first), gem camps assigned before gold, nearest-centroid LPT assignment, nearest-neighbour chain plus 2-opt, altar stops spliced at bag cap, team B rotates every route by one stop to break mirrors (`V16/planner.cpp:68-80`, `:194-366`).

## Micro and tactics

- Tactic dispatch order: peel > gank > lock > burst > deny > poke; skipped when carrying a winning bag (`V16/tactic/tactic.cpp:7-17`).
- Peel: rank enemy channels by remaining time x gems; use cc, primed Marcella attack, a basic hit on a recaller, or an altar lock on the tribute altar; unreachable channels go on the wire (`V16/tactic/peel.cpp:1-9`, `:110-155`).
- Gank: commit only when proven lethal inside the loot window (channel time left, or walk to altar + tribute ticks); target bag >= 20; never while carrying > 30; a runner counts only the in-range volley plus one ability (`V16/tactic/gank.cpp:1-50`, `:144-150`).
- Burst: spend ready abilities on enemy champions (issue #53: opponents land about 24 ability hits per game against our 1.3; champion damage 6.9k vs 12.7k); shield/heal below 70 per cent when threatened; attack-reset cast before a basic (`V16/tactic/burst.cpp:1-5`, `:178-224`).
- Deny: contest an enemy capture by standing on a free tile of the group; park on an enemy-claimed respawning group to delay respawn; budget 8 holds per group (`V16/tactic/deny.cpp:1-37`).
- Lock: insurance lock on our own tribute altar, denial lock on enemy-side altars within 2 (`V16/tactic/lock.cpp:1-23`).
- Capture stand: while capturing, hit adjacent groups only if they have no live attacker (damage wakes the whole pack) (`V16/exec.cpp:1140-1172`).
- Camp cast scoring: value = min(hp, damage) + one-tick stun credit on targets left alive; no kill credit (adding it regressed local_eval vs v14, no numbers); blast centres touching another group's spawn tiles are rejected (`V16/camp_scoring.h:23-85`, `V16/exec.cpp:699-724`).
- Escape: spend walking-compatible defensive spells while fleeing, stationary ones only after a failed step; basic-hit the lowest-hp attacker in range without chasing (`V16/exec.cpp:80-83`, `:883-957`).
- Banking target: sticky altar; open beats enemy-locked, path length beats geometry; switch from a locked altar only when the open one is less than 3 steps longer (lock costs 3 hits); occupied approach ring cools the altar 12 rounds (`V16/exec.cpp:93-106`, `:1284-1376`).
- Wards: placed after the turn's action, only when carrying gems, near the nearest live objective, bush tiles counted twice (`V16/warding.cpp:20-60`).
- WIP main (issue #103, untested): cast a ready cc before the basic on a committed target; prefer stunned targets for focus fire; bail when 2+ allies within 2 are stunned under threat; spread to tiles 3 apart around our camp when an enemy is within 5 (`bots/foundry/src/exec.cpp` diff vs v16, `disable_first`, `spread`).

## Other teams (measured from about 3,300 decoded games, issues #84 and #93)

Source: `bots/clones/{hello,ifish,mite,shawn}-clone/src/clone_config.h`. Shared measured traits: attack an enemy champion in range on sight, nearest first, and chase about 3 turns (p90 6); recall at 0.5 hp; tribute cap 44; wards with 60-round minimum gap.

| Team | Wait style | Respawn wait | Re-path | Wards/body |
|---|---|---|---|---|
| hello | park on ring | 8 rounds | on every objective change (4x rate) | 21 |
| ifish | never idle, drift to nearest objective | n/a | on invalidation | 20 |
| mite | park on ring | 20 rounds | on invalidation | 24 |
| shawn | never idle | n/a | on invalidation | 19 |

Per-map drafts (order as measured; elkring copies corners; fingerprint = width, height, group count):
- hello: C&I+Stellar first everywhere except corners (Ryth, Stellar, Prism, Ren, Tavi); expanse C&I, Stellar, Tavi; rows C&I, Tavi, Varg, Stellar.
- ifish: Ren, Stellar, C&I on collision, packs, interlock (+Ryth), corners and monolith (+Ryth, Varg); Vendetta, C&I, Varg on expanse, lanes (+Ryth, Ren), rows (+Ryth).
- mite: Varg first on 7 of 8 tables (collision Varg, Ryth, C&I); Ren, Stellar, C&I on expanse and packs.
- shawn: Varg, Ryth, Vendetta core on 7 of 8; Marcella on corners and monolith; collision C&I, Stellar, Ren.
Port: these tables give the draft overrides and wait styles of the archetype sparring bots; the four clones (foundry-v14 chassis) are runnable local opponents (VERSION.txt fidelity 0.85 for the hello policy skeleton).

## Tooling

- Their replay decoder drives the local `cambc view` server's WASM decoders headlessly at about 2 s per game (`.agents/skills/cambc-match/SKILL.md`, `tools/replay/extract.mjs`), vs our 1 to 5 minutes. `tools/replay/platform.py` pulls a team's replays by team id. Worth testing as a replacement for our extractor.
