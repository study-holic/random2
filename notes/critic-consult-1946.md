# Critic review: consult/1946-sprint2-priorities.md (Prompt 1 and Prompt 2 bodies)

Reviewer: critic, 20 Oct ~19:58 BST. Read-only check against notes/panel-v4-top10.md, results/v5-s3-diag.md, v5-s1.md, v5-s1-diag.md, v5-s2a.md, v5-s2b.md, gate-v5-20261020-183901Z-win(-mapgate).json, FACTS.md, GameRules.md, STATE.md, CLAUDE.md.

## 1. Leading or framing wording (old -> new)

P1 = Prompt 1, P2 = Prompt 2.

| # | Where | Old | New | Why |
|---|---|---|---|---|
| 1 | P1 "Our bot with no opponent" | "Two observed behaviours cause part of this" | "Two behaviours seen in traces on these maps:" | States a cause as settled |
| 2 | P2 same | "Traced: carriers stand ... (pathing treats the monolith's range as a wall) ..." and "A fix for both raises ..." | "Seen in traces: ..." and "A change that removes both raises ..." | "fix" names them defects, "treats ... as a wall" is a code-reading, not a measurement |
| 3 | P1 and P2 | "A change that removes both ... and slows no map" | "A change that removes both: unopposed gems 1.5 to 1.8 times on three maps, +12 % on one; unopposed median rounds to 1,000 -1.3 %, corners +15 % (slower)" | "slows no map" is false against v5-s1.md (corners +15 %, interlock +9 %, collision +4 %). That is a fact error that favours option 1 |
| 4 | P1/P2 tables | "The two fixes" (used 4 times) | "Change A (altar standoff and spawn-box exit)" | Fix = defect framing; name the content once |
| 5 | P1 contest row | "75 (73 is v4's own)" | "75 (v4 salted copies score 73)" | "v4's own" reads as a baseline owned by us; the 73 is the salted copies |
| 6 | P1 and P2 | "Our bots already use up to 94,000" / "peaks are 90,000-97,000" / "peak fuel 97,209" | One figure in both: "turn-fuel peaks in tests: 93,816 to 97,728 of 100,000" | "already" implies a ceiling argument, and the two prompts disagree. Sources: 97,209 (s1), 97,728 (s2a), 93,816 (s2b) |
| 7 | P1 and P2 constraints | "so ranked play has time to move the rating" | delete; keep "Last large change live by Fri 23 Oct 04:00 is the team's intended date" | Gives a reason that favours early, many small releases |
| 8 | P2 request 2 | "including fuel or code-risk failures" | "including failures of any kind" | Steers toward options that touch code the most (1, 6) |
| 9 | P2 data | "Our bot against salted copies of itself scores 73 of 126, not 63" | "...scores 73 of 126 (an even split would be 63)" | "not 63" reads as a correction aimed at a reader who has seen our analysis |
| 10 | P1 draft line | "Top-10 draft frequency: ... Vendetta 3 %. Our live bot opens with Vendetta on 14 of 21" | Add the next fact beside it: "Tavi 33 %, Prism 10 % in the top-10 sides (notes/top10-2026-10-20.md l.45); core-draft-only self-play 54 of 126" | Placing 3 % beside 14 of 21 invites a draft-is-wrong conclusion; the opposing datum is in the table but not next to it |
| 11 | Both | "Top 10" in every label, when n = ranks 2-10 (9 teams) | "ranks 2-10 (9 teams)" everywhere | Accuracy; rank 1 is not in the data |
| 12 | P1 request 1 | "Rank the options and give your reasons." | keep; add "or say they cannot be ranked from this data" | Forces a ranking even where the data does not support one |
| 13 | Both | "Please be concrete and critical, and tell us where our facts look unreliable" | keep | Neutral |

No use of "problem", "weak", "only (as a judgement)". "only" appears factually ("its only gem sources", "only altar").

## 2. Option balance

- Option 1 is first, is the only option with a method attached ("each judged against the top 10 in unranked games"), and in P1 has the longest text (about 28 words against 14 to 25 for the rest, and a parenthetical list of six components). Rewrite as one line of comparable form: "Release the measured changes in the table, one at a time or together."
- Evidence asymmetry (more serious than length). The change table (8 rows, about a third of each prompt) is evidence for option 1 only. Options 2, 3, 5 and 6 have no dedicated measurements; option 4 has the new-map paragraph. Add, in neutral form and equal size, one datum bearing on each: option 2 (fights opened 3.7 vs 19.6, ability share row 67 vs 75 without), option 3 (camps cleared 10.2 % vs 17.1 %; gems picked up 269 vs 1,112), option 5 (top-10 recall 15.8 % vs 4.6 %; draft table), option 6 (auction build 78 vs fixes alone 74, within noise; corners +15 %). Or cut the table to its totals.
- Both prompts list options in the same order. Reverse the order in P2 (6 down to 1) so order effects are not common to all four consultants.
- Options 7 and 8 are fine.

## 3. Fact spot-check (8 numbers plus extras)

