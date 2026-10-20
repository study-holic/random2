---
description: Write prompts for the outside consultants (DeepSeek and Copilot) on a plan or decision, built to avoid leading them
argument-hint: <the decision or plan to put to the consultants>
---
Decision for the consultants: $ARGUMENTS

Write `consult/<HHMM>-<topic>.md` containing two prompts, one for DeepSeek and one for Copilot. Xans pastes each into its tool and pastes the answer back.

Each prompt holds:

1. **Context packet.** The game in 6 to 10 lines (objective, camps, altars, tribute and lock rules, capture rules, team sizes per map), then the facts that bear on the decision with their numbers and dates. Facts only; no conclusions of ours.
2. **The question, asked openly.** List the options we see with equal space each, plus "an option not listed" and "none of these".
3. **Requests.** Rank the options with reasons; name the failure mode of each; propose one idea from another game or competition that maps onto this mechanic; say which fact, if wrong, would change their answer.
4. **Constraints.** Remaining time and compute, from PLAN.md.

Rules: never state our preferred option or our reasoning; include the evidence against our current lean; give DeepSeek the rules-heavy version and Copilot the code-and-data version so their angles differ.

When the answers come back, append them to the same file, then classify each claim as new, agrees, or contradicts, check every factual claim against GameRules.md or the data, and record what changed in PLAN.md.
