# w10: the contested rule, gold clusters, and why v2a is slow on stampede

Builder w10, copy `bots/w10` (from v2a). Session 1 ran 01:45-02:18 BST; the laptop then slept until 17:13. Session 2 ran 17:25-20:00 BST on the lead's 17:25 brief. All runs used 2 game jobs.

**Verdict: nothing admitted.**
- The contested bug is real but is not why we are slow.
- The retreat fix is rejected globally (17 maps, two variants).
- On stampede, every variant is level with v2 on the panel once 6 salts are pooled. The best head-to-head is V6 at 9/12 against v2 (mirror baseline 6/12), which fell to 12/18 over 9 salts.
- Pace stays far from the ladder: our stampede median win round is 1,460-1,670 against 1,752 for v2 and 686 for the top 50. Even against starter-cpp we need 1,364-1,619 rounds.

## Stampede layout
Decoded from local replays with `tools/v3/decode.mjs`, using the trace scripts in my scratchpad.
- Camp type codes match `sdk/cpp/cambc.h` `camp_type` (0 wolf, 1 elk, 2 gem_cluster, 3 gold_cluster, 4 monolith). Replay HP matches the GameRules table (elk 2,400, clusters 500).
- There are 13 groups:
  - 10 groups of 2 gold clusters along y=1-2 and y=47-48;
  - 4-gold blocks at (2-3,24-25) and (18-19,24-25);
  - one gem group of 6 elk + 6 gem clusters at (56-58,23-26), on the symmetry axis about 55 tiles from either spawn. It pays 360 gems (6x36 + 6x24) and 960 gold, deals 6x60 = 360 damage a turn, and respawns in 60 rounds.
- So 28 gold, 6 elk and 6 gem camps, as the lead's ladder replay showed. Every gem on the map is in that one group.

## 1. The contested rule (v2a file:line), and the fix
- **Marking.** `stuck.cpp:19-34` `update_contested` marks a group when 2 or more visible enemy champions are within 4 tiles of a spawn tile. The mark lasts 30 rounds (`stuck.cpp:36-38`).
- **Where a gem group is swapped for gold:**
  - `squads.cpp:103-118` `choose_target`: `alt_own_half` is true when any uncontested own-half group is ready within 30 rounds, and gold-only groups qualify. Contested groups are then skipped in passes 0-1, so a gold group is picked.
  - `stuck.cpp:42-67` `leave_contested` (run every farm turn from `micro.cpp:56`): unless the nearest camp is under 30% HP, it switches to `choose_target(v,target)`, which may be gold, and excludes the gem group for 30 rounds.
  - `micro.cpp:66-77`, the respawn wait: an enemy within 6 tiles makes the champion give up and re-pick, which may be gold.
- **The fix (`gem_alt`):** `squads.cpp:104-110` requires `g.gems > 0` for `alt_own_half`. `stuck.cpp:62-63` and `micro.cpp:72-77` stay on a contested gem group rather than take a gold-only one.
  - Gated on `any_gem_group` (`bot.cpp:145`), so contest, whose only gems are in Monoliths, is untouched.
  - `P_GEM_ALT` is 0 globally; stampede turns it on through `map_stampede.cpp:22`.
- **Result:** with `gem_alt` alone (V1, salt 0) the panel went from v2's 11/12 to 9/12, head-to-head against v2 was 0/2, and the gem margin fell. **The falsifier is met: rejected as the stampede fix.**
- **Why it barely matters:** in 4 of 5 traced v2 games, 2 or more enemies stood within 4 tiles of the gem group on only 11-50 rounds. Against x-v4-14 on side B it was 673 rounds.