| Claim in prompt | Source | Result |
|---|---|---|
| 2-178; 201 vs 976 gems; 23.4 vs 4.3 deaths; 11.6 vs 33.6 tributes; 58 vs 3 lost | notes/panel-v4-top10.md "all" row | Match |
| Ladder 1,429, rank 481 of 720, 258 games; r10 1,991, r20 1,919, r50 1,795, r100 1,713 | STATE.md l.11 | Match. 1,388 / 536 of 716 at 09:36 not verified (no source found) |
| Gems picked up from camps "about 270 / about 1,175" | v5-s3-diag.md l.15-16: 269 vs 1,112 (the table also says 85 % for them) | **Mismatch.** Them: 1,112, not 1,175. Use "about 270 / about 1,110" |
| Tribute share 59 % / 85 %; 81 % champion kills; 146 locks, 5 cancelled; time shares 22.2/5.1, 48.5/46.3, 10.2/17.1, 4.6/15.8, 9.4/6.8; fights 3.7/19.6; 47/79 % | v5-s3-diag.md l.15-16, 26, 41-64 | Match |
| Gear 1.9 vs 5.3 at round 200 | panel-v4-top10.md says gear is NOT decoded; v5-s3-diag not checked line by line | Unverified in the files read; check before sending |
| 70 wins, 17.0/22.9, +132, gear 2.71 vs 2.25, 4.0 locks | v5-s2b.md l.105-117: 70, +132, 17.0 (22.9), 4.0, 2.71 | Match. But the same file shows the pre-fuel-edit build scoring 78 / +146, and the "2.25" gear for v4 is not in the table I read |
| 82 full; 61 / 68 / 78 without spend / locks / gate | v5-s2a.md l.22-30: 82, 61, 68, 78 | Match. Omitted: v5-s2a.md l.82 caveat that the 82 build chose its draft exceptions from the same games ("optimistic"); v5-s2b.md l.5 "variants with the same intent ranged 71 to 78 of 126"; v5-s1.md l.134 "5 wins in 126, about one standard error". These are evidence against option 1 and the prompts omit them |
| Core draft only 54, 21.1/21.3, -21 | v5-s2a.md l.19 | Match |
| Task allocation 78, 20.4/21.9, median -1.3 %, corners +15 %, 15 of 21 (12 before), peak 97,209 | v5-s1.md l.9-15, 42-44 | Match. "The two fixes alone 74, 20.3/21.9" also matches (l.131) |
| New maps "one game a side": shortcut 0, stampede 80, pilgrimage 243, reverse_spiral 385, starvation 400, rushbait_v2 444, split 759, packs 974 | gate-v5 mapgate json | **Selective.** These are side A only. Side B: stampede 0, pilgrimage 298, reverse_spiral 372, starvation 404, rushbait_v2 1,007, split 807, packs 1,026. The prompts say "one game a side" yet give one number. Give both sides or say "side A" |
| P1 "swirl 36 deaths" listed with the sub-1,000 maps; "the other five reached 1,000" | json: swirl 1,037 and 1,013 (reached 1,000); the other five are quadrants, rows, starfish, the_farm, yellow_brick_road | **Misleading in P1** (swirl has no gem figure, so it reads as short). P2 gives 1,037 but then still says "the other five" |
| Candidate "scores 564 and 210 on shortcut (28 and 13 deaths)" | Not found in files read; the gate for v5 shows 0 and 0 on shortcut | Unverified. Name the build and file |
| Contest siege 333 gems vs 0, 6/6, deaths 13.7 / 43.7; ability row 67 (75 without); second-death row 73 | Not in main. results/v5-w1.md l.14, 92 says the siege is shipped OFF, best 300 gems / 21 deaths, and "loses contest 0/6 to v4" in self-play | Unverified on main; the figures may sit on a cloud branch. v5-w1.md conflicts with "6 of 6 won". Resolve before sending |
| Wolf/elk/monolith/gem/gold rewards and ranges | FACTS.md l.12-16 | Match |
| Killer takes all gems + 50 gold; camp kills destroy bag; 60-round respawn; shards 30; locks 10 gold; 8 + 1,024 bytes; capture 5 rounds, split | GameRules.md l.139, 243, 265, 320-322, 352 | Match |
| Duels: "loses to ... Varg (30 %) and Ryth (33 %)" | FACTS.md l.28: those are Heart of Ruin's win rates (loses to Cassi & Issac 0 %, Marcella 0 %) | Ambiguous. Reads as if the loss rate were 30 %/33 %. Write "wins 30 % against Varg, 33 % against Ryth, 0 % against Cassi & Issac and Marcella" |
| Team sizes (3: broken_curve, collision, fragment, gold_rush, packs, the_farm; 4: ... ) | FACTS.md l.7-9 covers 21 maps only; the 35-map list not checked | Partly unverified |
| "35 maps" | gate json tags: maps_n_local 35 | Match |

## 4. AI authorship

No explicit marker inside the two prompt bodies (no model names, no "as an AI", no em-dash pattern). Risks: (a) the section headers above each body name the consultants, "DeepSeek, Claude, ChatGPT, Copilot", and the intro (l.3-5) mentions the lead, PLAN-v4.md, the /consult skill, "the current plan" and "evidence against the current plan"; keep all of that out of the paste (paste from "We are a team..." only); (b) body jargon "salted copies", "mapgate", "unopposed" is internal and unexplained; define once ("copies of v4 with a small random tie-break"); (c) uniform bold labels and "Requests / Constraints" layout read as templated, which is harmless.

## Verdict

Prompts: proceed with named changes. Required before sending: items 1-3 and 5 of section 1, the 1,175 -> 1,110 figure, the A-only new-map numbers, the swirl line, the HoR duel wording, the omitted noise caveats (71-78 range, 5-win edge, same-games draft exceptions), and verification or removal of the siege, ability, second-death and shortcut 564/210 figures. Option block: proceed with changes (equalise option 1, add one datum per option, reverse order in P2).
