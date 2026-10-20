# 2010 game-changer consultation (Part 3), collected 20 Oct

Sources: DeepSeek (answers/2010-deepseek-part3.md), ChatGPT (answers/2010-chatgpt-part3.md). Copilot was skipped: copilot.cloud.microsoft showed "Authentication required" and no chat history was reachable without signing in. Both saved answers are condensed copies; consultant output is input to verify, not a verdict.

## DeepSeek, candidates in its ranked order (idea | rule exploited | claimed effect | falsifier | build hours)
1. A11 new-map exploration protocol: run the 10 new maps in unranked play on day 1, decode, build economics from data | none, measurement | +40-80 rating | decoded economics disagree with the map reader | 2-4 h plus background
2. A2 turn-1 generic map reader (supply class, side, contested count, rally tiles, camp order, shard plan into draft memory) | "The draft phase has additional fuel ... entire map layout"; 1,024 draft bytes | +40-80 rating, +30-60 on new maps | reader class disagrees with Appendix C on more than 2 kept maps | 8-12 h
3. A4 ability-opening rule (84% vs 32%) | "one successful ability cast per turn" | +0.05-0.1 gems/round, -1 to -3 deaths | audit rate stays under 70% | 2-3 h (check v3b first)
4. A5 commit or disengage before contact | "Melee champions can walk and attack in the same turn" | +0.1-0.15 per duel | duel lab already measured; recheck with a camp in the arena | 2 h
5. A1 shard-curve opening book (draft-time per-champion shard path, gold-driven recall) | "Shards cost 30 gold ... Five matching shards automatically combine" | +0.1-0.2 gems/round, -1 to -3 deaths | shard-points at round 300 not up 4 and deaths not down | 6-8 h
6. A10 fight-model pre-commit gate (do not enter below 55% predicted) | none (uses the fight model) | -2 to -4 deaths | deaths do not fall in 30 real-team games | 6-8 h
7. A7 one capturer per group, rest move on | "Extra allies do not speed capture" | +0.05-0.15 gems/round | camps per champion-round not up | 6-8 h
8. A8 tribute cadence by map supply class | "Captures resolve before channels" | +0.05-0.1 gems/round | deaths per tribute rise | 3-4 h
9. Composition rules (Stellar first, never Tavi+Varg, Ren in, 3-sets on balanced maps) | "you can select the same champions as your opponent" | +15-40 rating | counterfactual on 515 decoded games shows no gain | 4 h
10. A3 first mover on contested groups | "Any occupied spawn tile delays the entire group" | first tribute earlier by 5+ rounds | not earlier | 3-4 h
11. A12 priority-ordered stun then burst | "Priority, then draft order" | +1-2 kills a game | already ordered | 1-2 h
12. A6 jungle before power on high-supply maps | Jungle "30% less damage from camps" | +0.05 gems/round | no change | 2-3 h
13. A9 monolith siege from range 4 on new maps with a monolith | "All monsters have attack range 3" | map dependent | no gain on new map | 2 h
Its plan: day 1 A4, A5, A11 and a composition counterfactual; day 2 A2; day 3 A1, A8, A3, A12; day 4 A10, A7. Its top-pick risk: if v3b's real-team record is still near 27-109, micro items drop and structural items (A2, A11, A1) become the plan; run 30 unranked games of v3b first.

## ChatGPT, candidates in its ranked order
1. Gold-to-gear economic snowball (draft-time beam search over opening purchases) | "Shards cost 30 gold and add their champion's growth", "Tavi can buy shards from anywhere" | +0.15-0.40 gems/round | more early gear but no better round-300 score | 10-16 h
2. Generic map strategy compiler (nine steps; plans scored from map features) | "The map is fully visible in the drafting phase" | +0.10-0.35 gems/round, +5-15 points on unseen maps | no better than safe-nearest-camp on held-out maps | 8-14 h
3. Contested-objective ownership auction | "Empty or contested regions reset progress" | +0.10-0.30 gems/round, -1 to -3 deaths | still enters losing contests | 7-11 h
4. Initiative-engineered burst and focus fire | "Priority, then draft order" | +0.05-0.18 gems/round | no real-fight gain; check v3b | 5-8 h
5. Asymmetric roles (workers plus control unit) | "More allies do not speed capture" | +0.10-0.30 gems/round | defenders idle | 8-12 h
6. Early kill into economic expansion | "The killer receives all carried gems and 50 base gold" | +0.08-0.25 gems/round | kills do not raise later capture | 6-10 h
7. Camp conveyor | "The group respawns together ... Any occupied spawn tile delays the entire group" | +0.08-0.25 gems/round | no gain in no-enemy economy arena | 6-10 h
8. Macro-enabling composition draft | kit rules | not quantified | no execution of the macro | 4-6 h
9. Range-four farming | "All monsters have attack range 3" | +0.05-0.20 gems/round | clearing too slow | 5-8 h
10. Chokepoint wards | "Wards do not expire or take damage" | +0.03-0.15 gems/round | warnings stale | 5-9 h
11. Respawn blockade | "Any occupied spawn tile delays the entire group" | deny 0.10-0.40 enemy gems/round while held | blocker killed or its lost farming exceeds the denial | 5-8 h
Its critical experiment: decompose the equipment gap (earned gold, purchases, early fight losses, measurement) before committing; its plan runs v3b against the real panel first.

