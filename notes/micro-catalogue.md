# Micro catalogue (w14, 19 Oct)

> Measured on engine 0.0.3 and the 17-map pool; engine 1.0 changes (docs/rules/ENGINE-1.0.md) that may affect it: Heart of Ruin kit (self shield, dash with landing damage and stun), dash metadata now carries radius, modifier and duration (only Heart of Ruin uses them), Prism Focus 1.0 per gem, base stat rebalance, elk range 4 and monolith range 5 (camp-fight micro).

Every fight-level ("micro") decision the rules allow, per champion kit and shared mechanic. For each: the mechanism and the
rule line it rests on (`GR` = docs/rules/GameRules.md, `H` = sdk/cpp/cambc.h), the matchups it touches, whether v2a does it
(file:line in bots/v2a/src), and an a-priori value estimate with reasoning. The duel lab (results/w14-duels.md) replaces
the a-priori column with measurements where a row is marked **[lab]**.

Measured (base-stat 1v1, paired gains per duel): results/w14-duels.md. Headline: v2 walk-home at 45% costs -0.149/duel; early casts +0.043 (Marcella -0.15); Varg engage shield +0.19 for Varg; dashes +0.05..0.09; v2 kite_step ~0 or harmful; adaptive set +0.164.

## 0. Base numbers used below (GR l.418-792, no items)

Per-turn damage if every ability is cast on cooldown (8 turn ends, GR l.116) and a basic attack lands every turn:
`strength + reset_bonus/8 + spell_damage/8`.

| Champion | Range | HP | Str | Focus | Spells (base) | Dmg/turn | Notes |
|---|---|---|---|---|---|---|---|
| Stellar | 3 | 750 | 35 | 100 | 200 single r4, 140 AoE r4 (both excl. walking) | 77.5 | burst 235 on turn 1 (spell + attack) |
| Vendetta | 1 | 800 | 70 | 100 | reset, dash 2 | 78.8 | kill resets cooldowns (GR l.612) |
| Ryth | 1 | 1200 | 50 | 100 | 100 AoE + stun r4 (excl.), reset | 68.8 | Str +1% of bonus HP |
| Tavi | 3 | 750 | 35 | 100 | 220 + stun r4 (excl.), reset | 66.9 | field shop (GR l.676) |
| Marcella | 1 | 1200 | 45 | 50 | enhanced reset (next attack stuns), 100 single r4 (excl.) | 63.1 | stun immune |
| Prism | 3 | 700 | 35 | 80 | 120 + stun r4, 80 AoE r4 (both excl.) | 60.0 | +0.2 Focus per carried gem |
| Varg | 1 | 1000 | 50 | 100 | reset, 250 self-shield 5 turns | 56.3 (+splash) | shield = +31 eff. HP/turn |
| Ren | 1 | 900 | 40 | 100 | 125 + stun r4 (excl.), dash 2 | 55.6 | retaliates 20 per hit taken |
| Vesper | 3 | 900 | 45 | 100 | reset, dash 2 | 50.6 | +20% gold |
| Willow | 3 | 700 | 45 | 105 | heal ally 315 r4, reset | 50.6 | heal cannot target self |
| Cassi | 3 | 700 | 30 | 70 | 140 single r4 (excl.), stun r4 | 47.5 | dies with Issac |
| Issac | 1 | 900 | 40 | 100 | reset, dash 2 | 45.0 | dies with Cassi |

A-priori duel ordering by `HP x dmg/turn` (a Lanchester-style "fighting value"): Ryth 82.5k, Marcella 75.7k, Vendetta 63k,
Stellar 58k, Varg 56k (+shield ~ 64k), Tavi 50k, Ren 50k (+retaliation), Vesper 45.5k, Prism 42k, Willow 35k, Cassi 33k.
Range, stuns and turn order are what this table leaves out; that is what the lab measures.

## 1. Shared mechanics

