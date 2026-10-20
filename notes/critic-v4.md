# Critic: v4 base release candidate (20 Oct, read-only review)

Verdict: UPLOAD WITH NAMED CHANGES (one is optional-cheap).

1. Evidence. Gate 42/42, 0 faults, fuel max 90,697 (checked in the gate JSON). H2h 86/126 = 68.3 % vs v3b-e1, but v3b-e1 is not the live bot: live v3b mislabels the kept maps (mapid note in v4-base-release.md), so part of the 68 % is mapid, not play. Still, the live bot is at 1,388 and falling, and v4 beats starter-cpp 42/42 where v3b-e1 managed 36/42, so the downside is small. Corners 0/6, expanse 0/6, lanes 1/6 are lost to a bot that is itself weaker than the ladder; a ladder opponent of a different style may do worse or better. Falsifier: after 10 ladder series v4 below v3b's rating trend. Cause of those three losses is untraced (the v4-base-release.md lists new maps ids as suspect); no /watch was done. Limits: 6 games a cell, deterministic, so 0/6 is real but one-opponent.
2. Fuel. 90.7k peak (lanes B, islands B) is outside farm(), source unprofiled; 9.3k margin. Opponent-dependent paths (many champions in view, enemies.cpp dash valid_tiles, rows/siege) can add fuel on a larger fight than the gate's starter-cpp. Fuel exhaustion costs one turn, not a crash, so risk is bounded.
3. Map id. Identification is by size only on 10 maps. A new pool map of 41x41, 71x71, 80x80 or 90x30 would silently get a kept-map plan (collision/corners/expanse/lanes). An unlisted size gets unknown, which falls to the default plan (safe, verified only by the enum, I did not run it). Same-size maps with another camp count fall to the default branch (elk_arena, big_world, fragment, broken_curve, hooks), also safe.
4. Diff. P_UNSEEN_HIT: a hit with zero modelled damage and no visible enemy walks home instead of recalling; memory 3 rounds, bounded. Poison/DoT could trigger it; harmless. P_STAND_SIGHT: try/catch around visible_tiles_from, capped at 12 tiles, fallback intact. No bug seen.

Named changes before upload:
- A: for the size-only ids add a camp-group-count check (expected count per kept map); on mismatch return unknown. Test: gate mapid_checked still 21/21.
- B: run /watch on one corners loss (not blocking, do it after upload).
Can wait for v5: trace corners/expanse/lanes, profile the 90k peak, elk range 4.
