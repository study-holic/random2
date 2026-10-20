# w8: damage-only shard builds (builder w8, 19 Oct 17:37-19:25 BST)

**Verdict: reject A, B and C.** All three met the falsifier that was fixed before testing: "head-to-head vs v2 below 55 %, or median win round not down on 10+ maps".

| Variant | Head-to-head vs v2 | Median win round down | Panel ΔW vs v2 |
|---|---|---|---|
| A, damage only | 13/34 = 38 % | 7 of 17 maps against v2's salt-0 medians (5 against its 3-salt medians) | −31.67 |
| B, A + Monster emblem first | 44/102 = 43 % | 10 of 17 against salt-0 (7 against 3-salt) | −22.67 |
| C, Tavi Focus + field buys | 31/54 = 57 % | 2 of 9 changed maps | −4.00 |

Without Health our champions are burst down. Foundry's Stellar kills rise from 33 to 43 a game on contest and from 22 to 42 on stampede. Micron and Hidden Gem Thieves survive on base health because of how they take fights, which our engagement code does not reproduce. The shards alone do not make us fast.

## What changed

The code is in `bots/w8dmg/src/bot.cpp`, `buy_items` and `damage_stat_is_focus`, copied from v2a. `params.h` gains four knobs, all defaulting to v2a behaviour:

| Knob | Effect |
|---|---|
| `P_BUILD_DMG` | No Health shards. Every shard is a Power shard of the champion's damage stat. Power counts as 6 shards for 5 purchases (GameRules l.299). |
| `P_BUILD_MON_FIRST` | With `P_BUILD_DMG`, a Monster emblem of the damage stat comes first (5 shards, +10 % damage to camps, l.298). |
| `P_TAVI_DMG_FOCUS` | Tavi alone takes the damage build with Focus. |
| `P_FIELD_SHOP_HEALTH` | Kept from the w8 shards work. It decides whether Tavi buys Health in the field. |

The remaining changes:
- The field-shop guard is kept from the shards work: a failed field purchase stops field buying for 50 rounds (`field_shop_off_until`, bot.h).
- `can_shop` is false for Tavi when field shopping is on.

Each copy sets its knobs in `params_override.h`:

| Copy | Knobs |
|---|---|
| `bots/w8dmg` (A) | `P_BUILD_DMG 1` |
| `bots/w8mon` (B) | `P_BUILD_DMG 1`, `P_BUILD_MON_FIRST 1` |
| `bots/w8tf` (C) | `P_TAVI_DMG_FOCUS 1`, `P_FIELD_SHOP 1` |

A and B shop at the spawn only. C changes only Tavi, so it changes only the 9 maps where Tavi is drafted. On packs, where Tavi is not drafted, the C run reproduced v2 exactly: 10/12, round 611, gem margin 291.3.

## Damage stat per champion, from the kit (GameRules champion sections)

Rules relied on:
- "Basic attacks deal Strength before modifiers" (l.251).
- "Abilities scale with focus ... multiplying by the ability multiplier" (l.255).
- 8-turn cooldowns, one cast per turn (l.415-416). A basic attack and a cast can share a turn (l.116).

Method: one 8-turn cycle with a basic attack every turn, one extra basic per attack reset, and each damaging spell cast once. The table gives the added damage per cycle from one shard of each stat.

| Champion | +1 Strength shard | +1 Focus shard | Choice |
|---|---|---|---|
| Cassi | 3.5 × 8 = 28 | 15 × 2.0 = 30 (l.436, 450) | Focus, close |
| Issac | 4.75 × 9 (reset) = 43 | 0: reset and dash only | Strength |
| Ryth | 5 × 9 = 45, plus 1 % of bonus health as Strength (l.486) | 10 × 1.0 per target, area stun | Strength |
| Vendetta | 5 × 9 = 45 | 0: reset and dash | Strength |
| Varg | 5 × 9 = 45, plus splash equal to Strength (l.770) | shield 2.5 × 10, defensive | Strength |
| Ren | 5 × 8 = 40 | 12.5 + 0.2 × 10 per hit taken (l.738-742) | Strength |
| Stellar | 3.5 × 8 = 28 | 10 × (2.0 + 1.4) = 34 (l.584-590) | Focus |
| Tavi | 5 × 9 = 45 | 10 × 2.2 = 22 (l.686) | Focus, as the lead specified (burst); the cycle arithmetic favours Strength |
| Prism | 3.5 × 8 = 28 | 10 × 2.5 = 25, plus passive | Focus (unchanged; not drafted) |