## 2. The real causes (traced)
1. **The retreat formula** (`bot.cpp:388` with `util.cpp:28-33`).
   - `incoming` adds the strength of every camp within 4 tiles. At the elk block that is 360, so `need = 4*360 + 0.1*maxHP`, which is at least 1,440. That is above every unitemed champion's maximum health (700-1,200).
   - So any champion below 90% HP near the group retreats and recalls home, 55 tiles away. The group is chipped at roughly 25-80 HP a round. The first clear came at round 750-985, and at round 813 even against starter-cpp.
   - The rules disagree with that count. GameRules: "When a champion strikes a monster, it will awaken attacking the nearest champion every turn" and "All monsters have attack range 3 ... and do not chase".
2. **The squad claim keeps one squad off the group.** `squads.cpp:122` (`if (i == other_claim) continue;`) means only one squad can target the elk group. The other squad (Vendetta and C&I in our draft) farms gold for 53-73% of its rounds and gets strong, but rarely hits the group. This holds even against starter-cpp.
3. **Gold is useful.** Items are how v2 finally clears the group: Tavi at 3,075 HP solos it, and Vendetta reaches 3,920 HP for the second clear. Stopping gold farming (gems-first targeting, V2/V4) cut gold from about 12,000 to 5,000 a game and lost badly (7/12 and 6/12, deaths up to 26.8).

## 3. Variants on stampede
Base is v2 from `results/v2-panel.json`: 9.67/12, median win round 1,752, 17.4 deaths, gem margin +610.

| variant (flags in `map_stampede.cpp`) | salts | panel W /12 | dW | dStrong | dGem | median win round | deaths | h2h vs v2 |
|---|---|---|---|---|---|---|---|---|
| V1 gem_alt | 0 | 9 (base s0: 11) | - | - | - | 1,494 | 13.2 | 0/2 |
| V2 gems-first + shared claim | 0 | 7 | - | - | - | 1,579 | 19.2 | 1/2 |
| V4 V2 + nearest | 0 | 6 | - | - | - | 1,960 | 26.8 | 0/2 |
| V5 nearest + gem_alt + shop trip at 300 gold + Tavi field shop | 0-2 | 9.00 | -0.67 | -1.33 | -360 | 1,492 | 10.9 | **0/6** |
| V3 nearest + gem_alt | 0-2 | 11.00 | +1.33 | +0.67 | +360 | 1,615 | 14.2 | 3/6 |
| V3 | 3-5 | 8.33 | | | | 1,674 | 15.2 | 2/6 |
| **V3, 6 salts pooled** | 0-5 | **9.67** | 0 | | | 1,615 | 14.7 | **5/12** |
| V6 = V3 + shared gem claim (`squads.cpp:124`) | 0-2 | 10.33 | +0.67 | +0.33 | +720 | 1,521 | 18.4 | 5/6 |
| V6 | 3-5 | 9.00 | | | | 1,651 | 19.3 | 4/6 |
| **V6, 6 salts pooled** | 0-5 | **9.67** | 0 | | | 1,651 | 18.8 | **9/12** |
| nearest alone (from the 17-map run) | 0-2 | 10.00 | +0.33 | -0.33 | +300 | 1,457 | 12.9 | 3/6 |
| nearest alone, calm-only (from the 17-map run) | 0-2 | 10.67 | +1.00 | +0.67 | +600 | 1,461 | 12.8 | 3/6 |

- **V6 replication at salts 6-8** (`results/w10-v6-stampede-s678.json`, 20:00-20:11): panel 9.00/12, median win round 1,436, 14.2 deaths, head-to-head 3/6.
  - These salts burn 120-160k fuel on turn 1, which caused 252 late-fuel events. That distorts the opening, so treat them as weaker evidence.
  - **V6 over 9 salts:** panel 9.44/12 (v2: 9.67), median win round 1,521, 17.3 deaths, head-to-head **12/18** against a 9/18 mirror (p about 0.12). The head-to-head lead weakened on replication.
