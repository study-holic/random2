Read tools/cloud/v5-common.md first for setup and measurement tools. Do not run cambc login, submit, matches, ladder or match-groups. Push results to this session's branch. Stop by 19:30 BST (`TZ=Europe/London date`).

## What v6 is
`bots/v6` = `bots/v5` + the contest module from the S3 session (branch claude/happy-heisenberg-k4ttd6, report results/v5-s3.md on that branch):
- **v5** is S2 build B (branch claude/nice-cray-9ju6gn, report results/v5-s2b.md): draft core, altar locks from round 200, tribute gate, spend-now, fuel stop. It went live for the top-10 panel today. Its files: s2.h, s2.cpp, bot.cpp, bot.h, plan.cpp, stuck.cpp.
- **The contest module** changes 7 files, none of them S2's: enemies.cpp, map_contest.cpp, params.h, s3_contest.cpp, s3_contest.h, util.cpp, util.h. They are copied unchanged.
- **One integration edit:** plan.cpp's S2 core-draft line now skips contest, so contest keeps the module's draft (P_S3_DRAFT 2: Heart of Ruin, Ren, Stellar, Ryth, Varg). w7's draft table (W7_SET 3) covers only eight and stampede.
- It compiles on the laptop. It has not been played.

## Tests, in order
1. `python scripts/gate.py v6 --jobs 3`.
2. **Integration check:** v6 vs v4, 126 games (21 maps × 2 sides × salts 0-2), the same harness as `results/s2-vs-v4.json` (v5's run). Games are deterministic per salt, so on the 20 maps other than contest every game must match v5's game for game: winner, round and both sides' gems. List any game that differs and trace it before going on. A difference means the two modules interact.
3. **Contest:** v6 vs v4 and v6 vs v3b-e1 on contest, both sides, salts 0-2 (6 games each). Report tributed gems a game, deaths a game for both sides, wins and gem margin. Compare them with the module alone (333 gems and 13.7 deaths vs v4; 433 and 8.8 vs v3b-e1) and with v5 alone on contest.
4. v6 vs v5, 126 games: report it, but it is not an admission line.

## Null baseline
v4 vs v4 in this harness is 73/126 (the S3 session measured it), not 63. Report every head-to-head next to that null. A self-play figure near 73 means no change.

## Admission (fixed before testing)
1. gate.py PASS.
2. The 20 non-contest maps match v5 game for game (test 2).
3. On contest, at least 150 tributed gems a game vs v4 and vs v3b-e1, and fewer deaths than v5's on contest against the same opponent.
4. Deaths per game vs v4 over all 126 games: not more than v5's 17.0 + 10 %.

Write results/v6-integrate.md: the admission table first, then the per-map table for contest and any differing games. Model statements cite file:line. If a line fails, say so plainly; do not adjust the rule.
