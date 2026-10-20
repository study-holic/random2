Read tools/cloud/v5-common.md for setup and tools. Do not run cambc login, submit, matches, ladder or match-groups. Push to this session's branch. Stop by 06:00 BST Wed 21 Oct (`TZ=Europe/London date`). Plan context: PLAN-v4.md "Draft 5".

## Why
All four consultants (consult/1946-sprint2-priorities.md) ranked reliability on all 35 maps first or second. Tonight no build passes the 35-map gate:
- v5: shortcut 0-0, lost on gold.
- `bots/s1g1` (v4 + S1's BANK_REACH and NSB walk-out): shortcut side A lost 84-115 with 29 deaths, and starvation hit 100,000 fuel with 2 exhausted turns (results/s1g1-newmaps-vs-starter-20261020.json).
In a 15-game single-elimination series, one stuck or fuel-exhausted state can decide the pairing.

## 1. Sweep (first; push results/sweep35.md as soon as it is done)
Bots: v4, v5, s1g1. Opponents: bots/idle (passes), starter-cpp, v4. All 35 maps, both sides, seeds 0-2. Per game, record:
- tributed gems; deaths split into monster and champion kills;
- per-turn fuel peak, and the number of turns over 90,000 and over 100,000;
- **stall rounds:** alive champion-rounds with no change of position for 20 or more rounds while not channelling, capturing or fighting;
- **altar stalls:** rounds a carrier spends within 3 tiles of an altar for 20 or more rounds without starting a tribute;
- zero-gem games.
Report per map; list every map-bot pair with a zero-gem game, a stall over 200 champion-rounds, or a fuel turn over 90,000. For each fuel case, name the code path, using the fuel stamps as S1 did (P_S1_FUELDBG).

## 2. Arena: what happens past 100,000 fuel
Build a probe bot that burns a set amount of fuel on chosen rounds (round 1, then round 50). Measure what the engine does: is the turn cut short with actions kept or lost, are later turns affected, is the bot removed? Add the result to results/arena-1020.md and FACTS.md.

## 3. Progress watchdog (P_W_WATCHDOG, in a copy bots/wd = bots/s1g1)
**Model.** A Lyapunov-style progress certificate (control theory: a potential that must keep falling certifies convergence, and a stalled potential marks a fault; Khalil, *Nonlinear Systems*, ch. 4), run as an embedded watchdog timer. For champion i with objective o:
- **Potential:** Φ_i(t) = the path distance to o's goal tiles while travelling, the remaining monster HP while clearing, and the remaining channel turns while tributing.
- **Progress event:** Φ_i falls, or an economic event occurs (damage to a monster, gems or gold gained, a tribute started, an item bought).
- **Stall counter:** S_i(t) = rounds since the last progress event. Trigger when S_i > W_o, with W = 12 for travel, 20 for clearing and 10 for the final approach to an altar.
- **Escalation, one level per trigger:**
  1. Recompute the path with soft hazards (zone walls become costs).
  2. Release the claim and block that objective for 60 rounds (hysteresis, so the champion cannot oscillate back).
  3. Take the nearest feasible task.
  4. Step to a fixed escape tile.
- **Cost:** at most 2,000 fuel a turn; skip it below 20,000 fuel remaining.
Source of the idea: Copilot's "progress watchdog and deadlock recovery" (consult 1946).

**Admission (fixed before testing):**
1. Stall champion-rounds fall by at least 50 % over the sweep.
2. No map is more than 10 % slower unopposed.
3. Shortcut is won on both sides against starter-cpp at seeds 0-2.
4. Peak fuel is at most 90,000 on all 35 maps.
5. Self-play vs v4 is at least 62/126 on the old 21 maps (rejection gate only).

Write results/sweep35.md (sweep tables, arena result, watchdog admission). If a line fails, say so.
