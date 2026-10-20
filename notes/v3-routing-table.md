# v3 routing table: which existing bot is best per map (3 salts, panel of six)

Written 01:16 BST 19 Oct 2026. Runs 23:42 to 01:10 BST (18-19 Oct) on the 17 bundled map files. Data: `results/v3sim-<bot>.json` (612 games each: 17 maps x 6 opponents x 2 sides x salts 0,1,2), `results/v1-base.json` (22:21 18 Oct, same recipe), comparisons in `results/v3sim-cmp-<bot>.txt`. Panel: x-v4-14, x-foundry-v16 (strong pair), x-hello, x-ifish, x-mite, x-shawn.

## Method

- Bots: `v1`, `b-v418d` (run as `bots/b-v418d-salt`), `b-msnext` (`bots/b-msnext-salt`), `v1-414` (`bots/v1-414-salt`). The two b-* copies burn `P_SALT*4000` volatile iterations on the first play turn in `main.cpp` (pattern of `bots/v2/src/main.cpp`); mapgate builds the -s1 and -s2 copies.
- Departure from the brief: `bots/v1-414` has NO salt code (grep finds none), so `bots/v1-414-salt` carries v1's own burn in `Plan::init`. Check: 14 of 17 maps are cell-for-cell identical to `v1-base` including gem margins (the three that differ are corners, expanse, rows), so the salt placement matches v1's.
- Per map: "wins" is the mean over 3 salts of wins out of 12 (6 opponents x 2 sides). dW, dStrong (4 games against the strong pair), dGem (summed gem-margin change) are `scripts/cmpcells.py` against `v1-base`. Admission: dW >= 1.5, dGem > 0, dStrong >= 0; among several admitted bots take the largest dW; else v1.
- Rating adjustment: not applied. Every bot plays the same six opponents on the same maps, so opponent strength cancels in the differences.
- Fuel: the late count re-parses the kept statistics JSON and counts `fuelExhausted` entries for our side on rounds > 3 only (mapgate's `fuelL` includes round 3, where the salt burn spills: it showed 48 on expanse for two bots, 0 after the filter).
- Rerun: 7 games (v1-414-salt, expanse, salt 2) timed out at 300 s while the machine stalled (the batch took 3,327 s, not about 12 min) and mapgate scored them as losses. They were rerun alone (7 wins, same final rounds as at the timeout) and patched into the JSON; the original is `results/v3sim-v1-414-salt.orig.json`. Unpatched, v1-414 expanse read 9.33 (-1.33), which was an artefact.

## Per-map table (mean wins of 12, 3 salts, 36 games per bot per map)

Map id: the six old-layout files are ids `<name>_old`; the ladder has `<name>_v2`, which none of these runs touched. `rushbait-v2` is id `rushbait`.

| map id | v1 | b-v418d | b-msnext | v1-414 | best (admission rule) | dW / dStrong / dGem of best |
|---|---|---|---|---|---|---|
| ambush | 10.67 | 11.00 | 10.67 | 10.67 | v1 | |
| collision_old | 6.67 | 7.67 | 6.33 | 6.67 | v1 | (v418d +1.00 / +0.33 / +381) |
| contest | 10.00 | 10.00 | 10.67 | 10.00 | v1 | |
| corners_old | 3.67 | 5.33 | 4.33 | 4.00 | **b-v418d** | +1.67 / +0.33 / +823 |
| eight | 8.00 | 9.00 | 8.00 | 8.00 | v1 | (v418d +1.00 / 0.00 / +548) |
| elkring | 9.33 | 8.33 | 9.33 | 9.33 | v1 | |
| expanse_old | 10.67 | 11.67 | 11.33 | 11.67 | v1 | (v418d +1.00, v1-414 +1.00) |
| fragment | 9.00 | 8.00 | 9.00 | 9.00 | v1 | |
| interlock_old | 6.67 | 7.00 | 6.67 | 6.67 | v1 | |
| lanes_old | 7.67 | 8.67 | 7.33 | 7.67 | v1 | (v418d +1.00 / 0.00 / +15) |
| monolith | 11.00 | 11.00 | 11.00 | 11.00 | v1 | |
| packs_old | 10.00 | 10.00 | 10.00 | 10.00 | v1 | |
| pilgrimage | 6.67 | 10.33 | 5.67 | 6.67 | **b-v418d** | +3.67 / +2.33 / +2,552 |
| rows | 11.67 | 12.00 | 12.00 | 11.00 | v1 | |
| rushbait | 5.00 | 6.33 | 6.00 | 5.00 | v1 | (v418d +1.33 / +0.33 / +1,484) |
| stampede | 9.67 | 9.33 | 10.00 | 9.67 | v1 | |
| swirl | 6.33 | 7.33 | 7.00 | 6.33 | v1 | (v418d +1.00 / +0.67 / +1,669) |
| total /204 | 142.67 | 153.00 | 145.33 | 143.33 | routed 148.00 | |

Deltas for every candidate and map are in `results/v3sim-cmp-b-v418d.txt`, `-b-msnext.txt`, `-v1-414.txt`. Per-salt wins are in the appendix at the end.

## What the data shows

- b-v418d is the only candidate that clears the rule: pilgrimage (+3.67) and corners_old (+1.67). Routing by the rule gives 148.00 against v1's 142.67 (+5.33 games of 204; biased upward, see caveats).
- b-v418d wins more than v1 on 11 maps and loses on elkring (-1.00, dStrong -1.00), fragment (-1.00, dStrong -1.00) and stampede (-0.33). The three losses are against the strong pair, as `PLAN.md` recorded for one sample.
- b-msnext admits nothing (largest dW +1.00 on rushbait; -1.00 on pilgrimage). v1-414 admits nothing (expanse +1.00; rows -0.67 with dGem -2,303, the cost of lacking the v4.15 rows relay). 14 maps are identical to v1.
- Replicate of b-v418d on its 5 earlier maps (`results/w3-dispatch.json`, B route in the dispatcher, 3 more salts, 22:48): pilgrimage +3.33 (now +3.67), eight +1.00 (+1.00), lanes +0.67 (+1.00), interlock -0.33 (+0.33), collision +2.33 (now +1.00). Pilgrimage and eight replicate; collision regressed from admitted to not admitted on the same code.
- Faults: 0 on all 1,836 games (`botFaults`), 0 engine diagnostic codes. Late fuel (rounds > 3), 36 games per bot per map:

| bot | late fuel events | where |
|---|---|---|
| b-v418d | 1 (1 of 612 games) | corners_old vs x-foundry-v16, side A, salt 1, round 27 |
| b-msnext | 0 | |
| v1-414 | 0 | |
| v1 | not recorded: `v1-base.json` has no fuel fields and its statistics JSON was not kept | `results/v1-fuelchk.json` (salt 0, expanse, lanes, corners): 1 late event on corners; PLAN records one inherited round-20 overrun |

  Rounds 1-3 events (b-v418d 2,594; b-msnext 2,600; v1-414 3,234, summed over 17 maps and 3 salts) are the salt burn plus map initialisation, by construction. The 146 lanes events that `w3-dispatch` logged for the B route did not recur: b-v418d on lanes has 0 events beyond round 2 here (the dispatcher added one `camp_groups()` call; cause not tested).

## Interpretation (mine, not data)

- Route pilgrimage to b-v418d. Evidence: two independent 3-salt runs agree (+3.33, +3.67), dStrong is positive in both (+2.00, +2.33) and the per-salt wins are 10/10/11 against v1's 6/5/9. This is already v2's route.
- Corners_old: do not route. The bar was cleared by 0.17 games on a 12-game cell set, the layout is old (ladder uses corners_v2), and v3's base is v2, whose own corners draft scored 6.00 (`results/v2-panel.json`) against b-v418d's 5.33. For swirl the same holds: v2's exclusion scored 8.00 against b-v418d's 7.33.
- Not admitted but worth a held-out look if time remains: rushbait (+1.33 / +0.33 / +1,484; b-msnext also +1.00, so both newer lines beat v1 there, which hints at something real) and collision_old (+1.67 pooled over 6 salts; old layout, failed the v2 held-out check).
- Expanse_old: all three candidates read about +1.00 because v1's salts 1 and 2 dip to 10 (v1 12/10/10, others 12/12/10-11). That looks like a v1 chaos dip, not a candidate gain.

## Caveats

- Sample size: 36 games per bot per map, but the 3 salts replay the same 12 cells with different fuel trajectories, so they are not 36 independent games. dStrong rests on 12 games per bot (below 30: treat every dStrong as indicative). Each cell is 3 games. Totals over 17 maps are 612 games (all above 30).
- Salt noise only: pooled salt SD of a map's wins is 0.8 to 0.9 games (out of 12), so the SE of a 3-v-3 dW is about 0.7 and a null candidate reaches dW >= 1.5 with probability about 1.7 % per map. Over 51 candidate-maps that is about 0.9 spurious admissions before the dGem and dStrong conditions. Salts do not sample opponents outside the panel; that gap (not salt noise) is what selection bias acts on, and the collision regression shows it.
- Selection bias: best of 3 candidates per map, and the routed total (+5.33) is the winner's-curse figure. Recommend a held-out check (tung-clone, hunter, raider2, locker, v5-base, as in R4) for any map routed away from v1: pilgrimage (v2 as a whole scored 142 against v1's 139 on the held-out panel, one sample, `VERSIONS.md`) and corners_old if it is ever routed.
- Six maps are old layouts (collision, corners, expanse, interlock, lanes, packs): their rows say nothing about the `_v2` ladder layouts; routing of those ids is untested.
- Single panel of six: wins over 17 maps x 6 sides x 3 salts (102 games each) for v1 / b-v418d: x-v4-14 52 / 63, x-foundry-v16 46 / 44, x-hello 68 / 75, x-ifish 87 / 97, x-mite 80 / 83, x-shawn 95 / 97. b-v418d's gain is mostly against x-v4-14 and x-ifish; against x-foundry-v16 it is level.
- Baseline: `v1-base` is v4.15 plus hooks, not v2 (v2 adds the corners draft and the swirl exclusion). Rerun against `v2a` before routing corners or swirl.
- Two harness effects: a batch stall turns timeouts into scored losses (checked by `error` fields: none remain), and the salted copies add opening overruns on rounds 1-3.

