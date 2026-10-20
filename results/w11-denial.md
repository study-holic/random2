# w11: conversion denial (altar locks, interrupting enemy tributes, carrier focus) over v2. PARTIAL

Status: partial. The machine was suspended from about 02:22 to 17:05 BST while the steps 1+2 head-to-head was running, so the 04:30 stop passed mid-run. Done: the measurement the lead asked for, steps 1-3 built, and head-to-head against v2 for steps 1+2 and steps 1+3. Not run: the panel (test 1), pace (test 3), the starter-cpp safety run (test 4), and step 4.

## What changed (bots/w11, base v2a)

- `src/denial.cpp` (new):
  - `init_denial` caches every altar and the stun range.
  - `deny_log` prints a TRIB line per visible enemy tribute when P_DENY_LOG is on.
  - `buy_lock` is step 1.
  - `place_lock` and `lock_drops` are step 2 and step 3a (the lock interrupt).
  - `interrupt` is step 3b: stun, then hit, then chase at most 2 tiles. It does not chase when retreating or below 50 per cent health.
- `src/bot.cpp`:
  - `buy_items` now calls `buy_shards()` and then `buy_lock()`, so the lock is bought after the shards with what is left.
  - `lock_drops(...)` runs after `update_contested`. It is a free action.
  - `interrupt(...)` runs just before `enemy_response`, after our own submit-in-range check.
- `src/bot.h`: new members. `src/params.h`: P_DENY_LOG, P_DENY_EVT, P_LOCK_BUY, P_LOCK_DROP, P_INTERRUPT, P_INTERRUPT_CHASE.
- `src/params_override.h`: the current build is steps 1+2 (P_INTERRUPT 0).

## Rules and SDK lines relied on

- GameRules.md:235: "Altar locks are a consumable you can purchase for 10g, and carry one at a time ... cancelling enemy channels".
- GameRules.md:237: "blocks opposing tributes but allows its owner's team to tribute. Placing it cancels existing tribute channels at that altar. Three basic attacks remove it."
- GameRules.md:241: tributing against a lock is converted to attacks on it.
- GameRules.md:310: "Buy them on your spawn or while dead".
- GameRules.md:314: "Place on an altar within two tiles".
- GameRules.md:223: "Damage does not interrupt it; stun, death or an altar lock does."
- GameRules.md:361: "The killer receives all carried gems".
- Stuns (enemy-targetable cc):
  - Cassi slot 1 (GameRules.md:442)
  - Ryth slot 0, radius 1 (GameRules.md:490)
  - Prism slot 0 (GameRules.md:554)
  - Marcella enhanced attack (GameRules.md:648; an attack reset, so `interrupt` does not use it)
  - Tavi slot 1 (GameRules.md:686)
  - Ren slot 0 (GameRules.md:742)
  - Stellar, Varg, Vendetta, Issac, Vesper and Willow have none.
- cambc.h:
  - `is_submitting_gems()` at 3518, `remaining_channel_time()` at 3568, `gem_count()` at 3682. `gem_count()` only needs a live, visible champion, and 2,706 of 2,706 readings on enemies returned a value.
  - Lock calls: `consumables()` at 3442, `consumable::use(vec2)` at 2668, `buy(consumable_type)` at 3701, `can_buy(consumable_type)` at 3634, `vision::altar_locks(team)` at 4831.
  - Placing a lock left `can_move` and `can_attack` true in every logged use (LOCK lines "mv11 at11"). It costs no action allowance.
- One point is unverified: whether a lock cancels our own channels at that altar. `place_lock` therefore skips an altar where a visible ally is tributing.

## Measurement first (lead, 01:50)

The bot was v2 plus TRIB logging. It played 102 games, salt 0, all 17 maps, both sides, against foundry-v16, v4-14 and v2 (`results/w11-measure.json`, script `results/w11-trib.py`). The numbers are ceilings, in gems per game.

