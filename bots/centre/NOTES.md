v4-13 = bots/v5-base (v4-11 defaults) + src/params_override.h: P_OPENING_ROUNDS=2000 (cluster-first harvesting all game on maps with 3+ gem-cluster groups).
Evidence (both sets positive; the micro trio of v4-12 was the drag):
- Selection set, 72 cells vs v4-11 (sweep micro2/open2000): +1.88 gems/100r [-0.16, +4.16], dDeaths +0.01.
- Held-out set, 54 cells vs v4-11 (opponents v4-8, v3d, v4-3a; WS/h-open2000.compare.txt): +3.84 [+0.18, +7.92], dDeaths +0.02, wins 41 -> 40, PASS.
- v4-12 on the same held-out set: -1.35 [-6.42, +3.87], wins 41 -> 32 (chase4 alone -3.60, kite+vision+chase4 -4.97).
Same source as the h-open2000 and sw-open2000 builds.
