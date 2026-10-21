# Reset, 22 Oct 00:30 BST: start here

Xans asked for a reset after the 20-22 Oct sessions spent heavily (over $150 of cloud tokens alone) and did not lift the rating. Read TIMELINE.md first (the full story: every submission, test, consult, assumption and mistake), then this file, then docs/rules/PATCH-NOTES-1.0.md. Treat everything else in the repo as data to audit, not as decisions.

## Ground truth (checked 21-22 Oct)
- **Engine and pool:** engine 1.0.2, CLI and SDK 3.0.4 (3.0.1 is archived in archive/sdk-3.0.1). The pool is 40 maps; gem_fields, gemfight, great_wall, lost_world and road_of_riches were added 21 Oct.
- **Rules sources:** docs/rules/PATCH-NOTES-1.0.md, then sdk/cpp/cambc.h, then GameRules.md, which lags.
- **Ladder** (21 Oct 23:38): rating 1,356, rank 570 of 740. Rank 100 sits at 1,743, rank 50 at 1,831, rank 20 at 1,957.
- **Live bot:** v4, uploaded 20 Oct 09:46.
- **Sprint 2:** locks Sat 24 Oct 23:59 BST; single elimination, 15 games per pairing.

## Data (engine named on each line)
- **Our ranked record** (`py -I -X utf8 tools/ranked_summary.py` after `py -I tools/v3/fetch_team.py <team id> xanarchy`; first move data/v3/raw/listings/ranked/<team id> aside, because the listing cache goes stale):
  - since patch 1.0.1: 405 wins in 830 games (49 %);
  - under 30 %: shortcut 1/22 (0 gems every game), fragment 2/24, broken_curve 2/20, elk_arena 3/29, gold_rush 4/28, reverse_spiral 5/22, packs 6/28, split 6/23, swirl 6/22, starfish 7/24, pilgrimage 5/17; gemfight and great_wall 0/8;
  - over 75 %: the_farm 29/32, expanse, interlock, yellow_brick_road, ambush, islands, rows, monolith;
  - we draft Cassi & Issac in every game.
- **v4 against the top 10** (engine 1.0.0, 180 games; results/imitate.md):
  - record 2-178;
  - gems tributed 201 vs 976; pickups 291 vs 1,047, mostly elk and gem clusters;
  - deaths 23.4 vs 4.3; dead 23.8 % vs 3.9 % of champion-rounds;
  - conversion 68 % vs 88 %;
  - the top 10 recall at full health to shop and to bank; v4 recalls only to heal, at about 35 % health.
- **Candidates** (none uploaded):
  - **v5r** = v5 + pathing fixes + map ids.
    - Engine 1.0.2: 52/80 vs v4. Against starter-cpp it wins 77/80 (v4: 74/80), with 1 fuel-exhausted turn (v4: 2).
    - It loses road_of_riches to starter-cpp, 368-840 with 32 deaths.
    - RESULTS.md, 22 Oct 00:20.
  - **r1** (v4 + pathing), **im** (+ P_I_PRESS: 81/126 vs v4 on engine 1.0.0), **v6** (contest module) and **wd** (watchdog; failed 2 of its 5 admission lines).
- **Engine fact:** going past 100,000 fuel keeps the turn's actions and freezes that champion for about 10 rounds (FACTS.md).

## What went wrong (lead's account)
1. **The maps were never studied.** Only sizes, group counts and win rates were read; no layout, camp-type or golem placement was examined, and no game on them was watched.
2. **Nothing was re-audited after the patches.** Decisions, drafts and per-map setups were not checked against the 1.0.x changes: wolf gems 22 → 4, gem clusters, golems, kill reward 100, champion stats. Most of v4's logic predates them.
3. **The consultant prompts led the consultants.** They listed our own options and our framing.
4. **Outside ideas were thin.** Cross-domain material (MOBAs, other Battlecodes, other repos Xans supplied) was used little.
5. **We measured against the wrong opponents:** self-play, starter-cpp and the top 10. The rating band's own games (830) were ignored until 21 Oct.
6. **The upload gate was absolute, not relative to the live bot,** so nothing shipped after 20 Oct 09:46.
7. **Too many parallel cloud sessions with long reports;** the cost was out of proportion to the progress.

## Reset order (Xans decides each step)
1. **Budget first.** Xans sets the cloud and token cap for the rest of Sprint 2.
2. **Critic audit of every standing decision and fact,** in the context of patch notes 1.0.0-1.0.2, SDK 3.0.4 and the 40 maps.
   - Scope: FACTS.md, PLAN-v4.md Drafts 1-5, v4's per-map setups and drafts, params, and the candidates' claims.
   - Mark each item keep, stale or wrong, with the rule line it rests on.
3. **Map study of all 40 maps.**
   - For each map: the layout image; camp groups by type (wolf, elk, gem cluster, gold cluster, golem, Monolith) with their gem and gold value under 1.0.x; altars; spawn-to-camp and camp-to-altar distances; choke points.
   - Join this with our ranked record per map.
   - Watch one ranked loss on each of the 6 worst maps.
4. **Outside ideas:** MOBA, RTS and earlier-Battlecode sources and Xans's repos, each mapped to a named 1.0.x mechanic, with a source.
5. **Consultant audit with neutral prompts:**
   - give them the rules, the patch notes, the map facts and the ranked data;
   - ask open questions, with no option list of ours;
   - the critic checks each prompt for leading before Xans sends it.
6. **Upload decision on v5r** (numbers above), or on whatever the audit puts first.
