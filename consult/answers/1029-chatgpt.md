ChatGPT (chatgpt.com), model not shown (reasoning "High", worked 10m 5s), answer saved 2026-10-20 ~11:05 BST. Copied from page text, lightly condensed by the copier where marked [..]; LaTeX and chart widgets were garbled and are summarised.

Verdict: build B, but not as a conventional routing optimiser

My ranking is B → G → E → C → A → F → D → H.

The highest-value change is a team-level, economy-first objective controller that assigns champions to productive tasks, understands camp respawn timing, and coordinates when to farm, capture, contest, tribute and buy equipment. I would build this by extending B with a small, persistent task-state controller (G), rather than writing a new pathfinder or replacing the entire bot.

Qualification: the evidence establishes a structural deficit, but not that routing is its primary cause. You could be losing camp income because champions travel inefficiently, arrive without enough combat power, abandon objectives, die while farming, or cannot hold contested territory. Those need different fixes. Spend the first part of Tuesday distinguishing these, then commit most engineering time to the winning hypothesis.

What the results tell us
1. v4's survivability fixes worked but did not solve the game. Deaths fell 37.9 to 23.0 (39%), scored gems rose 383 to 499 (30%); opponent output barely changed (781 to 782).
2. The failure is geographically concentrated. On the four contact maps (contest, fragment, hooks, gold_rush) v4 is 0-32 with about 51.5 deaths per game; on expanse 5-3 with 922 gems. A universal explanation (weak draft, bad combat) is less convincing than one about map geometry, objective access and contested space.
3. Top teams' advantages (gear 3.8 vs 1.7, ability openers 85% vs 35-43%, locks 6.4 vs 0) are observational; they may be effects of a winning economy rather than causes.
4. Self-play overestimated generalisation. 86/126 locally; real paired improvement was eight net wins in 160 games, +5 pp, McNemar p = 0.13, approx. 95% interval -0.7 to +10.7 pp. The under-30% rule is a reasonable heuristic, not a causal test.

1. Ranking (estimated win-rate change vs the mid-ladder panel from 23.1%; subjective, not additive)
1. B Economy and routing: +5 to +15 pp (medium-low)
2. G Stateful objective controller: +3 to +12 pp (medium-low)
3. E Contact-map recovery: +2 to +9 pp overall (medium)
4. C Draft overhaul: +1 to +8 pp (low)
5. A Fight module: +1 to +10 pp (low)
6. F Gear curve and early recall: 0 to +7 pp (low)
7. D Denial and aggro: -3 to +5 pp (very low)
8. H No change: 0 pp
Scale: 23% to 28% is about +46 Elo; 33% about +87 Elo.

Failure modes and falsifiers
- B: optimising distance rather than economic throughput; a short route can pass enemies or arrive before a useful respawn. An occupied spawn tile blocks respawn indefinitely, so a champion lingering on its own farm tile suppresses future income. Falsifier: replays show champions already reach profitable camps but cannot clear, control or bank them; then B drops below E and A.
- G: new thrashing from badly scaled utility scores; use commitment hysteresis and a hard no-progress deadline. Per champion: task id, stage, start round, progress counter, deadline; replan on events. Separate from B because B improves routes, G makes them persist.
- E: overfitting eight games per map; only 4 of 21 maps. contest has zero gems in 8 paired and 9 top-10 games for both versions, a persistent map-specific failure.
- C: copying picks without the behaviour that makes them work; test a few substitutions, not 21 tables at once; include Heart of Ruin only where duel matchups justify it. Probably the best return per engineering hour.
- A: a module that maximises fight wins while lowering game wins (chasing low-HP enemies, burning abilities on unimportant fights). v4's own death causes are unmeasured. Build a small subset: ability-first when it changes the engagement, focus a reachable enemy, interrupt tribute when denial exceeds risk, peel for a carrier with a cap on farm tempo. Could jump above E or B if replays show productive objectives lost to winnable fights.
- F: round-200 gear may be an effect of better farming; trap is optimising round-200 gear at the cost of gems or camp ownership. Measure first useful purchase time and recall cost.
- D: opportunity cost and denying your own respawns; run one narrow monolith-denial test only.
- H: release fallback only.

2. Controller to implement
Treat the team as 3-5 workers with different capabilities competing for time-dependent opportunities. Score each champion-objective pair as expected tributed gems times probability they reach an altar, plus denied enemy income and a gold/gear/map-control proxy, over total time (travel, fight, capture, bank, wait), minus a risk penalty. Compute the marginal value of a second champion so two do not sit on one spawn tile. Use greedy assignment first, reuse existing movement and ability executors, require a 15-20% estimated improvement before voluntary task switches, but allow immediate pre-emption for imminent death, a valuable enemy tribute channel, or an ally in danger (starting values, not optima).
Fuel: 90,700 of 100,000 ordinary peak and 186,800 of 200,000 on turn one leave only 9,300 and 13,200 headroom, so adding a planner is unsafe. Generate geometry, attack positions, camp distances and altar routes offline in compact tables indexed by map id; keep camp_groups() cached; replan on events only; simple dynamic risk overlay. Target 80,000-85,000 on stressed turns, hard guarantee under 100,000, under 170,000 on turn one. The new controller should replace expensive work, not add to it.

