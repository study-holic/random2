# FACTS (verified 20 Oct): one fact per line, source in brackets. No opinions.
Site doc = docs/rules/GameRules.md (20 Oct copy, lags engine). ENG = engine tables read with tools/maps21/designs.mjs (data/designs-1.0.0.json). Full diff: docs/rules/ENGINE-1.0.md.
## Versions
- Engine 1.0.0 = ladder engine; CLI cambc 3.0.1 (%APPDATA%\Python\Python314\Scripts\cambc.exe); SDK 3.0.1 in sdk/ [`cambc --version`, ladder champion id 11 seen]
- Old: engine 0.0.3, CLI 2.2.5, SDK in archive/sdk-0.0.3/ [archive/]. Site doc lags SDK: Prism 0.2, Vesper 20%, no Heart of Ruin/golem, old base stats [diff]
## Maps (21; champions per side; w x h) [data/maps21.json from maps/*.map26a via MapBuilder_FromFileBytes]
- 5 per side: ambush 100x100, big_world 100x100, boss_battle 50x41, concentrated_wealth 61x61, contest 50x50, corners 71x71, corridors 51x40, eight 41x30, elk_arena 50x50, elkring 30x30, expanse 80x80, fracture 61x71, islands 60x60, lanes 90x30, monolith 50x50 (no golem camp on any map)
- 4 per side: hooks 31x31, interlock 40x40
- 3 per side: broken_curve 40x40, collision 41x41, fragment 30x30, gold_rush 31x31
- Kept maps ambush, collision, contest, corners, eight, elkring, fragment, interlock, lanes: terrain identical to maps-engine0/, only camps changed (more gem/gold clusters) [notes/maps-21.md]
## Camps (engine 1.0 per-camp base reward read from map group data, data/maps21.json; site doc l.147-155 shows older rewards) [verified 20 Oct]
- Wolf: 1350 HP, str 20, 40 gold, 4 gems (doc 90/22), respawn 40, range 3
- Elk: 2400 HP, str 60, 90 gold, 15 gems (doc 160/36), respawn 60, range 4 (was 3) [ENG]
- Gem cluster: 120 HP (doc 500), str 0, 0 gold, 7 gems (doc 24), respawn 50 [ENG, SDK camp_type comment]
- Gold cluster: 120 HP (doc 500), str 0, 20 gold (doc 35), 0 gems, respawn 50 [ENG, SDK]
- Monolith: 20000 HP, str 200, 300 gold, 200 gems, respawn 150, range 5 (was 3) [ENG, SDK]
- Golem (camp_type 5): 5000 HP, str 120, range 4 [SDK, ENG]; rewards/respawn unknown; absent from all 21 maps
- Some map groups carry a x2 reward multiplier (group totals in notes/map-economics.md) [data/maps21.json info]
- Monsters: vision 8, see through bushes (so bush rows can hide a camp that is hitting you: corridors), do not chase; damage wakes group; 2 rounds with no champion in range and they sleep [doc l.164-166; arena 1]
- Group = camps the map defines together (engine group data; not always cardinally connected); rewards combine; respawn at the longest member timer; an occupied spawn tile delays the group [doc l.133,139]
## Measured in engine arenas (cloud, 20 Oct; results/arena-1020.md)
- Rewards paid match ENG exactly: wolf 4 gems/40 gold, elk 15/90, gem cluster 7, gold cluster 20 (Vesper 26) [arena test 1]
- First-damage range: wolf 3, elk 4, monolith 5 (site doc l.164 "all range 3" is wrong); asleep camps never attack; sleep 2 rounds after the last in-range round [arena 1]
- Monster target = the NEAREST champion of either team; ties alternate; switches at once when someone comes nearer; a group we woke keeps hitting a nearer enemy who never attacked it (camp aggro works as a weapon) [arena 2]
- Occupied spawn tile: the respawn timer runs normally, then at 0 the group waits as long as ANY spawn tile is occupied (own or enemy) and respawns the round it is freed; an adjacent champion has no effect [arena 3]
- Capture region = the group's spawn tiles only: an adjacent enemy does not contest; an adjacent ally gets no share; allies on tiles split (21 -> 11 + 10, remainder to the lower id) [arena 3-4]
- Prism Focus 80/101/122 at 0/21/42 gems (+1.0 per gem); stun 1.5x, area 1.0x Focus. Duels: beats Stellar 80-95 % with 21-42 gems, loses to Ren and Varg at every gem level [arena 5; duel bots only, no items]
- Heart of Ruin duels: beats Prism, Stellar, Willow, Tavi, Vesper (100 %), Vendetta 83 %, Ren 67 %; loses to Ryth 33 %, Varg 30 %, C&I 0 %, Marcella 0 %; HoR+Stellar vs Ren+Varg 41 % [arena 6; duel bots only]
## Capture
- After every monster in the group is dead, occupy a spawn tile of the group uncontested; first capture phase starts a 5-round timer; empty region or enemy present resets; more allies do not speed it; reward split among allies present, lowest ID gets remainder [doc l.139]
## Death, recall, tribute
- Death: respawn on spawn tiles after 60 owner turn-end ticks, full health, abilities reset; dead champions keep shopping turns [doc l.320]
- Kill reward: killer gets all carried gems + 50 base gold; assists no gold; camp kills destroy carried gems [doc l.320]
- Cassi and Issac: if one dies the other dies; both lose gems; one kill reward [doc l.387]
- Recall: channel 5 owner turn ends (3 with Home emblem), interrupted by damage, stun, death; carried gems kept; arrival at turn start, heal needs staying to turn end [doc l.229,332]
- Tribute: within Chebyshev 1 of altar; length max(1, ceil(bag/10)) owner turn ends; start turn counts; completes at next turn start; tributes entire bag at completion; DAMAGE DOES NOT INTERRUPT IT, only stun, death or a newly placed altar lock (recall IS interrupted by damage; teleport crystal: not by damage, by stun and death) [doc l.181, l.229]; damage does not interrupt; stun, death or a new altar lock does [doc l.181-188,229]
- Win: first to 1000 tributed gems, or round 2000: tributed gems, then lifetime gold, then kills, then Team B wins [doc l.14]
- Altar lock (consumable): within 2 tiles of altar, blocks enemy tributes, 3 basic attacks remove it, cancels tributes at that altar [doc l.195-199,271]
## Shop
- Buy on own spawn tiles or while dead; Tavi buys shards anywhere (not consumables), only Tavi sells (full value) [doc l.233,279,635]
- Shard 30 gold (adds champion growth in one stat); 5 matching shards auto-combine to an emblem (150 gold total) [doc l.241-247]
- Emblems: Monster +10% camp damage; Power counts as 6 shards; Durability -10% damage taken; Treasure +20% gold; Jungle -30% camp damage; Home recall 3; Bruiser 5+1+1 [doc l.253-261]
- Consumables 10 gold, one of each carried [doc l.265]; spawn purchase of Health heals the amount; Tavi field purchases do not [doc l.279]
## Vision, memory, fuel
- Vision 8, symmetric shadowcasting; walls and bushes occlude; own bush cluster transparent; one free ward per champion; vision not shared [doc l.294-312]
- Fuel: 100k/turn C++/Rust (OCaml 5x, Python 25x); draft first turn 10x; first play turn 2x; unused fuel not carried; out of fuel suspends and resumes next round [doc l.344-348]
- Shared memory 8 bytes read/write all match; draft memory 1,024 bytes written in draft, read-only in match [doc l.352-358]
- Draft: 3 to 5 picks per team by map; A submits then B submits, both revealed together each round (B does not see A's pick of that round); no duplicate in own team, duplicates across teams allowed; draft bot sees full map and opponent picks so far; no pick/crash = random pick [doc l.58-85]
## Champions (12 drafts; base health/str/focus at engine 1.0; all abilities cooldown 8; move 1; vision 8) [ENG]
- Ryth melee 1200/45/100: aoe 1.0xFocus r4 rad1 stun1; attack reset; passive health adds strength (str 50->45 in 1.0)
- Vesper Vale ranged(3) 900/50/100: attack reset; dash 2; passive +33% gold (was 20); str 45->50
- Vendetta melee 800/70/100: reset; dash 2; passive kills (incl. camps) reset cooldowns; str growth 5->6
- Cassi ranged 600/30/70 (was 700) + Issac melee 900/40/100 (growth 95->80), one pick, shared fate; Cassi damage 2.0xFocus r4 + stun1; Issac reset + dash 2
- Stellar ranged 750/30/85 (was 35/100): damage 2.0xFocus r4; aoe 1.4xFocus r4 rad1
- Prism ranged 700/35/80: stun+damage 1.5xFocus r4; aoe 1.0xFocus rad1; passive +1.0 Focus per carried gem (was 0.2); growth cut (hp 60, str 2.5, foc 8)
- Marcella melee 1200/45/60 (was 50): reset; damage 2.0xFocus r4; stun immune; +2 gold per gem tributed (new); foc growth 15
- Willow ranged 700/55/120 (was 45/105): heal ally 3.0xFocus r4; reset
- Ren melee 900/35/100 (was 40): stun 1.25xFocus r4; dash 2; retaliation passive
- Varg melee 1000/40/100 (was 50): reset; self shield 2.5xFocus 5 turns; splash basic attack
- Tavi ranged 750/45/100 (was 35): reset; stun 1.4xFocus (was 2.2) r4; field shopping passive
- Heart of Ruin (new, draft id 11) melee 1000/40/100, growth 100/5/15: shield 1.5xFocus 5 turns; dash 2 landing rad1 1.75xFocus stun1; passive up to +50% all damage by missing health [SDK constants, ENG]
- Ranged attack range 3, melee 1; one move, one attack, one ability cast per turn [doc l.50]
