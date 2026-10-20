Read tools/cloud/v5-common.md for setup and tools. Do not run cambc login, submit, matches, ladder or match-groups. Push to this session's branch. Stop by 06:00 BST Wed 21 Oct (`TZ=Europe/London date`). Plan context: PLAN-v4.md "Draft 5"; consultants' answers in consult/1946-sprint2-priorities.md.

Data: data/real/v4-panel, 180 unranked games of v4 against the teams ranked 2-10, on 20 maps (index.csv; baseline-v4top.csv). Decode them with tools/top10/decode.mjs and your tools/cloud/metrics.py.

## 1. Reconcile three numbers the consultants questioned (first; push at once)
- **Top-10 deaths a game:** 4.3 (tools/v3/panel_v4_top10_full.py, 180 games) against 6.0 (results/v5-s3-diag.md, 90 games). Do both use the same games, the same counting of Cassi & Issac, and the same cause rules? Give one figure per sample, with its method.
- **Recall share:** recalling is 15.8 % of top-10 champion-rounds, which implies about 170 recalls a game. Count recall starts and completions per team per game, and the share of starts that complete. Hand-check two games round by round.
- **Conversion:** banked ÷ picked up, from one sample only. The prompt mixed 159/269 (90 games) with 201 (180 games).

## 2. Production queries (Claude's three, consult 1946), for the top 10 and for v4 in the same games
1. Pickups per game by camp type: wolf, elk, gem cluster, gold cluster, monolith; also gems taken by kills.
2. Recalls: distance from their own spawn at the start, HP fraction at the start, bag at the start, and what follows within 10 rounds (purchase, heal, death).
3. Gold spent by round 200, the order of purchases, and the round each emblem is completed.

## 3. Conditional policies (imitation conditioned on state, not copied actions)
Copying actions fails under covariate shift: our bot reaches states the demonstrators rarely reach (Ross, Gordon and Bagnell 2011, DAgger). So estimate decision rules as functions of the state, for the top 10 and for v4:
- P(recall | HP fraction, bag, distance to spawn, enemies within 8);
- P(bank now | bag, distance to the nearest usable altar, enemies within 8);
- P(engage | own HP and gear vs the nearest enemy's, allies within 4).
Use binned tables, or a logistic regression if each bin has enough rows. Report where the top 10's probability crosses 0.5, next to v4's.

## 4. Output
Write results/imitate.md:
- the reconciled figures;
- the three query tables;
- the policy tables;
- at most three candidate rules, each written as a formula with the state variables, the expected effect on a named metric, and the measurement that would falsify it.
No bot changes in this session unless all of the above is pushed. After that, you may implement the single best rule behind P_I_<NAME> in bots/im (a copy of bots/s1g1), and test it on 35 maps: unopposed, and vs v4 (reject below 62/126 on the old 21 maps).
