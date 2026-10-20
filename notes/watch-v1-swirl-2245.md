# Watch: v1 (team B) vs x-foundry-v16 on swirl, salt 0 (22:45 BST)

Replay: `matches/v1-222117/swirl__x-foundry-v16__B__s0.repl26a` (visualiser via `cambc view`, Claude in Chrome). Deterministic rerun with statistics: `results/watch/swirl-fv16-B.json`.
Final (stats JSON; `tools/replay/final.mjs` is not on this laptop): foundry-v16 1,066 gems at round 885, 24 kills, 7 deaths, 7,130 gold earned; v1 48 gems, 5 kills, 39 deaths, 570 gold. Drafts: foundry C&I, Varg, Stellar, Vendetta; v1 Vendetta, C&I, Varg, Ryth.

## Observations

1. **Round 15 (opening):** both spawns are inside the central chamber of the spiral, 4 to 6 tiles apart, beside the two altars. The chamber camps are gold clusters; the gem camps ring the outer spiral. Both teams' first steps go to the central gold camps.
2. **Round 24 (first fight):** all five of ours fight the gold camps between the spawns, mixed in with all of foundry's champions. Their Stellar kills Cassi and Issac at (18,10) and (18,9).
3. **Rounds 24 to 312:** 15 of our deaths, every one at x 15 to 25, y 9 to 17 (the chamber). Killers are enemy champions (Stellar, Cassi, Varg, Vendetta) except 2 to camps at round 187.
4. **Round 292:** foundry has 190 gems tributed, 1,620 gold, 6 kills and 5 wards. We have 0, 0, 0 and 0. Our five are still beside the enemy spawn and altar; two foundry champions farm outer-spiral gem camps.
5. **Rounds 298 to 312:** four of ours die in 14 rounds next to the enemy altar.
6. Across the game, 39 deaths, all but one (Varg at (10,22), round 731) in the chamber; 7 of the 39 to camps.

## Candidate cause

Target choice keeps selecting the contested central gold groups next to the enemy spawn, where the enemy heals and respawns at once. v1 loses every fight there, earns no gold (each death pays them 50), and never reaches the gem camps on the spiral. The 43.5 deaths a game against starter-cpp fit the same picture: starter-cpp's champions stand by their spawn in the chamber and hit whatever comes near.

Passed to w2 at 22:46 (fix through the swirl target hook: exclude chamber groups near the enemy spawn).
