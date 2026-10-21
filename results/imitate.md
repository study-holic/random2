# Imitation study: v4 against the teams ranked 2-10 (20 Oct, cloud)

Brief: tools/cloud/imitate.md. Data: data/real/v4-panel, 180 unranked games of v4 against nine teams ranked 2-10 (sets A-D, 20 maps), decoded at stride 1 with tools/top10/extract_pilot.mjs. Measures: tools/cloud/metrics.py, plus the scripts named in each section. `metrics.py --panel` now reads sets C and D too: their side, map and team come from baseline-v4top.csv, matched by the match ids inside each group zip (matches.jsonl covers only A and B). The win flag agrees with the platform result in 180/180 games.

## 1. Reconciled figures

### Top-10 deaths a game: 6.0 or 4.3?
Both are right; they come from different samples. The two methods agree game by game in all 180 games.

| sample | method | top-10 deaths a game | v4 deaths a game |
|---|---|---|---|
| sets A + B, 90 games (results/v5-s3-diag.md) | decoder: alive flag 1 -> 0 between round snapshots (metrics.py) | **6.0** | 22.7 |
| sets A-D, 180 games (tools/v3/panel_v4_top10_full.py) | platform summary: `death` event entries (baseline-v4top.csv) | **4.3** | 23.4 |
| sets A-D, 180 games | decoder (metrics.py) | 4.3 (180/180 games equal to the csv) | 23.4 (179/180 equal; one game differs by 2) |

