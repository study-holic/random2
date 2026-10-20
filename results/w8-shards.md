# w8: Tavi buys shards in the field (builder w8, 19 Oct 01:19-02:20 BST, written 17:20 BST)

**Status: partial.** The machine slept from about 02:20 to 17:16 BST, which is past the 04:30 stop. When it woke, I stopped the queued tests (head-to-head against v2, the draft-override variant and the starter safety run) and wrote this report from what had finished by 02:20. **Not admitted:** the head-to-head against v2 that the brief requires was never run.

## Question 0 (lead, 01:50): is our gold idle?

I measured v2's panel replays (`matches/v2-230053`, salt 0, 204 games, all 17 maps). Each replay was decoded with `tools/v3/extract_pilot.mjs` at stride 10 and summarised by a scratch script.

- **Spawn and death are already spent down to the last shard.**
  - Of about 9,860 spawn exits, none left with 30 gold or more (Ren 1 of 62).
  - Of all respawns, none came back with 30 gold or more. `buy_items` already spends everything on every spawn visit and while dead.
  - So the "surplus on each shop visit" part of the card has nothing to fix, and I built nothing for it.
- **Gold sits idle in the field.** Mean gold held while alive, and the share of alive rounds holding at least 30 gold:

| Champion | Mean held | Share ≥ 30 |
|---|---|---|
| Tavi | 190 | 0.52 |
| Varg | 146 | 0.47 |
| Vendetta | 100 | 0.40 |
| Stellar | 86 | 0.43 |
| Ryth | 84 | 0.39 |
| Cassi | 84 | 0.34 |
| Issac | 52 | 0.26 |

