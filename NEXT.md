# NEXT: resume here (saved 22:20 BST Mon 19 Oct; Sprint 2 lock Sat 24 Oct 23:59 BST)

Read this first, then `PLAN-v3.md` (root causes, workstreams) and the tail of `RESULTS.md` (every test today).
No agents are running; every agent stopped at about 21:00 on the session limit. Their files are saved and listed below.
Standing rules from Xans: unranked matches may be run freely (`cambc match-groups create`, 50 groups an hour); **ask him before every upload**.


## Resume here (20:00 BST 20 Oct)
- **Map pool is now 35** (preflight --fix installed 14 maps; committed). Ranked games already use them. Live bot is still v4 (ranked drafts open with Vendetta, id 2).
- **v5 must NOT be uploaded:** 35-map gate FAIL: shortcut lost to starter-cpp 0-0 gems (gold tiebreak), mapid unknown on the 14 new maps. bots/s1 wins shortcut, so S1's BANK_REACH fix is needed.
- Next candidate **v5r** = v5 + S1 BANK_REACH + NSB walk-out + 14 map ids: brief tools/cloud/v5r-release.md for the "v5 s1 economy" session. Then critic, laptop gate, Xans submits, top-10 panel.
- S1 report reviewed (results/v5-s1.md): controller not admitted (median -1.3 %, corners +15 %); the two fixes are the keepers.
- v6 (v5 + S3 contest) integration check still in the "parity" session; rebuild v6 on v5r afterwards.

## Resume here (16:15 BST 20 Oct)
- **v5 = S2 build B** (bots/v5, frozen copy of nice-cray 25ec32b): gate PASS on laptop and cloud (42/42, max 93,816), 70/126 vs v4, deaths 17.0 vs 22.9, gem margin +132, gear 2.71 (FAIL vs 3.0, waived in PLAN-v4.md). Upload blocked for Claude by the permission classifier: Xans runs `cambc submit bots/v5`. After that: VERSIONS.md entry, then `python tools/v3/queue_panel.py v5top --sets ABCD` (check top10.csv is current) and judge by PLAN-v4.md "Waiver and panel thresholds".
- **v6 = v5 + S3 contest module** (heisenberg's, 13.7 deaths vs 35.4 for archimedes's): bots/v6, compiles, brief tools/cloud/v6-integrate.md for the idle "parity" cloud session.
- **Null for self-play:** v4 vs v4 = 73/126 in mapgate (S3 session). w1's 73/126 equals it: per-map N2 shows no gain, not integrated.
- Build A (sleepy-clarke) kept as bots/s2a, not uploaded (no gate cap, latching lock flag). S3 "archimedes" module stays on main as bots/s3 (data).
- Sessions: S1 economy still running (nifty-cray).

## Where we stand

- Live bot: v2 (platform "v1"), uploaded 23:33 BST 18 Oct. Ladder at 18:00 Mon: rating 1,417, rank 499; 515 ranked games, 46 % wins.
- **Real-team baseline** (`notes/unranked-v2-baseline.md`, 136 of 160 unranked games vs 8 teams rated 1,453-1,771; raw in `data/unranked/`): v2 **27-109** (20 %); gems 563 vs 919; tributed per round 0.89 vs 1.52; tributes per game 11.8 vs 22.3; deaths 12.0 vs 4.8. Per map 0-8 on collision, eight, elkring, fragment; 1-7 on contest, corners, interlock; 4-4 on ambush, expanse. Against umamusume (1,771) 0-17; Tozo's (1,686) 1-16.
- **Six local maps were old layouts** (collision, corners, expanse, interlock, lanes, packs): the current files were found in the old workspace's `new maps.zip` (16 Oct 16:09, after the re-layout; the other 11 maps are byte-identical to ours). Saved to `maps-1016/` (named by map). `maps/` is unchanged. **First job tomorrow**: copy `maps-1016/*` over `maps/` (backup in `maps-backup-1019/`), re-run the v2 panel baseline (`python scripts/mapgate.py v2 all --jobs 8 --salts 0,1,2 --out results/v2-panel-1016.json`), and use that as the base from now on. Every local result on those six maps so far is on layouts the ladder does not use.