## Ideas proposed by both consultants
- Map-reading planner at draft time, generic first and overrides second (DeepSeek A2, ChatGPT #2): both put it in their top two, both write a plan into draft memory (about 55 bytes vs up to 1,024), both validate on held-out kept maps and then new maps.
- Early shard / gear curve (DeepSeek A1, ChatGPT #1): both lean on the 12.8 vs 5.0 shard-points gap and both name that figure as the fact that would change their top pick.
- Contested-group ownership or first-mover assignment (DeepSeek A3 and A7, ChatGPT #3 and #5): one capturer per group, the rest move on; ownership by arrival time.
- Priority-ordered burst scheduling (DeepSeek A12, ChatGPT #4): check v3b first; both rank it mid.
- Respawn-tile blockade (DeepSeek Part 2 C2, ChatGPT #11): DeepSeek ranked it third in Part 2, ChatGPT last now; both warn the blocker may simply be killed.
- Counter-draft pair tables are dead; composition-level draft only (both).
- Camp conveyor and one-capturer split (DeepSeek A7, ChatGPT #7).

## Merged top 5 (for the lead to verify, not a ranking of truth)
1. Decompose the equipment gap and re-run v3b against the real panel on the corrected, 21-map pool (both say this decides the rest; cheapest, a few hours).
2. Generic turn-1 map reader writing a plan into draft memory, held-out tested on the kept maps.
3. Early shard / gear curve scheduled from that plan (gold-driven recall).
4. Contested-group ownership (one capturer, first mover, feasibility gate) to repair the 38% gem-scoring rejection.
5. Respawn-tile blockade as a cheap map-conditional experiment (highest novelty, lowest confidence).

## Rule claims checked against docs/rules/GameRules.md
- "Captures resolve before channels, so gems captured this round are included" and "Its length uses your bag when you start. Completion tributes your whole bag, including gems gained since": true (231-232). Consequence neither consultant stated cleanly: start a 1-tick tribute with a small bag and gems captured during the channel are still paid out in full. DeepSeek's C1 version (capture during the channel) fits this; it needs a camp reachable from an altar-adjacent tile; unverified on any map.
- "Any occupied spawn tile delays the entire group": true (193).
- "Priority, then draft order"; "set_priority takes effect from the next round and persists": true (127-128).
- Altar lock blocks opposing tributes, allows the owner's team; tribute against a locked altar becomes auto attacks on the lock; three basic attacks clear it: true (237-241).
- Tribute: "damage does not stop it, but stun, death or a new altar lock does"; recall "interrupted by damage, stun or death": true (272).
- "Health bought away from spawn raises maximum health without healing" (Tavi): true (676). DeepSeek's reading that it pre-pays a later recall heal: unverifiable. Line 322 adds that spawn purchases heal and Tavi field purchases do not.
- Drafting: "you can select the same champions as your opponent": true (61). The draft turn gets 10x fuel and the first play turn 2x: true (389).
- Wards: one active, permanent: true (348-349).
- Shard and emblem figures (30 gold, five shards combine, Power 6 shards, Durability 10%, Jungle 30%): true (286-302).
- Prism +0.2 Focus per carried gem: true (550). Vesper +20% gold, +20 points with Treasure: true (518). Marcella immune to stuns: true (644). Ren retaliates 0.2 x Focus: true (738).
- Monster table (elk 2,400 HP, strength 60, range 3, 36 gems, 60 respawn; monolith 20,000, 200, range 3, 200 gems, 150): true (182-185).
- Gold-tiebreak order (tributed gems, then gold, kills, Team B): true (34).
No rule claim was found false. Unverifiable without data: "12.8 vs 5.0 shard-points" definition; whether a stationary enemy is recalling; DeepSeek's claim that tools/v3 decodes 1,456 of 1,456 games.
