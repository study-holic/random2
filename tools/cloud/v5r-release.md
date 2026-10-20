Read tools/cloud/v5-common.md for setup and measurement tools. Do not run cambc login, submit, matches, ladder or match-groups. Push to this session's branch. Stop by 23:00 BST (`TZ=Europe/London date`).

## Why
The platform pool is now **35 maps** (maps/ on main; 14 new or returning: packs, pilgrimage, quadrants, reverse_spiral, rows, rushbait_v2, shortcut, split, stampede, starfish, starvation, swirl, the_farm, yellow_brick_road). Ranked games already use them. The upload candidate `bots/v5` (S2 build B, results/v5-s2b.md) fails the 35-map gate on the laptop (results/gate-v5-20261020-183901Z-win.json):
- **shortcut:** lost both games to starter-cpp, 0-0 gems in 2,000 rounds (gold tiebreak). `bots/s1` wins both (564 and 210 gems to 94 and 11), so S1's P_S1_BANK_REACH very likely fixes it. Confirm the cause with a trace.
- **mapid:** the 14 new maps print `mapid=unknown`, and gate.py requires mapid = map name.
Also weak against starter-cpp: split (36-45 deaths a game), starvation (about 400 gems in 2,000 rounds).

## Build bots/v5r = bots/v5 + exactly these changes
1. **P_S1_BANK_REACH** (default 1) and **P_S1_NO_SELF_BLOCK** walk-out only (P_S1_NSB_WALK 1, P_S1_NSB_STEP 0), ported from bots/s1 with the same switch names, comments and behaviour, plus only the helpers they need (e.g. `awake_tiles`). Nothing else from S1: no AUCTION, THREAT_COST, ONE_CAPTURER, SPAN or measurement salt. S2's edits in bot.cpp sit next to these, so port by hand and keep both.
2. **mapid for the 14 maps:** new MapId values with map_name = the pool name, exact-table entries, no setup function (they play the default plan, as `unknown` does now). Keys {w,h,groups} read on the laptop: packs 31,31,8; pilgrimage 50,65,23; quadrants 45,45,21; reverse_spiral 30,30,14; rows 50,50,9; rushbait_v2 30,80,18; shortcut 40,40,21; split 45,35,32; stampede 60,50,13; starfish 50,50,16; starvation 80,80,14; swirl 45,30,22; the_farm 50,40,28; yellow_brick_road 40,30,28. The old ids (rows, pilgrimage, stampede, swirl, packs_v2, rushbait) belong to the retired layouts: do not map the new layouts to them.

## Tests (35 maps)
1. `python scripts/gate.py v5r --jobs 3`: must PASS, shortcut included.
2. v5r vs v5, both sides, salts 0-2 (210 games): wins, deaths and gem margin per map. Name the maps where any game differs from a mirror of v5. BANK_REACH should change only maps with an asleep Monolith near an altar.
3. Unopposed vs bots/idle (35 maps × 2 sides, salt 0) for v5 and v5r: gems and round to 1,000 per map.
4. v5r vs v4, 210 games, next to the null (v4 vs v4 scores 73/126 on 21 maps; measure it on 35 if time allows).

## Admission (fixed before testing)
1. gate.py PASS on 35 maps.
2. Unopposed: no map more than 10 % slower than v5, and shortcut above 0 gems.
3. Deaths per game vs v4 not more than v5's (results/v5-s2b.md: 17.0) + 10 %.
4. vs v4 at least 45 % (rejection gate only).

Write results/v5r.md, admission table first, then per-map tables for the 14 new maps, then the trace of shortcut's cause. If a line fails, say so plainly.

## Added by the lead (laptop probe, 20 Oct evening)
`bots/s1g1` = bots/s1 with AUCTION, THREAT_COST and ONE_CAPTURER at 0 (v4 + the two fixes only), vs starter-cpp, salt 0 (results/s1g1-newmaps-vs-starter-20261020.json):
- shortcut: side A **lost 84-115 with 29 deaths**; side B won 376-32. So the pathing change alone may not secure shortcut; check v5r on both sides and 3 seeds, and trace what kills us there.
- starvation (80×80): **fuel_max 100,000 with 2 exhausted turns** on side A, 99,508 on side B. v5 peaked at 72-91k there. Find which path field or loop the change adds on large open maps before anything else; the gate fails on any exhaustion after round 1.
- stampede: 0-0 and 80-80 gems (won on gold); pilgrimage 371 and 243.
Unranked groups always play the live submission (no version option), so v4 cannot run as a concurrent control on the panel.

## Then build R1 and R2a (PLAN-v4.md Draft 5; consultants want the pathing change released first and on its own)
- **bots/r1** = bots/s1g1 (on main: bots/s1 with P_S1_AUCTION, P_S1_THREAT_COST and P_S1_ONE_CAPTURER at 0 in params_override.h) + the same 14 map ids + the starvation fuel fix you find for v5r. Gate on 35 maps; r1 vs v4 (210 games; reject below 62/126 on the old 21); unopposed vs bots/idle on 35 maps.
- **R2a** = v5r with P_S2_DRAFT_CORE 0 (v4's drafts): gate only. R2b is v5r itself.
Push r1 first if v5r is still failing: R1 is the first upload.
