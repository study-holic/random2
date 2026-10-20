# w14 duel lab: what each micro primitive is worth, per matchup (19 Oct, final for tonight)

> Measured on engine 0.0.3 and the 17-map pool; engine 1.0 changes (docs/rules/ENGINE-1.0.md) that may affect it: dash metadata (landing radius/damage/stun; existing dashes unchanged), base stats of 11 champions (HP, strength, Focus and growth), Tavi stun 1.4 x Focus, Prism 1.0 Focus per gem, Marcella gold per gem, Heart of Ruin not in the duel lab; every per-matchup table needs re-measuring.

Tables: `python tools/micro/report.py` (also results/w14-duels-tables.md; every cell with n, W/L/E/X/T counts, median HP left and time to kill, and every paired gain in results/w14-duels.json). Raw duel records: results/w14-lab/<run>.json.

**Incident (19:21 BST):** while stopping my own `cambc view` server I ran `taskkill //F //IM cambc.exe`, which also
killed two other agents' `cambc run-batch` processes: `matches/w10-185200` (stopped at monolith, salt 2, game "w10-s2
vs x-ifish") and `matches/w13-191856` (stopped at eight, salt 1, "w13-s1 vs v2"). Their mapgate summaries will count
the unplayed games as losses ("missing or unreadable statistics count as a loss", scripts/mapgate.py). Those two runs
need a re-run. I have only killed processes by PID since.

## Harness

- **Arena maps** are written with the engine's own map editor API (`MapBuilder_*` exports of the cambc 2.2.5 engine
  WASM, fetched from a local `cambc view` into `tools/micro/.cache`): `tools/micro/mapbuilder.mjs`, `mkarena.mjs`,
  `mkmaps.mjs`. A map made from `MapBuilder_Create` is rejected by the runner (engine error 7); a map made by loading
  `maps/pilgrimage.map26a` (50x65, x-mirror symmetry, 5 champions), clearing every tile and camp, and redrawing it is
  accepted. The runner also rejects 2x38 and 3x3 spawn areas but accepts 4x4 and 5x5. Arena maps live in `maps-lab/`;
  `cambc run --map` and `run-batch` take any path, so nothing was added to `maps/`.
- **Layout**: team A spawn x 0-3, y 22-25 (B mirrored), a connector column, five 6-row corridors (rows 6+8c..11+8c,
  x 6-43) separated by 2-tile walls (they block sight, so corridors are independent). The duel spec (each side's five
  picks, 16 policy bits, team-fight groups) is written as holes inside a wall block (rows 50-55), which the bots read
  in the draft and at init; one build of the duel bot serves every configuration.
- **Duel bot** `bots/w14d` (1v1) and `bots/w14t` (adds team fights, v2's lone-flee rule, the race retreat, focus,
  heal and the adaptive table). Champion k of a team fights in corridor k (team fights: a group of picks shares a
  corridor). Cycle: heal to full on the spawn, walk to a jittered start tile (x 21-22 vs 27-28, any of the 6 rows, 0-1
  rounds of delay; gap 5-7), wait until the foe (and all allies) are on their start tiles, fight with the side's
  policy until a death, an escape past x 9 / x 40 (about 15 tiles of flight), or 150 rounds; repeat. About 25 duels per
  corridor per 2,000-round game, 5 corridors per game, about 6 s per game. Each champion prints one line per duel.
- **Baseline policy (bit 0 = v2a's micro)**, reproduced from v2a: focus the lowest-health foe (enemies.cpp:21-30,
  99-107); cast the first ready damage/area/stun ability that can target enemies only when the foe is inside basic
  attack range (enemies.cpp:111-114); attack, then reset and attack again (util.cpp:59-72); chase toward attack range;
  Varg shields below 60% health (util.cpp:95-104); below 45% health with an enemy in view, walk home and do nothing
  else (bot.cpp:377, 395, 405-413); bots/w14t adds the lone-flee rule (no ally within 4 and 2+ foes within 4: walk to
  an ally or the spawn, enemies.cpp:78-88), which fires against Cassi + Issac.