## Results today (details in RESULTS.md)

| Work | Verdict |
|---|---|
| w9 monolith siege from range 4 (contest) | **admitted on contest**: h2h vs v2 6/6, median win round 2,000 -> 1,510 |
| w7 data drafts (eight D, stampede R) | **admitted** on eight (6/6) and stampede (5/6); counter-draft table off (no stable signal) |
| w8 Tavi Focus + field buys, monolith map only | **admitted** on monolith (panel 12/12, h2h 6/6) |
| w8 damage-only builds (all champions) | rejected locally (h2h 38-43 %), conflicts with ladder evidence -> settle on real teams |
| w12 engagement rules (regroup, avoid) | rejected (h2h 28/102); real cause of lost fights is the damage gap (we dealt 6,893, took 60,065 in 60 lone fatal fights) |
| w10 retreat fix | rejected (regressions on pilgrimage, collision, packs) |
| w11 locks / interrupts | locks marginal (not admitted); interrupts rejected |
| w13 gems-per-round targeting | rejected (h2h 39/102). Mode 2 (gold-only groups +20 tiles) h2h 54 %; swirl 11/12 h2h: retest |

Caveat: at 19:21 the w14 agent killed every cambc.exe, which cut two runs short (`matches/w10-185200` at monolith salt 2; `matches/w13-191856` at eight salt 1); mapgate counted the unplayed games as losses. w13's h2h and w10's V6 numbers are slightly understated; neither verdict flips on that alone, but re-run before quoting them.

## Unfinished work (pick up in this order)

