# Critic review: v5 cloud briefs (20 Oct, ~11:20 BST)

Scope: tools/cloud/v5-common.md, v5-s1-economy.md, v5-s2-quickwins.md, v5-s3-diag-contest.md. No games run. Checked against FACTS.md, GameRules.md, sdk/cpp/cambc.h, results/arena-1020.md, data/maps21.json, notes/top10-2026-10-20.md, bots/v4 source, scripts/mapgate.py.

## Fact checks (reported results)
| Claim | Result |
|---|---|
| Tribute not cancelled by damage; stun, death, new lock cancel (GameRules l.181/229) | TRUE |
| Lock 10 g, within 2 tiles, 3 attacks remove, cancels channels, owner's team may still tribute | TRUE (l.191-199) |
| Occupied spawn tile holds respawn at timer 0, own occupant included (arena rule 2) | TRUE (arena-1020 l.113, FACTS l.25) |
| Contest monolith: 1 spawn tile, 200 gems, 300 gold, respawn 150; other groups 0 gems | TRUE (maps21 grp[0] info 200/300/150/1; others 0/80 or 0/40, respawn 50). 20-turn channel = ceil(200/10) correct |
| Shared memory 8 bytes, draft memory 1,024 read-only in match | TRUE. SDK l.5004: "reads and writes always reach the host", so a write is visible to the next champion at once |
| Top 10: Vendetta 3 %, Stellar 96 %, 6.4 locks, gear 3.8 vs 1.7, tribute 41.5 vs 24-30 | TRUE (top10 note l.20, 32, 34, 45) |
| Draft picks "simultaneous, duplicates across teams allowed, none in own team" | TRUE (FACTS l.48) |
| Round-1 fuel 186,807 of 200,000; peak 90,697 | TRUE (VERSIONS l.59, v4-cloud l.6) |

## (1) Factual errors and omissions
1. **S1 "distance tables built at the draft turn (1,000,000 fuel)" cannot work.** The draft bot is a separate instance; it passes only the 1,024 draft bytes. A 50x50 table per group (7 to 23 groups) does not fit. Tables must be built in each champion's Bot::init, where round 1 has about 13k fuel spare (186.8k of 200k), so they must be staged over rounds. Edit this line.
2. **S1 ignores that v4 already uses the 8 shared bytes.** bot.cpp:534 writes target+1 to byte ally_index (0-4) every turn; rows.cpp:105 writes pack stamps to bytes 5+k; siege.cpp uses byte 6 (kSquadClaimByte); micro.cpp reads bytes 0-4 for cohesion. "Byte k holds champion k's claim" collides with all of these. State which v4 users are removed or remapped.
3. **S1 claims "expire otherwise" but the byte has no timestamp** (6-bit task + 2-bit ETA). A stale claim from a champion out of fuel, recalled to shop, or whose task vanished never clears. Needs a round-parity or age field, and 8 bytes leave room for 5 claims plus 3 spare bytes.
4. S1 says the 2-bit ETA bucket decides outbidding by "at least 2 worse": buckets need defined widths in rounds, or this is not buildable.
5. v5-common still says read PLAN-v4.md "Draft 3" only; Draft 4 supersedes it. Its 10:10 and 10:45 "Update" paragraphs say to point work at four contact maps and fights, which Draft 4 withdrew; S1 and S3 contradict this. Edit common to point to Draft 4 and mark the 10:10 override withdrawn.
6. S2 omits the evidence caveat: lockers beat non-lockers 16 of 31 (52 %, n small); Nyx, strongest 1.0.0 team, buys none (top10 note l.72-77). S2 (c) says "enemy's altar", but altars are shared and locks are team-owned; reword as "an altar the enemy is tributing at".
7. S2 draft: Stellar presence excess is 0.0 pt, Ren +1.6; usage share is correlated with team skill, not shown to cause wins. Hence the per-map exception rule (2 of 6) is too weak: 6 deterministic games vs one opponent. Use the 126-game head-to-head plus the live panel.

## (2) Feasibility: S1
Budget: 126 games at about 1 s CPU-equivalent a game, 3 jobs, is about 45 s wall per head-to-head. The compute is cheap; the risk is build time. Five switches, each needing an ablation run (about 5 x 1 minute) and a trace, fits. The auction, hysteresis, tank swap, threat-cost paths and replacement of resolve_target is a large rewrite of the core of a 90k-fuel bot in under 30 hours of cloud time; likely only P_ONE_CAPTURER, P_NO_SELF_BLOCK and P_THREAT_COST land. Fuel: round-1 headroom is 13k and mid-game 9k; the replacement must delete the old scorer, and a probe of shared-memory host-call cost (not stated in the SDK) is required.
**Riskiest assumption:** that sequential claims through shared memory create a coherent allocation. Writes are immediate (SDK), but within a team the order is fixed by priority then draft order, so champion 0 always bids first with no knowledge of the others' positions: a sequential auction in fixed order is biased, and v4's existing end-of-turn target+1 scheme already does a weak version. Also champions see only 8 tiles, so camp-alive status for unseen groups comes only from timers. Probe design: write at turn start, read at the next ally's turn start, same round; log every round.
Also: C_i "60 rounds x income per champion-round" needs a stated income estimate or it is untestable.

