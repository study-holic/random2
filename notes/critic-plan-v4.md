# Critic review of PLAN-v4.md (fresh context, Tue 20 Oct)

Checked: GameRules.md, ENGINE-1.0.md, notes/unranked-v3b.md, notes/matchups.md, consult/2010-gamechanger.md. Not read: RESULTS.md (no claim in the plan depends on it that I could find), cambc.h.

## 1. Fact checks

| Claim (PLAN-v4 line) | Result |
|---|---|
| Respawns wolf 40, elk 60, gem cluster 50, monolith 150 (l.82) | Correct (GameRules.md l.149-153). Timer starts after the reward is claimed. |
| "Extra allies do not speed capture"; capture 5 rounds (l.83-84) | Correct (l.139, l.163). |
| Tribute max(1, ceil(gems/10)) (l.85) | Correct (l.181). |
| Killer takes all carried gems (l.86) | Correct (l.320). Also 50 base gold; the plan omits that, and it is a real lever for picking off carriers. |
| Shards 30 gold, five combine (l.94) | Correct (l.243). |
| Prism 1.0 Focus per gem, Vesper 33 %, Marcella 2 gold per gem (l.94) | Correct (ENGINE-1.0.md l.9-11). But l.9-10 says the site doc "still" says 0.2 / 20: the rules doc and the SDK disagree. Say which one wins in FACTS.md. |
| Draft memory 1,024 bytes, drafter sees the whole map, extra fuel (l.64) | Correct (GameRules.md l.77). The extra fuel is never quantified; module A's whole budget depends on it. Measure it before designing A. |
| "Any occupied spawn tile delays the entire group" (l.126, quoted as a rule) | **Not in GameRules.md** (grep for "entire group" and "occupied spawn" returns nothing). It appears only as a quote in the ChatGPT and DeepSeek answers (consult/2010-gamechanger.md rows 7 and 10). The plan labels rule quotes as from GameRules.md (l.62). Module G's blockade rests on an unverified quote. Test it in the engine before building G. |
| Monsters "range 3, never chase, attack nearest champion" (l.106) | Range 3 and no chase correct (l.164). "Nearest champion" not verified. Elk range 4 and monolith range 5 on engine 1.0 (ENGINE-1.0.md l.1), so "range 3" is stale for those. |
| "golem positions" (A, l.71) and "golem per SDK" (B) | ENGINE-1.0.md l.52: no bundled map contains a golem, rewards unknown. Dead code on today's pool. Drop it. |
| Section 2 table header "Opponents / top-10" | Wrong label. 19.3 vs 37.5 tributes and 38 vs 21 deaths are v3b versus its 8 opponents (unranked-v3b.md l.11), not top-10. The 12.8 vs 5.0 shard-points and 84 % vs 32 % are top-10 vs our games, on engine 0.0.3, cross-population (matchups.md l.61 says not rating-adjustable). The table mixes two comparisons under one header. |
| 10-66 on new maps (l.53) | Matches unranked-v3b.md l.48, but 9 of those 76 are draws counted as losses (0-0 boss_battle, islands). Wins 10 of 76 is 13 %; do not over-read. |
| Fight AUC 0.86 (l.56) | Matches (matchups.md l.19-21), but fit on engine 0.0.3 and the same note says composition adds nothing. Plan l.113 and l.79 are consistent with that. |
| "Rating is not the bottleneck... climbs 470 points within a day" (l.15) | Unsupported. CLAUDE.md gives +16 for a 5-0 series near 1,500 and 6.1 series an hour. 470 points needs about 20-30 series (3-5 hours) at near-perfect results. Plausible only for a bot already at 1,900 strength. Hours of climbing must be scheduled after the freeze or the final upload cannot reach rank 20. See section 6. |
| Milestone ratings (l.19-21) | Internally inconsistent. Panel 1,453-1,771 averages about 1,600, so 50 % there is about 1,600, not 1,650. A top-50 panel starts at 1,746 and likely averages near 1,900, so 50 % is about 1,900, not 1,800. A top-20 panel averages about 2,000, so 45 % is about 1,960, which is rank 10, not rank 20. The milestones are harder than labelled (M3 would be a rank-10 result). Either recompute or relabel. |
| Sprint 1 lock (CLAUDE.md) 18 Oct 23:59 vs "Sat 24 Oct 23:59" (title) | The plan's deadline has no source I can find in the files read. CLAUDE.md says the lock was 18 Oct. Put the source (tournament rules, date, whether lock means rating or a bracket) in FACTS.md, because the freeze time drives the whole schedule. |

## 2. Lenses on the plan as a whole

