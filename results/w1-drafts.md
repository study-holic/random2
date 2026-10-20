# w1: per-map drafts (builder w1, 18 Oct 22:21-23:00 BST)

Base: `results/v1-base.json` (v1, salts 0,1,2, 612 games). Candidate: `results/w1-drafts.json` (w1, salts 0,1,2, maps collision, corners, eight, pilgrimage; panel x-v4-14, x-foundry-v16 and four clones, both sides; 144 games). Compared with `scripts/cmpcells.py` (admit when dW >= 1.5 of 12, dGem > 0, dStrong >= 0).

Files: `bots/w1/src/map_<map>.cpp`, `setup_<map>()` sets `p.draft` only. No other file changed. Variants tried: 1 per map (time budget; the cap is 2).

## Drafts tried and why the code can play them

v1's list on these maps (team size 5: collision is 3): Vendetta, C&I, Varg, Ryth, Tavi (5) and Vendetta, C&I, Stellar (3). Stellar is eighth on v1's large list, so v1 never fields her on 5-champion maps, while top-10 teams draft her on 94-100% of games on all four maps.

Kit check in v1 source (micro.cpp, util.cpp, enemies.cpp, bot.cpp):
- Casting is generic: `cast_on_camp` (util.cpp:82) and `cast_on_enemy` (enemies.cpp:57) fire any ready ability that is damage, AoE damage or CC and can target camps/enemies. Stellar's single-target damage+stun and area hit, Ren's targeted stun, Ryth's area stun and Tavi's stun are all used this way. Attack resets (Ryth, Varg, Vendetta) are used by `reset_and_attack_again`; Varg's shield by `cast_shield`.
- Items (bot.cpp:161): Stellar, Cassi and Prism get the Focus build; everyone else Strength; Varg, Ryth, Vendetta get health first (tank). Ren gets Strength without health-first.
- Never used: dashes (Ren, Vendetta, Vesper), Willow's heal, Marcella's stun immunity is passive. Prism and Willow were therefore not drafted here; their ladder signals sit on maps outside this test (stampede, rows, ambush are already 10-12/12 in base and cannot clear dW >= 1.5).

| Map | Size | v1 draft | w1 draft (preference order) | Evidence |
|---|---|---|---|---|
| collision | 3 | Vendetta, C&I, Stellar | C&I, Stellar, Ren (then Ryth, Tavi) | top-10 C&I+Ren+Stellar n=18, 72% (+17.1); Varg z -2.2 avoided |
| corners | 5 | Vendetta, C&I, Varg, Ryth, Tavi | C&I, Stellar, Varg, Ryth, Ren (then Tavi) | top-10 modal C&I+Ren+Ryth+Stellar+Varg n=14; Stellar 100% of top-10 games |
| eight | 5 | same as corners | C&I, Stellar, Varg, Ryth, Tavi (then Ren) | top-10 C&I+Ryth+Stellar+Tavi+Varg n=5, 80%: v1's list with Vendetta swapped for Stellar |
| pilgrimage | 5 | same as corners | C&I, Stellar, Varg, Ryth, Tavi (then Ren) | Vendetta -8.9 pp (n=252, z -2.8); top-10 C&I+Ryth+Stellar+Tavi+Varg n=8, 75% |

All ladder figures are from `notes/top10-by-map.md`; top-10 draft cells are below 30 games and the excess is correlational.

## Results

`py -I scripts/cmpcells.py results/v1-base.json results/w1-drafts.json` (mean wins of 12 over salts 0,1,2):

| Map | Base wins | Cand wins | dW | dStrong | dGem | Decision |
|---|---|---|---|---|---|---|
| collision | 6.67 | 7.00 | +0.33 | +0.00 | +39 | reverted (dW < 1.5) |
| corners | 3.67 | 6.00 | +2.33 | +0.00 | +1762 | **ADMITTED** |
| eight | 8.00 | 6.00 | -2.00 | +0.00 | -2772 | reverted |
| pilgrimage | 6.67 | 7.00 | +0.33 | +0.33 | +665 | reverted (dW < 1.5) |

Total dW +1.00 over the four maps. Faults (flt) 0 on all 144 games. `bots/w1` is rebuilt with only `map_corners.cpp` changed from v1 (`diff -rq bots/v1/src bots/w1/src` lists that file alone).

Interlock, rushbait-v2 and swirl drafts were written (C&I+Stellar+Varg+Ren; C&I+Stellar+Varg+Ren; C&I+Stellar+Ryth+Varg) but not tested before the 23:00 stop; their files were reset to empty.

### Caveats
- **Salts gave no independent samples for w1.** On corners and eight every w1 cell is identical across salts 0, 1 and 2 (same round, deaths, gems). In the base, corners salts 1 and 2 are identical and differ from salt 0 in round and gems in all 12 cells; eight is identical across all three. The corners admission is therefore one effective w1 sample against about two base samples. dW +2.33 also clears the single-salt threshold (2.0), but only just.
- **Corners per cell:** gained ifish A (0 to 1), mite A (0 to 1), v4-14 A (0 to 1), ifish B (0.67 to 1); lost v4-14 B (1 to 0). foundry-v16 is still 0/2 and lost by more (gems 188 vs 1032 on A, 9 deaths vs 4).
- **Fuel:** corners vs x-foundry-v16 side B has 1 late `fuelExhausted` event (the same game in all three salts; a loss in both base and candidate). The base JSON predates the fuel_late field, so there is no base count to compare against.
- **"bot error" strings:** a raw grep of team-a/team-b stdout finds the string in 363 of 612 base bundles and 50 of 69 w1 bundles, from both teams and padded with null bytes. That is the same rate as v1, so the drafts did not cause it. The engine fault count (flt) is 0.

## Loss analysis (corners vs x-foundry-v16, side A, salt 0; per-game stats, no replay trace)
The replay tools (`tools/replay/final.mjs`, `extract.mjs`) are not in this checkout, so this reads only the mapgate per-game stats. w1 lost at round 405 with 188 gems against 1032 and 9 deaths (v1: round 423, 366 gems, 4 deaths). Likely cause: foundry-v16 wins the race on corners by fighting, and w1's team of C&I, Stellar, Varg, Ryth and Ren has one tank fewer than v1's (Vendetta out, Stellar in), while `enemy_response` (enemies.cpp) returns at once and never fights. Stellar is fragile, and the code does not use the dashes or stuns to escape, so the lineup bleeds deaths in contact. Not watched in the visualiser (time).

## Variants per map
One variant per map on collision, corners, eight and pilgrimage (the cap is 2). Draft list parameters are in the table above; nothing else changed.
