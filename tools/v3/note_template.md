# v3 data scope: all historical games of today's top-50 (written 19 Oct 01:15 BST; data to 19 Oct 00:07Z)

Scope only, no win-rate analysis (so no rating adjustment here). Raw responses: `data/v3/raw/`; scripts: `tools/v3/`; counts: `data/v3/scope_counts.json`.

## 0. Bottom line
- **Replays are not downloadable now.** Every match JSON carries `replayUrl` (`/api/v1/matches/{id}/bundle`), but it returns 401 `Sign in to watch or download replays` (tested 18 Oct 23:41 BST; `/summary` and `/log` also 401; `cambc matches list` says `Sign in first with cambc login`). On 15-16 Oct it was public: `intel/copied/cambc-top30-intel` holds 429 bundles from then. I did not run `cambc login` (an account action for Xans).
- So the pilot used 20 of those 429 (engine v0_0_2, 16 Oct), not top-10 v top-10 on engine v0_0_3. Of the 429, 1 is top-10 v top-10 (old map id); the 20 picked have one team in the top 10 and the other at ranks 15-26.
- Without replays I pulled all 22,848 ranked groups of the 50 teams and 6,761 group details. The decoder works on engines v0_0_1 and v0_0_2; v0_0_3 (all games since 16 Oct 22:20Z) is untested for want of a replay.
- "All" is feasible for metadata (about 2 h more of polite crawling), not for full action logs (about 55 s per game; 113,870 games = 1,700 CPU-h).

## 1. Current top 50 (ladder snapshot 18 Oct 22:39Z, 674 teams; id = first 6 hex of teamId; groups = ranked groups since 13 Oct, listing complete)
| # | team (id) | rating | groups | # | team (id) | rating | groups |
|---|---|---|---|---|---|---|---|
{{TOP50}}

42 of these 50 were in the 19:10Z sample behind `intel/meta-2026-10-18.md`; invisibleAgain (rank 9 then) is out and 8 teams entered (ranks 31-49). Group JSON holds rating before and after, so a cohort can be fixed by rating at game time rather than today's rank.

## 2. Counts (listing: `kind=ranked`, 25 groups per page, 1,315 requests, 4 min, 18 Oct 22:41-22:49Z)
| Item | Count |
|---|---|
| Ranked groups listed (distinct) | 22,848 (22,774 finished, 74 error); 13 Oct 09:30Z to 18 Oct 22:40Z |
| Team-groups summed over 50 teams | 32,187 (per team {{PT_MIN}} to {{PT_MAX}}, median {{PT_MED}}); ladder `matches` is 1 lower for every team |
| Groups with both sides in the top 50 (counted once) | 9,413; both top-10 1,702; at least one top-10 5,197 |
| Finished games (5 per group) | 113,870 |
| By day (groups, 13-18 Oct) | 1,447; 3,805; 4,559; 4,125; 4,623; 4,215 (18 Oct to 22:40Z) |
| Engine v0_0_1 (to 16 Oct 07:30Z) | 11,182 groups; both top-10 549; both top-50 3,118 |
| Engine v0_0_2 (16 Oct 07:55-22:10Z) | 2,448 groups; both top-10 225; both top-50 1,117 |
| Engine v0_0_3 (16 Oct 22:20Z on) | 9,144 groups = 45,720 games; both top-10 928 (4,640 games); both top-50 5,178 (25,890 games) |

Map ids (group details fetched: 6,761 groups, 33,805 games; a tiered, not random, sample, so shares are indicative):
- v0_0_3: {{CUR_0.0.3}} of {{N_0.0.3}} games are on the 17 current ids; all 17 appear. Both-top-10 games per current id: 241 to 306, median 271 (n=4,640, complete). Id list: `intel/meta-2026-10-18.md` section 2.
- v0_0_2: {{CUR_0.0.2}} of {{N_0.0.2}} on current ids ({{IDS_0.0.2}} distinct ids). v0_0_1: {{CUR_none}} of {{N_none}} ({{IDS_none}} ids). The others are retired maps.
- Detail coverage (have/listed): both top-10 1,702/1,702 (all eras); v0_0_3 both top-50 3,914/5,178; v0_0_3 at least one top-10 1,534/1,982; v0_0_3 all 4,266/9,144; v0_0_1 965/11,182; v0_0_2 316/2,448.
- Remaining 16,013 details: about 1.9 h at 2.3 requests/s (`fetch_groups.py` resumes, era v0_0_3 first).
- Unranked scrimmage groups are far more numerous: 30 probes (6 teams x 5 times, 25 groups each, so each probe is below 30) give 47-139 per hour for ranks 1-2, 20-97 for rank 5, 15-88 for rank 10, 4-39 for ranks 25 and 50, against 6.1 ranked per hour. Excluded from this plan; version and submission fields are null there too.

