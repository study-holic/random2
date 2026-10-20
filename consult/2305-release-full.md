# Consult 23:05 BST: release decision for Sprint 1, with the complete project context

Attachment for both consultants: `consult/pack/xanarchy-context.txt` (= `CONTEXT.md` + full `GameRules.md` + `api-index.md`, the 629-signature Bot API index). Submitted through Claude in Chrome at Xans's request (23:00).

## Prompt to DeepSeek (rules angle)

The attached file is the complete context for a decision our team must make in the next 20 minutes: the project, the full game rules, the C++ Bot API index, our code, our test harness, and every result so far. Please read all of it before answering. You are an outside reviewer with no stake in our choices; be critical.

Decision: what to upload before tonight's Sprint 1 snapshot (one upload; single-elimination tournament, 15 games a match). Options:
(a) upload the release candidate v2 as it is;
(b) upload v2 without one or more of its four changes (the corners draft, the swirl group exclusion, pilgrimage routed to v418d, old-layout collision routed to v418d): say which;
(c) upload v1, the base (v4.15 plus no-op hooks);
(d) upload nothing tonight;
(e) an option not listed.

Requests:
1. Rank the options with reasons, citing the rule sections that matter.
2. Name the most likely failure mode of each option.
3. Judge the R2 fuel test against the Fuel rule: are suspensions on a champion's first two turns, and one at round 20, a safety problem in this engine?
4. For the next sprint, rank these three by expected gain and name the rule each relies on: counter-drafting from the opponent's revealed picks, champion-specific ability use (for example Willow's heal, dashes), and in-game adaptation (fight evaluation, plan switching).
5. Propose one idea from another game or competition that maps onto a named rule here.
6. Say which fact in the file, if wrong, would change your answer.

## Prompt to Copilot (code and data angle)

The attached file is the complete context for a release decision due in 20 minutes: the project, the full game rules, the C++ Bot API index, our code structure, the test harness, and all measurements. Please read all of it first. You are an outside reviewer; we want the failure points, not reassurance.

Decision: what to upload before tonight's Sprint 1 snapshot (one upload). Options:
(a) release candidate v2 as it is;
(b) v2 minus some of its changes (corners draft, swirl exclusion, pilgrimage→v418d, collision_old→v418d): say which;
(c) v1, the base;
(d) nothing;
(e) an option not listed.

Requests:
1. Rank the options using the numbers in section 6: salted means, the held-out panel, and the admission rule.
2. Name the failure mode of each, including code-level risks of the two-namespace dispatcher (static state, exceptions, fuel of the map scan) and whether fuel-burn salts sample the noise that real code changes cause.
3. Is the admission rule (ΔW ≥ 1.5 of 12 over 3 salts) too loose or too strict, given a no-op flips about 7 % of cells? What false-admission rate do you expect across the roughly 12 map-variants we tried?
4. For the next sprint, rank by expected gain per hour of work: counter-drafting from revealed enemy picks, champion-specific ability use, and in-game adaptation. Name the API calls each would use, from the index.
5. Propose one idea from another game or competition that maps onto a named mechanic here.
6. Say which data point, if wrong, would change your answer.

## Answers

Submitted through Claude in Chrome. DeepSeek (Deep thinking, sent 23:09): https://chat.deepseek.com/a/chat/s/063ef23b-69ad-4b13-a6b9-4bd92fcf5604. Copilot (M365, sent 23:28, with v2's release numbers inline): conversation f8743abe-5747-4485-aa80-1dd0fb9b13eb. Full texts are in those chats; the summaries below are faithful, not verbatim.

**DeepSeek (v2 results not yet in its prompt):**
- Decision 1: upload v2 without the corners draft and without the old-collision route; keep pilgrimage→v418d; keep swirl only if R3/R4 are green. Otherwise upload v1. "Nothing" ranks last.
- Corners: admitted on the old layout only. Old collision: dead code on the tournament layout, with misroute risk.
- Decision 2: build no new feature; de-risk instead. If forced, a death-loop breaker or region stop-loss, then ability use, then routing, then counter-drafting.
- R2 is too strict as written (v1 fails it). Opening suspensions are acceptable as initialisation; the round-20 one is a risk to check in the replay.
- Cross-domain idea: a poker stop-loss mapped onto the death rule (carried gems pass to the killer; 60-round respawn).
- Fact that would change its answer: the six local maps being old layouts.

**Copilot (with v2's numbers):**
- Decision 1: upload v2 unchanged. Then v2 minus corners, then v1, then an emergency patch, then nothing. Key evidence: every admitted change points the same way, and the held-out panel (142 vs 139) did not reject them.
- Decision 2: per-map routing among existing versions, then counter-drafting, then a small death-loop breaker, then ability use.

## Classification

| Claim | Source | New / agrees / contradicts | Checked against | Result |
|---|---|---|---|---|
| Remove the old-collision route | DeepSeek | Agrees (removed 23:14 after R4) | `results/v2-heldout-cmp.txt` | Done before the answer arrived |
| Remove the corners draft (old layout only) | DeepSeek | Contradicts critic and Copilot | Local test was on the old layout (true); ladder top-10 data on the current corners supports Stellar and Ren | Kept. Two independent sources point the same way, and the base is weakest there (3.67/12). Logged as a disagreement |
| R2 as written fails v1 | DeepSeek | Agrees with the critic | `results/fuelchk/v1.json` (Issac, round 20) | True |
| Upload v2 unchanged | Copilot | Agrees with the critic | `results/v2-*.txt` | Uploaded 23:35 with Xans's approval |
| Death-loop breaker is the best new feature | DeepSeek | Contradicted by data | `results/w5-loop.md`: no map admitted; outside swirl, 96–100 % of deaths are to champions | Rejected tonight |
| Per-map routing is the best new feature | Copilot | New for the remaining time | Single-sample data only | Not built (no time); Sprint 2 candidate |
