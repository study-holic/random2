---
description: Multi-lens evaluation of a plan, prompt, idea card, test design, or result before it drives a decision
argument-hint: <file path or short description of the thing to evaluate>
---
Evaluate: $ARGUMENTS

Read the thing in full. If you authored it in this session, spawn the critic agent to run this evaluation on it instead, passing the file path and these lenses.

Answer each lens in two to four sentences, with numbers where the data has them:

1. **Creativity.** Is it a re-run or re-tune of something already tried? Is it so novel that it needs mechanics nobody has verified, or more build time than remains? Name one bolder variant and one safer variant.
2. **Restriction.** Does its framing rule out options without evidence (for example "which draft" when the draft may not matter on this map)? List the excluded options.
3. **Causation.** What mechanism produces the claimed gain, step by step? Which link rests on correlation alone?
4. **Limitations.** Opponent realism (the panel sits near v4.14's level), sample size (12 deterministic games a map), selection bias from picking the best of several, fuel, and crash risk.
5. **Feasibility.** Games needed = values × maps × panel bots × 2 sides. Time = games × seconds a game ÷ parallel jobs, plus builds. Does it finish before its slot in PLAN.md? If not, cut values or maps and say which.
6. **Falsification.** Which result would make us drop it, fixed before the test runs?

End with a verdict: proceed, proceed with named changes, or drop, and one sentence on what the consultants should be asked about it, if anything. Write the evaluation under the item in PLAN.md or IDEAS.md.