- Per set, top-10 deaths a game are A 3.33, B 8.71, C 2.16 and D 3.11. Set B (corridors, broken_curve, gold_rush, boss_battle, concentrated_wealth) has the high count. So the 90-game A+B mean is 6.0 and the 180-game mean is 4.3.
- **Cassi & Issac:** both methods count each body. The platform emits one death entry per champion; the decoder sees two alive flags fall. A C&I kill is 2 deaths in both.
- **Cause rules:** the panel script has none; it counts every death. metrics.py splits by cause: champion if an enemy action hit the victim in the death round or the one before, with C&I partners sharing the killer; else monster if within 8 of a monster camp (camps mirrored by the map's symmetry); else other. On 180 games the top 10's 4.3 is 0.6 champion, 3.6 monster and 0.2 other. v4's 23.4 is 20.2 champion, 3.1 monster and 0.2 other.
- **One figure per sample:** 6.0 top-10 deaths a game on sets A+B (90 games); 4.3 on sets A-D (180 games). Use 4.3.

### Recall share: 15.8 % was a counting artefact
The 15.8 % "recalling" share in results/v5-s3-diag.md counted every round with a recall-start step (kind 11). Pantheon (team 6) starts a recall at the end of every walking turn and cancels it the next turn (kind 16 = cancel channel). The pattern each turn is [16 cancel, 4 move, 11 recall start]. Its champions are therefore always in an armed recall that completes whenever they stand still for 5 turns. That one team averages 5,027 recall starts a game; the other eight teams average 40-83.

Recall starts and completions (`tools/imitate/recalls.py`). A start away from spawn is completed when the champion next appears within 2 tiles of its own spawn, with no move, hit, tribute or cancel in between. Arrival is checked against the next snapshot, because the champion arrives at its turn start and may act in the same round.

| team | games | recall starts a game | completed a game | completed share | cancelled | died in the channel | rounds inside completed recall channels, share of champion-rounds |
|---|---|---|---|---|---|---|---|
| v4 | 180 | 43.9 | **39.9** | 91 % | 3.2 | 0.7 | 3.7 % |
| top 10 without Pantheon | 160 | 66.4 | **61.2** | 92 % | 4.7 | 0.3 | 6.0 % |
| Pantheon | 20 | 5,027 | **83.7** | 2 % | 4,936 | 3.5 | 6.5 % |

The top 10 complete about 1.5 times as many recalls as v4 (61 vs 40 a game), not 3.4 times. A completed recall occupies about 6 rounds of channel. The "about 170 recalls a game" that 15.8 % implied does not exist.

Hand checks, round by round:
- **v4top-t05-A-03 (Sponge).** 38 starts: 27 completed and 11 cancelled by the first script. Re-checked with arrival at the next snapshot: start r93 at (59,67), health 920, no action r94-98, at spawn (96,96) r99 with health 1,000. Start r136 at (43,29), health 785, still r137-140, shops at r141 (it arrived at its turn start), at spawn r142 with health 1,700. Start r132 at (28,54): still r133-136, then shop and move r137, at spawn r138. The first script called that one cancelled; that was the arrival-timing bug, now fixed.
- **v4top-t06-A-03 (Pantheon).** Champion 8 (C&I) makes 1,347 starts, 1,307 cancels (kind 16) and 883 moves. Rounds 2-8 each show [16, 4, 11] while it walks from (92,93) to (92,87). Start r49 at (77,57) with health 300: one step, then still r50-53, at spawn r54 and healed to 900 at r55 (completed). Start r79 at (42,67) with health 360: at spawn r85 with health 1,200.

Corrected time use (metrics.py: a channel round now has no move or hit; a channel survives the move in its own starting turn; any later action ends it), 180 games:

| team | dead | walking | clearing camps | capturing | fighting | tributing | recalling | idle |
|---|---|---|---|---|---|---|---|---|
| v4 | 23.8 % | 48.1 % | 11.5 % | 4.7 % | 0.9 % | 0.5 % | 3.9 % | 6.6 % |
| top 10 without Pantheon | 3.9 % | 52.6 % | 20.0 % | 6.0 % | 1.9 % | 1.8 % | 6.4 % | 7.4 % |
| Pantheon | 3.8 % | 56.7 % | 18.4 % | 0.0 % | 1.6 % | 1.6 % | 17.8 % (every still round is an armed recall) | 0.0 % |

### Conversion, one sample (180 games, sets A-D)
| team | picked up from camps | picked up by kills | tributed | conversion = tributed / all picked up | destroyed by monsters | lost to champion kills | carried at the end |
|---|---|---|---|---|---|---|---|
| v4 | 291 | 3 | 201 | **68 %** | 10 | 59 | 25 |
| top 10 | 1,047 | 59 | 976 | **88 %** | 49 | 4 | 79 |

The residual (picked up - tributed - destroyed - lost - carried) is -0.2 and -0.3 a game: every gem is traced. On this sample the production gap is 3.6 times (291 vs 1,047) and the conversion gap is 20 points. The earlier "159/269" mixed a 90-game numerator with a 90-game denominator and set them beside the 180-game 201. On 180 games alone it is 201/294.

## 2. Production queries (180 games; v4 and the top 10 in the same games)

### 2.1 Pickups per game by camp type (`tools/imitate/pickups.py`)
Each rise in a champion's bag or gold is attributed. It is a kill when the champion's kill count rose in the same interval. Otherwise it is a capture by the group whose spawn tile the champion stands on, split over the group's member types in proportion to their engine rewards (a group pays the sum of its members' rewards; maps21.json, mirrored). Every gem is matched: v4 295 and top 10 1,106 a game, equal to the funnel's camps plus kills.

| source | v4 gems | top-10 gems | ratio | v4 gold | top-10 gold |
|---|---|---|---|---|---|
| wolf | 72 | 147 | 2.0 | 724 | 1,533 |
| elk | 135 | **504** | 3.7 | 812 | 3,203 |
| gem cluster | 84 | **319** | 3.8 | 0 | 0 |
| gold cluster | 0 | 0 | | 1,223 | 2,413 |
| Monolith | 0 | 78 | | 0 | 127 |
| champion kills | 3 | 59 | 19 | 40 | 1,629 |
| other (unmatched gold) | 0 | 0 | | 0 | 57 |
| **total** | **295** | **1,106** | 3.7 | 2,798 | 8,962 |

The gem gap is elk (+369 a game) and gem clusters (+235), then wolves (+74) and Monoliths (+78). The top 10's gold is 3.2 times v4's: 1,629 of it comes from kills (50 a kill), and elk pay 4 times as much gold.