3. Outside idea: multi-robot task allocation with time windows (auctions)
Cited: Nunes and Gini 2015, auction-based allocation for tasks with overlapping time windows, accounting for travel and existing commitments (AAAI). Mapping: champions are heterogeneous robots, camps are jobs, altars are delivery points, respawns are availability windows. Implementation: candidate jobs (clear available elk, capture a cleared camp, bank a carrier's bag, deny an enemy monolith spawn, escort a carrier); each champion bids from travel time, capability, risk and availability; allocate greedily, recomputing marginal team value after each award; about eight candidate tasks per champion; for recurring objectives use observed availability windows, not an assumed exact respawn time. Experiment to isolate the source of gain: (1) current independent scoring, (2) greedy unique-task assignment, (3) windows plus persistent commitments.

4. Single number I would challenge: 499 gems per game for v4
Is it tributed gems, and how does it decompose into acquired, carried, lost and banked? If about 550 were acquired and 499 banked, it is a production problem and B is right. If about 950 were acquired and only 499 banked, the bottleneck is conversion (deaths, tribute opportunities, interruption, detours), promoting A, F or a banking controller. The 74 gems lost to champion kills needs context, and rescuing all of them would not close the 283-gem deficit. Audit 499 first.

5. Testing
Do not use self-play win rate as the main acceptance criterion.
- Level 1, mechanism tests per build, logging a funnel per game (camp available, reached, cleared, banked) and where value disappears (deaths, invalid paths, unproductive captures, stalled tributes, self-blocked respawns). Examples: B productive gems per champion-turn and respawn utilisation; G task switches and path cycles; E first meaningful farm/capture/tribute on the four contact maps; C first-200-round income and survival by draft; A tribute interruptions and fight cost; F first useful item round and net income after recall; D enemy income suppressed minus own opportunity cost.
- Level 2, local tests that deliberately disagree with self-play: scripted adversaries (early contest, coordinated ability-first attacks, safe farming with early gear, altar interruption, carrier hunting) based on observed replays; paired games over all 21 maps, both sides, identical seeds; hold out seeds and at least one opponent style from tuning; accept on improvement in previously failing matchups without regression elsewhere plus movement in the targeted mechanism.
- Level 3, real opponents as the final gate: do not deploy five variants and pick the winner; 1-89 is too close to zero to rank small gains. Test one integrated candidate against the v4 reference, stratified by opponent and map, A-B-A if switching is practical. Measure banked gems at rounds 200 and 500, differential and wins, first productive camp and first tribute, deaths before banking, tribute interruptions and denial, and the fraction of map/opponent pairs with essentially no economy. Compare at common round horizons; paired or stratified bootstrap clustered by opponent and map.

6. Time plan for 60 hours
- 0-4: forensic sprint on contest, fragment, hooks, gold_rush and a good expanse game; instrument funnel, task switching, first-200-round activity.
- 4-20: S1 B+G core; S2 E contact maps; S3 C/A/F probes, adversaries and regression harness; each a compile-time-switchable component; no concurrent edits to the same controller.
- 20-36: integrate and stress-test (all maps, asymmetric starts, edge cases, no-progress loops, worst-case fuel).
- 36-50: real-opponent validation vs v4; revert if the intended mechanism does not move.
- 50-60: freeze, verify fuel and draft legality, release by Friday 14:00 BST; use time to Saturday's lock for monitoring and critical fixes only.
What would change my mind: champions reach objectives but die in fights a timed ability would change, so A + E ahead of B; long chasing of unavailable camps, duplicate farming, respawn suppression or path loops, so all-in B + G; income fine until the first recall or gear breakpoint, so promote F.

Final recommendation: a narrow B+G architecture, first diagnostics on the four contact maps, cheap draft substitutions tested independently. No sophisticated combat search, full opponent model or comprehensive denial without stronger evidence. The top-20 target is extremely ambitious (509-point gap). The decisive question is why so many champion-turns fail to become banked or denied gems.

## Follow-up (correction)

Yes, but only slightly. I would revise the ranking to:

B → G → E → A → C → F → D → H

Three implications:

1. Tribute defence is easier than assumed. Ordinary damage cannot interrupt banking. The controller should commit to tribute whenever survival is likely and no stun or new altar lock is imminent. Avoid retreating merely because damage is incoming.

2. Combat priorities change. A gains importance because stun timing, lethal burst and altar locks are the actual interruption mechanisms. The top-10 advantage of 6.4 altar locks versus zero becomes particularly interesting.

3. Channel logic must distinguish abilities. Recall requires protection from any damage; tribute and teleport require protection primarily from stuns, death and, for tribute, new altar locks.

My main recommendation remains B+G. However, I would add a small channel-threat evaluator immediately.

The biggest missing measurement is now how many tributes fail specifically because of stuns, deaths or newly placed altar locks.