- **Creativity.** Mostly the safe consensus list (map compiler, conveyor, gear curve, fight gate); the ideas are from the 20 Oct consultation, not new. Bolder variant: skip G and H, and spend that time on a "copy the top-10 opening" bot: decode 50 top-10 games per map, replay their first 300 rounds' routes and purchase order as a lookup. Safer variant: ship only the v4 base plus an ability-opening rule and a fight gate (DeepSeek ranks those cheapest, 2-3 h).
- **Restriction.** The framing excludes: (a) a fixed per-map scripted opening taken from top teams' games (cheaper than a generic compiler); (b) a "do no harm" default for unknown maps; v3b's finding (unranked-v3b.md l.78) that a safe default beats a wrong specific plan is not carried into the modules; (c) not playing: the 9 draws on boss_battle and islands show a 0-0 tiebreak is worth something (3 wins, 5 losses); (d) dropping 5-champion drafts on some maps. Everything assumes the bot should be generic.
- **Causation.** Chain: more tributes → more gems banked → more gold and rating. The plan asserts income is the biggest lever, but tributes per game (19.3 vs 37.5) and deaths (38 vs 21) are correlated with losing itself; a team ahead tributes more and dies less. The shard-points gap (5.0 vs 12.8) is the same: richer teams buy more. The direction is untested. The only causal evidence is the 01islands case (0 tributes, 0 deaths: a clear bug), which the base fixes already cover. Link resting on correlation: "raise tributes to 30 raises win rate".
- **Limitations.** Panels of 80 games: at a 25 % win rate one arm has SE about 4.8 points, the paired difference about 6.8, so the 8-point admission bar is 1.2 sigma (false admit about 12 %, power to detect a true +8 about 55 %). Opponents change bots during the week (not stationary) and the paired test is sequential, not simultaneous. Per-map results at 80 games are about 4 games a map: no per-map regression detection. Engine date of the fight model is 0.0.3. Fuel and crash risk for A (BFS in draft plus champion rebuild) are not budgeted.
- **Feasibility.** See section 5.
- **Falsification.** The plan has kill rules per module but none for the plan: nothing says what result drops the whole premise. Add: if v4 base after the identification and bug fixes still wins under 30 % against the 8-team panel (it won 18 % with 0 of 10 new maps identified), the gap is micro and economy-by-design, and A is the wrong first build.

## 3. Modules

| Module | Verdict | Reason |
|---|---|---|
| A map compiler | Proceed, narrowed | Highest value (the 10 new maps are 48 % of the pool and 13 % wins). Cut scope to: map hash identification, camp-group value table, BFS fields, route order. Drop clear-time damage model and chokepoints from the first cut. |
| B economy | Proceed, merge with A | Route planning is A's output; two modules would fight over the same file and the same 200 bytes. One module "A+B". |
| C gear | Proceed | Cheap (purchase curve, gold-driven recall), and gear predicts fights (matchups.md). Check first what v3b already does. |
| D fight | Proceed, split | The ability-opening rule and disengage-before-contact are 2-3 h each and should ship in the base. The refit fight gate (6-8 h) waits for engine-1.0 data. |
| E draft | Drop to a lookup | Own data: pair tables dead (1 of 55 cells), composition adds nothing to fight outcomes (AUC 0.864 vs 0.86). A minimax matrix over 3 salts is the highest compute cost for the weakest signal. Keep Stellar-first and the existing 3-pick finding (+10.5 pp, n=300). |
| F coordination | Merge into A+B | 8 bytes: claims + focus target. Build with the route planner or not at all. |
| G denial | Cut (revisit Fri only if M2 passes) | Quoted rule unverified (section 1), requires walking into enemy territory, DeepSeek ranked it 10th of 13. |
| H opponent model | Cut | Vision is not shared (GameRules.md l.310), and a champion sees few enemies; the draft prior is the same information E would use. Little to learn in 2,000 rounds. |

## 4. Admission rules and the 50 groups an hour

