# HANDOFF (lead chat 1 -> next chat, Fri 23 Oct 2026, 09:30 BST)

Read with CAMBC-LEAD-PROMPT.md (version 10) and CONTEXT.md. LEDGER.md holds every judgement, pre-registration, and result with times.

## State

- G0 done. Xans delegated actions 2 onwards (08:41). Platform engine is 1.0.3 (same build as cambc 3.0.5). v5r uploaded 08:53 as platform "v4" and live from 09:03; rollback to platform v3 (our v4) by the rule pre-registered at 08:24 (LEDGER).
- A2.22: no separate near-rating unranked panel (ranked games are that panel); unranked groups stay on the top 10, read by gem share and production per 100 rounds. A2.8: cloud cap 0 until G2.
- Schedule: Sprint 2 lock Sat 24 Oct 23:59 BST; Sprint 3 lock Sat 31 Oct 23:59 GMT; UK qualifiers 11 Nov (Xanarchy eligible); finals 18 Nov. Sprints use unseen maps outside the pool.
- S1 done (G1 grader packet ready: project/lead/G1-GRADER-PACKET.md): census (ENGINE-FACTS.json), readers rerun on 1.0.3, MAPS.csv with renders, patch notes checked (0 mismatches), every B1a row reconfirmed, every B1b row settled by arena (lab3 modes 1 to 8). B4 map rows checked.
- Baselines on 1.0.3: F1 v4 75/80, v5r 77/80, v22 73/80 (none passes); v5r vs v4 52/80; v5r vs v22 46/80; v22 vs v4 39/80. Engines 1.0.2 and 1.0.3 give identical games (160 of 160).
- Xans (09:03): two-team membership with shared code is allowed as a team merger; no 1.0.3 change note on Discord.
- v5r live since 09:03 BST. Held-out proxy: v5r's default plan (all unseen maps) matches v5r on the pool within noise and edges v4's; v5r is the Sprint 2 build among existing ones.
- Ladder 09:16: Xanarchy 1,344, UK 130 of 168; UK rank 8 = 1,825.
- S2 started: public-API index of ranked games for the top 20, Xanarchy, MicroStella since 21 Oct 05:30 UTC (scripts/s2_index.py -> data/s2; about 6,600 groups, running).

## Files

- random2 main (commits 2371ca1, 53fb18d, and the 23 Oct 09:30 commit): project/lead (CONTEXT, LEDGER, HANDOFF, G1-GRADER-PACKET), scripts/lead, bots/census, bots/lab3, data/lead (ENGINE-FACTS.json, MAPS.csv, MAP-GROUPS.csv, renders, s2 ladder and index), results/lead.
- Sandbox set-up: scripts/lead/setup_engine.sh (no settings change needed; GitHub toolchain archives fetched with curl). Baseline runner: scripts/lead/baseline.py, s0_queue.sh, show.py, equiv.py.
- Lab bots: bots/census (constants, groups, designs), bots/lab3 (MODE 1 to 8: wake and growth, turn order, ranged resets, capture/recall/crystal, Marcella, kill gold, golem payout, shop limits); forced-unknown builds v4-u and v5r-u (one-line patch in a/mapid.h).

## Open items

1. A merged team's record and rating (decides how the team with Nmesoma starts).
2. The 1.0.3 change list (none as of 09:03).
3. Per-match summaries for chosen ids: Xans's laptop runs scripts/s2_metrics.py fetch (needs `cambc login`).
4. Match-database range reads: blocked from sandbox and browser (signed CloudFront redirect, CORS). Public /api/v1/match-groups/<id> needs no login and gives map, rounds, end reason, engine build, drafts, scores, and ratings per match; summaries and replays need login.
5. G1 grader: Xans pastes project/lead/G1-GRADER-PACKET.md into a fresh chat (A2.15).
6. v5r's first 100 ranked games (rollback check due about 12:30 BST) and its per-map record against v4's.

## Next action

Finish the S2 index, then: draft and pick-rate tables per map for the top 20 and UK top teams; v5r's ranked record per map after 100 games (rollback rule); choose about 500 match ids (top-20 games on 1.0.3, our ranked losses) for the laptop summary fetch; then S3 (clear-time production model, archetypes, drafts).
