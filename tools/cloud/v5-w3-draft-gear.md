**SUPERSEDED (11:30 BST 20 Oct) by tools/cloud/v5-s1-economy.md, v5-s2-quickwins.md and v5-s3-diag-contest.md (PLAN-v4 Draft 4). If you are a session working from this file: commit what you have to your branch, write a one-paragraph status in results/, and stop.**

Read tools/cloud/v5-common.md first and follow it. Your copy: bots/w3. Module: DRAFT, GEAR, RECALL, TIEBREAK.

Why: v4 keeps v3b's static drafts (bots/v4/src/a/w7_drafts.h): Vendetta first on 14 of 21 maps, Stellar on 7, Ren on 2 (notes/step0-deaths.md section 3); top-10 teams draft Stellar 96 %, Ren 69 %, Vendetta 2 %, with core Ryth, Cassi & Issac, Stellar, Ren, Varg (notes/top10-2026-10-20.md). Top-10 teams hold 3.8 gear at round 200 against our 1.7. Win tiebreak at round 2000: tributed gems, then lifetime gold (FACTS.md). Arena duels (results/arena-1020.md): Heart of Ruin beats squishies but loses to C&I, Marcella, Varg, Ryth; Prism with 21+ gems beats Stellar but loses to Ren and Varg.

Tasks (one switch each, ablate each):
1. Draft lookup: core order Stellar, Cassi & Issac, Ren, Varg, Ryth (3-pick maps take the first 3; 4-pick the first 4). Picks are simultaneous within a round and no pick can deny the opponent, so a fixed table is enough; avoid duplicates already taken by us. Compare forced drafts on all 21 maps (head-to-head of draft variants with identical play code), and keep a per-map exception only where a variant beats the core by >= 2 of 6 on that map.
2. Gear curve: spend gold on shards toward emblems early (target gear >= 3.0 at round 200), the right emblem per champion role (Durability for front line, Monster/Jungle for farmers); buy on spawn or while dead.
3. Predictive recall: recall when HP will not survive the next fight or the walk home, or when gold >= the next emblem'
4. Gold tiebreak mode on boss_battle and contest: when tributed gems are level or unreachable late (round > 1,700), farm gold.
Expected effect: >= 55 % vs v4 from draft and gear alone; gear at round 200 >= 3.0.
