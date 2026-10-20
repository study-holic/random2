# Consult 23:40 BST 18 Oct: the v3 program for Sprint 2 (ChatGPT, DeepSeek, Copilot)

Attachment: `consult/pack/xanarchy-context-v3.txt` (project, v2 results, watched losses, the v3 program draft, full rules, Bot API index). Sent through Claude in Chrome at Xans's request.

## Shared prompt (each consultant also gets an angle line)

The attached file is the complete context of our Cambridge Battlecode 2026 bot: the rules, the C++ Bot API index, our code, test harness, every measurement, two watched losses and our draft program for the next version (v3). Please read all of it. You are an outside reviewer; be critical and concrete, and say where you are unsure. We have 6 days to the Sprint 2 lock (Sat 24 Oct 23:59 BST), one person plus AI coding agents, and 13 cores (about 110 local games a minute).

QUESTION: what should v3 be, and in what order should we build it? Options we see, in no particular order:
(a) opponent modelling from the full match histories of the top-50 teams (drafts, routes, bag sizes, timings);
(b) a fight layer: focus fire, stun-then-burst, keeping Cassi & Issac together, handling enemy Stellar;
(c) counter-drafting from revealed picks;
(d) opponent-tactic answers: recall peel, carrier ganks, altar-lock interrupts and stocking, capture denial;
(e) macro conversion: bag size per map, tribute timing against enemy positions, spending surplus gold;
(f) "precognition": predicting enemy routes and fights from spawn geometry, camp respawn timers and per-team models;
(g) per-map routing among our existing versions;
(h) an option not listed;
(i) none of these.

REQUESTS:
1) Rank the options by expected gain per day of work, with reasons tied to named rules.
2) Name each option's failure mode.
3) Say exactly what to extract from top-team replays to drive v3: features, and how to turn them into bot behaviour within 100,000 fuel a turn and 8 bytes of shared memory.
4) Say how you would test so that we don't fool ourselves, given that a no-op change flips about 7 % of results.
5) Propose one idea from another game or competition that maps onto a named rule.
6) Say which fact in the file, if wrong, would change your answer.

Angle lines:
- ChatGPT: "Take the angle of a game-AI designer: architecture for adaptability (state machines, utility AI, opponent modelling) under these constraints."
- DeepSeek: "Take the rules angle: which rule interactions are most exploitable and least used by the teams in the data."
- Copilot: "Take the code-and-data angle: which modules to write first in this C++ codebase, and the data pipeline for the replays."

## Answers (collected 01:25 BST 19 Oct through Chrome; full texts remain in the three chat tabs)

### DeepSeek (rules angle)
- Bottom line: v3 = a "conversion kernel" (spend gold, escort carriers, lock altars, deny respawns, then fight); full top-50 modelling is a Sprint 3 project.
- Ranking: (e) macro conversion > (d) tactic answers > (b) fight > (c) counter-draft > (g) routing > (f) precognition > (a) modelling; (h) a turn-order/priority scheduler as part of (d).
- Exploitable rules: lock timing with turn order (a lock placed before the tributor's next turn start cancels a pending tribute); occupied camp spawn tile delays the whole group's respawn (deny or reset enemy captures); stun cancels tribute, any damage cancels recall; carried gems pass to the killer and C&I die together (do not let both carry); Tavi field shopping.
- Extraction: per-round champion state and events -> small per-map tables (route lists 20-40 bytes a map in draft memory, tribute thresholds, buy lists); 8-byte shared layout: mode, target id, C&I anchor x/y, carrier role, lock flag, priority parity.
- Testing: pre-register falsifiers and reject a change that misses its own (swirl was admitted after missing its death falsifier); 5-6 salts for release; a lockbox held-out; train/test split by time on replays.
- Cross-domain: MOBA lane freeze / camp blocking -> "any occupied spawn tile delays the entire group".
- Fact that would change it: if the bot can learn opponent identity, (a)/(f) rise.

### ChatGPT (game-AI architecture angle)
- Verdict: objective-driven hybrid controller: a small hierarchical state machine (NORMAL, CAPTURE_COMMIT, TRIBUTE_APPROACH, TRIBUTING, RECALLING, ESCAPE, INTERCEPT) with hysteresis, plus a utility scorer over camps/altars: expected secured gems per round = P(capture) x gems / (travel + clear + 5 + bank time) minus risk and over-allocation terms.
- Ranking: (h) objective arbitration > (d) > (e) > (g) > (b) > (c) > (a) > (f).
- Most important missing diagnostic: the gem pipeline (opportunities -> captured -> carried safely -> tributed) per map, vs opponents.
- Draft memory for static planning (map signature, camp groups, altar coordinates, travel summaries, roles); 8 shared bytes as two 32-bit records (team objective, event beacon).
- Testing: ~20 no-op control builds for an empirical null; split replays by team and time; ablations of combined features.
- Cross-domain: Dota 2 camp blocking -> occupied spawn tile delays respawn (test with a falsifier).
- Fact that would change it: "damage does not interrupt tribute".

### Copilot (code-and-data angle)
- Verdict: an objective-aware combat bot, not a prediction bot. Order: current maps, fight layer, macro conversion, tactic answers, routing, counter-draft, replay features, route prediction, team models, precognition.
- Extract: draft responses P(response | enemy pick, map), opening routes (first 150 rounds), per-tribute bag/round/score/enemy distance -> safe tribute threshold, fights, lock value per map.
- Testing: a second rerun with new salts (3,4,5), held-out always, promote only if panel, held-out and strong pair all agree; watch losses.
- Cross-domain: StarCraft threat maps (danger field) for routes, tributes and recalls.
- Fact that would change it: the six old local layouts.

## Classification and checks (lead, 01:30)

| Claim | Class | Check |
|---|---|---|
| Conversion/pace matters more than prediction (all three) | agrees with the ladder data: v2 won 10/50 ranked games at 01:25 and reached about half the opponents' gems (`data/ladder/xanarchy-v2-games-0125.psv`) | data |
| Occupied spawn tile delays a group's respawn (DeepSeek, ChatGPT) | new tactic | GameRules l.193 "Any occupied spawn tile delays the entire group." Verified |
| Damage does not interrupt tribute; stun, death, lock do | agrees | GameRules l.223 and l.272. Verified |
| A lock placed before the tributor's next turn start cancels the tribute | new | GameRules l.237 "Placing it cancels existing tribute channels at that altar" + completion at next turn start (l.223). Consistent; engine test needed |
| Respawns wolf 40, elk 60, gem cluster 50, monolith 150 (ChatGPT) | agrees | GameRules monster table. Verified (gold cluster also 50) |
| No opponent-identity API (DeepSeek, ChatGPT) | agrees | not in the API index; unverified beyond that |
| Threat map / danger field (Copilot) | contradicts ladder evidence | a former teammate reported their threat layer neutral or worse on the ladder (their code base, not ours) |
| Marcella bonus vs Stellar (Copilot) | contradicts | Marcella's stun immunity does not counter Stellar damage; the teammate reported Marcella counter-drafting neutral or worse |
| Gem clusters are the cheapest gems | lead's own reading | GameRules: gem cluster 500 HP, 0 strength, 24 gems (48 gems per 1,000 HP) vs wolf 16, elk 15, monolith 10. Gold clusters give 0 gems |

What changed in the plan: `PLAN-v3.md` v1 (01:35) puts pace (gems per round on each map) first, measured against real ladder teams through unranked matches; then denial (locks, interrupts, spawn blocking); counter-draft from the 9,245-game draft data after that.