### 2.2 Completed recalls (`tools/imitate/queries.py`; started at least 3 tiles from spawn)
| | v4 (7,176 recalls, 39.9 a game) | top 10 (11,473, 63.7 a game) |
|---|---|---|
| distance from own spawn: 0-10 / 11-20 / 21-30 / 31+ | 22 / 43 / 21 / 14 % (median 16) | 18 / 40 / 23 / 19 % (median 18) |
| HP fraction at start: < 0.3 / 0.3-0.6 / 0.6-0.9 / >= 0.9 | 37 / 52 / 11 / **0** % (median 0.35) | 20 / 26 / 25 / **29** % (median 0.66) |
| bag at start: 0 / 1-19 / 20-49 / 50+ | 80 / 13 / 6 / 0 % | 61 / 19 / 12 / 7 % |
| within 10 rounds of arrival: purchase / heal / death | 3 / 100 / 1 % | **16** / 75 / 0 % |

v4 recalls only to heal, at a third of its health. The top 10 recall at full health too (29 % of recalls): to shop (16 % buy within 10 rounds) and to carry bags home (7 % with 50+ gems). Recall keeps the bag (FACTS.md).

### 2.3 Purchases (per champion)
| | v4 (990 champions) | top 10 (932) |
|---|---|---|
| gold spent by round 200: mean / median | **59 / 30** | **151 / 120** |
| first emblem: median round; champions with none all game | 376; 240 (24 %) | 205; 20 (2 %) |
| most common first purchase | monster shard of strength (489), monster emblem of strength (182) | power shard of focus (110), bruiser shard of health (102), jungle shard of strength (93) |
| most common first emblem | monster of strength (546), monster of focus (195) | power of focus (151), treasure of health (139), jungle of strength (126), bruiser of health (114) |
| most common first three kinds | monster shard > monster emblem > power shard (strength) (220) | bruiser shard > durability shard > bruiser emblem (health) (41); power shard > power emblem of focus > durability health (38) |

v4 spends 40 % of the top 10's gold by round 200 and completes its first emblem 170 rounds later. It always opens with the Monster family (camp damage). The top 10 split across power (stat), bruiser and durability (survival), jungle (less camp damage taken) and treasure (gold).

## 3. Conditional policies (`tools/imitate/policies.py` -> `tables.py`; 180 games)
Unit: one row per alive champion-round outside a recall or tribute channel. Enemy counts use positions, not line of sight. Rates are P(event within a short window | state); cells under 200 rows show only their count. Pantheon is split off where its start-and-cancel pattern matters.

### 3.1 P(start a recall that completes, within 5 rounds | state), at least 3 tiles from spawn
| HP fraction | v4 | top 10 | Pantheon | v4, no enemy within 8 | v4, 1 enemy | v4, 2 | top 10, no enemy | top 10, 1 | top 10, 2 |
|---|---|---|---|---|---|---|---|---|---|
| < 0.3 | **48.7 %** | 46.1 % | 64.4 % | **72.3 %** | 25.0 % | 11.6 % | 46.9 % | **49.0 %** | 36.5 % |
| 0.3-0.5 | 32.2 % | 22.6 % | 28.3 % | 35.8 % | 25.0 % | 11.6 % | 22.2 % | 26.7 % | 20.3 % |
| 0.5-0.7 | 7.5 % | 10.5 % | 9.7 % | 8.1 % | 5.2 % | 4.1 % | 11.0 % | 9.5 % | 6.7 % |
| 0.7-0.9 | 2.7 % | **7.0 %** | 4.4 % | 3.0 % | 1.5 % | 1.0 % | 7.5 % | 5.4 % | 4.5 % |
| >= 0.9 | 0.1 % | **2.5 %** | 2.8 % | 0.1 % | 0.2 % | 0.2 % | 2.4 % | 3.8 % | 3.0 % |

- **Crossing 0.5:** v4 crosses it only below 0.3 HP with no enemy within 8 (72 %). The top 10 never cross 0.5 over a 5-round window, peaking at 49 % below 0.3 HP with one enemy near. Pantheon crosses at < 0.3 (64 %).
- The difference is shape. v4 recalls almost only when alone and low: an enemy within 8 cuts its low-HP recall rate from 72 % to 25 % (one) and 12 % (two), as its "no recall with an enemy near" rule implies. The top 10 keep recalling with enemies near (49 % and 37 %), and they recall 25 times as often as v4 at full health.
- By distance (HP < 0.5): v4 recalls 0.7 % at 3-5 tiles from spawn (it walks in) and 30-45 % from 6+. The top 10 recall 10 % at 3-5 and 24-33 % farther.

