# Fight model, champion-pair fight matrix, draft matrix (analyst A2, 19 Oct 2026)

> Measured on engine 0.0.3 and the 17-map pool; engine 1.0 changes (docs/rules/ENGINE-1.0.md) that may affect it: Prism Focus per gem 0.2 -> 1.0, Vesper gold bonus 33%, Marcella +2 gold per gem, base stats rebalanced for 11 champions (every fight and draft cell involving them), Tavi stun 2.2 -> 1.4 x Focus, Heart of Ruin added (no cells), gem/gold cluster HP 120, elk range 4, monolith range 5, 21-map pool.

Data (all engine v0_0_3): 15,855 fights from 575 full-action replays: 387 top-10 mutual games (10 teams, mean rating 1,996, 18 Oct 13:20Z-22:40Z) and 188 xanarchy games (40 teams, mean rating 1,428, 18 Oct 22:40Z-19 Oct 16:00Z; our bot version mixed). Fights: `data/v3/fights.jsonl` (fields: map id, start/end round, initiator, each side's champions with hp, maxhp, gear points, strength/focus estimates, cooldown flags, damage dealt/taken, damage per hit, casts, deaths with bag gems, nearest camp, monster hits). Code `tools/v3/a2_*.py`; tables `data/v3/matchups/`.
Fight = opposing champions trading damage, clustered by shared participant or <=4 tiles, ended by 5 idle rounds or one side all dead. Outcome = units killed minus units lost (Cassi+Issac = 1 unit); "decisive" = unequal. Round-1..3 spawn pokes dropped. Of 15,855 fights ~43% have no death.

## 1. Fight-outcome model (precognition)
Instance = one decisive fight from the INITIATOR's side (first damage dealer); label = initiator kills more units than it loses (n=7,185 decisive fights, 552 games; base rate 71.5%). Features: (initiator side minus other) over alive champions within 6 tiles of the fight centre, from fields the bot can read (health, max_health, strength, focus). Logistic regression.
Held-out = leave-teams-out (5 folds, test games contain a team never seen in training), pooled n=12,970 test rows (fights appear in 1-2 folds):

| model | AUC | accuracy | logloss |
|---|---|---|---|
| always "initiator wins" | - | 0.715 | - |
| count only (d_n6) | 0.66 | 0.71 | 0.58 |
| **count + HP (d_n6, d_hp6)** baseline | **0.776** | 0.748 | 0.517 |
| count + HP + gear points | 0.842 | 0.792 | 0.465 |
| **LEAN4: d_hp6, d_hpfrac6, d_minhp, d_pts6** | **0.861** (+-0.016 over folds) | **0.811** | 0.428 |
| LEAN4 + count / + spell potential / + all 14 | 0.865 / 0.868 / ~0.87 | ~0.81 | 0.425 |
Time split (train games before 18 Oct 23:00Z, test after, n=3,098, base 74.7%): count+HP AUC 0.824 acc 0.784; LEAN4 AUC 0.855 acc 0.825; constant 0.747.
By source (leave-teams-out): top-10 games AUC 0.853 (count+HP 0.726), our games 0.858 (0.815). Transfer top-10 -> ours AUC 0.86, ours -> top-10 0.85 (count+HP 0.82, 0.73).

Data shows: LEAN4 beats count+HP by +0.085 AUC (+6 accuracy points over the 0.748 baseline, +10 over "initiator wins"). Body count adds nothing once HP is in (coefficient CI spans 0). The gain comes from HP fraction, lowest-HP unit, and **gear points** (shard-equivalents, derived from max_health/strength/focus versus base; rating gap changes the gear coefficient by 1%). Cooldown state adds nothing (own-true-cooldown spell burst 0.867 vs all-ready 0.870). Champion composition adds nothing (0.864). Subsets: no camp within 4 tiles 0.896; excluding contest (3a5c2468) 0.848. Calibration (8 bins, mean predicted -> observed): 0.19->0.19, 0.46->0.43, 0.62->0.64, 0.71->0.79, 0.79->0.84, 0.87->0.90, 0.94->0.96, 0.99->0.98 (slightly under-confident 0.65-0.85).
Policy table (pooled held-out; share of initiated decisive fights kept and their win rate; count+HP in brackets): P>=0.5: 79%, 83.3% [87%, 76.8%]; >=0.6: 71%, 86.8% [77%, 80.6%]; >=0.7: 58%, 90.6% [58%, 86.5%]; >=0.8: 42%, 93.9% [35%, 89.6%]; >=0.9: 27%, 97.2% [12%, 92.1%].

C++ (`data/v3/matchups/fightmodel.h`): p = sigmoid(B0 + sum W_i x_i), x_i = initiator-side minus other-side:

| term | x (sum over alive bodies within 6 tiles of midpoint attacker/target) | coefficient | 95% CI (bootstrap by game) |
|---|---|---|---|
| B0 | - | 0.7448 | - |
| W_HP6 | current health / 1000 | 0.1024 | 0.005, 0.216 |
| W_HPFRAC6 | sum health / sum max_health (per side) | 2.2885 | 1.90, 2.71 |
| W_MINHP | lowest current health on the side / 1000 | 0.6247 | 0.49, 0.78 |
| W_PTS6 | gear points = (maxhp-base)/hp_growth + (str-base)/str_growth + (focus-base)/focus_growth | 0.0298 | 0.025, 0.034 |
Limits (interpretation): the bot sees only its own 8-tile view, so allies/enemies beyond it need shared memory; fights observed are ones that happened, so the model scores "if we start this" under the initiators' selection, not a randomised engagement. Top-10 initiators themselves enter fights with P<0.5 24% of the time (win 24%, n=859 decisive); our initiations with P<0.5: 282 of 1,470 decisive, won 25%; skipping them would net +142 units over 188 games (upper bound, ignores objective value). Damage-lead label is not predictable beyond baseline (accuracy ~ constant); not shown.

## 2. Champion-pair fight matrix
(a) Participant 1v1 duels, both sides dealt damage (2,836 fights, 31 pairs with >=30 fights; cells below 30 flagged by the CSV `fight_duel_matrix.csv`). Cell = share of kills by first-named; neither died in ~60%. Wilson 95%.

| A v B | fights | A kills / B kills | A share [CI] |
|---|---|---|---|
| Cassi v Varg | 44 | 1 / 28 | 0.03 [0.01,0.17] |
| Vendetta v Cassi | 32 | 20 / 1 | 0.95 [0.77,0.99] |
| Ryth v Issac | 76 | 1 / 12 | 0.08 [0.01,0.33] |
| Cassi v Issac | 32 | 0 / 13 | 0.00 [0.00,0.23] |
| Stellar v Varg | 44 | 8 / 23 | 0.26 [0.14,0.43] |
| Stellar v Prism | 27 | 13 / 0 | 1.00 [0.77,1.00] |
| Ryth v Cassi | 101 | 20 / 9 | 0.69 [0.51,0.83] |
| Ryth v Vendetta | 62 | 18 / 8 | 0.69 [0.50,0.83] |
| Ryth v Varg | 118 | 20 / 27 | 0.43 [0.30,0.57] |
| Ryth v Stellar | 151 | 11 / 14 | 0.44 [0.27,0.63] |
| Stellar v Ren | 314 | 38 / 32 | 0.54 [0.43,0.65] |
| Ren v Tavi | 100 | 14 / 14 | 0.50 [0.33,0.67] |
Other cells' intervals include 0.5. Only about 8 pairs are distinguishable from even; thin cells (<30) in the CSV.
(b) Rating/HP/strength-adjusted champion strength in decisive 1v1 exchange duels (logistic, controls log hp ratio 3.9/ln, strength +0.86 per 50, rating gap, initiator; n=660 duels in the earlier run, fights.jsonl since grew to ~830): positive = survives: Issac +1.6, Stellar +1.1, Tavi +0.5, Cassi +0.3; Ryth -1.0, Vendetta -1.1, Ren -1.0 (CIs exclude 0 for Issac, Stellar, Ryth, Vendetta, Ren); Varg, Prism, Marcella, Vesper near 0 but thin (see `duel_strength.json`).
(c) Larger round-level sample (`duel_kill_matrix.csv`, `duel_kill_bt.json`): 3,825 isolated-duel kills in 1,154 games (victim had exactly one enemy unit and no ally within 6). HP-ratio-controlled strength (killer logit): Stellar +1.7, Tavi +1.4, Prism +1.1, C&I +0.6; Ryth -1.75, Willow -1.6†(n=39), Varg -1.0, Vendetta -0.6; Ren, Marcella, Vesper ~0. Melee v ranged has no class effect (ranged kills melee 577 v 559). Stellar, Ryth, Vendetta agree across (b) and (c); Ren and Varg do not. The killer had the higher HP fraction in 89% of these kills.
(d) Numbers (initiator present within 6 tiles vs other, W/(W+L) for initiator, top-10 | ours; cap 3): 1v1 0.62 | 0.68; 2v1 0.74 | 0.85; 3v1 0.87 | 0.91; 1v2 0.56 | 0.55; 1v3 0.41 | 0.46; 2v2 0.62 | 0.70; 2v3 0.49 | 0.61; 3v2 0.73 | 0.81; 3v3 0.66 | 0.68 (each cell 100-1,000 fights; run before final extraction). Initiation alone is worth ~+12 points at equal numbers. Controlling for HP and gear, count has no extra effect (above): extra bodies matter through their HP.

## 3. Draft-level (done earlier; details in `data/v3/matchups/draft_*.csv`)
9,245 games, 1,849 groups, 18 Oct 09:20Z-19:13Z; Elo scale fit k=0.977. Exclusive champion effect (rating-adjusted, pp): Stellar +3.2 [1.0,5.4], Marcella -6.0 [-9.3,-2.9], Vendetta -2.2, Varg -2.0 (borderline). Of 55 pair cells 9 have CIs excluding 0 (2.75 expected by chance), 1 survives FDR (C&I v Marcella +7.8pp, n=563, driven by the 1,450-1,600 cohort: +8.3 n=540; top-50 n=23† -3.9). With map-specific champion effects 0 of 55 remain; champion x map improves held-out logloss 0.6920 -> 0.6873, pair terms worsen it. Cohort sign agreement 24% (34 cells), time halves 67%. Most common pairing C&I+Ren+Ryth+Stellar+Varg v C&I+Ren+Stellar+Tavi+Varg: 350 games, 50.3% (+0.1 [-5.6,5.6]). 3-pick maps: C&I+Ren+Stellar beats C&I+Stellar+Varg +10.5pp [4.9,15.9], n=300 (packs b135e0fc, fragment 9925f416, expanse 81929320).

## 4. Ours vs top (partial; not finalised tonight)
Initiator numbers adv/equal/disadv: top-10 34/35/31%; ours 32/47/21%; opponents vs us 43/40/17%. Initiator win (decisive): top 69%, ours 70%, opponents vs us 79%. Opening with a spell: top 84%, ours 32%, opponents 52%. Gear gap at initiation (initiator minus other, shard-points): top +8.3, ours -0.2, opponents vs us +13.2. Fighters in rounds 200-400 carry 12.8 shard-points (top, mostly Focus) vs 5.0 (ours), 7.4 (our opponents). Basic-attack damage per hit differs by build (top Ryth 87, ours 120; Cassi 32 both). Figures are cross-population (ratings ~2,000 v ~1,430), not rating-adjustable.