## Appendix: per-salt wins of 12 (s0/s1/s2)

| map | v1 | b-v418d | b-msnext | v1-414 |
|---|---|---|---|---|
| ambush | 11/11/10 | 11/11/11 | 10/11/11 | 11/11/10 |
| collision_old | 8/6/6 | 9/5/9 | 8/5/6 | 8/6/6 |
| contest | 11/9/10 | 10/10/10 | 11/11/10 | 11/9/10 |
| corners_old | 3/4/4 | 5/6/5 | 5/4/4 | 4/4/4 |
| eight | 8/8/8 | 9/9/9 | 8/8/8 | 8/8/8 |
| elkring | 9/9/10 | 8/8/9 | 9/9/10 | 9/9/10 |
| expanse_old | 12/10/10 | 12/12/11 | 12/12/10 | 12/12/11 |
| fragment | 9/9/9 | 8/8/8 | 9/9/9 | 9/9/9 |
| interlock_old | 6/7/7 | 8/8/5 | 7/7/6 | 6/7/7 |
| lanes_old | 8/8/7 | 8/9/9 | 6/8/8 | 8/8/7 |
| monolith | 11/11/11 | 11/11/11 | 11/11/11 | 11/11/11 |
| packs_old | 10/10/10 | 10/10/10 | 10/10/10 | 10/10/10 |
| pilgrimage | 6/5/9 | 10/10/11 | 6/6/5 | 6/5/9 |
| rows | 12/12/11 | 12/12/12 | 12/12/12 | 11/11/11 |
| rushbait | 4/6/5 | 6/7/6 | 5/8/5 | 4/6/5 |
| stampede | 11/9/9 | 9/10/9 | 11/10/9 | 11/9/9 |
| swirl | 7/7/5 | 6/8/8 | 7/7/7 | 7/7/5 |