### 3.2 P(tribute start within 3 rounds | bag, distance to the nearest altar)
| bag | v4, altar 0-1 | v4, 2-3 | v4, 4-7 | top 10, 0-1 | top 10, 2-3 | top 10, 4-7 |
|---|---|---|---|---|---|---|
| 1-9 | **66.6 %** | 50.9 % | 0.0 % | **13.3 %** | 16.0 % | 2.7 % |
| 10-19 | 56.9 % | 36.3 % | 0.0 % | 38.0 % | 34.5 % | 4.8 % |
| 20-39 | 79.2 % | 64.5 % | 0.0 % | **70.2 %** | 42.2 % | 8.2 % |
| 40-79 | 74.5 % | 74.6 % | 0.0 % | 76.6 % | 17.9 % | 8.2 % |
| 80+ | (6 rows) | (9) | (50) | (39) | 9.0 % | 4.3 % |

- **Crossing 0.5 (within 1 of an altar):** v4 at a bag of 1-9 (67 %); the top 10 at 20-39 (70 %), with 13 % at 1-9 and 38 % at 10-19. The top 10 rarely tribute small bags.
- They also hold big bags near an altar without tributing (bag 40-79 at distance 2-3: 18 %; 80+: 9 %). They wait out of the stun and lock reach, or carry the bag on.
- An enemy within 8 lowers the rate for both: at distance 0-1 with 10+ gems, v4 70 % -> 57 % and the top 10 63 % -> 51 %.

### 3.3 P(hit an enemy champion within 2 rounds | nearest enemy within 4)
| own HP / nearest enemy HP | v4, allies within 4: 0 | v4, 1 | v4, 2+ | top 10, 0 | top 10, 1 | top 10, 2+ |
|---|---|---|---|---|---|---|
| < 0.5 | 39.9 % | 44.5 % | 44.3 % | 34.1 % | 31.7 % | 45.4 % |
| 0.5-0.8 | 38.0 % | 41.7 % | 35.7 % | 39.4 % | 44.7 % | **54.2 %** |
| 0.8-1.25 | 30.6 % | 33.9 % | 29.8 % | 46.3 % | 43.9 % | **63.4 %** |
| 1.25-2 | 24.9 % | 30.5 % | 22.8 % | **56.6 %** | **54.7 %** | 42.9 % |
| >= 2 | 26.0 % | 30.2 % | 26.3 % | **66.5 %** | **58.4 %** | 50.1 % |

| gear points: own - nearest enemy | < -3 | -3..-1 | -1..1 | 1..3 | >= 3 |
|---|---|---|---|---|---|
| v4 | 37.9 % | 37.7 % | 32.7 % | 26.0 % | 28.0 % |
| top 10 | 36.5 % | 50.6 % | 53.7 % | 55.3 % | 56.0 % |

| distance to the nearest enemy | 1 | 2 | 3 | 4 |
|---|---|---|---|---|
| v4 | 62.0 % | 46.4 % | 32.0 % | 12.3 % |
| top 10 | 88.0 % | 63.6 % | 44.6 % | 30.3 % |

- **Crossing 0.5:** for the top 10 alone, at an HP ratio of 1.25 (46 % below, 57 % above); with two or more allies within 4, already at 0.5-0.8 (54 %). v4 never crosses 0.5 in any HP-ratio or gear cell (maximum 44.5 %). By distance, v4 crosses below 2 tiles and the top 10 below 3.
- The slopes are opposite. The top 10 fight more the stronger they are relative to the enemy (34 % -> 66 % alone; 37 % -> 56 % by gear). v4 fights less (40 % -> 26 %; 38 % -> 28 %): it answers attacks when weak, and lets weak enemies walk away. The top 10's 0.6 champion deaths a game (section 1) say they also choose their fights.

## 4. Candidate rules (at most three; state variables as in section 3)

