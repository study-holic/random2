**SUPERSEDED (11:30 BST 20 Oct) by tools/cloud/v5-s1-economy.md, v5-s2-quickwins.md and v5-s3-diag-contest.md (PLAN-v4 Draft 4). If you are a session working from this file: commit what you have to your branch, write a one-paragraph status in results/, and stop.**

Read tools/cloud/v5-common.md first and follow it. Your copy: bots/w2. Module: FIGHTS AT ALTARS AND CARRIER PROTECTION.

Why: in v3b's real-team games 43 % of our deaths were within 8 tiles of an altar (1,595 champion kills there), and we lost 69.5 gems a game to champion kills while taking only 31.7 from theirs (net -37.8 a game; notes/step0-deaths.md). Top-10 teams open 85 % of fights with an ability against our 35-43 % (notes/top10-2026-10-20.md). Rules: Cassi and Issac die together (one kill, both bags); recall is interrupted by damage or stun, but a TRIBUTE is NOT interrupted by damage: only a stun, death or a newly placed altar lock cancels it (GameRules l.181, l.229; FACTS.md); the killer takes all carried gems.

Tasks (one switch each, ablate each):
1. Ability-first opener: when an enemy champion comes within cast range and our abilities are ready, cast (stun first, then damage) before basic attacks; hold casts for the opener instead of spending them on camps when enemies are within 8 tiles.
2. Focus target: prefer Cassi (kills both), then the enemy carrying the most gems, then the lowest effective HP.
3. Channel interrupts: a STUN (or a kill, or placing an altar lock) cancels an enemy tribute; damage alone does not. Damage or a stun cancels an enemy recall. Save a stun for a tributing enemy with a big bag; altar locks (10 gold, consumable) cancel every enemy tribute at that altar (top-10 teams place 6.4 a game, we place 0).
4. Carrier protection: our champion carrying >= 20 gems does not path through tiles within 4 of a visible enemy; it tributes when the altar is safe or recalls early; teammates escort when a fight starts near a carrier.
5. Do not tribute into a visible enemy: if an enemy champion is within 4 of the altar and the bag is large, wait out of range or fight first.
Trace one v4 death at an altar in a v4-vs-v3b-e1 game before building, to check which of these is the real cause.
Expected effect: champion deaths per game down by a quarter and the gem margin from kills positive.

Added (lead, 10:10): your target maps are the small contact maps where v4 loses every real game with about 50 deaths: fragment (30x30, 3 a side), hooks (31x31, 4 a side), gold_rush (31x31, 3 a side). Trace at least two real losses on each from data/real/v4-panel (the map order is in index.csv) before building: who kills us, where, while doing what, and how the opponent's fights start (who casts first, at what range, on whom). Report deaths per game on these three maps before and after in self-play.
