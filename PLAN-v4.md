# PLAN-v4: from rank ~500 to the top 20 by Sat 24 Oct 23:59 BST

Draft 2, lead, 06:55 BST Tue 20 Oct (draft 1 06:35; critic review `notes/critic-plan-v4.md`, verdict "proceed with named changes", applied below). Next: consultants; Xans decides. Supersedes PLAN-v3.md (kept as history).
Versions: integer only from now on (Xans, 20 Oct). v4 = the first release on engine 1.0; later releases v5, v6.
Deadlines (platform Tournaments page, checked 20 Oct 06:50): **Sprint 2 lock Sat 24 Oct 23:59 BST** (single elimination, 15 games per pairing, locked submission); Sprint 3 lock Sat 31 Oct; qualifiers 4 Nov (international) and 11 Nov (UK); finals 18 Nov.
Xans's standing approvals (20 Oct 06:48): upload any candidate that passes `gate.py` and the critic, for real-team testing; up to 10 game jobs 00:00-07:00; upload v4 base once it passes the gate.

## Draft 3 (07:58 BST Tue): decisions after the consultation (`consult/2020-plan-v4.md`; ChatGPT, DeepSeek and Claude answered; Copilot blocked by the browser safety check)

This section supersedes §5 (schedule) and the module order in §3 where they differ. The lead decided under Xans's standing approval: proceed after consultation.

Releases are admitted, not single modules. Primary measures: paired gem margin and deaths per game on the 160-game real-team panel; win rate is secondary.

