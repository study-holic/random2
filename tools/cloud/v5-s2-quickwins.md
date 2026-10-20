Read tools/cloud/v5-common.md first and follow it, but use the admission rule below. Your copy is bots/s2. Module: **QUICK WINS** to release first, ideally live within 6 hours. These are the draft core, altar control and gold spending (consult/1029-v5-direction.md: all four consultants rank these cheap and early).

Keep every change small and separately switchable. Push a first candidate as soon as task 1 passes the gate, then add tasks 2 and 3.

1. **Draft core lookup (P_S2_DRAFT_CORE).**
   - Why: v4 keeps v3b's static drafts (bots/v4/src/a/w7_drafts.h) and opens with Vendetta on 14 of 21 maps. Top-10 teams pick Vendetta 3 % of the time and Stellar 96 %.
   - Top-10 core by pick count (notes/top10-2026-10-20.md l.49-51):
     - 5 picks: Ryth, Cassi & Issac, Stellar, Ren, Varg;
     - 4 picks: Ryth, C&I, Stellar, Ren;
     - 3 picks: C&I, Stellar, Ren.
   - Picks are simultaneous within a round, and duplicates across teams are allowed (FACTS.md), so no denial order is needed.
   - Check that v4's per-champion code supports each core champion: attack range, ability use, retreat and buy. List any gap. A drafted champion the code plays badly is the main risk.
   - Compare forced drafts with play code unchanged. The decision rests on the 126-game head-to-head over all maps and on the live panel, not on per-map counts. Keep a per-map exception only where a variant beats the core by at least 2 of 6 on that map AND the overall head-to-head does not drop.
   - Tavi is not in the core. Dropping Tavi loses its shop-anywhere passive, so every purchase must happen on spawn tiles or while dead. Check that v4's buy code does not assume Tavi.
2. **Altar control (P_S2_ALTAR_LOCK, P_S2_TRIBUTE_GATE).**
   - Facts: a lock is a 10-gold consumable (one carried at a time) placed within 2 tiles of an altar. It cancels the enemy's tribute channels there and blocks their tributes until 3 basic attacks remove it. A tribute against a locked altar becomes an attack on the lock (GameRules l.193-199). Top-10 teams place 6.4 locks a game; v4 places 0. The evidence of benefit is weak: in top-10 games, teams that lock won 16 of 31 against teams that don't (52 %), with 30.7 vs 24.3 tributes, and Nyx, the strongest team, buys none (notes/top10-2026-10-20.md l.77). So the lock is an experiment: ablate P_S2_ALTAR_LOCK on its own and keep it only if it helps.
   - Build:
     - (a) detect enemy locks, and clear one with 3 basic attacks, or go to another altar, depending on which is cheaper;
     - (b) buy a lock at every spawn visit when gold allows;
     - (c) place it on an altar where an enemy is tributing within our reach. Otherwise place it on the altar nearest the enemy spawn, or where we last saw an enemy tribute, when one of our champions passes within 2 tiles. Altars belong to no team;
     - (d) tribute gate: start a tribute unless an enemy that can stun, or that could place a lock or kill us, is within its reach. Damage alone does NOT cancel a tribute (GameRules l.181/229), so do not abort for damage.
3. **Gold spending (P_S2_SPEND_NOW).**
   - Buy shards toward an emblem for the champion's role at every chance: on spawn tiles or while dead (FACTS.md). Keep 10 gold for a lock.
   - Target: gear of at least 3.0 per champion at round 200 (top 10: 3.8; us: 1.7).
   - Report how much unspent gold v4 holds at rounds 100 and 200.

## Admission (fixed before testing)
1. gate.py PASS.
2. Head-to-head vs v4 at least 45 % (a rejection gate only).
3. Locks placed per game above 0, and gear at round 200 of at least 3.0 in the head-to-heads.
4. Deaths per game vs v4 not up more than 10 %.
Report the switch ablations. The lead uploads the passing candidate for a live top-10 panel.
