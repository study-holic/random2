# v5 cloud builders: shared brief (read with your module file)

You are one of three parallel cloud builders for Team Xanarchy's Cambridge Battlecode 2026 bot (this repo). Each builder works on ONE module in its own bot copy; a later session integrates the admitted modules into v5. Target for v5: Wed 21 Oct 18:00 BST.

Setup: `bash tools/cloud/setup.sh`. Then read: FACTS.md (rules; "Measured in engine arenas" section is engine-verified), STATE.md, PLAN-v4.md ("Draft 4" section, then "Draft 3"), notes/step0-deaths.md (where our deaths come from), results/arena-1020.md (monster targeting, respawn, Prism, Heart of Ruin), results/v4-cloud.md (how v4 was traced and fixed: copy that method), VERSIONS.md (v4 entry). Rules sources: docs/rules/ENGINE-1.0.md and sdk/cpp/cambc.h supersede docs/rules/GameRules.md; never invent an SDK method.

Rules of work:
- `cp -r bots/v4 bots/<your copy>` (no .cambc). Never edit bots/v4 (live release), bots/v3b or bots/v3b-e1.
- Every behaviour change sits behind a named switch (`#ifndef P_X / #define P_X 1`, with a comment naming the cause), like v4's P_UNSEEN_HIT. Keep changes inside your module's files where possible so integration is easy; list every file:line you touch.
- Trace before you fix: decode a lost game (tools/top10/decode.mjs, see tools/README-decode.md) and find the cause, as v4-cloud.md did for corridors.
- Fuel: no turn over 80,000 after round 1 from your code; the gate fails over 100,000.
- Tests use `--jobs 3`; commands over 2 minutes run in the background. Head-to-head: `python3 scripts/mapgate.py <copy> all --jobs 3 --salts 0,1,2 --panel <opp> --out results/<copy>-vs-<opp>.json` (126 games: 21 maps x 2 sides x 3 salts).
- Token use low: small outputs, never print whole files or logs.

Admission rule for a module (fixed before testing; report each line as PASS/FAIL; a module brief may replace it):
1. `python3 scripts/gate.py <copy>` PASS (42/42 vs starter-cpp, 0 faults, mapid correct).
2. Head-to-head vs v4 >= 57/126 (45 %): a REJECTION gate only. Self-play misled us: 68 % locally became 23 % against real teams.
3. Deaths per game vs v4 not up more than 10 %.
4. The module's own mechanism metric moves as predicted (named in the module brief).
The live top-10 panel, run by the lead (A-B-A against v4's panel on the same teams and map sets), decides the release. Report deaths per game, gem margin per game and per-map wins beside the win rate.

Style (Xans): state every mechanism you build as a model: the objective, the constraints, and the decision rule as a formula. Cite the source domain (operations research, robotics, game theory, MOBA play) and map each term onto a cambc rule or SDK limit.

Output: results/v5-<copy>.md (cause traces with rounds and positions; each switch and file:line; ablation of each switch you add; the admission table; what still fails). Commit your bot copy (source only), result JSONs and the report to this session's own branch and push it. Do not push to main. Final summary under 250 words.

Real-team evidence (data only; priorities are in PLAN-v4.md "Draft 4"):
- **v4 vs 8 mid-ladder teams:** 160 games paired with v3b (notes/panel-v4-vs-v3b.md): 37-123 (v3b 29-131); deaths 23.0 a game (v3b 37.9); gems 499 vs 782. Contest 0-8 with 0 gems; fragment, hooks and gold_rush 0-8 with 45-56 deaths.
- **v4 vs the top 10:** ranks 2-10, 90 games (notes/panel-v4-top10-AB.md): 1-89; gems 159 vs 943; deaths 22.7 vs 6.0; gems lost to champion kills 62 vs 3. Even corners runs 239 vs 1,009 gems, with 12.1 vs 1.0 deaths.
- Replays: data/real/v4-panel (README and index.csv there).

Measurement:
- Replays are decoded with tools/top10/decode.mjs (tools/README-decode.md). If data/top10/engine lacks the v1_0_0 wasm, copy it from the installed cambc package's visualiser assets, as the v4 cloud session did.
- tools/top10/analyze.py shows how gear points per champion by round, locks bought and used, and tributes are derived. tools/step0/ splits deaths by cause.
- S3 writes tools/cloud/metrics.py in its first hour and pushes it. Fields: banked gems by round, the round at which banked gems reach 1,000, deaths by cause, locks placed, gear at round 200, and the share of fights opened with an ability. S1 and S2 may write minimal versions with the same field names.

Switch names carry the session tag (P_S1_..., P_S2_..., P_S3_...), so integration has no clashes.