## (3) Measurability of admission rules
- mapgate JSON gives wins, deaths, gem margin, faults, fuel. It does NOT give: rounds to 1,000 gems (S1), locks placed, gear at round 200 (S2), share of fights opened with an ability (S3), gem funnel. Each needs a decoder pass (tools/top10/decode.mjs) over replays; none of the briefs names a script or runtime. Edit: require a single shared script, committed first, e.g. tools/cloud/metrics.mjs, that S1, S2 and S3 reuse.
- S1 rule 2: "1,000 gems" is ambiguous (banked vs acquired). Contest holds 200 gems per 150 rounds, so 1,000 banked takes about 750 rounds and may never be reached on other low-gem maps. Define it as banked gems and use gems at round 1,000 where 1,000 is not reached. Salt variation: 21 maps x 3 salts, medians over deterministic runs; "no map more than 10 % slower" on 21 single numbers will trip on noise. Name the idle bot as the opponent file and check that a passing idle opponent still lets the game run to its end.
- Fixed before testing: S1, S2 yes; S3 parts 2 and 3 use common's rule, which has no mechanism metric specified for the opener beyond "measure". S3 contest rule ("more than 0 gems vs v3b-e1 and v4") is trivially satisfiable by one kill; add a floor, e.g. at least 100 gems a game over 6 games.
- The 45 % head-to-head is vs v4 (self-play) and was stated as a rejection gate only; correct. Locks and the draft change are self-play-invisible, so rule 4 (deaths not up 10 %) is the only informative gate for them.

## (4) Overlap and merge conflicts
- S1 and S3 both implement the tank swap (S3 contest b "kills it with a tank swap"). Assign it to S1 only; S3 calls the S1 switch at integration.
- S1 replaces resolve_target and the farm choice; S2's tribute gate and lock detour and S3's opener also edit turn flow in bot.cpp/micro.cpp. Expect conflicts. Rule: each session adds code in a new file (econ.cpp, altar.cpp, opener.cpp) with one hook call line in bot.cpp, and lists the line.
- S2 (b) buys locks and shards on spawn visits and S1 decides when to recall; "reset" task in S1's model competes with S2's buying. Specify that S2 owns buy_items only.
- Shared memory bytes: S1 claims, v4's rows/squad, and any S3 contest escort coordination all want the 8 bytes. Allocate: bytes 0-4 S1 claims, 5 age/parity, 6-7 left for the v4 relay stamps (or drop them).
- All three copy bots/v4 and edit the same switch header; require distinct macro prefixes (P_ECON_, P_ALT_, P_OPEN_).

## (5) Missing, would waste the session
- Where data lives: S3 names data/real/v4-panel/v4top-* (18 groups of 5 inner zips) and the decoder, but not the unzip-two-levels step or decoder runtime per replay. S1's idle-bot diagnostic needs bots/idle written first; neither bots/idle nor results/ output paths exist in the repo; bots/s1 etc. are created by cp. Add bots/idle source to the brief (draft: first pick; play_turn: nothing).
- No stated priority order inside S1 if time runs out; recommend P_NO_SELF_BLOCK and P_ONE_CAPTURER first (cheap, directly arena-confirmed), the auction last.
- A rollback: the S1 draft diagnostics in step 1 ("push at once") are good; but add that S1 pushes a candidate after each switch passes, as S2 does, so partial work is releasable.
- Creativity: too safe on S2, too speculative on S1. A safer S1: replace the auction with a static assignment of champion to group by Voronoi distance at round 1, re-solved every 20 rounds, with the claim bytes only for tie-breaks. A bolder S3: deny contest by standing on the single spawn tile when the monolith respawns (per arena rule 2 an occupied tile holds respawn at timer 0; the enemy cannot capture, nor we, but we choose when to free it). Check that this exploit is not worth more than the monolith itself.
- Falsification for S1: if the idle-bot test shows v4 already reaches 1,000 gems with under 5 % self-blocked rounds, the self-block premise is dead; if the deaths split in S3 is above 80 % champion kills, S1's value rate (value up to 8 gems per avoided death) cannot close the 159 vs 943 gap alone.
- Causation weak link: "top-10 teams capture 15 groups by round 300" is correlation with their lower deaths (6 vs 23); more captures may be caused by surviving, not by allocation.

## Verdicts
- **v5-common: GO WITH NAMED EDITS.** Point to Draft 4, remove the withdrawn 10:10 and 10:45 fight-priority text, add the shared metrics script, and name distinct switch prefixes.
- **S1: GO WITH NAMED EDITS.** (a) Delete the draft-turn table sentence; stage tables in init. (b) Add a byte map reconciling v4's existing bytes 0-6 with the claims, plus an age field. (c) Define ETA buckets, income estimate, and "1,000 gems" as banked. (d) Order the work: diagnostics, P_NO_SELF_BLOCK, P_ONE_CAPTURER, P_THREAT_COST, then auction. (e) Write bots/idle. (f) Make admission 2 a median-only test with a noise band.
- **S2: GO WITH NAMED EDITS.** Add the lock-evidence caveat; reword "enemy's altar"; add the metrics script for locks and gear; state that the draft comparison is on the 126-game head-to-head and the live panel, not the 2-of-6 rule alone; add the Tavi buy rule (Tavi's shop-anywhere is a draft cost, not a core champion).
- **S3: GO WITH NAMED EDITS.** Give the funnel and time-use decoder commands; change contest admission to a gem floor; drop the tank swap build (S1 owns it); keep diagnostics first and push by the first hour.