## 3. Replay availability, size and cost
- Public API: `replayUrl` present on 33,805 of 33,805 matches; fetch needs sign-in; the CLI needs `cambc login` (7-day session, so one login on Mon covers the week).
- Size from 429 bundles (15-16 Oct, engines 0.0.1/0.0.2): zip mean 17.2 KB, median 14.7, max 66.8; inner replay mean 95 KB; 24.5 zipped bytes per round (n=2,833 indexed bundles). Projection: both-top-10 v0_0_3 (4,640 games) 73 MB; both-top-50 v0_0_3 (25,890) 0.43 GB; all v0_0_3 top-50 (45,720) 0.75 GB; all eras (113,870) 1.9 GB.
- Download is request-bound, one request per game, at 3 per second: 26 min, 2.4 h, 4.2 h, 10.5 h for those four sets.
- Replays are only read as bytes by the official engine WASM (WASI, no preopens); never executed.

## 4. Decoder status
- `ext/tarot/tools/replay` does not run as is: `assets.sh` needs a `cambc.exe` in the repo root and `decode.mjs` knows engines v1 and v2 only. Adapted as `tools/v3/decode.mjs`: it loads the three engine WASMs bundled in cambc 2.2.5's visualiser (copied from a local `cambc view` to `data/v3/engine/cache2_2_5`; the server is stopped), picks the engine from replay header byte 10 (1, 2 or 3), and reads zips itself (no `unzip`).
- Tested: v0_0_1 x1, v0_0_2 x133 distinct (132 round-level, 20 of them also at full action level, plus 1 more full), 0 failures. v0_0_3: WASM and decoders load (`engine_check.mjs`; snapshot 184,696 bytes as for the others) but a real v0_0_3 replay was not available, so that decode is **untested**. First job after login: decode one v0_0_3 bundle.
- Validation: final submitted gems equal the platform `score` for both sides in 20/20 full and 132/132 round-level games; winner 20/20.
- Speed (machine at about 91% load from other jobs, 14 logical cores): full action log, every timeline step: mean 55.1 s per game, median 42.5, range 7.0-185.6 (3.7 ms per step, n=20). Round-level, one state per 13 steps: mean 2.9 s, median 1.9, max 22.4 (n=132, mean 15,462 steps). Idle single game (v0_0_1, 6,676 steps, n=1): 18 s full, 0.64 s round-level. Cost follows the number of seeks (about 1.2 ms idle), not game length. I found no "2 s a game" claim in Tarot's docs.

## 5. Fields per game
| Field | Without replay (match-group JSON, 1 request per 5 games) | Needs replay |
|---|---|---|
| Identity | match, group, map id and name, patchId (engine), created/started/finished time, rounds, endReason (gems, tiebreak_gold, tiebreak_gems, tiebreak_second_team) | |
| Sides | team id and name, a/b, result, score (= submitted gems), draft in pick order, botError (59 of 67,610 sides), group wins and ratingBefore/After | |
| Per champion per round: tile, hp, bag, gold, kills | | yes |
| Deaths (round, tile, bag lost), recalls, tributes (round, bag size, tile), altar locks, camp captures, purchases, wards, casts | | yes |
- Availability by engine era (share of games in fetched details): endReason v0_0_3 {{ER_0.0.3}}, v0_0_2 {{ER_0.0.2}}, v0_0_1 {{ER_none}}; draft and score v0_0_3 {{DR_0.0.3}}, v0_0_2 {{DR_0.0.2}}, v0_0_1 {{DR_none}} (4,825 v0_0_1 games carry only winner, map, rounds).
- Always null or absent: kills, deaths, gems collected (`statistics` removed about 05:30Z 16 Oct; 0 of 33,805 matches have it), `version` and `submissionId` (so no bot version per game), bot logs (`hasLog` false), per-game rating (group level only).
- Draft pick order: the JSON order equals the replay's draft order in 20/20 pilot games (the CSV in `intel/` sorts it, losing the order). Champion body types: draft id 3 (Cassi and Issac) has two bodies (types 3 and 4); Stellar 5, Prism 6, Marcella 7, Willow 8, Ren 9, Varg 10, Tavi 11; ids 0-2 map to themselves.
- Untested lead: `summaryAvailable` is true for all v0_0_2 and v0_0_3 games and `cambc matches summary --json` exists (sign-in). If it returns kills and gems it replaces part of the decoding.