- Throughput is fine. The v3b panel (160 games, 32 groups, 5 games per group) was queued 06:12-06:16 and finished before 06:30, so 80 games is about 16 groups, about 20 minutes. The constraint is not volume.
- The constraint is that every test needs an upload, because unranked games use the live bot (l.41). Each admission test is: upload candidate, wait for its panel, upload the live bot again? No: the paired baseline must be measured at the same time, which is impossible with one live slot. The plan has a candidate and a baseline measured sequentially, against opponents who may have changed. Fix: keep the baseline panel from the same hour (run baseline first, then candidate, then baseline again, and require the two baselines to agree), or use a second team account if the rules allow.
- 80 games gives power about 55 for +8 points (section 2). Two-panel kill rule (l.169) will drop real gains about 20 % of the time and keep noise 12 %. Use 160 games (32 groups, about 40 minutes) for any module claimed above +5, and pool the three weeks' data.
- Uploads cost rating on ranked play for the duration, and perhaps an upload limit. Unknown; ask. The standing approval question (l.178) is therefore on the critical path: without it, every night is lost.
- The top-20 panel (M3) needs about 400 games (20 teams x 21 maps) for per-map signal, about 8 hours of groups; it cannot happen on Saturday afternoon. The M3 panel has to be sampled Friday night.

## 5. Schedule

Time left: Tue 08:10 to Sat 20:00 upload is about 108 hours elapsed, about 55 productive for one human plus 3 agents, and 4 game jobs in the daytime on 14 cores.

- **Tue 08:00-12:45 is overbooked.** It holds preflight, STATE, FACTS, gate.py, mapgate tags, a 1.0 decoder, plus a v4 base with seven engine-1.0 fixes, gate pass, h2h and a critic review. That is two days of work for one person; the base needs a builder and the tooling another. The gate alone is 21 maps x 2 sides = 42 starter games; add time per game (not stated in the plan) and 4 jobs. Realistic: base uploaded Tue 18:00, not 12:45. Do the base first (it stops 10-66), tooling second, and cut `FACTS.md` and `STATE.md` to the minimum that preflight needs.
- **Wed:** A, B, C in one day each module 6-12 h by the consultants' own estimates (A 6-8 h plus tests, B 6-10 h, C 4-6 h) is 16-30 builder hours. With 3 agents it is possible only if they do not share files, but a single human reviewing three modules and critic passes is the limit. Expect A+B Wed, C Thu.
- **Thu D+F+E:** after the cuts, Thu holds C and the fight gate refit. S3 draft matrix is not needed.
- **Fri G+H:** cut. Friday becomes panel runs, weakest-map fixes, and the top-20 sample.
- **Sat:** freeze 18:00, upload 20:00 leaves 4 hours of ladder at 6 series an hour = 24 series, about +9.6 per 4-1 series from 1,500 but less above 1,700. A bot that is 1,700 at upload cannot reach 1,888 in 4 hours. The real freeze for a rank-20 result is Fri night; Saturday is for a safe patch only.
- Compute: S3 and the overnight 10-job request are only valuable if E survives; drop it.

## 6. Missing

1. A per-map regression gate for new releases: aggregate +8 points can hide a map at 0-8. Require no map worse than the live bot by more than 3 games in the 8-team panel.
2. A rollback rule and a named "last known good" upload. v3b was uploaded untested; there is no one-command revert.
3. Top-team behaviour study is S5 but never scheduled (it sits in a table). A decoded study of the top-10's first 300 rounds per map is the cheapest source of routes and purchase order, and it is what rank 1-20 teams actually run.
4. Unknown-map default plan (fall back to safe-nearest-camp) and a map-hash miss counter, since the pool changed overnight once and can again.
5. Fuel accounting for the draft bot and for rebuild on turns 1-5 (80,000 limit per CLAUDE.md; each `std::endl` 3,500).
6. Ladder cost of test uploads, upload rate limits, and whether the Sprint 1 standing counts the final upload only or a rating at lock. CLAUDE.md says to record this in PLAN.md.
7. Statistical honesty: confidence intervals in every panel table, paired by team-and-map, not just win counts.
8. Fallback if Xans does not answer the three questions: act on a default (case-by-case approval, 4 jobs, no overnight).

## Verdict: proceed with named changes (rework the schedule and cut four modules)

The diagnosis (identification, economy, deaths) is supported by the data; the build list and timeline are not.

Five changes first:
1. Move the v4 base to Tue 18:00 and put it first; identification of all 21 maps plus the safe-default fallback is the only change with direct evidence (0 of 10 new maps identified, 10-66).
2. Merge A, B and F into one "route planner" module; cut G and H; reduce E to a lookup; split D so the ability-opening and disengage rules ship in the base.
3. Fix the admission test: 160 games for claims above +5, baseline measured immediately before and after the candidate, per-map floor; write down the sequence in section 6 of the plan.
4. Move the real freeze to Fri 22:00 (rating takes hours to climb) and recompute milestones M1-M3 with the correct panel-average ratings.
5. Verify before building: the spawn-tile blockade rule (not in GameRules.md), draft-bot fuel, the golem (not on any map), and the Sprint 1 deadline source; correct the section 2 table header.
