# DeepSeek (DSeek, DeepThink on) answer to 2020-plan-v4 (lightly condensed; all numbers and recommendations kept)

Verdict: PLAN-v4 diagnosis right but over-scoped, mis-ordered, too dependent on local/old-engine evidence. Critic mostly right. Fastest route: base + safe default + map id + survival/recall/shop + ability-first fights + bigger bags + draft top-10 core; then route planner; then micro/gear. Top 20 by Fri 22:00 is a stretch.

## 1) Critique
Right: data gap is survival, bag size, gear, ability opening, not tribute count (tributes 24.5/24.3 vs 26.7). v3b 29-131; tributes 19.3 vs 37.5; deaths 37.9 vs 21.4. Top10: 0.57 gems/rnd, 41.5 gems/tribute, 17.2 deaths, 85% spell-opened, 3.8 gear@200, 6.4 locks. Ours 0.30/0.29, 30.2/24.2, 32.8/44.7, 35/43%, 1.7/4.4, 0.
Wrong:
- Mis-ordered: preflight/STATE/FACTS/gate/decoder + seven-fix base in the same Tue morning. Critic right: base uploaded Tue 18:00 not 12:45. Tooling minimum only.
- Too many modules: cut G and H now; E a lookup not minimax; merge A+B+F; split D (ability-first/disengage in base; fight-gate refit later).
- No safe default (map unknown on 6 of 10 new maps, wrong plan on 4; 0/10 correct).
- Admission underpowered/sequential: 80 games ~55% power for +8 pts; use 160 for claims >+5. Measure baseline immediately before and after candidate (only live bot plays unranked).
- Milestones mislabeled: 50% vs a 1,453-1,771 panel is ~1,600 not 1,650; top-20 sample ~1,950-2,000 so M3 is a rank-10 result.
- Freeze is Friday; Saturday bugfix only.
- Missing: rollback/last-known-good, per-map regression floor, upload rating cost/rate limit, fuel budget for draft BFS, kill rule.
Critic wrong on two facts: "Any occupied spawn tile delays the entire group" IS in GameRules (but unsure engine 1.0 implements it; test before G). Sprint 2 deadline is sourced in the header.
Cut: G (except 2-4 h falsifier), H, E matrix. Merge A+B+F. Split D. Keep C cheap parts in base (gold recall, buy curve). Add: P0 safe default/map hash/bugfixes; per-map regression + rollback; draft-core change; monster-edge fight rule; carrier pickoff; gold-tiebreak map specialization; top-team opening lookup if time.

## 2) Plan (build order | hours | admission | drop)
- Tue 08-18, P0 v4-base "do no harm" (~10 h): 21-map hash + safe default; fix contest 0-gem collapse, islands stall, gold_rush feeding; audit blocked-target throw; fuel/mapid prints; ability-first opening; disengage before melee; carrier bank >=30 gems; recall when gold>=90 or HP<40 and no fight; draft Stellar/CI/Ren/Varg/Ryth; 200-byte draft plan; rollback. Admit: gate.py PASS 21 maps x 2 sides; local h2h >=45%; critic; upload; 160-game panel vs 8 teams. Kill if <30%. Drop per-map overrides, HoR, locks.
- Wed 08-18, P1 Route/Economy (8-10 h): draft-time BFS from altars/spawns; camp-group value table; side/contested class; route order; one capturer per group; bank target ~40; respawn windows; 8-byte claims; no clear-time damage model/chokepoints in first cut. Admit: 160-game panel; +8 pts win rate or +15% gems/rnd with deaths +<=10%; no map >3/8 worse. M1 >=50% vs 8-team panel. If fails: static per-map route table + safe default.
- Thu 08-18, P2 Micro/Gear (8-10 h): damage-first gear, Focus for casters; recall/shop cadence; fight gate refit on engine 1.0; focus lowest effective HP; stun visible channels; monster-edge fights; carrier pickoff; dash-cross-hole escape; HoR safety only. Admit: 160-game panel; M2 vs ranks 50-100. Drop HoR/Prism/Marcella experiments.
- Fri 08-22, P3 Main release (6-8 h): weakest-map fixes; per-map overrides only where panel shows; top-team opening lookup if ready; final gate; upload 22:00; top-20 sample overnight. Admit: 400-game top-20 sample, M3 >=40%. Freeze v6; bugfix only.
- Sat: bugfix only; final upload by 20:00; hard stop 23:30.

