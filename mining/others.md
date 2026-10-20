# Ideas mined from public team repos (2026-10-18)

Root: /home/claude/cambc-2026/top100-intel/data/repos/. Ranks come from leaderboard/ladder-20261016T145705Z.json (members field): ELTheo77 = "Princess Donut the Queen Anne Chonk" #182 (1,542); DE41H = BITSP #320 (1,457). Probable by display name: jamesrdainton = "Opus 5.5" #238 (James Dainton); RealAhmedKhairi = "Midnight Wardens" #343 (XKhairi); srikar2429 = "Narla's Gang" #373. Other owners unmatched. ELTheo77's docs quote top-team data (Mite, Univardisty, hello, KratosMessi), which is the most useful content here.

Starter template only (no ideas): SzyZuu (gorlock = starter-cpp byte-identical), srikar2429, thomasyzy7, rafancoding, PreslavGG, riic10 (bots identical to starter-python; scripts/arena.py is a match runner), Sammy-The-Fish and Not-TNB (Rust starter plus a stub draft), DeySag (194-line starter rewrite, no tactics).

## Top 5

### 1. Altar lock on enemy tributes (tribute denial)
- Mechanism: a lock placed within 2 tiles of an altar cancels a channelling enemy tribute that turn; a tribute attempt on a locked altar becomes a basic attack on the lock; 3 hits clear it and the tribute restarts from zero (engine-tested). Costs 10 gold, bought on spawn or while dead.
- Evidence: ELTheo77 docs/ENGINE_TESTS.md:105-110; docs/ROADMAP.md:154 (Univardisty, top 3, places 2.6 locks a game, 4.1 in top-10 games; IETHal 1.4); ROADMAP.md:167 (one lock stopped every v2 tribute on collision).
- Code: ELTheo77 bots/main-4/src/combat.cpp:803-817 (hunt_lock: place when an enemy tributes or a carrier of >= 20 gems is within 6), economy.cpp:57-59 (buy on spawn); DE41H bots/main-bot/src/bot.py:275 (lock any altar where a visible enemy `is_submitting_gems`); dpark2380 bots/v3/src/bot.cpp:3,176-179; jamesrdainton bots/mybot/src/bot.py:65-68,395-400 (drop on any unlocked altar passed).
- Counter-side: break enemy locks on our altar (ELTheo77 economy.cpp:174-180; dpark2380 bot.cpp:45 prefers a free altar within 6 extra tiles).
- Maps: all; most value where altars are few and shared (collision, elkring, rows: 2 altars each).
- How to add: per-map tunable `lock_enable`, `lock_gold_floor`, `lock_trigger_gems`; buy at spawn when gold >= floor; each turn, if holding a lock and a visible enemy tributes within 2 of a placeable altar, place it before other actions. Plan override can force it on 2-altar maps.

### 2. Capture reset by stepping on the group's spawn tiles
- Mechanism: any enemy body on any spawn tile of a group cancels the capture at once and the 5-round timer restarts; with both teams on the region nobody captures.
- Evidence: ELTheo77 docs/ENGINE_TESTS.md:95-103 (interlock capture delayed past round 60).
- Code: Swapnanil-06 bots/sparring-denial/src/policy.py:1-10,69-120 (invaders Varg and Tavi score enemy-side groups x3, own side x0.35; when a visible enemy stands on a region with reward pending, step onto a free region tile, cast, strike, ward toward it); DE41H bot.py:21-23 (hold a contested capture 4 turns, then back off 30).
- Maps: maps with central or mirrored groups within reach of both spawns.
- How to add: target-choice hook: when a visible enemy is within 1 of a group region whose reward is available and a free region tile is within N steps, give that tile a contest score (reward gems x P(hold)). Per-map tunable for N and for the invader roster.