The A/B mapping equals v2a's main-stat mapping except for Tavi. A and B differ from v2a only in dropping Health and in Power versus Monster.

## Build took effect (decoded replays, stride 1; monolith A/B and stampede A/B vs foundry-v16, salt 0)

| | v2 | A | B | C |
|---|---|---|---|---|
| End max health (Cassi / Issac / Ryth / Varg / Vendetta / Tavi) | 1,626 / 1,969 / 3,200 / 3,075 / 1,975 / 3,412 | 700 / 900 / 1,200 / 1,000 / 800 / 750 (base) | base | 1,484 / 2,372 / 3,250 / 2,825 / 2,225 / 750 |
| Tavi damage per damaging cast | 212 | 475 | 390 | 417 |
| Tavi damage per basic hit | 97 | 35 | 34 | 35 |
| Varg / Vendetta hit, rounds 300-700 | 103 / 130 vs camps | 109 / 128 | 116 / 165 | 95 / 118 |

- A and B bought no Health, since every maximum health stayed at base.
- Tavi in A, B and C stacked Focus: cast damage doubled and basic hits stayed at base.
- Per-hit means mix in early rounds and gold income, so they are noisy for the Strength champions.

## Test 1: screen (1 salt, contest, monolith, stampede, packs)

Each cell is wins out of 12, then deaths a game.

| Map | v2 | A | B | C |
|---|---|---|---|---|
| contest | 11 / 21.7 | 7 / 39.4 | 8 / 39.1 | 10 / 21.3 |
| monolith | 11 / 3.7 | 11 / 4.6 | 11 / 3.7 | 12 / 3.2 |
| stampede | 11 / 15.0 | 5 / 27.6 | 9 / 30.0 | 10 / 17.8 |
| packs | 10 / 4.5 | 9 / 4.8 | 9 / 4.8 | 10 / 4.5 |

## Test 1 continued: panel against `results/v2-panel.json`

**A and B, 1 salt, all 17 maps** (`results/w8dmg-s0.json`, `results/w8mon-s0.json`):
- A: 120/204 vs v2 155. cmpcells total ΔW −31.67; no map admitted.
- B: 129/204. Total ΔW −22.67; no map admitted.
- B's worst maps: pilgrimage −5.00, collision −4.00, interlock −3.67, elkring −2.33, fragment −2.00, contest −2.00. Its only gains are expanse +1.00, ambush +0.33 and rows +0.33.
- **No salts 1-2 for A or B.** One-salt deficits of −22 to −32 are far beyond the 1.5-game noise threshold, and test 2 fails as well.

**C, 3 salts, 9 Tavi maps** (`results/w8tf.json`): total ΔW −4.00, no map admitted.

| Map | ΔW | ΔGem |
|---|---|---|
| ambush | −0.67 | |
| contest | +0.33 | |
| eight | −1.00 | |
| elkring | −1.67 | −2,028 |
| lanes | +1.00 | |
| monolith | +1.00 | +261 |
| pilgrimage | −2.33 vs v2 (v418d); +1.00 vs v2a | |
| rows | +0.33 | |
| stampede | −1.00 | −1,800 |

## Test 2: head-to-head against v2 (each map: wins out of 6 for C and B, out of 2 for A)

| Map | A (1 salt) | B (3 salts) | C (3 salts) |
|---|---|---|---|
| ambush | 2/2 | 6/6 | 3/6 |
| collision | 0/2 | 2/6 | - |
| contest | 0/2 | 0/6 | 4/6 |
| corners | 2/2 | 6/6 | - |
| eight | 0/2 | 0/6 | 3/6 |
| elkring | 1/2 | 3/6 | 3/6 |
| expanse | 1/2 | 3/6 | - |
| fragment | 1/2 | 3/6 | - |
| interlock | 1/2 | 3/6 | - |
| lanes | 2/2 | 3/6 | 5/6 |
| monolith | 1/2 | 5/6 | 6/6 |
| packs | 1/2 | 3/6 | - |
| pilgrimage | 0/2 | 1/6 | 2/6 |
| rows | 0/2 | 3/6 | 2/6 |
| rushbait-v2 | 1/2 | 2/6 | - |
| stampede | 0/2 | 0/6 | 3/6 |
| swirl | 0/2 | 1/6 | - |
| **Total** | **13/34 (38 %)** | **44/102 (43 %)** | **31/54 (57 %)** |

