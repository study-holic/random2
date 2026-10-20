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
