# w7: ladder-grounded static drafts (part 1) and a data-driven counter-draft (part 2)

Builder w7, 19 Oct, 01:19-02:20 and 17:25-18:40 BST. The machine slept from 02:20 to 17:05; the lead moved the stop
to 21:00. Base: `bots/v2a/src`. Panel: x-v4-14, x-foundry-v16, x-hello, x-ifish, x-mite, x-shawn, both sides.
Base numbers: `results/v2-panel.json` (v2, 3 salts). Note that v2 routes pilgrimage to v418d; w7 plays v2a code there.

## Verdict

- **Part 1: admit eight (D list) and stampede (R list). Everything else stays on v2a's lists.**
  - eight, C&I+Stellar+Tavi+Ryth+Ren: panel 8.00 to 10.00 of 12, dStrong +1.00, dGem +2,684. Head-to-head vs v2 6/6, +336 gems a game.
  - stampede, Tavi+Stellar+Ryth+Varg+Ren: panel 9.67 to 11.67, dStrong +1.33, dGem +2,760. Head-to-head 5/6, +480 gems a game.
  - Median win round improves on stampede (1,752 to 1,371) and on eight (686 to 656).
  - Held-out panel: eight 6/10 to 9/10, stampede 8/10 to 9/10; no cell lost.
  - Starter-cpp: 4/4 wins, 0 faults.
  - Deviations for the release check:
    - Caught "bot error" lines (engine error 97 on units our own spells killed) rise with caster drafts, for example eight 27 to 96 per 36 games. Cost: under 0.1% of champion-turns. Cause located (Test 4); not fixed.
    - One late `fuelExhausted` (our Stellar, stampede, round 1,353) in one game.
- **The effects are small and thinly sampled.**
  - Over all 17 maps the data-driven lists (D) score dW -1.00. The ladder-report lists (R) score +0.67 on the 15 maps where they differ from D.
  - Eight's gain is one effective sample: all three salts give identical games in every cell. Stampede has about two.
  - The head-to-head is not independent evidence. On eight and stampede, v2 plays exactly like x-v4-14: the h2h games equal the x-v4-14 panel cells move for move.
- **Part 2: built, but the table fires on 0-0.1% of the ladder's partial drafts and on 0 of 96 panel games.** It changes no game: part 2 vs part 1 gave 96/96 identical cells. Evidence below.
  - The ladder data backs no counter-pick at our thresholds: every candidate swap that clears the margin on all data reverses sign between the two halves of the data.
  - Ship it with its flag off (`W7_CD_ALL 0`). It is a no-op until pick-ordered data exists.

## What changed (`bots/w7/src`, diff against `bots/v2a/src`)

| File | Change |
|---|---|
| `w7_drafts.h` (new) | Per-map lists: `draft_r` (:25), `draft_d` (:43), `draft_p1` (:84), `counter_on` (:96), `draft_for` (:101). `W7_SET` selects 0 v2a / 1 R / 2 D / 3 P1; the default is 3. |
| `plan.cpp:64-65` | `p.draft = *w7::draft_for(id)` after the map's setup (it overrides corners' map-file list only where P1 names a list: eight, stampede). `p.counter = w7::counter_on(id)`. |
| `plan.h:25` | `MapPlan::counter` (bool, default false). |
| `draft.cpp:30-33` | When `counter` is set, `w7cd::pick` may replace the static last pick. |
| `counter.h` (new) | The counter rule. It mirrors `decide()` in the generator. |
| `counter_table.h` (new, generated) | Written by `tools/v3/w7_counter_table.py --static tools/v3/w7_static.json`. |

Tools (new files): `tools/v3/w7_drafts.py` (per-map draft excess), `w7_query.py`, `w7_counter_table.py`, `w7_static*.json`, `w7_merge.py`, `w7_boterr.py`.
Test copies: `bots/w7d` (D), `bots/w7r` (R), `bots/w7c` (P1 with the counter on), `bots/w7dbg` (error-stage tags, not for release).

