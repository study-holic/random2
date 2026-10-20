---
description: Watch a replay in the visualiser through Claude in Chrome and record what happened
argument-hint: <replay bundle path> <what to look for>
---
Watch: $ARGUMENTS

1. Run `cambc view <bundle>` and note the local URL it serves (or open the website replay page for a ladder game).
2. With the Claude in Chrome tools, open the URL and take screenshots at: round 30 (opening routes), the first fight, the first tribute of each team, every death of ours in the first 400 rounds (sample 4 if there are more), and the final round.
3. For each screenshot, record the round, where each of our champions stands, what it is doing, and what the enemy is doing nearby.
4. Cross-check with `node tools/replay/final.mjs` on the inner replay.repl26a (final gems, kills, deaths, gold).
5. Write `notes/watch-<bot>-<map>-<HHMM>.md`: 3 to 6 observations, each with a round number, and one candidate cause for the result. Link the note from the idea's entry in RESULTS.md.

If the Chrome tools are missing in this session, say so to Xans and ask him to watch the same moments, then record his notes in the same format.