| opponent | enemy tributed | seen by us | within 2 of the altar (lock) | stun range + 2 |
|---|---|---|---|---|
| v2 | 836 | 281 (34%) | 101 | 78 |
| foundry-v16 | 776 | 207 (27%) | 23 | 38 |
| v4-14 | 802 | 303 (38%) | 87 | 95 |

By map:
- **Large ceiling:** swirl, collision, corners, interlock, elkring, monolith, eight and fragment (32 to 78% of enemy gems seen).
- **Near zero:** ambush, contest, expanse, lanes, packs, pilgrimage, rows, rushbait and stampede.

A stun only delays a tribute. Only a kill takes the gems. So the 9-of-17 falsifier could not be met through interrupts on half the maps.

## Head-to-head against v2

Each map has 6 games: 2 sides × 3 salts. Salts often reproduce the same game, so there were 67 distinct games of 102. Positive means more than 3 of 6, or 3 of 6 with a gem margin above 0; I fixed that definition before reading the results.

| map | 1+2: W | gemD | 1-3: W | gemD | hits per game (1-3) |
|---|---|---|---|---|---|
| ambush | 3 | 19 | 3 | 19 | 0 |
| collision | 3 | 217 | 3 | 188 | 21.8 |
| contest | 5 | 0 | 5 | 0 | 0 |
| corners | 6 | 668 | 6 | 473 | 5.5 |
| eight | 3 | 102 | 3 | 180 | 19.5 |
| elkring | 6 | 180 | 6 | 90 | 18.0 |
| expanse | 2 | -24 | 2 | -24 | 0 |
| fragment | 0 | -422 | 0 | -422 | 1.5 |
| interlock | 4 | 109 | 5 | 87 | 4.8 |
| lanes | 2 | -50 | 2 | -57 | 0 |
| monolith | 2 | 8 | 4 | 16 | 8.8 |
| packs | 3 | -34 | 3 | -34 | 0.5 |
| pilgrimage | 3 | 74 | 3 | 74 | 0.2 |
| rows | 2 | -11 | 2 | -11 | 0 |
| rushbait-v2 | 1 | -128 | 1 | -128 | 0 |
| stampede | 2 | -180 | 2 | -180 | 0 |
| swirl | 3 | 5 | 3 | 12 | 2.8 |
| **total** | **50/102**, mean +31.4 | 9/17 positive | **53/102**, mean +16.7 | 10/17 positive | |

Falsifier, fixed before testing: reject a step if fewer than 9 of 17 maps are positive, or if the gem margin falls.
- **Steps 1+2:** 9 of 17 maps positive, mean margin +31.4. Not falsified, but only just; four of the nine are 3 of 6 wins carried by gem margin alone.
- **Step 3's own effect (1-3 minus 1+2):** +3 wins (monolith +2, interlock +1), but the margin falls from +31.4 to +16.7 (corners -195, elkring -90). Rejected by the fixed rule.
- **Step 4** (P_FOCUS_RULE 1, already in `enemies.cpp`): not run.

Bot errors: 0.73 and 0.76 per game. The kinds are 97, 98 and 22, which v2 also produces, at 0.82 per game on its panel.

## Traced loss

fragment, side A, salt 0. The game is identical under steps 1+2 and steps 1-3 (675 vs 1,009). The no-op baseline won 1,020 vs 590.

- The first divergence from v2 is the lock purchase at r88: Issac buys a lock with 10 gold left over after shards. A lock is dropped at (14,5) at r192.
- After that, our Cassi and Issac die 9 times each, against 6 in the baseline. The enemy Stellar's tributes rise from 328 to 615.
- The interrupt fired three times (r85-87, a 22-gem tributer) and changed nothing.
- **Likely cause:** the trajectory changed after the 10-gold spend, not the denial itself. `extract`/`deaths.py` did not finish before the stop, so this is unconfirmed.

## Verdict (partial)

- **Step 3:** reject.
- **Steps 1+2:** do not admit yet. They are positive on corners, elkring, interlock, collision and contest, but lose on fragment, rushbait, stampede, lanes and expanse, and the panel and safety runs have not been done. If they are tried again, run them only on maps with a large ceiling: corners, elkring, interlock, collision, eight and swirl.