### 3. Carrier and channel hunting with an expected-value rule
- Mechanism: a kill takes every carried gem plus 50 gold and removes the victim for 60 rounds; recall is cancelled by any damage or stun, tribute by stun, death or a new lock. ELTheo77 S3 targets tributing enemies (40 + bag), recalling (15 + bag), carriers >= 30 (bag), divided by steps, stun first, only at >= 50 per cent health, not outnumbered by 2+, fight score >= -0.3.
- Evidence: STRATEGIES.md §5.4 (EV = P(kill) x (bag + 60 r_them + 50 gold) - P(loss) x (our bag + 60 r_us) - T r_us; pays from P(kill) about 0.1 on a 40-gem carrier); TOP_TEAMS.md §3 (winners kill 9.3 a game, losers 4.6; 40 per cent of their deaths carried gems).
- Code: ELTheo77 bots/main-4/src/combat.cpp:772 (handle_hunt), bot.cpp:72; Swapnanil bots/sparring-hunter/src/policy.py:1-30 (leash 11 toward visible carriers, pursue last-seen tile 6 rounds within 14); DE41H bot.py:240-273 (engage carriers of >= 20 gems when own health >= 1.2x theirs, else kill shots only).
- Not proven: S3 was never platform-tested before main-4 was retired.
- How to add: one scored candidate in target choice, value from the EV formula, with per-map tunables for leash, minimum bag and health floor.

### 4. Turn-order control with set_priority
- Mechanism: within a team, champions act in descending priority this round (ties by entity id); teams alternate, A first in odd rounds. Stunners first means a stun lands before the target's turn.
- Evidence: ELTheo77 docs/ENGINE_TESTS.md:47-52.
- Code: ELTheo77 bots/main-4/src/economy.cpp:94-97 (B6: enemy within 6 -> stunners 130, melee 100, casters 90); RealAhmedKhairi bots/my-bot/src/bot.cpp:114-130 (carriers 100 so tributes start first, Willow 90, then a fixed order).
- How to add: per-turn priority set at end of turn for the next round; per-map table of role priorities. Cheap (one call), no draft change.