| # | Primitive | Mechanism and rule | Matchups | v2a | A-priori value |
|---|---|---|---|---|---|
| S1 | **Kite (step back, shoot)** **[lab]** | Ranged must choose walk or attack (GR l.98-100); melee walks and hits (GR l.100). A ranged champion at distance 2-3 of a melee can step away and lose its attack that turn; the melee closes one tile per turn. Kiting only gains when the melee needs >1 turn to re-close: after a stun, a dash, or the turn-order double turn (teams alternate who starts each round, GR l.122-127, so a champion acting last in round r and first in r+1 gets two turns in a row). | ranged vs melee | `kite_step` enemies.cpp:34-54, **off** (P_RANGED_KITE 0, params.h:172-174) | Low without a stun/dash: a pure step-back trades our attack for nothing (the melee follows and hits, GR l.100). High with stun (S3) or dash (S8). |
| S2 | **Attack-reset weave** **[lab]** | Reset restores the attack allowance (GR l.100). Two uses: (a) melee or standing ranged: attack, reset, attack = double hit; (b) ranged: walk (consumes the attack allowance), reset, attack = move and shoot in one turn. | Tavi, Vesper, Willow (ranged + reset); Ryth, Varg, Vendetta, Issac, Marcella (melee: only (a)) | (a) always, immediately after the first hit: util.cpp:59-72 called from enemies.cpp:115,126,132 and micro.cpp:22-23,176,179. (b) never. | (a) = +1 basic hit per 8 turns (+4-9 dmg/turn). (b) is worth more against melee: one free tile of separation per 8 turns while still dealing damage. |
| S3 | **Stun then step** **[lab]** | Stun = one round with no actions (GR l.440-444, H error 33 `invalid_action_cant_act`). A ranged stunner (Tavi, Prism, Cassi) stuns an adjacent melee then steps away: the melee loses its hit and one tile. | Tavi/Prism/Cassi vs melee | Stuns are cast at once on the focus target (enemies.cpp:57-67,114); no follow-up step. | Medium: one denied melee hit (40-70) plus a tile per 8 turns. |
| S4 | **Stun on channels** | Stun interrupts tribute (GR l.223) and recall (GR l.272, 373); damage interrupts recall only. A stun (range 4) on a tributing carrier cancels the whole bag's tribute; any hit cancels a recall. Use the stun *for* the channel rather than on cooldown. Enemy channel is visible: `remaining_channel_time()` (H 3567, "visible champion's remaining channel rounds"). | any stunner (Tavi, Prism, Cassi, Ryth, Ren, Marcella's enhanced hit) vs a channelling enemy | No: stuns go on the focus at once, and on camps too (util.cpp:82-93; P_SAVE_CC 0, params.h:178-180). | High where altars are contested (a 40-gem tribute = 4 turns; a stun resets it and the carrier is then in our range). Map-dependent; not a duel-lab item. |
| S5 | **Save stun from camps** | Same stun used on a camp is not available for 8 turns (GR l.116). | stunners on contested camps | P_SAVE_CC exists, **off** (params.h:178-180) | Low-medium; only matters when a fight follows within 8 turns. |
| S6 | **Cooldown tracking / baiting** | Enemy ability cooldowns are readable when the enemy is visible: `ability::cooldown()` calls `sdk_champion_cooldown(owner_, slot_)` (H 3218-3226, "throws for hidden owners"). Engage when the enemy's stun/burst is on cooldown; disengage when ours are. | all | No (no cooldown() read in v2a: grep). | Medium: engaging into a ready 220 Tavi stun vs a spent one is a ~220 dmg + 1 turn swing. Needs the lab to confirm the read works on enemies. |
| S7 | **Focus fire by effective HP** **[lab partially: target choice in 2v2]** | Kill order matters: dead enemies deal no damage. Pick the enemy with the lowest `HP / our dmg vs it`, counting shields (`buffs().has_shield()`, H 2758-2800), Durability (10%), Ren retaliation (20/hit taken, GR l.738). Cassi/Issac: killing either kills both (GR l.367, 428): the effective HP of the pair is min(Cassi, Issac). | team fights; C&I | Lowest raw health (P_FOCUS_RULE 0, enemies.cpp:16-30,99-107); ignores shields and the C&I link (it does pick Cassi when she is lower). | Medium in 2v2+: the C&I link alone means 700 HP removes 2 bodies (92.5 dmg/turn). |
| S8 | **Dash** **[lab]** | Dash = up to 2 steps, may cross holes (GR l.157-159), "Walking: Compatible" (Vesper, Ren, Vendetta, Issac). Uses: close the gap and hit the same turn (melee: dash + walk + attack = 3 tiles closed and a hit); escape (2 tiles + walk = 3 tiles away); ranged Vesper: dash out and still shoot (dash is a cast, not the walk). | Vendetta, Ren, Issac vs ranged (gap close); Vesper vs melee (kite) | **Never cast** (no `is_dash` in v2a; w7 finding). | High for melee vs ranged kiting champions (removes 1-2 free ranged turns per engagement) and for escape. |
| S9 | **Disengage before death (deny gems)** **[lab: escape test]** | Killer takes all carried gems + 50 gold (GR l.361); camp kills destroy gems. Walking away from a same-speed chaser never opens distance (both walk 1 tile, GR l.92), so escape needs a stun, a dash, allies, or the spawn. Recall is cancelled by any damage (GR l.373). | all | Retreat at 45% when enemies are in view (bot.cpp:377,388-413); lone vs 2+ foes walks to an ally (enemies.cpp:78-88); P_ENGAGE_MIN_HP 0 (params.h:169-171). | Depends entirely on escape tools: a walking retreat from a ranged chaser just eats free hits (ranged keeps distance 3 and shoots every other turn). Lab measures P(escape) per pair. |
| S10 | **Fight at camp edges** | Awake monsters hit the nearest champion every turn at range 3, never chase, sleep after 2 rounds without damage (GR l.189, 206-207). Pull an enemy into a camp we did not wake... no: monsters attack whoever is nearest once awake, so standing 4+ from an awake camp while the enemy stands within 3 makes the camp fight for us. Monoliths hit 200 (GR l.185). | anyone near an awake camp | No (danger_zone is only used for our own pathing, util.cpp:35-40). | Medium where fights happen at camps (most ladder deaths). Not in the duel lab (no camps). |
| S11 | **Bush vision** | Bushes block sight from outside; inside you see out (GR l.149, 337-341). Unit spells need a visible target (GR l.265). A champion in a bush cannot be targeted by spells or basic attacks from outside. Hidden enemies still block movement (GR l.163-167). | ambush, pilgrimage (bushy maps) | No | Medium on bush maps: a free opening burst (Stellar 200 + attack) from unseen. |
| S12 | **Wards** | Free ward, vision radius 6, placed within 2 tiles, movable once a round (GR l.104, 344-349). | scouting altars and camps | Not used (no place_ward in v2a). | Low for micro, medium for macro (see enemy carriers coming). |
| S13 | **Priority / turn order** | Within a team, higher priority acts first; set_priority takes effect next round (GR l.124-129). Teams alternate who starts each round (GR l.122). Order inside a team decides e.g. whether the stunner acts before the burst. | team fights | Not used (no set_priority in v2a). | Low-medium: matters for stun then burst sequences in 2v2+. |
| S14 | **Shield timing** **[lab]** | Varg: 250 shield for 5 turns (GR l.778-780); 8-turn cooldown. Casting it before the enemy's burst (or when the enemy enters range) absorbs a spell; casting when no damage comes wastes 5 of 8 turns. | Varg vs anyone | Only when a damaging camp is within 4 or health < 60% (util.cpp:95-104); **not** when only an enemy champion is near and health is high. | Medium: an extra 250 effective HP at fight start is ~25% of Varg's HP. |
| S15 | **Heal timing** | Willow: 315 heal on an ally within 4, "Walking: Compatible" (GR l.708-712). | Willow + any ally | **Never cast** (no is_heal; w7 finding). | High in 2v2+ (one heal = 40% of a Stellar), zero in 1v1 (no self-heal). |
| S16 | **Altar lock play** | Lock (10g) placed within 2 of an altar cancels enemy tribute channels and blocks tributes until 3 basic attacks remove it (GR l.235-241, 314). | contested altars | Clears enemy locks (bot.cpp:267-296); never buys or places one. | High on maps where enemies tribute near us (the 1,000-gem race); macro, not duel-lab. |
| S17 | **Spawn as a fortress** | Spawn heals each round, enemy cannot step on it (GR l.151, 122). Fights adjacent to our spawn favour us. | all | Retreat walks home within 6 tiles (bot.cpp:407). | Medium; macro. |
| S18 | **Chase limit / leash** | A same-speed chase never closes distance (GR l.92); chasing a melee into its allies or a camp loses. | all | Chase at most 2 tiles beyond range and only near our camp (enemies.cpp:118-129). | Lab measures how often the leash lets a hurt enemy escape. |
| S19 | **Excludes-walking order** | Excl.-walking spells cannot follow a move (H error 126 `spell_cast_already_moved`) and block a later move (H error 127). Cast before deciding to move; a melee with an excl. spell (Ryth stun, Ren stun, Marcella damage) loses its walk that turn. | Ryth, Ren, Marcella, all ranged casters | Ranged: kite_step runs before the cast (enemies.cpp:110-114) so a kite step forfeits the spell; melee chase moves first then attacks (enemies.cpp:125-127). | Low-medium: order bug costs one cast when kiting is on. |