Rule lines relied on:
- GameRules, Drafting: "both teams select champions simultaneously, revealing their picks as they go".
- GameRules, Drafting: a champion "Selected by the opposing team" is allowed.
- GameRules, Drafting: draft memory is 1,024 bytes.
- `sdk/cpp/cambc.h:5045`: `draft::current_draft(team side)` "completed draft picks".
- `BotAPICpp.md:624`: "the first draft turn gets 10x [fuel]".
- GameRules kit sections, quoted in the kit table below.

## Data (intel/matches-2026-10-18.csv), and why pick order is unusable

- All 18,490 side-drafts in the CSV are sorted alphabetically, so pick order cannot be recovered. The brief's per-pick table was therefore built on sets.
- Excess = win minus the Elo expectation from pre-group ratings (400-point scale). Shrunk = excess × n / (n + 30).
- **D rule (written before testing):**
  - Take the highest shrunk excess among drafts with n >= 30 on the map.
  - Exclude Willow, whose heal is never cast.
  - Break ties within 2 pp toward full kit use (no unused dash), then toward more distinct teams.
- **R lists:**
  - 5-pick maps: Tavi/Stellar/Ryth/Varg/Ren.
  - 3-pick maps: C&I, Tavi, Stellar.
  - 4-pick maps: C&I, Tavi, Stellar, Ryth.
  - swirl: "Fish's roster", C&I+Ren+Ryth+Stellar. Intercontinental Fish (rated 2,089) went 2-0 on swirl with it. `bots/x-ifish` has no swirl entry in its table (`clone_config.h`), so it falls back to C&I, Varg, Stellar, Vendetta.

| Map (size) | D list: n, excess, shrunk, teams | R list on ladder: n, excess | v2a list on ladder: n, excess |
|---|---|---|---|
| ambush (5) | C&I+Ryth+Stellar+Tavi+Varg: 81, +11.8, +8.6, 27 | 11, -4.5 | 11, -13.2 |
| collision (3) | Prism+Ren+Stellar: 31, +17.7, +9.0, 5 | 71, -1.7 | 5, -29.8 |
| contest (5) | C&I+Prism+Ryth+Stellar+Tavi: 40, +18.5, +10.6, 13 | 22, +0.1 | 13, +19.2 |
| corners (5) | Prism+Ren+Ryth+Stellar+Tavi: 56, +13.4, +8.7, 11 | 22, -9.0 | (w1 list) 359, -2.8 |
| eight (5) | C&I+Ren+Ryth+Stellar+Tavi: 86, +7.4, +5.5, 17 | 17, -15.0 | 7, +7.0 |
| elkring (5) | C&I+Ryth+Stellar+Tavi+Varg: 99, +1.1, +0.9, 27 | 14, +13.9 | 11, +4.6 |
| expanse (3) | C&I+Ryth+Varg: 32, +37.5, +19.3, 12 | 26, -7.7 | 9, -13.4 |
| fragment (3) | C&I+Stellar+Tavi: 70, +10.1, +7.1, 23 (= R) | = D | 2 |
| interlock (4) | C&I+Ren+Stellar+Varg: 154, +6.5, +5.4, 47 | 19, -6.2 | 36, +8.4 |
| lanes (5) | C&I+Ryth+Stellar+Tavi+Varg: 67, +9.5, +6.5, 23 | 5, -50.0 | 11, -31.9 |
| monolith (5) | C&I+Ren+Ryth+Varg+Vendetta: 53, +21.3, +13.6, 10 | 6, -33.6 | 12, +0.1 |
| packs (3) | C&I+Stellar+Tavi: 80, +12.5, +9.1, 23 (= R) | = D | 4 |
| pilgrimage (5) | C&I+Ren+Ryth+Stellar+Varg: 329, +8.0, +7.3, 102 | 15, +3.2 | 18, -5.7 |
| rows (4) | C&I+Stellar+Varg+Vendetta: 79, +12.5, +9.0, 21 | 4, -24.9 | (rows list) 124, +5.9 |
| rushbait (4) | C&I+Ren+Ryth+Stellar: 185, +6.3, +5.4, 54 | 14, +29.3 | 41, -8.7 |
| stampede (5) | C&I+Prism+Ryth+Stellar+Tavi: 116, +16.9, +13.4, 32 | 18, +0.1 | 6, -16.9 |
| swirl (4) | C&I+Ryth+Stellar+Varg: 138, +6.1, +5.0, 44 | Fish: 202, +2.1 | 27, -16.6 |