- **Stats**: base stats, no items. Passive gold is 0 (earnedGold timelines stay 0 until a kill, statistics JSON), so
  shards could only come from kill gold (50) or camps; a shard-level arena was not built. Matchups at mid-game item
  levels may differ (growth per shard differs by champion: GameRules l.418-792).
- **Scoring**: per duel W = 1, L = 0, escape / foe escaped / timeout = 0.5. Variant gains are **paired**: the same
  schedule game, corridor and side run once with the baseline and once with the variant on one side (the corridor and
  side change the duel sequence, so unpaired cells mix configurations; the first unpaired analysis showed spurious
  changes such as Marcella v C&I -0.33 under a Varg-only variant, which vanished when paired).
- **Worlds**: A = both sides keep v2's 45% retreat walk (what v2 does on the ladder); B = both fight to the death
  (raw duel strength).

## Tables

**World B, base stats, both fight to the death (raw 1v1 strength)** (row's score vs column; W 1, escape/timeout 0.5, L 0; 13578 duels)

| | Ryth | Vesp | Vend | C&I | Stel | Prsm | Marc | Will | Ren | Varg | Tavi | mean |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Ryth | 0.50 | 1.00 | 1.00 | 0.96 | 1.00 | 1.00 | 0.37 | 1.00 | 1.00 | 1.00 | 1.00 | 0.93 |
| Vesp | 0.00 | 0.50 | 0.00 | 0.00 | 0.00 | 0.14 | 0.00 | 1.00 | 0.00 | 0.00 | 0.00 | 0.11 |
| Vend | 0.00 | 1.00 | 0.50 | 0.09 | 0.00 | 0.90 | 0.00 | 1.00 | 0.00 | 0.00 | 0.00 | 0.30 |
| C&I | 0.04 | 1.00 | 0.91 | 0.51 | 0.56 | 0.77 | 0.00 | 1.00 | 0.82 | 0.00 | 0.60 | 0.57 |
| Stel | 0.00 | 1.00 | 1.00 | 0.44 | 0.50 | 0.75 | 0.00 | 1.00 | 1.00 | 0.00 | 0.61 | 0.58 |
| Prsm | 0.00 | 0.86 | 0.10 | 0.23 | 0.25 | 0.50 | 0.00 | 1.00 | 0.00 | 0.00 | 0.00 | 0.24 |
| Marc | 0.63 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 | 0.50 | 1.00 | 1.00 | 0.00 | 1.00 | 0.86 |
| Will | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 | 0.00 | 0.50 | 0.00 | 0.00 | 0.00 | 0.00 |
| Ren | 0.00 | 1.00 | 1.00 | 0.02 | 0.00 | 1.00 | 0.00 | 1.00 | 0.00 | 0.00 | 1.00 | 0.50 |
| Varg | 0.00 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 | 1.00 | 0.50 | 1.00 | 0.90 |
| Tavi | 0.00 | 1.00 | 1.00 | 0.40 | 0.39 | 1.00 | 0.00 | 1.00 | 0.00 | 0.00 | 0.50 | 0.48 |

**World A, base stats, v2a micro on both sides (incl. the 45% walk home)** (row's score vs column; W 1, escape/timeout 0.5, L 0; 16885 duels)

| | Ryth | Vesp | Vend | C&I | Stel | Prsm | Marc | Will | Ren | Varg | Tavi | mean |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Ryth | 0.50 | 1.00 | 1.00 | 0.04 | 1.00 | 1.00 | 0.21 | 1.00 | 0.52 | 0.50 | 1.00 | 0.73 |
| Vesp | 0.00 | 0.50 | 0.00 | 0.01 | 0.50 | 0.50 | 0.00 | 0.50 | 0.50 | 0.00 | 0.50 | 0.25 |
| Vend | 0.00 | 1.00 | 0.50 | 0.00 | 0.50 | 0.86 | 0.00 | 1.00 | 0.29 | 0.00 | 0.48 | 0.41 |
| C&I | 0.57 | 0.97 | 0.97 | 0.51 | 0.51 | 0.92 | 0.32 | 0.95 | 0.66 | 0.71 | 0.77 | 0.74 |
| Stel | 0.00 | 0.50 | 0.50 | 0.22 | 0.50 | 0.50 | 0.00 | 0.50 | 0.19 | 0.19 | 0.50 | 0.31 |
| Prsm | 0.00 | 0.50 | 0.14 | 0.06 | 0.50 | 0.50 | 0.00 | 0.50 | 0.10 | 0.00 | 0.50 | 0.23 |
| Marc | 0.79 | 1.00 | 1.00 | 0.48 | 1.00 | 1.00 | 0.50 | 1.00 | 0.50 | 0.80 | 1.00 | 0.86 |
| Will | 0.00 | 0.50 | 0.00 | 0.04 | 0.50 | 0.50 | 0.00 | 0.50 | 0.33 | 0.00 | 0.50 | 0.24 |
| Ren | 0.48 | 0.50 | 0.71 | 0.00 | 0.81 | 0.90 | 0.50 | 0.67 | 0.50 | 0.20 | 0.69 | 0.55 |
| Varg | 0.50 | 1.00 | 1.00 | 0.30 | 0.81 | 1.00 | 0.20 | 1.00 | 0.80 | 0.50 | 0.74 | 0.73 |
| Tavi | 0.00 | 0.50 | 0.52 | 0.03 | 0.50 | 0.50 | 0.00 | 0.50 | 0.31 | 0.26 | 0.50 | 0.31 |

**Paired gains** (variant side vs baseline foe minus baseline vs baseline, same game/corridor/side; mean over the 11 foes per champion; blank = no effect possible)

| world | variant | mean | Ryth | Vesp | Vend | C&I | Stel | Prsm | Marc | Will | Ren | Varg | Tavi | biggest cell changes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| A | NORETREAT: fight to the death (no 45% walk home) | +0.149 | +0.21 | +0.07 | +0.41 | +0.20 | +0.13 | +0.14 | +0.15 | -0.10 | +0.09 | +0.21 | +0.13 | Varg v Marc +0.97; Ryth v C&I +0.95; Vend v Marc +0.89; Marc v C&I +0.85 |
| A | RETREAT30: walk home at 30% instead of 45% | +0.090 | +0.13 | -0.01 | +0.26 | +0.17 | +0.07 | +0.03 | +0.13 | -0.06 | +0.04 | +0.16 | +0.07 | Varg v Marc +0.97; Marc v C&I +0.66; Ryth v Marc +0.59; Vend v Marc +0.56 |
| A | EARLYCAST: spells at spell range 4, before moving | +0.043 | +0.06 |  |  | +0.05 | +0.08 | +0.06 | -0.15 |  | +0.12 |  | +0.25 | Stel v Varg +0.79; Tavi v Vesp +0.50; Ren v Vesp +0.50; Tavi v Prsm +0.50 |
| A | KITE (v2 kite_step, P_RANGED_KITE) | +0.004 |  | +0.08 | -0.01 | -0.04 | -0.02 | +0.04 | +0.01 | +0.02 |  |  | -0.03 | Vesp v Varg +0.48; Stel v Vend -0.48; Tavi v Vend -0.42; Vesp v Ryth +0.40 |
| A | DASHIN: melee dash next to a foe 2-3 away | +0.017 |  |  | +0.05 | +0.06 | -0.01 |  |  |  | +0.09 |  |  | Ren v Vesp +0.32; Vend v Tavi +0.26; Ren v Varg +0.21; Ren v Stel +0.17 |
| A | DASHOUT: Vesper dashes away from an adjacent melee, shoots | +0.006 |  | +0.07 |  |  |  |  |  |  |  |  |  | Vesp v Varg +0.49; Vesp v Ryth +0.19; Vesp v Vend +0.11 |
| A | SHIELDENG: Varg shields when the foe is within 4 | +0.017 |  |  |  |  |  |  |  |  |  | +0.19 |  | Varg v Marc +0.97; Varg v C&I +0.50; Varg v Tavi +0.27; Varg v Stel +0.19 |
| A | WEAVE: ranged step + reset + attack | -0.001 |  | +0.02 |  |  |  |  |  | -0.02 |  |  | -0.01 | Tavi v Vend -0.24; Will v Ren -0.19; Vesp v Varg +0.16; Tavi v Ren +0.13 |
| A | STUNSTEP: step away from a stunned melee | -0.002 |  |  |  |  |  |  |  |  |  |  | -0.02 | Tavi v Vend -0.24; Tavi v Ren +0.13; Tavi v Varg -0.13 |
| A | CDWAIT: hold off while our burst is down and theirs is up | -0.003 |  |  |  | +0.01 | +0.04 | +0.08 | -0.15 |  | -0.10 |  | +0.08 | Marc v Tavi -0.50; Ren v Prsm -0.50; Marc v Stel -0.50; Tavi v Ryth +0.35 |
| A | control HOLDRESET: never use the attack reset | -0.066 | -0.10 |  | -0.07 | -0.12 |  |  | -0.24 | -0.02 |  | -0.14 | -0.03 | Marc v Ryth -0.63; Vend v Vend -0.50; Marc v Tavi -0.50; Vend v Prsm -0.35 |
| A | control NOSPELL: never cast on champions | -0.082 | -0.14 |  |  | -0.22 | -0.07 | -0.01 | -0.22 |  | -0.17 | -0.01 | -0.05 | Marc v Ryth -0.60; Tavi v Vend -0.52; Ryth v Ryth -0.50; Stel v Vend -0.50 |
| B | EARLYCAST | +0.075 | +0.07 |  |  | +0.11 | +0.14 | +0.18 | +0.04 |  | +0.17 |  | +0.10 | Ren v Stel +1.00; Ren v C&I +0.91; Prsm v Stel +0.85; Tavi v C&I +0.60 |
| B | KITE | -0.023 |  |  |  | -0.01 | -0.13 | -0.03 |  |  |  |  | -0.09 | Tavi v Vend -1.00; Stel v Vend -0.95; Stel v Ren -0.50; C&I v Vend -0.33 |
| B | DASHIN | +0.019 |  |  | +0.03 | +0.08 | +0.01 |  |  |  | +0.09 |  |  | Ren v Stel +0.64; Ren v C&I +0.34; Vend v Tavi +0.23; C&I v Tavi +0.22 |
| B | DASHOUT | +0.000 |  |  |  |  |  |  |  |  |  |  |  | - |
| B | SHIELDENG | +0.012 |  |  |  |  |  |  |  |  |  | +0.14 |  | Varg v Ryth +1.00; Varg v Varg +0.50 |
| B | CDWAIT | +0.026 |  |  |  | +0.06 | +0.10 | +0.07 |  |  |  |  | +0.06 | Stel v Tavi +0.45; Stel v Ryth +0.42; Prsm v Prsm -0.42; Tavi v Ren +0.40 |
| B | control HOLDRESET | -0.086 | -0.14 | -0.06 | -0.14 | -0.06 |  |  | -0.21 | -0.05 |  | -0.26 | -0.04 | Vend v Prsm -1.00; Varg v Marc -1.00; Varg v Ren -1.00; Varg v C&I -0.68 |
| B | control NOSPELL | -0.222 | -0.34 |  |  | -0.36 | -0.56 | -0.27 | -0.27 |  | -0.18 |  | -0.46 | Ren v Tavi -1.00; Marc v Ren -1.00; Prsm v Will -1.00; Tavi v Vesp -1.00 |
| T | NORETREAT (also no lone flee) | +0.103 | +0.16 | +0.01 | +0.36 |  | +0.14 | +0.08 | +0.11 | -0.15 | +0.13 | +0.18 | +0.10 | Varg v Marc +1.00; Vend v Marc +0.90; Vend v Ren +0.68; Vend v Ryth +0.65 |
| T | RACE: walk home only if losing the strength race | +0.025 | +0.04 |  | +0.13 |  |  |  | +0.05 |  | +0.02 | +0.05 |  | Vend v Tavi +0.54; Vend v Stel +0.50; Marc v Ren +0.50; Ryth v Ren +0.47 |
| T | RACEK2: ... losing it by more than 2x | +0.060 | +0.12 | -0.02 | +0.33 |  | +0.03 |  | +0.07 | -0.04 | +0.04 | +0.14 |  | Varg v Marc +1.00; Vend v Ren +0.68; Ryth v Marc +0.55; Vend v Tavi +0.54 |
| T | CIUNIT: do not lone-flee from Cassi+Issac | -0.032 | -0.04 | -0.05 | -0.04 |  | -0.02 | -0.06 | -0.04 | -0.05 | -0.01 | +0.01 | -0.05 | Tavi v C&I -0.60; Prsm v C&I -0.60; Vesp v C&I -0.58; Will v C&I -0.54 |
| T | ADAPT2: ADAPT without CIUNIT (keeps v2's lone flee from Cassi+Issac) | +0.164 | +0.11 | +0.06 | +0.38 | +0.22 | +0.11 | +0.13 | +0.07 | -0.05 | +0.28 | +0.15 | +0.33 | Vend v Varg +1.00; Varg v Marc +1.00; Stel v Varg +0.81; Vend v Ren +0.67 |
| T | ADAPT: per-champion table (RACEK2+CIUNIT+EARLYCAST not Marcella+dashes+shield+kite Vesper/Prism) | +0.143 | +0.14 | +0.03 | +0.34 | +0.20 | +0.10 | +0.08 | +0.07 | -0.09 | +0.26 | +0.17 | +0.28 | Vend v Varg +1.00; Varg v Marc +1.00; Stel v Varg +0.81; Vend v Ren +0.67 |


## Findings (base stats, 1v1 unless stated)

1. **The 45% walk home is v2's costliest micro.** Below 45% with an enemy in view v2a walks to the spawn and does
   nothing else (bot.cpp:377, 395, 405-413). Removing it gains +0.149 score per duel on average (World A), +0.41 for
   Vendetta, +0.21 Ryth/Varg, +0.20 C&I; walking home at 30% instead recovers 60% of that (+0.090). A same-speed chaser
   keeps hitting a walker (GameRules l.92, l.100), so the walk converts trades into deaths. It is right only for the
   weakest duelists: Willow -0.10, and Vesper/Prism/Willow against Ren, Tavi or C&I, which lose the fight anyway.
2. **Spells at spell range (EARLYCAST)**: v2 casts only inside basic-attack range (enemies.cpp:111-114), so range-4
   spells are wasted for melee casters and opened late by ranged ones. Casting at 4 gains +0.043 (A) / +0.075 (B):
   Tavi +0.25, Ren +0.12-0.17, Stellar +0.08-0.14, Prism, C&I; **Marcella -0.15** (her 100-damage spell excludes walking
   and costs her the step to the foe).
3. **Varg shield at engage** +0.19 for Varg (A), +0.14 (B). **Dashes** (never cast by v2): Ren +0.09, Issac (C&I)
   +0.06-0.08, Vendetta +0.05; Vesper dash-out +0.07.
4. **No gain or harmful**: v2's own kite_step (P_RANGED_KITE) ~0 in A and -0.023 in B (Stellar/Tavi vs Vendetta -0.4
   to -1.0: stepping back from a melee forfeits the attack and the melee follows); WEAVE and STUNSTEP ~0;
   CDWAIT mixed (+ for Prism/Tavi/Stellar, - for Ren/Marcella). Controls: no reset -0.07/-0.09; no spells on champions
   only -0.08 in A (v2 casts so late that spells matter little) but -0.22 in B.
5. **Matchups** (B, raw): Ryth (0.93), Varg (0.90), Marcella (0.86) win most 1v1s at base stats; Stellar 0.58,
   C&I 0.57, Ren 0.50, Tavi 0.48; Vendetta 0.30, Prism 0.24, Vesper 0.11, Willow 0.00. Under v2's micro (A) the order
   compresses because most duels end in escapes. Ren vs Ren is a mutual kill (retaliation, GameRules l.738).
6. **Adaptive rule set (ADAPT2, bots/w14t)** = fight on unless losing the strength race by more than 2x, spells at
   range except Marcella, Ren/Vendetta/Issac dash in, Vesper dash-out + kite, Prism kite, Varg engage shield, keep v2's
   lone flee from Cassi+Issac: **+0.164 per duel**, better than any single change (NORETREAT +0.103 in the same world).
   The strength-only race alone is too timid (+0.025 at margin 1, +0.060 at margin 2); not fleeing from C&I is wrong
   for most champions (CIUNIT -0.032: the lone flee at full health escapes).
7. **Team fights** (2v2/3v3 in one corridor) are implemented in bots/w14t (`lab.py team`) but were not run tonight.

## Real maps (one cheap read only)

bots/w14 = v2a + P_RETREAT_RACE (margin 1) + early casts (not Marcella) + Varg engage shield + dash-in
(bots/w14/src/params_override.h; code in bot.cpp wins_race, enemies.cpp, util.cpp cast_shield). Head-to-head vs v2,
salt 0, 17 old layouts: **18/34** (results/w14-h2h-s0-a.json). Not a verdict: one salt, old maps, and it carries the
timid margin-1 race rather than ADAPT2. Not admitted.

## Adaptation table design

- **Enemy types need no draft memory.** Every champion can read the full enemy roster from turn 1:
  `cambc::champions(team::enemy)` plus `champion_name()`, which "live champions read without vision" (cambc.h 3457).
  So the table is compiled into the bot and indexed once at init (2x first-turn fuel); no per-turn cost.
- **Table, keyed by (our champion, enemy champion)**, filled from the paired gains above (cell > +0.05 and n >= 20):
  `fight_on[12][12]` (1 bit: ignore the 45% walk when this enemy is the focus; false for Willow/Vesper/Prism vs
  Ren/Tavi/C&I/strong melee), `kite[12][12]` (Vesper/Prism vs melee only), plus per-our-champion constants:
  early-cast (all casters but Marcella), dash-in (Ren, Vendetta, Issac), engage shield (Varg), keep the lone flee
  from C&I. 12x12x2 bits = 36 bytes.
- **Run-time inputs from observation**: enemy `strength()`, `focus()`, `max_health()` and ability `cooldown()` are
  readable for a visible enemy (lab probe lines "C <name> cd.. str.. foc.." in bots/w14t stdout, e.g. "Issac cd7"),
  so item levels and burst timing are read, not inferred. The race test at the retreat point should use measured
  rates (HP lost per own turn and focus HP lost per own turn, short EMA) instead of strength, since strength misses
  spells, resets and Ren's retaliation (the cause of RACE's timidity). Per-champion state only: each champion
  observes its own fight; the 8 shared bytes are fully used by v2 (bytes 0-5 targets, 6-7 squad claims, bot.h:45-46).
- **Draft memory (1,024 bytes)** is optional: the drafter could write the 36-byte table rows for our actual picks
  against their picks seen so far, but the enemy's last pick arrives after our last draft turn, so the in-game
  read at init is the reliable source. Use draft memory only for per-map overrides (e.g. a map where fleeing to a
  near spawn is cheap: fight_on off).
- **Fuel**: init lookup negligible; the per-turn race reads at most 5 visible enemies' health/strength (a few host
  calls each, not measured tonight) only while below the retreat threshold.

## Limits

Base stats only (passive gold is 0, so shards need farming; an item-level arena with gold-cluster farming and four
builds is coded in bots/w14t + mkarena `gold` but not run, per the 23:00 instruction). Arena duels have no camps,
allies or altars; escape = 0.5 is a modelling choice (an escape saves the bag but costs tempo). Outcomes at base
stats are near-deterministic per configuration (cells mostly 0/1); the jitter samples 24 start configurations.
