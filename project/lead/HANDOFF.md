# HANDOFF (lead chats 1 and 2 merged -> next chat, Fri 23 Oct 2026, 11:13 BST)

Read with CAMBC-LEAD-PROMPT.md (version 10) and CONTEXT.md. LEDGER.md holds every judgement, pre-registration, and result with times; lines after the 11:13 MERGE line carry [c1] or [c2] for the chat that wrote them.

## State

- Two lead chats ran in parallel from 09:34 to 11:10: chat 2 started from chat 1's 09:30 HANDOFF in a new sandbox; chat 1 kept working. Chat 1 merged both at 11:13 and pushed the result. One lead chat from here.
- G0 done; Xans delegated actions 2 onwards (08:41). The platform has run engine 1.0.3 since 22 Oct 07:00 to 07:10 UTC. v5r has been live as platform "v4" since 09:03 BST: 11 of its first 20 ranked games (chat 2), 34 of its first 60 at 11:05 (chat 1; pace 40.2, opponents' mean rating 1,352). Rollback rule of 08:24: rollback_check.py (chat 2, due 12:25) and team_record.py (chat 1, scheduled 12:35).
- Schedule: G2 Sat 24 Oct 08:00; G3 Sat 12:00; Sprint 2 lock Sat 23:59 BST (proposed last upload 20:00); Sprint 3 lock Sat 31 Oct 23:59 GMT; UK qualifiers 11 Nov; finals 18 Nov. Sprints use unseen maps.
- S1 done. project/lead/G1-GRADER-PACKET.md carries the S1 pre-registrations and results, the held-out proxy, arena parts 1 to 3, and D1. Xans pastes it into a fresh chat at G1 (A2.15); no lead chat grades it.
- S2 index done (chat 2): 31,220 ranked matches since 21 Oct 05:30 UTC for the top 20, UK top 8, Xanarchy, and MicroStella; tables pick_rates, team_drafts, finish_by_map, xanarchy_by_map. Chat 1's smaller index (22,465) is superseded; its pace figures and Monolith-altar finding stand in CONTEXT 3f.
- S3 v1 done (chat 2): production model, calibration, v5r at 0.76 of top teams' production (12 maps under 0.6), sensitivities, k = 6 archetypes.
- D1 (chat 1): the top-20 draft on v5r's role code gives 48/80 against v5r (pre-registered lines 49 and 31); deaths halve and pace rises 22 per cent head to head; 75/80 against the starter. Verdict for the grader.
- F2 references (chat 1): v5r against the 19 random2 panel bots 1,284 of 1,520; against the 9 competitor bots 415 of 720, with deysag-baseline (Howrah River Trading, rating 1,663) winning 79 of 80 at top-20 farming rates. deysag-baseline is the local reference opponent for S3 and S4.
- S2 measures, partial: 241 top-20 vs top-20 games (laptop fetch 1); CONTEXT 3j. v5r takes 0.39 to 0.42 camp damage per point dealt (top 20: 0.23 to 0.26). Fetch 2 (chat 2's list, with champions.csv) is running on the laptop.

## Files

- random2 main, merge commit of 23 Oct 11:15: project/lead (CONTEXT, LEDGER, HANDOFF, G1-GRADER-PACKET); scripts/lead (chat 2: s2_index.py parallel version, s2_tables.py, s2_select.py, s3_model.py, s3_archetypes.py, rollback_check.py; chat 1: team_record.py, panel_show.py, competitors.sh, s2_pace.py); data/lead/s2 (index, ladder, pick_rates, team_drafts, finish_by_map, xanarchy_by_map, fetch_ids, record-2026-10-23T08); data/lead/S3-PRODUCTION.csv, S3-ARCHETYPES.csv; results/lead (d1-meta-*, panel-v5r-*, competitors.tsv, v5r-meta.patch, v5r-u.patch).
- Sandbox set-up (each chat): sparse clone of random2 (A2.3 paths), scripts/lead/setup_engine.sh (about 2 minutes), bots copied from the main ZIP into ws305/bots; v5r-meta and v5r-u rebuild from the patches. The raw groups.jsonl (41 MB) stays out of the repo; s2_index.py rebuilds it in about 10 minutes.
- Laptop: fetch_ids.csv from chat 2 (500 ids; the copy Xans holds) with `python s2_metrics.py fetch fetch_ids.csv --out matchdb` after `cambc login`; returns matchdb/metrics.csv.

## Open items

1. A merged team's record and rating (decides how the team with Nmesoma starts).
2. The 1.0.3 change list, and whether it caused the top-20 pick shift between 1.0.2 and 1.0.3.
3. Laptop summary fetch: matchdb/metrics.csv.
4. Match-database range reads (laptop test with a signed link).
5. G1 grading in a fresh chat.
6. Rollback rule complete (19:30): part 1 51/100, part 2 1 of 6 on road_of_riches; v5r stays live (154/305 since 09:03).
7. F2 panel completion for v5r (random2 panel, then competitors).

## Next action

When metrics.csv arrives: per team and game, gems by source, gems lost, deaths, dead rounds, gold, and items for the top 20 and UK top 8 against our losses (S2 measures), each with its sample. Then S3's remaining items: per-mechanic effects that need data (kills, deaths, altar locks, shards) and model v2 with gold-to-shard growth; draft effects per archetype, with D1's deaths and pace figures as the local reference. Then S4a candidates against G3 (Sat 12:00), with S4b after Xans attaches history-S4b.zip.