- The ladder report's 5-pick list without C&I is rare on the ladder (5-22 games a map) and has mixed excess. The data favours C&I in nearly every strong draft.
- The ladder figures measure other teams' code, not ours. The panel results below show how little of it transfers.

## Kit check (which champions our code actually plays)

Casting is generic by ability kind:
- `cast_on_camp` (util.cpp:82-93) and `cast_on_enemy` (enemies.cpp:57-67) fire any ready ability where `is_damage() || is_aoe_damage() || is_cc()` and the target is in `valid_tiles()`.
- `reset_and_attack_again` (util.cpp:59-72) handles attack resets.
- `cast_shield` (util.cpp:95-104) is used for Varg only.
- No file calls `is_dash()` or `is_heal()` (grep: 0 hits).

Casts were measured from the action log of `tools/v3/extract_pilot.mjs` (ability slot and kind per cast) in four games: v2 corners and ambush vs x-hello, w7d stampede and monolith vs x-hello.

| Champion | Slot 0 | Slot 1 | Coverage |
|---|---|---|---|
| Cassi | 2xF damage r4: cast 17-83 | stun r4: 14-72 | full |
| Issac | attack reset: 12-81 | dash: **never** | dash unused |
| Ryth | 1xF area stun (ground r4): 19-30 | reset: 20-30 | full (the ground spell fires through `use(camp)`/`use(foe)`) |
| Stellar | 2xF damage r4: 16-54 | 1.4xF area (ground r4): 14-50 | full |
| Tavi | reset: 22-48 | 2.2xF dmg+stun r4: 23-58 | spells full; field shard buying unused |
| Varg | reset: 18-24 | shield: 25-45 | full |
| Ren | 1.25xF dmg+stun r4: 11-28 | dash: **never** | dash unused |
| Prism | 1.5xF dmg+stun r4: 51 | 1xF area r4: 47 | full |
| Vendetta | reset: 23-32 | dash: **never** | dash unused |
| Willow | heal: **never** | reset | excluded from all lists |

- Cast rates sit well below the cooldown ceiling of about 59 casts per 475 rounds. Ryth stunned 25 times in 1,822 rounds of stampede. This is execution, not kit.
- No one-line fix exists: a dash needs a destination policy, the heal a target policy.

## Test 1: panel, 3 salts (`cmpcells` against v2-panel; admit at dW >= 1.5, dGem > 0, dStrong >= 0)

