Read tools/cloud/v5-common.md first and follow it, but use the admission rule below instead of its rule. Your copy is bots/s1. Module: **ECONOMY CONTROLLER**, a multi-robot task allocation with time windows. This is the consultants' unanimous first priority (consult/1029-v5-direction.md; PLAN-v4.md "Draft 4").

## Why
Against the top 10, v4 scores 159 gems a game to their 943. Their teams capture 15 camp groups by round 300, against our 10-11. Their tributes average 41 gems, ours 24-30 (notes/top10-2026-10-20.md).

v4 chooses targets per champion from independent scores. Nothing stops two champions from duplicating work, and nothing stops a champion from waiting on a spawn tile. Arena rule 2 says an occupied spawn tile holds that group's respawn after its timer reaches 0, even when the occupant is our own champion (results/arena-1020.md). Monsters attack the nearest champion (rule 1).

## Model (implement this; deviate only with a measured reason)
Champions i ∈ I (3-5 a side). Tasks j ∈ J are of three kinds:
- **capture(g):** clear group g, then one champion holds a spawn tile for 5 rounds;
- **bank(a):** tribute the carried bag at altar a;
- **reset:** recall and shop.

Every task has a value, a time window [e_j, l_j] and a service time s_ij. Champion i's value rate for task j is

  u_ij = [ G_j(1 − ρ_ij) + λ·O_j − ρ_ij·(C_i + B_i) ] / ( τ_ij + max(0, e_j − t − τ_ij) + s_ij )

where:
- G_j, O_j are the task's gems and gold (camp_group values from the SDK);
- τ_ij is the travel time, from distance tables built once;
- e_j is the earliest round the group is available. Use the respawn round of groups we captured (capture round + respawn), or "now" if the group is seen alive. Expire stale estimates.
- s_ij is the clear time (Σ monster HP ÷ the dps of the champions assigned to clear it) + 5 capture rounds;
- ρ_ij is the death risk on the path and at the task: awake camps within 3/4/5 tiles, plus enemies seen in the last k rounds;
- C_i is the cost of a death: 60 rounds × the team's income per champion-round, plus the 50 gold the killer gains. Income per champion-round = gems banked so far ÷ (champions × round), floored at the map's prior from notes/map-economics.md. "1,000 gems" always means tributed (banked) gems;
- B_i is i's carried bag, lost on death.

The bank task competes on the same scale, u_bank = B_i(1 − ρ) / (τ_altar + ceil(B_i/10)). A tribute is cancelled by a stun, a death or a newly placed lock, NOT by damage (GameRules l.181).

**Allocation:** a sequential single-item auction (Koenig et al. 2006, "The power of sequential single-item auctions for agent coordination", AAAI; with time windows, Nunes and Gini 2015, AAAI, TeSSI). Champions share no vision; they share 8 bytes of memory (writes are visible at once, SDK cambc.h about l.5004, so allies later in the same round see them) and 1,024 read-only draft bytes.

**Byte map.** v4 already writes byte k (ally index 0-4) = target + 1 every turn, and 0 when dead (bot.cpp:534). That is a claim channel that refreshes itself, so stale claims clear by construction. Extend it:
- low 6 bits = task id (group index + 1; the maximum is 46, on islands);
- high 2 bits = ETA bucket: 0 for 10 rounds or fewer, 1 for 25 or fewer, 2 for 50 or fewer, 3 for more.

Bytes 5-6 are written only on the rows map (not in the pool; rows.cpp:105) and by the contest siege (byte 6, kSquadClaimByte; siege is off by P_SIEGE_CONTEST 0). So on the 21-map pool, byte 5 is reserved for S3's focus target, byte 6 for a channel alert (an enemy tributing or recalling, with its altar id), and byte 7 is free.

On its turn, champion k takes the best u_kj among unclaimed tasks, or outbids a claim whose ETA bucket is at least 2 worse than its own, then writes its claim. Turn order is fixed within a team, so champion 0 always bids first. The 2-bucket outbid rule makes the outcome mostly independent of that order; measure duplicate and orphaned tasks to confirm it.

**Commitment (hysteresis):** keep the current task unless u_new > 1.2·u_cur. Pre-empt at once on: death risk above a threshold, an enemy tribute or recall channel that we can stun or lock within reach, or an ally in danger.

**One capturer:** clearing may be joint (elk, monolith), but only the claimant stands on spawn tiles, and everyone leaves a group's tiles as soon as its capture completes.

**Camp fights, tank swap** (WoW raid tank swap mapped onto nearest-champion targeting): the ally with the highest effective HP stands nearest each awake monster, and the others keep strictly farther away. Below 35 % HP, the next tank steps one tile closer and the old tank steps back.

**Paths:** add a cost per tile within range 3/4/5 of awake wolves, elk and monoliths. Use a cost, not an exclusion; excluding elk range 4 cost wins in v4's ablation.

**Fuel:** v4 peaks at 90,697 of 100,000 (186,807 of 200,000 on round 1), so this controller must REPLACE v4's target scoring (resolve_target and farm target choice), not add to it. The draft bot is a separate instance, and only the 1,024 draft-memory bytes reach the match (for example a precomputed group order). So build distance tables in Bot::init, staged over the first rounds; round 1 has only about 13k spare fuel. Evaluate at most 8 candidate tasks per champion a turn, using table lookups.

## Order of work
1. Diagnostics, done before building:
   - v4 unopposed against an idle bot (create bots/idle: drafts, then passes every turn) on all 21 maps: the round at which it reaches 1,000 gems, or its gems at round 2,000;
   - the rounds per game v4 champions stand on our own spawn tiles of a group whose timer is 0;
   - the rounds per game with two allies on one group's tiles.
   Push these at once to this session's branch as results/v5-s1-diag.md.
2. Build in this order, each behind its own switch and ablated: P_S1_NO_SELF_BLOCK, P_S1_ONE_CAPTURER, P_S1_THREAT_COST, then P_S1_AUCTION, then P_S1_TANK_SWAP (S1 owns the tank swap; S3 reuses it). Push a working intermediate after each step.

## Admission (fixed before testing)
1. gate.py PASS.
2. Unopposed farming speed: median round to 1,000 gems over the 21 maps is at least 15 % earlier than v4's, with no map more than 10 % slower.
3. Head-to-head vs v4 at least 45 % (a rejection gate only; self-play misled us before: 68 % locally became 23 % on the real panel).
4. Deaths per game vs v4 not up more than 10 %.
The live top-10 panel, run by the lead, decides the release.