1. **P_I_PRESS (press an advantage).** Let e be the nearest visible enemy within 3 tiles, h = own HP / e's HP, a = allies within 4. Engage e (cast in range, else step to attack range and attack) when h >= 1.25, or h >= 0.8 and a >= 2, unless carrying >= 40 gems or within 4 of an awake camp. Source: the top 10's engage curve in 3.3. *Expected effect:* enemy deaths a game vs v4 rise (champion kills by us), and our engage rate at h >= 1.25 moves from 25 % toward 55 %. *Falsified if* the h2h vs v4 is below 62/126, or our deaths a game rise more than 10 % against v4's in the same games, or the engage rate at h >= 1.25 does not rise.
2. **P_I_BAG20 (no small-bag tribute trips).** Do not start a tribute with a bag < 20 unless banked + bag >= 1,000, round >= 1,900, or an enemy within 4 threatens the bag (then bank at once). Source: 3.2, where the top 10 cross 0.5 at 20-39 and v4 at 1-9. *Expected:* tributes a game fall, gems tributed a game rise (fewer walks), and time walking falls. *Falsified if* tributed gems a game vs v4 do not rise, or gems lost to kills rise.
3. **P_I_SHOPRECALL (recall to shop at full health).** Recall when HP >= 0.7, gold >= 150 (one emblem), no bag or a bag >= 40, no enemy within 8, and at least 11 tiles from spawn. Source: 2.2 and 3.1 (the top 10 recall at >= 0.9 HP 25 times as often as v4, and 16 % of their recalls end in a purchase); 2.3 (v4 spends 59 gold by round 200, the top 10 151). *Expected:* gear at round 200 rises from 2.0 toward 5.3 points. *Falsified if* gear at round 200 does not rise by at least 1 point, or h2h < 62/126. S2's P_S2_SPEND_NOW (shop trips) covers part of this rule.

**Best single rule: P_I_PRESS.** Champion kills cause 81-86 % of v4's deaths (20.2 of 23.4 a game here), and 3.3 has the largest, opposite-signed gap. S2 already covers the shop part of rule 3, and rule 2 costs farming time only on small bags.

## 5. P_I_PRESS built and tested (bots/im = bots/s1g1 + one rule)
Code: `bots/im/src/a/enemies.cpp:122-138` (in `Bot::enemy_response`: choose the pressed foe; it also lifts the v4 chase-from-camp limit at line 177 for that foe) and `params.h:178-186` (P_I_PRESS 1, P_I_PRESS_RANGE 3, P_I_PRESS_BAG 40). Engine 1.0.0, `--jobs 3`, salts 0-2, both sides.

| test | im (s1g1 + P_I_PRESS) | s1g1 (baseline) | rule |
|---|---|---|---|
| vs v4, old 21 maps, 126 games (results/im/*-vs-v4.json) | **81/126** (64 %), deaths 19.8 vs v4's 24.7, gem margin +96 | 74/126, deaths 20.3 vs 23.4, +38 | reject below 62: **pass** |
| unopposed vs idle, all 35 maps, 210 games (results/im/*-unopp35.json) | 210/210, 751 gems a game, 0 faults | 210/210, 749, 0 faults | no regression: pass |
| our engage rate at own/enemy HP 1.25-2 / >= 2, alone (policies.py on the 126 games) | 59.9 % / 66.4 % | 54.2 % / 61.7 % | predicted to rise: **rose** (+5 points) |
| same, with 2+ allies within 4 | 50.0 % / 49.9 % | 43.9 % / 44.8 % | rose |
| v4's deaths a game in those games | 24.7 | 23.4 | rose by 1.3 |

- **Reading.** +7 wins is about 1.25 s.d. for 126 games: a weak but consistent signal. Our deaths fell (20.3 -> 19.8) while v4's rose (23.4 -> 24.7), as the rule predicts. In self-play s1g1 already engages 54-62 % at high HP ratios (v4 engaged 25-30 % against the real top 10), so the room for the rule is larger against real teams.
- **Fuel, inherited:** both builds touch 100,000 on the same salted games: salts 1-2 on 20 maps, and salt 0 on islands, starvation and yellow_brick_road unopposed. The P_I_PRESS diff adds no fuel peaks.
- The live top-10 panel is the test that matters: the falsifier in section 4 applies there unchanged.