- **Head-to-head baseline:** v2 against x-v4-14, which plays stampede identically to v2 (same game records), is won by side A in all 3 salts. So a no-op scores 3/6 head-to-head (6/12 over 6 salts).
- **V6's 9/12** is the only head-to-head signal. It wins 4 of 6 side-B games, where V3 and v2 lose. The binomial p is about 0.07, so it is weak.
- **Cell gains at 3 salts did not survive 6 salts.** Both V3 and V6 pool to v2's 9.67/12.

## 4. The retreat fix on all 17 maps (lead's item 2), 3 salts, against v2
`P_NEAREST_INCOMING 1`, applied globally, with no stampede flags.

| map | v2 | nearest dW | dStrong | dGem | h2h | calm-only dW | dStrong | dGem | h2h |
|---|---|---|---|---|---|---|---|---|---|
| ambush | 10.67 | 0.00 | 0.00 | -33 | 0/6 | 0.00 | 0.00 | -28 | 0/6 |
| collision | 9.00 | -2.33 | -0.67 | -2,235 | 1/6 | -1.33 | -0.33 | -1,229 | 1/6 |
| contest | 10.00 | 0.00 | 0.00 | 0 | 2/6 | 0.00 | 0.00 | 0 | 2/6 |
| corners | 6.00 | -1.00 | -0.33 | -415 | 3/6 | 0.00 | -0.33 | +473 | 3/6 |
| eight | 8.00 | 0.00 | -1.00 | +734 | 0/6 | 0.00 | -1.00 | +80 | 0/6 |
| elkring | 9.33 | -0.67 | -1.67 | -948 | 1/6 | -0.33 | -0.67 | -906 | 4/6 |
| expanse | 11.00 | -0.67 | -0.67 | -93 | 2/6 | -0.67 | -0.67 | -244 | 2/6 |
| fragment | 9.00 | 0.00 | 0.00 | -256 | 6/6 | -2.00 | -1.00 | -717 | 0/6 |
| interlock | 6.67 | +0.33 | 0.00 | -425 | 4/6 | +1.00 | +0.33 | -150 | 4/6 |
| lanes | 7.33 | +0.67 | 0.00 | -161 | 3/6 | +1.67 | +0.33 | +458 | 4/6 |
| monolith | 11.00 | +0.67 | +0.67 | +877 | 6/6 | +0.67 | +0.67 | +933 | 6/6 |
| packs | 10.00 | -2.00 | 0.00 | -1,012 | 3/6 | -1.00 | 0.00 | -956 | 3/6 |
| pilgrimage | 10.00 | -2.00 | -1.33 | -1,573 | 4/6 | -2.67 | -1.67 | -2,113 | 4/6 |
| rows | 11.67 | 0.00 | 0.00 | -132 | 2/6 | 0.00 | 0.00 | -132 | 2/6 |
| rushbait-v2 | 4.33 | +1.00 | +1.00 | +1,131 | 3/6 | +1.00 | +0.67 | +847 | 1/6 |
| stampede | 9.67 | +0.33 | -0.33 | +300 | 3/6 | +1.00 | +0.67 | +600 | 3/6 |
| swirl | 8.00 | -0.33 | 0.00 | -1,011 | 1/6 | +0.67 | +0.33 | +305 | 5/6 |
| **total** | | **-6.00** | | | | **-2.00** | | | |

- **Elk-map head-to-head**, nearest / calm-only:
  - corners 3/6 and 3/6
  - eight 0/6 and 0/6
  - elkring 1/6 and 4/6
  - ambush 0/6 and 0/6
  - swirl 1/6 and 5/6
  - rushbait-v2 3/6 and 1/6
- **Why the calm-only variant exists:** in the collision trace (x-hello, side B, salt 0) the nearest variant lost with 14 deaths against v2's 5, and 11 of the 14 were to enemy champions. An enemy standing nearer a camp also lowers our counted damage, so champions stayed in enemy contact longer. Calm-only (`P_NEAREST_CALM`, the new count only while no enemy is in view) halved the loss (dW -2.00) but still regressed pilgrimage (-2.67), fragment (-2.00) and collision (-1.33).
  - Lanes clears the bar (+1.67) in that run only; it was +0.67 in the other, and cmpcells' own note warns lanes moves +2 on a no-op. I do not trust it.