## 2. Per-champion kit primitives

| Champion | Primitive | Rule | v2a | A-priori |
|---|---|---|---|---|
| Stellar | Open with the 200 single at range 4 before the enemy reaches 3; AoE when 2+ enemies within radius 1 of a tile (else single first) | GR l.582-592 | Casts the first ready damage ability in slot order on the focus (enemies.cpp:60-63): slot 0 single first, then AoE on the same target | Medium: range-4 opening is free damage vs melee; AoE choice matters only in team fights |
| Tavi | (1) stun-then-step vs melee; (2) walk+reset+attack weave (S2b); (3) save the stun for a channel (S4); (4) field shop mid-fight (Health bought away from spawn raises max HP but does not heal, GR l.676) | GR l.676-692 | stun at once; reset only as double hit | Medium-high vs melee |
| Prism | Stun (1.5x) + AoE; carry gems for +0.2 Focus each | GR l.550-562 | stun at once | Low-medium |
| Cassi | Stun is "Walking: Compatible" (GR l.444): stun + step away in one turn | GR l.434-444 | stun at once, no step | Medium vs melee |
| Issac | Dash to protect Cassi / body-block; reset | GR l.458-468 | reset only | Low-medium |
| Ryth | AoE stun r4 (excl.): opener that stuns from range 4 then walk next turn; reset | GR l.488-498 | cast at once (enemies.cpp:114) | Ryth is the duel king a priori (HP 1200, stun); timing matters vs Stellar burst |
| Varg | Shield before burst (S14); splash hits two adjacent enemies (C&I standing together, GR l.770) | GR l.770-782 | shield only near camps or <60% | Medium |
| Vendetta | Dash + walk + attack + reset on engage; kill resets cooldowns -> chain (dash again after a kill) | GR l.612-624 | reset only; no dash | High vs ranged (closes the kite) |
| Marcella | Enhanced reset: next attack stuns (GR l.648); stun immune (a counter to Tavi/Ryth/Ren/Prism/Cassi stuns) | GR l.644-656 | reset right after an attack (util.cpp:66-68) gives the enhanced stunning second hit at once | Medium; immune passive is automatic |
| Ren | Retaliation 20/hit taken (punishes Varg splash, Issac double-hits, many small hits); stun r4; dash | GR l.738-748 | stun at once, no dash | Medium |
| Vesper | Dash-and-shoot kite (dash is a cast, attack still allowed), reset double-hit | GR l.520-530 | reset only | High vs melee if the dash kite works |
| Willow | Heal ally (S15); reset weave | GR l.708-716 | no heal | High in team fights |

## 3. What adapts to the enemy draft (a priori, to be checked in the lab)

- Kite rules (S1, S2b, S3, S8) apply only when the enemy duelist is melee and we are ranged; they are wasted (or harmful)
  against ranged enemies, who out-range nothing and simply shoot back.
- Stun timing (S3, S4): pointless against Marcella (stun immune, GR l.640).
- Focus rule (S7): C&I present -> focus the lower of Cassi/Issac; Ren present -> avoid many small hits on him
  (retaliation), prefer spells; Varg shielded -> switch target while the shield holds.
- Shield timing (S14): engage-time shield is worth more against burst drafts (Stellar, Tavi) than against sustained
  basic-attack drafts.
- Disengage threshold (S9): depends on whether we can escape the specific enemy (dash/stun vs none) and whether it can
  chase (melee with dash vs ranged).
