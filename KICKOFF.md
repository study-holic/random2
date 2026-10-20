# First message for Claude Code in C:/cambc-solo

Paste everything below the line.

---

You are the lead on a solo Cambridge Battlecode 2026 bot. Sprint 1 locks at 23:59 BST tonight. Read CLAUDE.md, then PLAN.md (a draft from another session), IDEAS.md, notes/contest-experiments.md, and the three files in mining/.

Work as a lead with authority: you decide, and every decision passes the /evaluate lenses (critic agent for anything you wrote) and, for the plan and the release, /consult for DeepSeek and Copilot. Treat the old MicroStella repository, v4.14, and every earlier verdict as data points to learn from and to find failure points in. Look for ideas outside cambc as well: MIT Battlecode, earlier Cambridge seasons, League of Legends and Dota 2, RTS games, sports, and routing problems, with the web as the first resource. Watch games in the visualiser through Claude in Chrome (/watch); a test that changes behaviour is unfinished until one loss has been watched.

Start these now, in parallel:
1. **Setup.** Build v1 and the six x-* panel bots. Time one game. Start `python scripts/mapgate.py v1 all --jobs <cores minus 1> --out results/v1.json` in the background and write the laptop's game rate into PLAN.md.
2. **scout-web agent.** Brief: cross-domain idea cards in ideas/cross-domain.md, at least 12, stop 22:05.
3. **analyst agent.** Brief: from intel/matches-2026-10-18.csv and intel/meta-2026-10-18.md, per map, what the top 10 teams draft and how fast they win; which champions carry the largest rating-adjusted win excess per map; output notes/top10-by-map.md, stop 22:00.
4. **/evaluate PLAN.md** with the critic agent, then rewrite it as plan v1, then **/consult** on plan v1 and give me the two prompts.

Then follow plan v1. Keep VERSIONS.md (behaviour changes, with what changed, why, and why it is better), TUNING.md (parameter changes), and RESULTS.md (every test) current. Ask me before any upload.
