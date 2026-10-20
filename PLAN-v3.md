# v3 plan v1 (lead, 01:35 BST 19 Oct; replaces draft 0, which Xans sent "back to the drawing board")

> Measured on engine 0.0.3 and the 17-map pool; engine 1.0 changes (docs/rules/ENGINE-1.0.md) that may affect it: the 21-map pool, base-stat rebalance, Prism/Vesper/Marcella changes, Heart of Ruin, cluster HP 120 and new camp rewards; every draft table, duel result and map plan in it needs re-measuring.

**Goal (Xans):** v3, the leaderboard destroyer: adaptable, with counter-drafting at a high level and strong strategies, grounded in the data.
**Deadline:** Sprint 2 locks **Sat 24 Oct 23:59 BST**. Live now: v2 (platform "v1").

## What the evidence says (01:25)

| Fact | Source |
|---|---|
| v2 won **10 of 50** ranked games (1,500 → 1,404) against teams rated 1,414–1,500, i.e. mid-ladder | `data/ladder/xanarchy-v2-games-0125.psv` |
| When we lose, the opponent reaches 1,000 while we hold ~550 on average. Zero-gem games: stampede ×2, contest ×2. Blowouts on 3-champion maps (collision 168/292/372, fragment 168, packs 180) and pilgrimage (160) | same |
| Draft is not the main gap: Micron (Vendetta, C&I, Ren, Varg, Ryth) is one pick from ours and won 5–0 | same |
| Even unopposed we are slow: v2's median win round vs the top-50 median rounds to 1,000: stampede 1,752/686, swirl 1,249/600, pilgrimage 1,278/844, rushbait 1,006/660, ambush 899/594, monolith 584/447 | `results/v2-panel.json`, `intel/meta-2026-10-18.md` §3a |
| Gem economics (GameRules monster table): gem cluster 24 gems / 500 HP / 0 strength (48 gems per 1,000 HP); wolf 22/1,350 (16); elk 36/2,400 (15); monolith 200/20,000 (10) but strength 200 at range 3, monsters never chase; gold cluster 0 gems | GameRules l.179-206 |
| The local panel (v4.14-era clones) sits at our level, so panel wins are not a yardstick for the ladder | 151.7/204 panel vs 10/50 ladder |
| "Play unranked" lets us play our Live bot against any of 1,052 teams on chosen maps with no rating change | platform `/api/v1/match-groups/options` |
| All three consultants (DeepSeek, ChatGPT, Copilot) rank conversion/objective play above prediction; two independently propose camp-spawn blocking | `consult/2340-v3-program.md` |
| A former teammate's six changes (draft, Tavi field shards, range-4 monolith poke, contested-rule fix, locks everywhere, tribute interrupts + carrier focus) climbed the ladder **on a different code base**; each is a hypothesis for ours | Xans 01:40, 01:50 |

## Yardsticks (in order of trust)

1. **Real teams:** unranked matches of the Live bot against a fixed ladder set (rated ~1,450-1,800) on all 17 maps; ranked results after each upload. Needs Xans's go-ahead and an upload per candidate.
2. **Pace:** median rounds to 1,000 per map against the top-50 median; the gem pipeline (camp gems captured → carried → tributed, gems lost on death) per map.
3. **Head-to-head vs v2**, both sides, 3 salts.
4. Panel and held-out (regression check only).

**Resume point: `NEXT.md` (22:20 Mon 19 Oct).** Real-team baseline: v2 27-109 vs 8 teams (`notes/unranked-v2-baseline.md`). Admitted so far: w9 siege (contest), w7 drafts (eight, stampede), w8 Tavi Focus (monolith). Rejected: w8 damage-only builds, w10 retreat fix, w11 interrupts, w12 engagement rules, w13 rate targeting. Current maps for the six old layouts found: `maps-1016/`.

## Root causes found in v2 (17:50 Mon 19 Oct; laptop slept 02:20-17:05)

| # | Cause | Evidence | Owner |
|---|---|---|---|
| R1 | **Target value ignores gems**: score = distance + wait − 0.05 × gems (20 gems = 1 tile); gold-only groups are ordinary targets | `squads.cpp:131`; stampede 30–42 % of champion-rounds on gold clusters (w10) | w13 (gems-per-round scoring) |
| R2 | **Monoliths are never targeted**, so contest (2 monoliths + 20 gold clusters) has no gem source | `squads.cpp:117`; ladder contest 0, 0 | w9 (siege), w13 hook |
| R3 | **Health-heavy builds**: ours alternate Health with the main stat; Micron bought no Health and hits 1.3–1.5× harder; HGT's Tavi stacks Focus (1,512 a cast at range 4, 23 kills); our Tavi buys Strength | decoded ladder games (maxhp, damage per hit/cast) | w8 (damage-only builds, Tavi Focus) |
| R4 | **Lone even fights**: 60/79 deaths with no ally within 4; 74/79 at ≥ 50 % health; 1v1s always taken | `enemies.cpp:78`; w12 | w12 (engagement rules) |
| R5 | **Retreat formula** counts every elk within 4 as hitting us: champions flee the stampede group at 90 % health | `bot.cpp:388`, `util.cpp:28-33`; w10 | w10 (all-map retreat fix) |
| R6 | Drafts: secondary (Micron's draft ≈ ours) | ladder | w7 (data drafts + counter-draft) |

## Workstreams (stop 21:00 Mon; v3a candidate assembled 21:00 with `scripts/integrate.sh`)

| # | Stream | Owner | Output | Status |
|---|---|---|---|---|
| P1 | Retreat fix (all maps) + stampede V3/V5 | w10 | `results/w10-contested.md` | running |
| P2 | Monolith siege from range 4 | w9 | `results/w9-monolith.md` | running |
| P3 | Damage-only builds; Tavi Focus stacking in the field | w8 | `results/w8-builds.md` | running |
| P4 | Gems-per-round target scoring (harvest planner core) | w13 | `results/w13-scoring.md` | running |
| F1 | Engagement: no lone even fights, regroup | w12 | `results/w12-engage.md` | running |
| C1 | Data drafts + counter-draft engine | w7 | `results/w7-draft.md` | running |
| D1 | Locks (steps 1+2 marginal: 9/17 maps, +31 gems vs v2); interrupt rejected | w11 | `results/w11-denial.md` | stopped; build kept |
| A1 | Our ~103 ladder games + top-10 replays decoded | analyst (data) | `notes/v3-top10-behaviour.md` | partial 20:00 |
| D2 | Camp-spawn blocking (GameRules l.193) | after v3a | card | queued |
| I1 | Current maps: `cambc maps pull` fails (HTTP 303 on map download) | lead | – | open |

## Integration

21:00: `bash scripts/integrate.sh v3a <admitted builders>` → 3-salt panel + head-to-head vs v2 + starter safety → ask Xans to upload before he sleeps, so the ladder (server-side) measures v3a overnight against real teams. Then iterate daily: Tue–Fri new candidates from the ladder data; freeze Sat 24 Oct 18:00.

## Limits

- Agents: Xans lifted the three-agent cap (00:20, "use that to your hearts content"). Game jobs stay ≤ 13 (14 cores).
- Uploads: Xans approves each upload unless he sets a standing rule.