| Map | v2 | D | D dW / dStrong / dGem | R | R dW / dStrong / dGem | Decision |
|---|---|---|---|---|---|---|
| ambush | 10.67 | 10.33 | -0.33 / -0.33 / -166 | 11.00 | +0.33 / +0.33 / +252 | v2a |
| collision | 9.00 | 7.33 | -1.67 / -0.33 / -263 | 6.00 | -3.00 / -0.67 / -832 | v2a |
| contest | 10.00 | 10.67 | +0.67 / +0.67 / 0 | 11.33 | +1.33 / +1.33 / 0 | v2a (dGem 0, dW < 1.5) |
| corners | 6.00 | 4.00 | -2.00 / -0.33 / -3,229 | 5.00 | -1.00 / -0.33 / -1,447 | v2a (w1 list) |
| **eight** | 8.00 | **10.00** | **+2.00 / +1.00 / +2,684** | 10.00 | +2.00 / +1.00 / +1,866 | **D** (higher dGem; h2h 6/6 vs R's 3/6) |
| elkring | 9.33 | 9.33 | 0 / -0.67 / -792 | 9.00 | -0.33 / -0.33 / -432 | v2a |
| expanse | 11.00 | 11.33 | +0.33 / -0.33 / +743 | 11.33 | +0.33 / 0 / +802 | v2a |
| fragment | 9.00 | 9.00 | 0 / -1.00 / +1,203 | = D | | v2a |
| interlock | 6.67 | 6.67 | 0 / 0 / -75 | 7.33 | +0.67 / +0.33 / +869 | v2a |
| lanes | 7.33 | 7.33 | 0 / 0 / -63 | 8.33 | +1.00 / -0.33 / -153 | v2a |
| monolith | 11.00 | 10.33 | -0.67 / -0.67 / +71 | 11.00 | 0 / 0 / +453 | v2a |
| packs | 10.00 | 10.00 | 0 / 0 / -846 | = D | | v2a |
| pilgrimage | 10.00 (v418d) | 10.33 | +0.33 / 0 / +883 | 9.67 | -0.33 / -0.33 / +671 | v418d (v2 route) |
| rows | 11.67 | 12.00 | +0.33 / 0 / +301 | 11.67 | 0 / 0 / +62 | v2a |
| rushbait-v2 | 4.33 | 4.33 | 0 / +0.33 / -1,303 | 3.33 | -1.00 / +0.67 / -2,104 | v2a |
| **stampede** | 9.67 | 9.67 | 0 / +0.67 / +720 | **11.67** | **+2.00 / +1.33 / +2,760** | **R** |
| swirl | 8.00 | 8.00 | 0 / +0.67 / +476 | 6.67 | -1.33 / +0.67 / -2,947 | v2a |
| total | 151.67 | 150.65 (dW -1.00) | | | +0.67 over 15 maps | |

Runs and anomalies:
- Files: `results/w7d-panel.json` (612 games); `results/w7r-panel.json` (540 games, merged from `w7r-panel-s0.json`, `-s12.json` and `w7r-eight-fv16-s0.json`).
- The merge replaces one R game (eight vs foundry-v16 B, salt 0) that timed out at round 47 while the machine slept. It was not a bot fault.
- Two variants a map: D and R; on fragment and packs they are the same list.
- Swirl's "Fish" roster cost 1.33 wins and 2,947 gems.
- R's 5-pick list (no C&I) cut deaths on every 5-pick map. Examples: stampede 17.4 to 7.7, contest 20.0 to 9.6, eight 9.7 to 3.6 a game.
- R also gained a win or more without clearing the bar on contest (+1.33, but dGem is 0 because nobody scores there) and lanes (+1.00, dStrong -0.33). A further salt or held-out run could settle those.

## Test 2: head-to-head vs v2, 3 salts (`results/w7-h2h.json`, `results/w7r-h2h-eight.json`)

| Map | List | Wins of 6 | Gem margin a game | Median win round |
|---|---|---|---|---|
| eight | D (P1) | 6/6 | +336 | 666 |
| eight | R (rejected) | 3/6 (side A loses 678-1,004 in every salt) | -152 | 643 |
| stampede | R (P1) | 5/6 (A salt 0 lost 900-1,260) | +480 | 1,371 |

- On these maps v2 behaves exactly like x-v4-14: v2a's lists equal v4.14's, and the rows relay is irrelevant here.
- Every h2h game reproduces the matching x-v4-14 panel cell move for move (same rounds, gems and deaths).
- So the h2h confirms the panel, but it is not an extra sample.

## Test 3: pace (median win round; top-50 winners' median rounds to 1,000 from meta §3a)

| Map | v2 | P1 | top-50 yardstick |
|---|---|---|---|
| eight | 686 | 656 (D) | 640 |
| stampede | 1,752 | 1,371 (R) | 686 |

- Stampede is faster, but we are still twice the yardstick.
- Of the other maps, D is faster on pilgrimage (1,210 vs 1,276), fragment (765 vs 852) and swirl (1,184 vs 1,249). None of these was admitted.

## Test 4: safety

| Check | Result |
|---|---|
| starter-cpp on eight and stampede, both sides (`results/w7-starter.json`) | 4/4 wins, 0 faults, 0 late fuel events |
| Panel and h2h faults | 0 bot faults in all 1,152 panel games and 18 h2h games (the one "error" row is the sleep timeout) |
| Late fuel | 1 `fuelExhausted` after round 2, new against v2 (whose stampede had 0): our Stellar at round 1,353 of 1,495, stampede side A salt 0, against x-v4-14 and against v2 (the same game; stats kept in `results/w7-h2h-stampede-keep.json`). One suspended turn, which resumes the next round. The other 35 R stampede panel games and both starter games have none. expanse and lanes salt-1 late events match v2's (pre-existing salt burn). |
| Caught "bot error" lines (our side) | Rise with caster drafts. eight: 27 (v2) / 96 (D) / 45 (R) per 36 games; stampede: 46 / 113 / 92; vs starter eight 1 to 3, stampede 0 to 2. **Deviation from "no new bot error lines"**; see below. |

**Cause of the extra "bot error" lines** (debug copy `bots/w7dbg`, which tags the turn stage in the catch; `results/w7dbg.json`, 8 games): every line is engine error 97, "Unit is not in vision", from a handle to a unit our own spell had just killed.
- About 60% occur at enemies.cpp:120: `near_camp = ... min_distance(foe->position(), ...)` runs after `cast_on_enemy` killed the focus. `foe_gone` is only computed `if (P_ALLY_VISION_FIX)` (enemies.cpp:116), and that flag is 0 (params.h:199-200).
- The rest occur in `bank` → `choose_altar(v, enemies, ...)` (bot.cpp:262), on the same stale `enemies` list. `keep_visible` (bot.cpp:425) is also behind the same flag.
- Casters drafted: Stellar, Prism, Cassi and Tavi.
- Cost: each error ends that champion's turn. At 2-3 a game against about 3,000-9,000 champion-turns, the cost is under 0.1% of turns.
- Not fixed: setting `P_ALLY_VISION_FIX 1` would also change targeting on every map and needs its own test.

## Held-out check (tung-clone, hunter, raider2, locker, v5-base; salt 0; both sides; `results/w7-heldout.json` vs `results/v2-heldout.json`)

| Map | v2 | P1 | Cells flipped |
|---|---|---|---|
| eight (D) | 6/10, gem margin +158 | 9/10, +420 | won raider2 B, locker B, v5-base B; none lost |
| stampede (R) | 8/10, +558 | 9/10, +666 | won raider2 A; none lost |

R4 ("no changed map loses more held-out games than its base") passes on both maps.
- Caveat: v5-base, x-v4-14 and v2 play identical games against us on these maps.
- So the held-out stampede late-fuel event (v5-base A) is the same game as the one in the safety table.

## Test 5: traced loss (eight vs x-foundry-v16, side B, salt 0; identical in all salts)

`matches/w7d-013757/eight__x-foundry-v16__B__s0.repl26a`; decoded with `node final.mjs`, `extract.mjs` + `deaths.py`, and `tools/v3/extract_pilot.mjs`.

- D lost 22 to 1,030 at round 567. v2 lost the same cell 694 to 1,030 at round 857.
- Our side died 18 times and tributed 22 gems in total.
- The change visibly at work: the D roster has no Varg or Vendetta, so Ryth is our only frontliner.
  - Foundry's Stellar was credited in most of our deaths.
  - Our Stellar died three times carrying all of her 226 gems (22 at round 47, 72 at round 252, 132 at round 518 at (18,15), killed by foundry's C&I and Tavi).
  - Our C&I gained 0 gems and 0 gold in 567 rounds (3 linked deaths).
  - In v2's version of the game, Tavi earned 634 gems behind Varg and Vendetta.
- Foundry-v16 is lost in both runs (v2 0/2 on this map). D trades a slower loss there for wins against hello B and v4-14 B.
- Likely cause of the loss: our casters lack a front line against a burst-heavy enemy, and the code does not focus the enemy Stellar. This is the same pattern w6 traced on pilgrimage.

## Part 2: counter-draft

**Mechanism** (`counter.h`, `tools/v3/w7_counter_table.py`). It acts at our last pick only: there is no denial, so waiting is free.
- Preconditions:
  - Our first k-1 picks must equal the static list's first k-1.
  - The static last pick s must be in the table (its roster needs n >= 30).
- Scoring, for each candidate c (never Willow or Vesper):
  - score = base(c) + mean over the seen enemy picks e of I(c, e).
  - base = the shrunk excess of our roster R'+c.
  - I(c, e) = the shift of that roster's excess when the enemy held e, shrunk by n / (n + 30). It counts only with n >= 30 on all data and >= 15 in each half (halves split by crc32(group_id)).
- Firing condition, all of:
  - the gap over s is at least 5.0 pp on all data;
  - at least 2.5 pp of the gap comes from the enemy's picks (gap minus base gap);
  - the gap is positive in both halves.
- Fuel: one `current_draft` call and fewer than 300 table reads on one draft turn. The table is 10.7 KB of constexpr data; draft memory is untouched.

**Result (falsifier: "no data-backed response exists").**
- Simulated over every ladder side-game, dropping each enemy champion in turn to stand for the unseen last pick (about 80,000 partial drafts), the rule fires on:
  - 0.0% of partial drafts on all 17 maps with the D lists;
  - 0.1% with the P1 lists (corners only, Tavi for Ren, 6 cases);
  - 0 of 102 panel opponent drafts.
- Where some other pick beats the static one by the score, the halves disagree in sign every time. Examples: rows, Tavi for Vendetta, -6.5 / +7.2 pp; stampede, Ren, +7.8 / -4.8; fragment, Ren, +7.9 / -4.7.
- Pooled across maps, the largest champion-vs-champion interactions are 5-6 pp (z 3). They are uniform across our champions, for example everything does better against Marcella or Vesper. So they do not separate one candidate from another.
- The ladder report's "counter-drafting Marcella neutral or worse" agrees with this.
- **Game test, part 2 vs part 1** (salt 0, 8 maps where the table has entries, 96 games each; `results/w7c-panel-s0.json` vs `results/w7-p1-s0.json`):
  - The draft log shows `keep` in all 96 games; no override fired.
  - All 96 cells are identical in win, round, gems, opponent gems and deaths, and the wins are the same (65/96 each).
  - Part 2's own contribution on the panel is exactly zero.
- Recommendation: keep `W7_CD_ALL 0`. Regenerate the table from pick-ordered ladder replays (`tools/decode/`) before switching it on. With pick order and 10x more games, the cells could reach n >= 30 for real partial drafts.

## Falsifiers, set before the runs, and whether they were met

- Part 1: "a data list that loses panel games on its map" was met on corners (-2.00), collision (-1.67) and others, so those maps keep v2a's lists.
- Part 2: "no cell clears n >= 30 with half-sample agreement" was met. The table is a no-op on current data.

## Parameters

- Shrinkage K = 30 games; NMIN = 30; halves split by crc32(group_id) & 1.
- Counter rule: MARGIN 50 and INFO 25, both in 0.1 pp units (`W7_CD_MARGIN`, `W7_CD_INFO`).
- `W7_SET 3`; `W7_CD_ALL 0` (`bots/w7c` uses 1).
- P1 drafts:
  - eight: C&I, Stellar, Tavi, Ryth, Ren;
  - stampede: Tavi, Stellar, Ryth, Varg, Ren;
  - all other maps: v2a's lists, including the w1 corners list and v2's v418d route on pilgrimage.