1. **v3a release candidate** (integrator stopped mid-way). `bots/v3a` is partial and unverified: rebuild it cleanly. Spec: v2a + w7 part 1 (eight, stampede drafts) + w9 siege on contest only + w8 build C (Tavi Focus + field buys) on monolith only, gated at run time by map plan; every other map must play move-for-move like v2. `scripts/integrate.sh v3a w7 w9` applies the first two as patches (stops on conflict); add the w8 piece by hand from `bots/w8tf` (macros P_TAVI_DMG_FOCUS, P_FIELD_SHOP, P_FIELD_SHOP_HEALTH 0). Then: starter-cpp 34/34 safety + fuel check, panel 3 salts, h2h vs v2, pace. Then **ask Xans to upload**, and queue the same 8-team unranked panel (`data/unranked/panel.csv`; groups A-D per team as in `groups.csv`) against v3a for the paired comparison.
2. **Consultation** (`consult/1905-strategy-wide.md`; prompts in `consult/prompts/`; attachment `consult/pack/xanarchy-context-v4.txt`). Part 1 sent to ChatGPT (chat "Rank Battlecode Strategies") and DeepSeek (chat "Battlecode diagnosis and priorities", DeepThink on). Copilot: file attached and text pasted, Send click timed out: check whether it went. Then send Part 2 to all three, paste the answers into the consult file, classify and check claims (the /consult skill).
3. **Micro lab (w14)**: catalogue `notes/micro-catalogue.md`; harness `tools/micro/` (arena maps via the engine's MapBuilder API, `maps-lab/`), duel bots `bots/w14d`, `bots/w14t`; partial tables `results/w14-duels.md/.json`, runs in `matches/w14-lab`. Remaining: base-stat 1v1 matrix -> micro-variant gains per matchup -> item-level arenas -> adaptation table written to draft memory at draft time (the drafter sees all enemy picks).
4. **Fight model (A2)**: fight dataset `data/v3/fights.jsonl` (19 MB) written; `notes/matchups.md` not yet written. Remaining: champion-pair fight matrix, draft-level win-excess matrix with intervals, logistic fight-outcome model on bot-visible features, exported as a C++ table.
5. Item builds on real teams: the local panel (our own old bots) punishes no-Health builds while the ladder winners use them. Decide with unranked games after v3a is live, not with the local panel.

## Key facts for the next session

- API limits for adaptive micro: each champion's memory persists all match; 1,024 bytes of draft memory (written in the draft, read-only later); 8 bytes shared team memory; no shared vision; about 100,000 fuel a turn; nothing persists between matches (learning is offline, shipped in uploads).
- Data: `data/v3/` (1,923 top-team replays decoded; behaviour tables in `notes/v3-top10-behaviour.md`), `data/ladder/` (our decoded losses), `notes/map-economics.md` (per-map camp groups and gem supply), `intel/` (9,245-game draft data).
- Old workspace (MicroStella era, 13-18 Oct) archived to `archive/cambc-context-2026-10-18.zip` (notes, experiment records, tools, every old bot's source; no replays or builds).

## Clean-up done 22:30 Mon (all to the Windows Recycle Bin, recoverable until Xans empties it)

- `C:\Users\willi\OneDrive\Desktop\cambc-context` (1.5 GB, 30,464 files syncing through OneDrive): needed parts migrated first (current maps -> `maps-1016/`; notes, experiment records, tools and every old bot's source -> `archive/cambc-context-2026-10-18.zip`, 6,639 files, verified); old-engine replays, build output, debug traces and SDK copies not kept.
- `matches/`: 124 finished experiment run folders (their numbers are in `results/*.json` and RESULTS.md). Kept: `v2-*` baselines, `v1-222117` (watched swirl loss), `w14-*` (micro lab in progress).
- `bots/*-s<N>`: 74 salt copies; `scripts/mapgate.py --salts` rebuilds them on demand.

## Update 22:50 Mon

- `maps/` now holds the current layouts (six maps replaced from `maps-1016/`; old ones in `maps-backup-1019/`). v2 re-baselined on them: `results/v2-panel-1016.json` is the 17-map base for cmpcells from now on. (The lanes fuel_late 144 was a salt-copy artifact, not a bot fault: salt 0 had 0.)
- Resumed at 22:45 (stop 01:00, short reports): integrator (v3a -> `results/v3a-release.md`), w14 micro lab (`results/w14-duels.md`), A2 fight model (`notes/matchups.md`). Data agent finished (`notes/unranked-v2-baseline.md`).
- Consultation: ChatGPT answered Part 1 (~9 min) and has Part 2; DeepSeek answered Part 1 (27k chars) and has Part 2. Neither answer is copied into `consult/1905-strategy-wide.md` yet (read each once, summarise, classify). Copilot: Part 1 status unknown; page froze Chrome repeatedly, retry tomorrow.
- CPU: all cambc/node/python processes are held at Idle priority by a background script (`%TEMP%/cambc-lowprio.ps1`, runs until about 05:00); game jobs capped at 7 in total. Xans asked to cut token use: prefer fewer, cheaper agents (sonnet for analysis), short reports, no long file dumps.
- A2 done (`notes/matchups.md`): fight model AUC 0.861 (coefficients `data/v3/matchups/fightmodel.h`); top-10 open fights with a spell 84 % vs our 32 %; they carry 12.8 vs our 5.0 shard-points at rounds 200-400; draft pair effects not robust. v3b candidates from this: spell-first engage, fight-model engage gate, faster gold-to-gear.
- w14 done (`results/w14-duels.md`): at base stats the costliest v2 micro is the 45 %-health walk-home mid-fight (+0.149 per duel without it); Varg shield at engage +0.19; dashes +0.05-0.09; range-4 casting +0.04-0.08 (not Marcella); per-champion adaptive set +0.164. Next: v3b = v3a + that per-champion micro table + spell-first engage + fight-model gate (A2), tested on the current maps vs v2 and then on real teams. Item-level and team-fight arenas are coded in tools/micro/ but not run.
- 23:00: v3a release tests near done (starter 34/34). v3b = v3a + w14 per-champion micro table being built and tested (`results/v3b-micro.md`, stop 01:30): h2h vs v3a and v2, 3 salts; reject if < 55 % vs v3a or deaths +15 % with no gem gain.

## Update 06:30 Tue 20 Oct

- **v3b uploaded 06:05** (Xans approved). Map pool is now 21: kept ambush, collision, contest, corners, eight, elkring, expanse, fragment, interlock, lanes, monolith; new big_world, boss_battle, broken_curve, concentrated_wealth, corridors, elk_arena, fracture, gold_rush, hooks, islands; removed packs, pilgrimage, rows, rushbait-v2, stampede, swirl. `cambc maps pull` still fails (HTTP 303).
- Running (sonnet, short reports): maps builder (rebuild new maps from replays, identify() check, starter safety on 21 maps, v3b/v2 baselines → `results/newmaps-safety.md`, stop 10:30); panel analyst (v3b live check, 8-team unranked panel, 20 maps per team with no repeats → `notes/unranked-v3b.md`; top-team play on new maps → `notes/new-maps-intel.md`, stop 12:00); consult runner (Part 1/2 answers → `consult/1905-strategy-wide.md`; Part 3 "game-changers" `consult/prompts/2010-gamechanger-part3.txt` → `consult/2010-gamechanger.md`, stop 10:30).
- Next wave after those: counter-draft simulations per map (our drafts × common enemy drafts, local), plans for elkring and monolith (v3b's regressions) and the new maps, consultants' game-changers built and tested.
- Clean-up 06:20: 39 finished builder copies zipped to `archive/builders-2026-10-19.zip` (sources only) and recycled with salt copies, old match runs and `maps-backup-1019`. Old workspaces at C:\cambc-work (1.0 GB), C:\cambc-top30-intel (0.9 GB), C:\cambc-handoff (0.15 GB) and five small cambc-* folders (15-16 Oct) untouched: ask Xans.
- Low-CPU keeper restarted at 06:15 for 18 h.

## Engine 1.0 (20 Oct)

- Ladder, CLI 3.0.1 and SDK 3.0.1 run engine 1.0.0. Full list: `docs/rules/ENGINE-1.0.md`; facts for briefs: `FACTS.md`; maps: `notes/maps-21.md`, `data/maps-21.png`, `notes/map-economics.md` (regenerated for 21 maps; old tables kept as `notes/map-economics-engine0.md`). The site rules page lags the engine (no Heart of Ruin or golem, old Prism, Vesper and base stats).
- Changes: Heart of Ruin (12th champion; missing-health damage passive, self shield, dash with landing damage and stun); Prism 1.0 Focus per gem (was 0.2); Vesper 33% gold; Marcella +2 gold per gem; base-stat rebalance of 11 champions (read from the engine tables); Tavi stun 1.4 x Focus; gem and gold clusters 120 HP; elk range 4, monolith range 5; golem camp type (on no map); per-camp rewards (wolf 4 gems/40 gold, elk 15/90, gem cluster 7 gems, gold cluster 20 gold; some groups x2); SDK: in_vision, awake/asleep camp tile bitboards, ward owner, path-move treats the target as free (throws on a blocked target).
- Re-measure before trusting: draft and counter-draft tables (matchups.md, intel/meta) for Prism, Marcella, Vesper, Willow, Tavi; the w14 duel lab and v3b micro table (base stats, Tavi stun, dash fields); map-economics based targeting (cluster HP and rewards changed, so gem-per-HP and capture-time ratios shift); monolith and elk siege ranges (camp range 5/4); v3b/v2 baselines on the 21 maps and under engine 1.0; Prism carry behaviour; code paths using the SDK path-move on possibly blocked tiles.

## Resume here (laptop closed 08:04 BST Tue 20 Oct)
- Cloud (claude.ai/code, repo cambc-xanarchy, env cambc) is running `tools/cloud/arena-tests.md`, then `tools/cloud/v4-base-tests.md`. Each pushes to its own `claude/...` branch: `git fetch` and merge, then read `results/arena-1020.md` and `results/v4-cloud.md`.
- If v4 passes the gate and is ≥ 45 % vs v3b-e1: run the critic on it, run preflight, upload from the laptop (standing approval), log it in VERSIONS.md, then run the 160-game real-team panel (laptop, platform).
- Step 0 (death causes, v3b drafts) was running locally as an analyst. If `notes/step0-deaths.md` is missing or partial, restart it.
- Then v5 per PLAN-v4 "Draft 3", with v5 tests run in the cloud.