## 6. Pilot (20 replays, engine v0_0_2, 16 Oct 10:30-14:10Z, all on current map ids)
Full table: `data/v3/pilot/pilot_table.txt`; events: `data/v3/pilot/derived_full.json` (20 games), `derived_s13.json` (132 games).
- Extracted per game (from round snapshots; 5.9 KB of events): per-champion death round, tile, hp, bag lost; tribute round, bag size, tile; recall completions (teleport to spawn with hp restored); consumable buys and uses; camp captures; purchases. Mean over 132 games: 18.3 deaths, 31.4 tribute events, 50.2 recalls, 3.7 consumable uses, 59.8 captures.
- Example (monolith f7bc3bb7, 394 rounds, hello v Ely2sh, final gems 1004-388): death `[224, champ 6, team 1, type 10, tile (37,19), hp 311, bag 22]`; tribute `[49, team 1, champ 11, type 6, bag 22, tile (31,29)]`; recall `[54, champ 8, team 1, type 3, from (49,17), hp 540]`; use `[87, champ 7, team 1, cons2, tile (32,28)]`. First tribute round 49 (team 1) v 61 (team 0); 13 v 20 tribute events; mean bag 29.8 v 50.2.
- Action log (full mode only): kind 4 move, 5 attack, 6 cast (slot and type recorded), 12 ward, 13 shop, 20 consumable use. Kind 20 matches the consumable-mask clears in 20/20 games and its `to` is the altar tile (repeated tiles), so a lock is consumable bit 2.
- Caveats: kinds 9, 14, 16, 23 are not decoded. Recall completions are a tile-jump heuristic checked by hand on 4 events; channel starts (kind 11, 1,262) are fewer than detected recalls plus tributes (1,557), so recall counts are unvalidated. Deaths are at least the opposing kills in 20/20 games (the rest are camp deaths). Round-level events are exact to one round, not one turn.

## 7. Pipeline proposal
| Step | Script (`tools/v3/`) | Cost |
|---|---|---|
| 1 ladder, listing, details | `fetch_ladder.py`, `fetch_listings.py ranked`, `fetch_groups.py --deadline` (done except 16,013 details) | 1.9 h more, 2.3 req/s, about 140 MB in all |
| 2 tier lists | `select_games.py`: T1 both top-10, T2 both top-50, T3 any top-10, T4 any top-50, all v0_0_3 | seconds |
| 3 login (Xans), test 3 ids | `cambc login`; `fetch_replays.py T1.txt --limit 3` (UNTESTED; fails cleanly when signed out) | 5 min |
| 4 download | `fetch_replays.py <tier>.txt --rate 3` | T1 26 min, T2 2.4 h |
| 5 decode | `decode_batch.py <tier>.txt --workers 3 --stride 13`, then `derive_events.py` | T1 3.7 CPU-h (1.3 h wall at 3 workers, loaded); T2 20.9 CPU-h |
| 6 validate | `pilot_report.py` (gems and winner against platform score) | minutes |
- Sample if "all" is infeasible: (1) T1 in full, round-level: 4,640 games, 241-306 per current id; (2) T2 round-level at stride 26 (not yet timed; expect about half of stride 13); (3) full action logs on about 200 chosen games (3.1 CPU-h), from T1 losses or the opponents Xans expects; (4) skip v0_0_1 and v0_0_2 games (other engine and balance) and unranked scrimmages.
- Keep events plus gzipped snapshots (358 KB per game raw at stride 13; 4,640 games = 1.7 GB raw).
- Schedule: login Mon 19 Oct; T1 decoded by about noon; T2 by Tue evening; analysis Wed-Thu; build before the lock (Sat 24 Oct 23:59 BST). At most 3 decode workers while simulations run.
- Open decisions for Xans: log in; accept T1 and T2 rather than "all"; whether to spend CPU on full action logs; fix the cohort by rating at game time.