Several 3/6 cells are side-decided: one side wins all three salts.

## Test 3: pace (median win round)

| Map | v2 (3 salts) | A (s0) | B (s0) | C (3 salts) | Top-50 yardstick |
|---|---|---|---|---|---|
| ambush | 899 | 980 | 975 | 941 | 594 |
| collision | 761 | 719 | 725 | = | |
| contest | 2000 | 2000 | 2000 | 2000 | 1,602 |
| corners | 466 | 507 | 481 | = | |
| eight | 734 | 795 | 722 | 670 | 640 |
| elkring | 590 | 605 | 672 | 630 | |
| expanse | 464 | 464 | 468 | = | |
| fragment | 852 | 846 | 877 | = | |
| interlock | 576 | 561 | 561 | = | |
| lanes | 435 | 458 | 440 | 437 | |
| monolith | 584 | 556 | 581 | 570 | 447 |
| packs | 611 | 575 | 595 | = | |
| pilgrimage | 1278 | 1304 | 1255 | 1301 | 844 |
| rows | 481 | 492 | 475 | 489 | 386 |
| rushbait-v2 | 1006 | 1049 | 1505 | = | 660 |
| stampede | 1752 | 2000 | 2000 | 1779 | 686 |
| swirl | 1249 | 1674 | 1567 | = | 600 |

"=" means C cannot change the map, because Tavi is not drafted there. Where A or B is faster, it usually also wins fewer games, so the median covers a smaller set of wins.

## Test 4: safety

- Faults: 0 in every run.
- Our "bot error" lines (salt 0) are all engine error 97 ("Unit is not in vision"), a known type. Counts: v2 207 over 204 games; A 287; B 326; C 211 over 108 games, against v2's 108 on the same 9 maps.
- The rise tracks the rise in deaths and fights. No purchase errors appeared.
- I did not run the starter-cpp safety test, since nothing is admitted.

## Traced moments (decoded, `tools/v3/extract_pilot.mjs`; no visualiser watch)

**A loss: contest vs foundry-v16, side B, salt 0.** v2 won on gold earned (10,405 vs 9,895); A lost (8,295 vs 12,455). Our deaths rose:

| Champion | v2 deaths | A deaths |
|---|---|---|
| Cassi & Issac | 9 | 16 |
| Ryth | 7 | 12 |
| Vendetta | 8 | 13 |
| Varg | 6 | 9 |

Foundry's Stellar, a 2 × Focus nuke at range 4, went from 33 to 43 kills. Our Tavi went from 5 to 12 kills, so the Focus Tavi does kill. On stampede vs foundry B the pattern repeats: Varg 6 to 12 deaths, Vendetta 5 to 13, foundry Stellar 22 to 42 kills.

**C loss: elkring vs x-hello, side B, salt 0.** v2 won at round 582; C lost at round 719.
- Tavi's camp damage fell from 21.5 to 15.5 a round. v2 dealt 8,824 basic plus 3,668 cast; C dealt 3,672 basic plus 7,454 cast.
- Tavi's damage to champions rose from 2,319 to 5,021.
- Team camp damage fell from 127.5 to 112.8 a round.
- Focus Tavi trades camp clearing for champion picks. That is a pace loss on farming maps and fits the cycle arithmetic above.

## Leads (not admitted, below the rules)

C on monolith:
- Panel 12/12 vs 11/12: ΔW +1.00, ΔStrong +1.00, ΔGem +261.
- Head-to-head 6/6.
- Median win round 570 vs 584.

This fits ladder report item 3, casters poking from range 4. A monolith-only Tavi Focus rule would need its own card and a test against the 1.5 rule; monolith is near the ceiling.

## Files

Results: `results/w8dmg-scr.json`, `results/w8dmg-scr2.json`, `results/w8dmg-s0.json`, `results/w8dmg-h2h.json`; the same set for w8mon; `results/w8tf-scr.json`, `results/w8tf.json`, `results/w8tf-h2h.json`.

Bot copies: `bots/w8dmg`, `bots/w8mon`, `bots/w8tf`.