| Release | Target time | Contents (consensus items marked ★ = all 3 consultants) | Admission |
|---|---|---|---|
| **v4** "do no harm" | Tue ~18:00 | ★ 21-map hash identification + safe default for unknown maps; ★ islands/holes reachability (never target unreachable groups); ★ anti-feeding on contest and gold_rush (monolith range 5, elk range 4); fuel ≤ 70k a turn; Heart of Ruin and enum safety; v3b kept as rollback | gate.py PASS; local h2h vs v3b on engine 1.0 ≥ 45 %; panel: kill if < 30 %, deaths ≤ 25 a game |
| **v5** survival + gear | Wed 18:00 | ★ camp threat model (monster ranges 3/4/5; camp kills destroy carried gems); ★ spawn anti-feed (enemies spend 8-9 champion-rounds near our spawn vs 0.8 for top-10); ★ ability-first opener (≥ 70 %); ★ carrier protection, with tribute timing set by exposure rather than a fixed bag; ★ predictive recall (gold or HP); ★ focus Cassi (C&I die together); ★ stun enemy channels (tribute, recall, crystal); gear curve (gear at round 200 ≥ 3.0); draft lookup (★ core Stellar, C&I, Ren, Varg, Ryth; measure v3b's actual drafts first); ★ gold-tiebreak mode on boss_battle and contest | panel ≥ 45 % vs the 8 teams (M1 50 % stretch); deaths ≤ 20 |
| **v6** economy | Thu 18:00 | ★ offline map compiler: hash-keyed tables per map from the econsim, made HP-aware, unpacked at draft; ★ one capturer per group, conveyor, respawn-window routing; runtime dispatcher; capture-into-tribute geometry; narrow G only if its arena test passes (monolith lock-out or respawn blockade) | ≥ 50 % vs ranks 50-100 (M2); gems per round ≥ 0.40 |
| **v7** main | **Fri 14:00** (patches until 22:00) | Per-map fixes from panel data; experiments that passed (Heart of Ruin initiator, Prism big-bag, Marcella gold) | top-20 sample ≥ 40 % (M3) |
| Sat | – | Bug fixes only; last upload by 20:00 | gate |

Main release moves to Fri 14:00 (Claude's computation: a top-20-strength bot needs about 3-5 hours of ranked play to climb from about 1,700).

**Step 0, now:** split our deaths by cause (camp, enemy, near our spawn, while carrying), check what v3b actually drafts, and make the same split for the top 10. Analyst, cheap, data already decoded.

**Engine arena tests before building on them** (★; cloud once parity passes):
- do monsters attack the nearest champion?
- does an occupied spawn tile delay respawn (GameRules l.134/139), and by how much?
- Prism at 0/20/40 gems;
- Heart of Ruin 2v2 and 3v3;
- camp aggro used against enemies.

**Cut:** H (opponent model); E as minimax (no pick can deny the opponent, so a lookup table); S3 full draft matrix (forced-draft comparisons only).

Odds stated by a consultant: Claude gives about 1 in 8 for top-20 strength by Friday. The others call it a stretch.

## 0. Target and arithmetic

| Mark | Rating (06:30 Tue) |
|---|---|
| Rank 1 | 2,231 (Intercontinental Fish) |
| Rank 10 | 1,975 (AlphaSlop) |
| **Rank 20** | **1,888** (Trzy Kwaterki) |
| Rank 25 | 1,862 |
| Us | about 1,420, rank about 500 (preflight will record the exact figure) |

- Rating is not the bottleneck; bot strength is. Teams play about 6 ranked series an hour (CLAUDE.md), so a bot that really is top-20 strength climbs 470 points within a day.
- Strength today: live v3b went **29-131** against 8 real teams rated 1,453-1,771 (`notes/unranked-v3b.md`); v2 went 27-109. To sit at rank 20 a bot must win about half its games against ranks 10-30 and nearly all against teams below 1,700.
- Milestones, each measured against real teams in unranked games (the only yardstick that has matched the ladder). Ratings recomputed per the critic (a 50 % record is worth about the panel's average rating):
  - **M1, Wed 22:00:** at least 50 % against the 8-team panel (1,453-1,771, average about 1,600). Implies about 1,600.
  - **M2, Thu 22:00:** at least 50 % against a ranks 50-100 panel (about 1,670-1,750). Implies about 1,710.
  - **M3, Fri 22:00 (main release):** at least 40 % against a top-20 sample, plus at least 60 % against ranks 20-50. Implies about rank 20. The top-20 sample (about 400 games) runs Friday night.
- The Sprint 2 tournament uses the locked submission; the ladder top 20 needs the main release live by Fri 22:00 so Saturday's ranked series can climb. Saturday is for bug fixes only.
- **Plan-level kill rule:** if v4 base (bugs fixed) still wins under 30 % against the 8-team panel, the gap is structural: module A+B+F goes first and micro work waits.
- Honest odds (lead's estimate): M1 likely if the gear and economy modules work; M2 possible; M3 a stretch from 18 % today.

## 1. Why we keep making simple errors, and the remedy

| Error (last 3 days) | Root cause |
|---|---|
| Six local maps were old layouts for days | No check of local files against the platform |
| Map pool changed overnight (10 new, 6 retired); v3b uploaded untested on it | Same |
| All local tests ran on engine 0.0.3 while the ladder ran 1.0.0 (new champion, golem, dash, Prism/Vesper/Marcella) | No engine or version check; the site's download page still says 2.2.5 |
| v3b +15.7 wins locally, 29-131 against real teams | Local panel = our own old bots; local wins were treated as evidence |
| Lanes "fuel bug" reported, then retracted (salt-copy artifact) | Lead read a number without its test conditions |
| An agent killed every cambc.exe and cut two runs | Brief did not forbid it |
| Replay decoder silently rejects engine 1.0.0 | No version check in tools |
| Lead context overloaded; stale facts (17 maps, 11 champions) copied into briefs | Facts lived in the lead's memory, not in a checked file |

Remedy (all mandatory from today):
1. **Ground truth is automated.** `scripts/preflight.py` checks CLI, SDK, ladder engine, map pool (byte-compare with the official files), rules doc and SDK hashes, champion ids seen on the ladder, our rating, and the live version. It writes `STATE.md`. It runs at the start of every work block, before every test wave and before every upload. A FAIL blocks all three.
2. **One fact sheet.** `FACTS.md` (at most 70 lines, every fact with its source line and verification date) plus `STATE.md` head every agent brief. No fact enters a brief from memory.
3. **Automated upload gate.** `scripts/gate.py <bot>` reads the pool from STATE: starter games on every pool map, both sides; 0 faults; no turn over 100,000 fuel; no fuel exhaustion after round 1; turn-1 `mapid=` print checked for every map. Then the critic reviews, then Xans approves.
4. **Real teams are the yardstick.** Local games only filter candidates; admission needs a real-team panel. This needs uploads, because unranked games use the live bot (see §8, a question for Xans).
5. **Every result carries its conditions:** engine version, map-set hash, opponents and number of games. Mapgate writes them; results without them are not quoted.
6. **Lead context discipline.** The lead holds only PLAN-v4, STATE, FACTS and agent reports of at most 250 words. Agents own the depth and write it to files. NEXT.md is rewritten, not appended, at the end of each block (at most 60 lines).
7. **In-bot self-checks:** a map-hash table for identification (no shape guessing), a `static_assert` on champion and camp enum sizes, and turn-1 `mapid=` and fuel prints in debug builds.
8. **Fresh-context critic** on every plan, brief template and release; consultants on the plan and on each big module.
9. **Timestamps come from `date`**, never from estimates (twice now the log ran ahead of the clock).

## 2. The gap (real-team data, 20 Oct, engine 1.0)

| Measure | v3b | Comparison | Source |
|---|---|---|---|
| Tributes per game | 19.3 | 37.5 (its panel opponents) | `notes/unranked-v3b.md` |
| Deaths per game | 38 | 21 (its panel opponents) | same |
| New maps (10) | 10-66 | – | same |
| **Engine 1.0 only (live since 03:00Z 20 Oct):** | | | `notes/top10-2026-10-20.md` (126 top-10 sides decoded, 2,055 listed) |
| Deaths per game | 33-45 | 17 (top 10) | same |
| Fights opened with an ability | 35-43 % | 85 % (top 10) | same |
| Gear points per champion at round 200 | 1.7 | 3.8 (top 10) | same |
| Gems per tribute | 24-30 | 41 (top 10); tributes per game: no gap | same |
| Altar-lock buys per game | 0 | 6.4 (top 10); lockers beat non-lockers 16 of 31 (no clear edge) | same |
| Enemy champion-rounds near our spawn | 8-9 | 0.8 | same |
| Top-10 gems per round, engine 0.0.3 → 1.0 | – | 1.10 → 0.57 (camp rewards cut) | same |
| Drafts | Vendetta, Tavi sets | Stellar in 96 % of top-10 drafts; core Ryth, C&I, Stellar, Ren, Varg; Tavi sets −10.8 pts; Prism −14 pts (55 sides); Heart of Ruin 0 of 2,055 | same |
| Fight outcome predicted by (engine 0.0.3) | HP fraction, lowest-HP unit, gear (AUC 0.86) | | `notes/matchups.md` |

Three levers, in order of size: **income** (gems captured per round), **conversion** (tributes, gems lost on death) and **survival** (fights taken and won). Gear feeds all three.

## 3. Architecture: eight modules, each tied to cambc rules

Rule quotes come from `docs/rules/GameRules.md` unless marked SDK (engine 1.0, `docs/rules/ENGINE-1.0.md`).

**Build order after the critic review:**
- A, B and F are one module (plan compiler, economy and claims); they share data and are tested together.
- C and the cheap part of D (ability-first opening, fight gate) come first; they are the fastest large levers.
- E becomes a lookup table from the S3 simulations.
- G is only a respawn-blockade experiment, run after M2. The rule is verified: GameRules.md l.134, "Any occupied spawn tile delays the entire group." The critic misread it as missing.
- H moves to Sprint 3.
- No current map has a golem (ENGINE-1.0.md), so golems get enum safety only.

**A. Map compiler (draft phase).** Rules: the drafter "can see the entirety of the map layout" and has extra fuel; 1,024 bytes of draft memory.
- Compute at draft time:
  - BFS distance fields from each altar and spawn;
  - camp groups with value (gems, gold, HP, strength, respawn);
  - clear time for each candidate composition (damage model);
  - side and contested classes from our distance minus the enemy's distance;
  - chokepoints (narrow cuts between regions);
  - monolith and golem positions.
- Output a plan in draft memory, about 200 bytes:
  - map class;
  - for each champion slot, an ordered route of camp-group ids;
  - bank thresholds and altar choice;
  - composition class;
  - denial targets.
- Champions rebuild only the fields they need, spread over turns 1-5 so no turn exceeds the fuel limit.
- Generic first, per-map overrides second. It is tested held-out: plans for kept maps are scored before any per-map tuning.

**B. Economy engine (macro: orienteering with time windows).** Rules:
- camp respawn times (wolf 40, elk 60, gem cluster 50, monolith 150 rounds after the claim; golem per SDK);
- "Extra allies do not speed capture";
- capture takes 5 rounds uncontested;
- tribute takes max(1, ceil(gems/10)) turns;
- the killer takes all carried gems.

Behaviour:
- Each champion follows a route that maximises expected banked gems per round: travel + clear + capture, respawn windows, and risk (gems at death are lost).
- One capturer per group; the others move on (a camp conveyor).
- The bank size b* is chosen per map class to minimise round cost per banked gem, (travel + ceil(b/10)) / b, discounted by death risk on the route.
- Target (revised after the top-10 study): top-10 gems per round under engine 1.0 (0.57) with bigger bags (about 40 gems per tribute, as top-10 teams bank) rather than more tributes; deaths at or below 20 a game, because gems carried at death are lost and camp kills destroy carried gems (GameRules l.315).

**C. Gear engine (gold to stats).** Rules: shards cost 30 gold; five matching shards combine; buying happens on spawn tiles or while dead; Tavi buys anywhere; Prism gains 1.0 Focus per carried gem (SDK 1.0); Vesper earns +33 % gold; Marcella gets 2 gold per gem tributed.
- A per-champion purchase curve: damage first; Focus for casters; Prism Focus combined with its gem stack.
- Gold-driven recall when gold ≥ k × 30 and no fight is near (MOBA recall timing).
- Target: 10 or more shard-points by round 300.

**D. Fight engine (micro).**
- Engage gate from the fight-outcome model, refitted on engine-1.0 fights: enter at p ≥ 0.55 and disengage before melee contact (melee walks and hits in one turn).
- Open with an ability, then focus the lowest effective HP.
- A per-champion micro table, re-measured in the duel lab on engine 1.0, including Heart of Ruin and the new dash.
- Carriers at 30 or more gems avoid fights and bank.
- Pick off enemy carriers when p is high (the killer takes all gems).
- Stun visible enemy channels.
- Fight at camp edges so monsters hit the enemy: monsters have range 3, never chase, and attack the nearest champion.

**E. Draft engine (game theory).** Rules: picks are simultaneous each round; revealed picks are visible to later rounds.
- Per map class, a payoff matrix from local simulations (our compositions × common enemy compositions, both sides, 3 salts) and top-team draft data.
- Pick by minimax or a mixed strategy each round, conditioned on revealed picks.
- Composition roles (farmer, carry, stunner, tank) chosen to enable modules B and G.
- Re-evaluate Heart of Ruin and Prism under engine 1.0.
- Pair counter tables are dead (1 of 55 cells survived; none after map effects). Composition-level only.

**F. Team coordination (8 shared bytes).**
- Byte layout:
  - camp-group claims (who captures what);
  - one danger flag with its region;
  - a focus target id;
  - a plan phase;
  - a regroup tile.
- Writes are rate-limited and versioned so stale claims expire.

**G. Map control and denial (MOBA).**
- Respawn blockade: "Any occupied spawn tile delays the entire group". Used on enemy-side camps when a champion would otherwise idle; dropped if the blocker dies more than it denies.
- Altar locks on enemy altars that lie on our routes.
- Wards at chokepoints.
- Objective trading: when they contest our camp, take theirs.

**H. Opponent model (in-match adaptation).**
- Per visible enemy champion:
  - observed damage per hit or cast (which reveals its gear);
  - aggression (fights it starts);
  - where it is seen.
- Updates the fight-gate threshold and route risk.
- The prior comes from the enemy draft, written into draft memory by the drafter.

## 4. Simulation stack, built for cambc

| Sim | What it answers | Built from | Validated by |
|---|---|---|---|
| S1 duel and team-fight lab (engine 1.0) | micro table; fight-model calibration; Heart of Ruin | `tools/micro/` arena maps (MapBuilder), 1v1 all pairs at 0/5/10/15 shard-points, 2v2, 3v3 | known-outcome duels; ladder fight data |
| S2 macro simulator (Python, no combat) | routes, bank thresholds, conveyor vs. group farming per map | map files, camp table, BFS, capture and tribute rules | v4 vs. an idle bot in the engine: gems per round within ±10 % |
| S3 draft matrix | composition payoffs per map class | forced-draft engine games | stability across salts and halves |
| S4 real-team panels | admission of every module | unranked groups (50 an hour): 8 mid teams, a top-50 panel, a top-20 sample; each map once per team | ladder rating after upload |
| S5 top-team data | behaviours to copy (gear curve, cadence, openings) | decoder or `cambc matches summary` on engine 1.0 | – |

## 5. Schedule

| When | Work | Gate |
|---|---|---|
| Tue 08-11:30 | Remedy tooling: preflight, STATE, FACTS, gate, mapgate tags, decoder for 1.0 | preflight PASS |
| Tue by 18:00 (critic's estimate) | **v4 base**: engine-1.0 fixes (map identification for 21 maps, fuel, Heart of Ruin safety, dash, contest 0-gem collapse, islands stall, gold_rush feeding) | gate PASS; h2h vs v3b on engine 1.0 ≥ 50 %; critic; upload (approved); 160-game panel |
| Tue 13:00 | Plan to 4 consultants (ChatGPT, DeepSeek, Copilot, and Claude in the browser as an independent reviewer) | – |
| Tue pm | S2 macro simulator; S1 lab on engine 1.0; top-10 study → behaviours; fight model refit | – |
| Tue night (10 jobs) | S1 duel/team-fight matrix; S3 draft matrix starts | – |
| Wed | **v5** = v4 base + C (gear curve, recall) + D1 (ability-first opening, refitted fight gate, carrier safety) | gate; 160-game panel; **M1** |
| Wed-Thu | **v6** = v5 + A+B+F (plan compiler, economy routing, conveyor, claims) | gate; 160-game panel; ranks 50-100 panel; **M2** |
| Fri | E lookup from S3; weakest-map fixes from panel data; G blockade experiment if M2 passed; **main release by 22:00**; top-20 sample overnight | **M3** |
| Sat | Bug fixes only; final gate; last upload by 20:00; hard stop 23:30 | – |

## 6. Admission rules (fixed before any test)

- **Safety:** `gate.py` PASS, otherwise nothing ships.
- **A module is admitted** when the real-team panel shows **either**:
  - win rate up at least 8 points; **or**
  - tributed gems per round up at least 15 % with deaths per game not up more than 10 %.
- **Panel design:**
  - 160 games: 8 teams × 20 maps, each map once per team.
  - Paired by team and map against the previous live bot's panel on the same teams and maps, measured the same day.
  - Only one bot can be live, so the baseline panel runs just before the upload and the candidate's right after.
- **Per-map floor:** no map more than 3 games (of 8) worse than the previous live bot.
- **Confidence intervals** in every panel table. With 160 games, a true +8 points is detected about 80 % of the time; with 80 games, only 55 % (critic).
- **Local h2h** only filters: below 45 % against the live build means it is not uploaded for a panel.
- **Kill rule:** two panels without gain means the module is dropped or redesigned. No re-tuning loops.

## 7. Compute and tokens

- Daytime: at most 4 game jobs at Idle priority (laptop stays usable). 00:00-07:00: up to 10 (Xans approved).
- At most 3 agents at once, sonnet by default; reports of at most 250 words; data stays in files.

## 8. Decisions by Xans (20 Oct 06:48)

1. Standing approval: upload any candidate that passes `gate.py` and the critic, for real-team testing. Each upload is reported to Xans and logged in VERSIONS.md.
2. Overnight: up to 10 game jobs 00:00-07:00.
3. Upload v4 base today once it passes the gate.