a) Macro: use engine per-camp values (wolf 4/40, elk 15/90, gem cluster 7, gold cluster 20, monolith 200/300). Route for banked gems/round. Camp group = node (reward, HP, strength, respawn, travel); BFS at draft. Capture 5 rounds; more allies do not speed; rewards shared -> one capturer, others move on. Blockade only after engine test. Tribute = max(1,ceil(carried/10)); top10 bank 41.5, us 24-30; target ~40; choose b* minimizing (travel+ceil(b/10))/b discounted by death risk. Locks (top10 6.4/game, Nyx 0): test on elk_arena/corners after M1, not core.
b) Micro: ability-first (85% vs 35-43%; one cast/turn, 8-turn cooldown). Deaths 17.2 vs 32.8/44.7; killer gets all carried gems +50 gold; camp kills destroy carried gems -> carriers >=30 avoid fights and bank; pick off enemy carriers. Monsters range 3, never chase, attack nearest champion; elk 4, monolith 5: fight at camp edges. Stun visible channels (recall 5 turns, 3 with Home, interrupted by damage/stun). Varg shield at engage. Dash may cross holes. HoR (passive up to +50% by missing health, dash landing 1.75xFocus stun): safety only, duel-test first.
c) Pathfinding: BFS fields at draft, ~200-byte plan in 1,024-byte draft memory; fuel 100k/turn, draft first turn 10x; do not BFS every turn, build over turns 1-5; audit blocked target now throws; dash crosses holes; teleport crystal range 3 ignores terrain, channels 3 turns. Maps: islands 60x60 46 groups, fracture 61x71 28, big_world 100x100 24, corridors 51x40 24, elk_arena 50x50 30. Safe default on unknown hash.
d) MOBA/RTS: tempo (top10 first tribute median 97); rotations (one capturer/group, claims); objective trading (they contest ours, take theirs); vision not shared; recall before fight (top10 62 recalls/game vs 37-43); power spikes (shards 30 gold, five combine); map control (enemy 8.9/7.3 champion-rounds near our spawn vs 0.8); gold tiebreak (boss_battle 74/122 top10 games end on lifetime gold).
e) Draft: Stellar 96%, CI 95%, Ren 72%, Varg 70%, Ryth 67%, Tavi 33%, Prism 10%, HoR 0/2,055. Core Ryth+CI+Stellar+Ren+Varg. Tavi sets -10.8, Prism -14. Default Stellar/CI first, then Ren/Varg/Ryth; avoid Tavi/Prism/Vendetta. 4-pick Ryth+CI+Stellar+Ren; 3-pick CI+Stellar+Ren. Conditional: enemy lacks Ren -> take Ren; enemy Tavi set -> Ren+Varg+Ryth (15/18, +28 pt). HoR only after duel test.
f) Sim: S1 duel/team-fight lab on engine 1.0 (1v1 all pairs at 0/5/10/15 shards, 2v2/3v3; validate vs known duels and ladder fight data). S2 macro sim on map files (camp table, BFS, capture/tribute; validate v4-vs-idle within +-10% gems/rnd). S3 draft lookup from top data. S4 real-team panels 160 games paired by team/map; local only filters. S5 top-team decode.
g) Memory: 8 bytes: b0-1 camp claim/owner/version; b2 danger region; b3 focus target; b4 plan phase; b5 regroup tile; b6 altar/lock; b7 checksum; rate-limit writes, stale claims expire. Draft memory: map class, route per slot, bank threshold, altar, composition, enemy priors. No shared vision; adapt from own vision + draft priors.

## 3) Game-changers (idea | rule | effect | falsifier | h)
1. Camp-spawn blockade denial | "Any occupied spawn tile delays the entire group" | deny high-value respawns (monolith 150), force rotation | engine test: occupy enemy spawn after claim, measure respawn delay + net gems over 200 rounds; no delay/net negative -> drop | 4-8
2. Monster-as-ally camp-edge fights | monsters range 3, never chase, attack nearest; elk 4, monolith 5 | fewer deaths (17 vs 33-45) | duel lab 2v2 near awake camp with/without edge positioning | 3
3. Marcella tribute-gold economy | Marcella +2 gold per gem tributed; tribute whole bag | ~40-gem bags -> +80 gold/tribute; gear spike; top usage 0.3% | engine macro/duel Marcella vs Stellar on 3 maps, gold and gear@200 | 3
4. Prism big-bag carry | Prism +1.0 Focus per carried gem; tribute removes bonus | 40 gems = +40 Focus | duel lab Prism at 0/20/40 gems vs Stellar/Ren | 3
5. Gold-tiebreak specialization boss_battle/contest | tiebreak gems, lifetime gold, kills; boss_battle 74/122 end on gold | gold clusters/Treasure/Vesper/Marcella win without 1,000 gems | 160-game panel on those maps | 4
Unsure #1 is implemented in engine 1.0. #3/#4 high variance, only after M1.

## 4) Single fact
"Unranked games use the live bot" (PLAN 1.4/8). If wrong, upload/baseline sequencing, rating-cost worry and Friday freeze change; candidates could be panelled directly. If forced to a game fact: the engine-1.0 camp reward cut / top10 0.57 gems/round.
