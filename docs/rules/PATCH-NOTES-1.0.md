# Patch notes 1.0.0 (week 2), 1.0.1, 1.0.2 (organisers' notes, pasted by Xans 21 Oct)

These supersede GameRules.md (site copy, 20 Oct) where they differ. The local engine is 1.0.2 with CLI/SDK 3.0.4 (22 Oct).

## 1.0.0, week 2 (20 Oct)
**Champions**
- **Cassi:** Health 700 → 600. Per shard: Health 71.25 → 65, Strength 3.5 → 3, Focus 15 → 13.
- **Issac:** per shard: Health 95 → 80, Strength 4.75 → 3. Keeps the highest combined base stats early, with much less scaling.
- **Stellar:** Focus 100 → 85, Strength 35 → 30, Focus per shard 10 → 9.
- **Varg:** Strength 50 → 40. Less single-camp clearing; the AoE niche for large camp groups stays.
- **Ren:** Strength 40 → 35, Focus per shard 10 → 9.
- **Ryth:** Strength 50 → 45.
- **Marcella:** passive is now +2 gold per gem tributed, plus stun immunity. Focus 50 → 60, Focus per shard 10 → 15.
- **Willow:** Strength 45 → 55, Focus 105 → 120. Aimed at early power and sustain.
- **Vesper Vale:** passive bonus gold 20 % → 33 %, Strength 45 → 50.
- **Vendetta:** Strength per shard 5 → 6.
- **Prism:** passive Focus per gem 0.2 → 1. Per shard: Health 75 → 60, Strength 3.5 → 2.5, Focus 10 → 8.
- **Tavi:** S2 stun damage 2.2 × Focus → 1.4 × Focus, Strength 35 → 45.
- **Heart of Ruin (new, melee):**
  - Health 1000 (+100 a shard), Strength 40 (+5), Focus 100 (+15).
  - Passive: up to +50 % damage from missing health (+25 % at half health). It applies to all damage.
  - S1: a shield on self of 1.5 × Focus for 5 turns.
  - S2: a 2-tile dash. On landing it deals 1.75 × Focus and stuns enemies and camps within Chebyshev radius 1 for 1 turn.

**Camps**
| camp | change |
|---|---|
| Wolf | Gems 22 → 4; Gold 90 → 40 |
| Elk | Gems 36 → 15; Gold 160 → 90; Range 3 → 4 |
| Gem cluster | Health 500 → 120; Gems 24 → 7 |
| Gold cluster | Health 500 → 120; Gold 35 → 20 |
| Monolith | Range 3 → 5 |
| Golem (new) | Health 5000, Strength 120, Range 4, Gold 160, Gems 36, Respawn 80 rounds |

The organisers state that group value is now set by adding gem and gold clusters, and that the range increases are meant to stop long-range cheese farming.

**Other**
- Kill reward: Gold 50 → 100 (to encourage invades and contested camps).
- Ward cooldown 1 → 10.
- Altar lock health 3 → 5.
- **SDK:** new enum values for camps and champions, and ability_dash/AbilityDash. Old submissions that rely on the old patterns fail to compile.

## 1.0.1 (20 Oct)
- **Heart of Ruin (emergency nerf, near 100 % use at the top):** Strength per shard 5 → 4, Focus per shard 15 → 12.
- **Ladder:** a submission that stops compiling is marked "invalid bot". It gets a 24-hour grace period, then auto-loses ranked matches.

## 1.0.2 (21 Oct)
- **Stellar:** Health 750 → 650, Health per shard 75 → 65, Focus 85 → 70, Focus per shard 9 → 7.
- **Heart of Ruin:** Strength 30 (from 40), Strength per shard 4 → 3.5, S2 landing damage 1.75 → 1.5 × Focus.
- **Ren:** S1 stun damage 1.25 → 0.75 × Focus.
- **Vesper Vale:** Health 900 → 1000, Strength 50 → 55.
- **Vendetta:** Health 800 → 1000.
- **Tavi:** Health 750 → 900, Focus 100 → 110.
- **Prism:** passive Focus per gem 1 → 1.5.
- Organisers' stated aim: less front-loaded Focus burst, and more early survivability for Strength champions.