- **Tavi holds the most.**
  - At game end Tavi holds a mean of 298 gold.
  - In one example (v418d's Tavi on pilgrimage) Tavi held 225 to 2,000 gold for the last 340 rounds.
  - Only Tavi can spend in the field, so Tavi is the only champion this change can help.

## Rules relied on (docs/rules/GameRules.md)

- Line 676, Tavi passive: "Tavi can buy shards from anywhere on the map, note he cannot buy consumables unless he is on his spawn tiles (or dead)... Note, Health bought away from spawn raises maximum health without healing."
- Line 290: "Shards cost 30 gold... Five matching shards automatically combine into the corresponding emblem."
- Line 116: "Shop on your spawn or while dead (subject to champion exceptions)."
- Line 322: "Buying Health on your spawn heals the amount gained; Tavi's field purchases do not."
- SDK (`sdk/cpp/cambc.h:3620`): `can_buy(item, count)` checks the shop permission (`sdk_champion_can_shop`) and gold only. It does not check `invalid_action_buy_item_inventory_full` (53) or `_stats_overflow` (54), which is why field purchases are wrapped in a try/catch.
- `passive_type::remote_shop` (cambc.h:245) is Tavi's passive.

## What changed (bots/w8)

- **`params_override.h`:** `P_FIELD_SHOP 1` turns on v4's existing, previously disabled field-shop path for any champion with `remote_shop`. Only Tavi has that passive, so maps where Tavi is not drafted are unchanged. A new knob, `P_FIELD_SHOP_HEALTH`, is 1 in w8 and 0 in w8b.
- **`bot.cpp:166`:** `field` = alive and off the spawn.
- **`bot.cpp:179`, `bot.cpp:203`:** field purchases are wrapped in try/catch. If a purchase throws, field buying stops for 50 rounds, so a failing purchase cannot stall Tavi every turn. Spawn purchases still rethrow as before.
- **`bot.cpp:190-194`:** when `P_FIELD_SHOP_HEALTH` is 0, field purchases replace Health shards with main-stat shards.
- **`bot.cpp:373`:** the call site honours the back-off.
- **`bot.cpp:393`:** the `can_shop` recall gate is false for Tavi. This code is dead while `P_SHOP_RECALL` is 0.
- **`bot.h:73`:** `field_shop_off_until`.
- **Build order is unchanged:** Monster emblem of Strength (Tavi is not in the Focus list), then Power shards alternating Health and Strength.
- **Variants:**
  - w8 = field purchases follow the full build order.
  - w8b = no Health shards in the field. Field Health does not heal, so it lowers the health fraction that the retreat rules read.
- **Built but not tested:** w8d and w8d0 (draft Tavi, Stellar, Ryth, Varg, Ren on every 5-pick map, with and without field shop).

Tavi is drafted on ambush, contest, eight, elkring, lanes, monolith, pilgrimage, rows and stampede. On pilgrimage, v2 actually runs the v418d bot, so v1-base (= v2a) is the like-for-like baseline there.

## Test 1: panel, 3 salts (`results/w8.json`, `results/w8b.json`; base `results/v2-panel.json`)

Each cell is wins out of 12, then median win round.

| Map | v2 | v2a (v1-base) | w8 | w8b | Top-50 yardstick |
|---|---|---|---|---|---|
| ambush | 10.67 / 899 | same | 11.00 / 894 | 11.00 / 859 | 594 |
| contest | 10.00 / 2000 | same | 9.33 / 2000 | 9.67* / 2000 | 1,602 |
| eight | 8.00 / 734 | same | 9.00 / 629 | 9.00 / 652 | 640 |
| elkring | 9.33 / 590 | same | 9.67 / 572 | 10.00 / 605 | - |
| lanes | 7.33 / 435 | 7.67 / 442 | 8.00 / 464 | 9.00 / 462 | - |
| monolith | 11.00 / 584 | same | 11.00 / 566 | 11.67 / 558 | 447 |
| pilgrimage | 10.00 / 1278 (v418d) | 6.67 / 1242 | 8.00 / 1268 | 9.00 / 1271 | 844 |
| rows | 11.67 / 481 | same | 12.00 / 477 | 12.00 / 477 | 386 |
| stampede | 9.67 / 1752 | same | 10.00 / 1504 | 8.33 / 1542 | 686 |

\* One w8b game was lost to the machine sleep (contest vs x-v4-14, side A, salt 1, "matches failed") and is counted as a loss.

**cmpcells against v2:**
- w8: total dW +0.33. No map admitted; the best are eight +1.00 and lanes +0.67. Pilgrimage −2.00 (against v418d).
- w8b: total dW +2.00. Lanes is admitted (+1.67, ΔGem +973). Stampede −1.33 (−3,240 gems) and pilgrimage −1.00.

**cmpcells on pilgrimage against v1-base (v2a):** w8 +1.33; w8b +2.33, which clears the rule (admit).

Faults: 0 for w8, and the 1 sleep failure above for w8b. Late fuel suspensions on lanes (6 per salted game) appear identically in v2's salted runs, so they are inherited.

## Gold held at game end (salt 0, 12 games per map, decoded replays)

Each cell is team gold held at the end, then Tavi's mean held gold while alive, then Tavi's final maximum health.

| Map | v2 (v2a on pilgrimage) | w8 | w8b |
|---|---|---|---|
| ambush | 894 / 229 / 2,444 | 414 / 15 / 3,275 | 497 / 14 / 750 |
| contest | 2,728 / 207 / 3,350 | 2,176 / 14 / 3,825 | 1,943 / 13 / 794 |
| eight | 759 / 181 / 2,925 | 567 / 14 / 3,256 | 554 / 13 / 750 |
| elkring | 941 / 302 / 2,725 | 652 / 14 / 3,206 | 664 / 15 / 800 |
| lanes | 362 / 78 / 1,062 | 188 / 9 / 1,269 | 218 / 10 / 750 |
| monolith | 1,312 / 107 / 1,538 | 1,121 / 8 / 1,650 | 1,155 / 9 / 750 |
| pilgrimage | 1,915 / 157 / 1,538 | 1,226 / 11 / 2,038 | 1,186 / 10 / 750 |
| rows | 117 / 20 / 750 | 88 / 4 / 750 | 88 / 4 / 750 |
| stampede | 1,908 / 380 / 4,281 | 1,363 / 10 / 5,200 | 1,512 / 11 / 750 |

## Falsifier (fixed before testing)

"Reject if gold banked at game end does not fall, or median win round does not fall on the maps where Tavi plays."

- **Gold:** it fell on all 9 maps for both variants. Not falsified.
- **Median win round, w8:** it fell on 6 of the 8 maps decided by gems: stampede −248, eight −105, elkring −18, monolith −18, ambush −5, rows −4. It rose on lanes (+29 against v2) and pilgrimage (+26 against v2a). **Falsified on lanes and pilgrimage.**
- **Median win round, w8b:** it rose on elkring, lanes and pilgrimage.
- On contest, every game reaches round 2,000, so its median cannot move.

## Traced moments (decoded snapshots, no visualiser watch)

**Where the change fired: stampede vs foundry-v16, side A, salt 0, w8 smoke game.**
1. Round 720: Tavi captures a group at (56,23) and gains 960 gold and 360 gems.
2. The same turn he buys 32 shards in the field. His maximum health goes from 900 to 2,325 while his current health stays at 570, which is 24.5 % of the new maximum.
3. The retreat rule (below 30 %) fires, and Tavi recalls to (0,0) at round 726, healing to 2,325.
4. He then walks to the altar at (30,22) and tributes the 360 gems around round 795.

So field Health buys trigger extra recalls. This is why I built w8b.

**Loss: contest vs foundry-v16, salt 0.**
- v2 won both sides; w8 and w8b lost both. Contest is decided on lifetime gold earned: w8 lost 7,890 to 12,935 (side A) and 7,055 to 13,515 (side B).
- Per game, our C&I deaths rose from 9 to 15.5 and foundry's Tavi kills from 5.5 to 18.5. Our Tavi's deaths were unchanged at 8.0, and his maximum health rose from 2,662 to 2,925.
- The change does not touch C&I, so the likely cause is divergence on a map with about 20 deaths a game, not field shopping itself. This is not established: I did not watch the game in the visualiser.

## Verdict

**Not admitted.** The required head-to-head against v2 was not run. The panel gains are small and mixed:
- w8 improves pace on 6 of 8 maps but admits no map.
- w8b admits lanes, and pilgrimage against v2a, but loses stampede (−1.33 wins, −3,240 gems).

If v3 drafts Tavi everywhere, run next, in order:
1. The head-to-head against v2: `results/w8-h2h.json` and `results/w8b-h2h.json`, both still unrun.
2. w8d against w8d0, salt 0, on the 9 five-pick maps.
3. `--panel starter-cpp` on the Tavi maps.