### 5. Teleport crystal to a sealed altar (map-specific)
- Mechanism: an altar no walk reaches (collision's east altar is behind a wall) can be reached with a teleport crystal (10 gold, 3-turn channel, range 3 through terrain); bank there, then recall out. Enemies cannot walk to contest it.
- Evidence: ELTheo77 STRATEGIES.md §5.7 B8 (lost all 3 collision games to the top four, margins -0.50 to -0.89); ROADMAP.md:154 (no top team used a crystal).
- Code: ELTheo77 bots/main-4/src/economy.cpp:62-64 (buy), 139-143 (sealed altars marked at init), 861-895 (sealed_bank: trip cost = walk to launch + 3 + walk in pocket, taken when within 6 of the walkable altar, +15 penalty on the walkable one if enemies seen near it in 30 rounds).
- Maps: collision (41x41, vertical symmetry, 2 altars on the axis). SPECULATION: which of our map files is collision is unchecked.
- How to add: per-map plan flag `sealed_altar_bank` with launch tiles precomputed in draft memory.

## Other ideas

### Counter-draft from revealed enemy picks
- Marcella (stun immune) replaces Ren at pick 5 when the enemy holds 2+ stunners (Cassi&Issac, Tavi, Ren, Prism, Ryth). ELTheo77 bots/main-4/src/draft.cpp:199; STRATEGIES.md §5.5. Untested on platform. Add as a draft rule keyed on `draft::current_draft(enemy)`.

### Map fingerprint drafts
- Swapnanil bots/topteam/src/draft.py:19-26 identifies maps by (width, height, group counts) and picks a roster per profile (corners/packs: Varg, C&I, Stellar, Ryth, Ren; lanes: Vendetta, C&I, Varg, Ryth, Tavi; else Vendetta, Tavi, Marcella, Varg, Ryth). 8/12 in-sample against their own main. Our per-map plan already does this.

### Map-feature-scored draft with composition bonuses
- dpark2380 bots/v3/src/draft.cpp:71-128: signals (elk share, boss, splash, aoe, contested share); bonuses for first tank, penalty for 2+ tanks, +0.3 x boss for stunners while fewer than 3 ("chain stuns to keep a monolith quiet"). A fallback drafter for unseen maps.

### Aggro by draft order; casters hit from range 3 behind the tank
- An awake camp hits the nearest champion, ties to the lowest handle (draft order), re-evaluated each round; others at equal or greater distance deal free damage. Range-4 spells hit camps with zero damage taken. ELTheo77 docs/PLAYBOOK.md §1; Mite's casters make 72-86 per cent of camp hits as basic attacks from range 3 (TOP_TEAMS.md §4). Add: draft the tank first; casters stand at range 3 when a melee is nearer the camp.

### Recall as transport and map-scaled bag size
- Mite recalls 26 times a game, 183 of 911 recalls start at >= 90 per cent health, median 17.5 tiles from spawn. Mite's median bag by map: collision 22, interlock 22, lanes 24, corners 36, elkring 36, packs 44, expanse 48, rows 132. ELTheo77 TOP_TEAMS.md §4. Add: per-map `bank_threshold` and a recall-home rule when the next target is nearer spawn than the current tile.

### Hold ground in even fights
- Top-10 teams retreat 5-35 per cent of champion-rounds at even numbers above 60 per cent health (Mite 9, IETHal 5); ELTheo77's v3 38 per cent; outnumbered Mite retreats 1 per cent and attacks 52 per cent. main-4's veto-based fight rules lost 0-5. ROADMAP.md §1.10. Tunable: retreat thresholds per map.

### Kill order by damage per health (Smith's rule)
- Focus the enemy with the largest d_i/h_i; a Cassi&Issac kill removes both, so the pair scores (d_C + d_I)/h_weaker; add Willow's heal to her d. Stellar (0.103) before Varg (0.045). ELTheo77 STRATEGIES.md §5.6 C1. Add to target selection.

### Stun chain through shared memory
- Every stun goes to the team focus (shared alert target) when in range and not already stunned this round, so stuns land on successive turns. ELTheo77 combat.cpp:351 (B3). Evidence against: stun chains are 4-7 per cent of top-team kills (ROADMAP.md:116).

### Shared-memory danger marker
- Bytes 2-4 hold x, y and round where the team found itself the weaker side; both squads avoid camp groups within 6 of it for 20 rounds. dpark2380 bots/v3/src/bot.cpp:5,20-26. Add as a 3-byte field in our layout if free.

### Wards
- Wards are permanent, one per champion, placement Euclidean 2 from the current tile, and they block tiles: never ward on capture tiles, altar neighbours or own spawn (a ward on a capture tile stalled a reward 60 rounds). ELTheo77 STRATEGIES.md §5.1 item 7 and 11; combat.cpp:692-766. B7 puts the ward in a bush on the enemy side (combat.cpp:720-728). Swapnanil's altar camper keeps its ward on the guarded altar (sparring-altar/src/policy.py:97-100).

### Altar camper
- Ryth and Tavi each guard the altar the enemy most likely uses (nearest to enemy-side groups weighted by gem value, enemy reach >= 6), post on a bush tile within 2, farm groups within 7, lock when an enemy carrier comes within 7, stun or kill tributing enemies within a leash of 6; starts at round 60. Swapnanil bots/sparring-altar/src/policy.py:1-12,49-84,122-168. A sparring archetype; useful as a local opponent.

### Crystal escape
- RealAhmedKhairi bots/my-bot/src/bot.cpp:381-395,430 buys a crystal and teleports 3 tiles toward spawn when critical and hunted. SPECULATION: the 3-turn channel is likely interrupted by the damage that triggers it; ELTheo77 notes no top team used crystals.

### Draft-memory route tables
- S0ulSyte bots/starter-cpp/src/draft_wip.cpp:17-65: the draft-memory write must happen before that turn's pick (the game rejects a write after the pick); packs routes from spawn to camps and altars (kind byte, coordinates) into the 1,024 bytes. Analysis cost 537,922 fuel on the 10x first draft turn.

### Monolith
- In 36 top-team games on monolith maps no team hit a monolith camp. Whole-team spells deal about 100-125 a round, so the monolith group takes about 190-240 team rounds for 392 gems and 300 gold, near farming pace. ELTheo77 STRATEGIES.md §5.7 B5 (expected to lose). Low priority.

### Testing technique: salted trajectories
- The engine is deterministic and trajectories are chaotic; a nonzero `salt` nudges near-tie target scores by up to 0.5 per cent to sample several equally good paths per map (5 trajectories x 2 sides). ELTheo77 economy.cpp:415. Lesson recorded in ROADMAP.md §2: local wins against own bots and passive-bot pace both failed to predict platform results.

### Engine and fuel facts worth checking against engine-truth.md
- Allies in bushes are invisible to allies; stepping onto one throws "known occupied destination" (212-234 times over 140 games); a refused tile should stay blocked 3 rounds. `champions(team::enemy)` costs about 46k when enemies are hidden; read enemy handles once and test `is_alive`. Exceptions cost about 1.9k-4k each. A ranged champion with a walking-compatible reset (Tavi, Vesper, Willow) can walk, reset and attack in one turn. ELTheo77 STRATEGIES.md §5.1 item 11; PLAYBOOK.md §1; ROADMAP.md §1.10.