- **Rerun:** the calm run's first batch stopped at monolith salt 2 (`run-batch` rc 1, 93 games missing). I reran those games (`results/w10-calm-s2fill.json`) and merged them into `results/w10-calm-all-merged.json`. The table uses the merged file.
- **Verdict: the retreat fix is rejected globally.**

## 5. Safety against starter-cpp (stampede, both sides, 3 salts)
- **V3:** 6/6 wins, 0 faults, fuel_late 0. Win rounds 1,396 and 1,611.
- **V6:** 6/6 wins, 0 faults, fuel_late 0. Win rounds 1,389, 1,364 and 1,619.
- **"bot error" lines:** 1 in each of the side-B salt-1 and salt-2 games, for both V3 and V6. All read `engine error 97: Unit is not in vision`.
  - v2's two starter games had none. But v2's own panel games show 46 such lines on stampede over 36 games, against 57 over 42 for V3.
  - The paths I touched make no unchecked unit calls: `incoming_nearest` checks `is_visible()` and has a try/catch. I believe the error is inherited, but I have not proven it.
- **Late fuel:** salts 3-5 show 3 late-fuel events in V6. That is the salt burn itself (60-100k on turn 1), not the bot.

## 6. The moment the change fired (V3 against v2, side A, salt 1; won 1,080-0 at round 1,206)
- **Rounds 227-229:** Tavi stood at (53,21) with 630/750 HP, and three elks were within 4 tiles. v2's rule needs 795 HP, so v2 would have recalled Tavi to the spawn 55 tiles away. Under V3, Ryth was nearer to those elks, the counted damage was 0-120, and the need was 225-555, so Tavi kept shooting.
- **What followed:** our first capture came at round 525. In the reverse game, where v2 held side A against V3, v2's first capture came at round 597. The replay approximation finds such rounds only 5-15 times per game, which fits the small effect.

## Code state (`bots/w10`, built as V6)
- `map_stampede.cpp:11`: `P_W10_STAMPEDE (2|4|8)`, which sets shared gem claim, nearest-champion retreat and `gem_alt` in stampede's map plan only.
- New code:
  - `bot.cpp:76-99` `incoming_nearest`, and `bot.cpp:414-416` where it is used;
  - `bot.cpp:424-430` the shop trip (off);
  - `bot.cpp:106` and `bot.cpp:399` the Tavi field shop (off);
  - `squads.cpp:104-110` and `squads.cpp:124`;
  - `stuck.cpp:62-63` and `micro.cpp:72-77`;
  - `bot.h:120-127`, and `params.h:260-271` (`P_GEM_ALT 0`, `P_NEAREST_INCOMING 0`, `P_NEAREST_CALM 0`).
- On every other map the flags are off, so behaviour equals v2a. That is an argument, not a measurement.
- My only `choose_target` edits are the `alt_own_half` condition (`squads.cpp:110`) and the `other_claim` line (`squads.cpp:124`). I did not touch the score.

## Falsifiers and verdict
- **Fix 1 (`gem_alt`):** the falsifier was "stampede gem margin and wins against v2 do not rise". It was met. Rejected.
- **Retreat fix, global:** the falsifier was "any map regresses beyond noise". It was met, on pilgrimage, collision and packs/fragment. Rejected.
- **Stampede V3/V6:** panel dW is 0 at 6 salts and -0.22 for V6 at 9 salts, below the 1.5 bar. Not admitted. V6's head-to-head fell from 9/12 to 12/18 on replication, so it is not a reason to ship.
- **Next lever for pace:** elk-fight micro.
  - Ranged champions at distance 3 from the x=56 column face only 1-2 elks; melee adjacent to the block takes all 6.
  - The other squad, which holds the items, should join the group.
  - Gold carried at the spawn should actually get spent.
