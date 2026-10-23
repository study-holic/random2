# HANDOFF (lead chat 1 -> next chat, Fri 23 Oct 2026, 09:10 BST)

Read with CAMBC-LEAD-PROMPT.md (version 10) and CONTEXT.md. LEDGER.md holds every judgement, pre-registration, and result with times.

## State

- G0 done. Xans delegated actions 2 onwards (08:41). Platform engine is 1.0.3 (same build as cambc 3.0.5). v5r uploaded 08:53 as platform "v4", selected for the next match; platform v3 (our v4) is the fallback. Rollback rule pre-registered 08:24 (LEDGER).
- A2.22: no separate near-rating unranked panel (ranked games are that panel); unranked groups stay on the top 10, read by gem share and production per 100 rounds. A2.8: cloud cap 0 until G2.
- Schedule: Sprint 2 lock Sat 24 Oct 23:59 BST; Sprint 3 lock Sat 31 Oct 23:59 GMT; UK qualifiers 11 Nov (Xanarchy eligible); finals 18 Nov. Sprints use unseen maps outside the pool.
- S1 nearly done: census (ENGINE-FACTS.json), readers rerun on 1.0.3, MAPS.csv with renders, patch notes checked (0 mismatches), every B1a row reconfirmed, B1b #1 to #8 and #10 to #12 settled by arena (lab3 modes 1 to 6). B4 map rows checked.
- Baselines on 1.0.3: F1 v4 75/80, v5r 77/80, v22 73/80 (none passes); v5r vs v4 52/80; v5r vs v22 46/80; v22 vs v4 39/80. Engines 1.0.2 and 1.0.3 give identical games (160 of 160).

## Files

- random2 main, commit 2371ca1: project/lead (CONTEXT, LEDGER), scripts/lead, bots/census, data/lead (ENGINE-FACTS.json, MAPS.csv, MAP-GROUPS.csv, renders), results/lead. A second commit adds lab3, the arena part 2 results, and this file.
- Sandbox set-up: scripts/lead/setup_engine.sh (no settings change needed; GitHub toolchain archives fetched with curl). Baseline runner: scripts/lead/baseline.py, s0_queue.sh, show.py, equiv.py.
- Lab bots: bots/census (constants, groups, designs), bots/lab3 (MODE 1 to 6: wake and growth, turn order, ranged resets, capture/recall/crystal, Marcella, kill gold).

## Open items

1. B1b #9 inventory and stat caps; B1b #13 golem payout.
2. One person in two teams, and shared code: ask the organisers (contact@battlecode.co.uk or Discord). Decides when and how the new team with Nmesoma starts.
3. The 1.0.3 change list (Discord).
4. Match-database range reads: blocked from sandbox and browser (signed CloudFront redirect, CORS). Public /api/v1/match-groups/<id> needs no login and gives map, rounds, end reason, engine build, drafts, scores, and ratings per match; summaries and replays need login.
5. G1 grader: a fresh chat receives the arena pre-registrations (LEDGER 08:16 and 08:58) and results (LEDGER 09:05) and nothing else, and writes verdicts (A2.15).
6. v5r's compile status and its first ranked games; the per-map comparison against v4's ranked record.

## Next action

Check v5r compiled on the Submissions page. Then finish S1 (B1b #9 with a farming lab mode; golem payout on lost_world) and start S2: confirm `cambc summary` fields on a downloaded summary, build the index from the public match-group API (scraper or browser), and plan the held-out map test that sprint builds need, since sprints use unseen maps.
