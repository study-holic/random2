# v5 cloud builders: shared brief (read with your module file)

You are one of three parallel cloud builders for Team Xanarchy's Cambridge Battlecode 2026 bot (this repo). Each builder works on ONE module in its own bot copy; a later session integrates the admitted modules into v5. Target for v5: Wed 21 Oct 18:00 BST.

Setup: `bash tools/cloud/setup.sh`. Then read: FACTS.md (rules; "Measured in engine arenas" section is engine-verified), STATE.md, PLAN-v4.md ("Draft 3" section only), notes/step0-deaths.md (where our deaths come from), results/arena-1020.md (monster targeting, respawn, Prism, Heart of Ruin), results/v4-cloud.md (how v4 was traced and fixed: copy that method), VERSIONS.md (v4 entry). Rules sources: docs/rules/ENGINE-1.0.md and sdk/cpp/cambc.h supersede docs/rules/GameRules.md; never invent an SDK method.

Rules of work:
- `cp -r bots/v4 bots/<your copy>` (no .cambc). Never edit bots/v4 (live release), bots/v3b or bots/v3b-e1.
- Every behaviour change sits behind a named switch (`#ifndef P_X / #define P_X 1`, with a comment naming the cause), like v4's P_UNSEEN_HIT. Keep changes inside your module's files where possible so integration is easy; list every file:line you touch.
- Trace before you fix: decode a lost game (tools/top10/decode.mjs, see tools/README-decode.md) and find the cause, as v4-cloud.md did for corridors.
- Fuel: no turn over 80,000 after round 1 from your code; the gate fails over 100,000.
- Tests use `--jobs 3`; commands over 2 minutes run in the background. Head-to-head: `python3 scripts/mapgate.py <copy> all --jobs 3 --salts 0,1,2 --panel <opp> --out results/<copy>-vs-<opp>.json` (126 games: 21 maps x 2 sides x 3 salts).
- Token use low: small outputs, never print whole files or logs.

Admission rule for a module (fixed before testing; report each line as PASS/FAIL):
1. `python3 scripts/gate.py <copy>` PASS (42/42 vs starter-cpp, 0 faults, mapid correct).
2. Head-to-head vs v4: >= 70/126 (55.6 %), OR deaths per game down >= 15 % with >= 63/126.
3. Head-to-head vs v3b-e1: >= 80/126 (v4 scored 86).
4. No map at 0/6 vs v4.
Self-play is a proxy; the real-team panel decides the release, so report deaths per game, gem margin per game, and per-map wins beside the win rate.

Output: results/v5-<copy>.md (cause traces with rounds and positions; each switch and file:line; ablation of each switch you add; the admission table; what still fails). Commit your bot copy (source only), result JSONs and the report to this session's own branch and push it. Do not push to main. Final summary under 250 words.

Update (lead, 10:10 BST 20 Oct): REAL-TEAM evidence now exists. v4 played 160 paired games against 8 mid-ladder teams (notes/panel-v4-vs-v3b.md): 37-123 (v3b 29-131), deaths 23.0 a game (v3b 37.9), gems 499 vs their 782. Large maps are near even (corners, expanse, lanes, monolith); the gap sits on four small contact maps: contest 0-8 (0 gems, 51.5 deaths), fragment 0-8 (53.6 deaths), hooks 0-8 (55.6 deaths, 5 tributes), gold_rush 0-8 (45.4 deaths). The replays of those games and of v4 vs the top 10 are in data/real/ (README there): trace REAL losses first, then confirm fixes in self-play. Add to your report the per-map deaths and gems on those four maps in your head-to-heads.
