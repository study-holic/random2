# Critic: v5 S2 quick wins, build A (main bots/s2) vs build B (origin/claude/nice-cray-9ju6gn)

Read-only review, 20 Oct. No games run. Sources: tools/cloud/v5-s2-quickwins.md, results/v5-s2.md, results/s2*-vs-v4.json (main), the branch's s2.h/s2.cpp/bot.cpp and its 10 h2h JSONs, docs/rules/GameRules.md.

## Key findings first

1. A and B are two independent implementations of the same brief, not one build with variants. A's s2.cpp has `s2_lock`, `s2_tribute_ok`, `s2_shop_recall` and a role emblem (Durability-Health). B's has `s2_lock_turn`, `s2_tribute_gate` (capped by P_S2_GATE_MAX 3), `s2_shop_detour`, no role emblem, plus LOCK_FROM 200 and a fuel guard. Both use `bots/s2`, so merging or swapping will conflict.
2. No B h2h file corresponds to B's committed defaults (Q1). B's committed build has never played v4.
3. A fails its own pre-registered gear line, and the gear gap is income, so no switch in A can close it (Q2).

## Q1. Which B results file matches the committed defaults?

Committed defaults (s2.h at b1ade4b): DRAFT_CORE 1, CORE_VARG 1 (C&I, Stellar, Varg, Ryth, Ren), DRAFT_EXC 0 (core everywhere), LOCK_FROM 200, ALTAR_LOCK, TRIBUTE_GATE and SPEND_NOW all 1, BANK_VIA 0, FUEL_STOP 35000, FUEL_FLOOR 45000, GATE_MAX 3.

Timing check (JSON `tags.date`): t1 11:07, nolock 11:17, nogate 11:22, nospend 11:27, a 11:33, b 11:38, c 11:43, d 11:49, e 11:54, f 12:00. The fuel guard, fuel stop, LOCK_FROM and CORE_VARG arrived in the 12:37 commit. The gate FAILed at 12:27 (`fuelExhausted ... lanes/B`, max_fuel 100,000) and PASSed at 12:36 (max_fuel 95,804, 42/42, mapid checked).

No h2h file post-dates the commit, and no params_override in the repo records what a-f varied. The commit message ("candidate defaults") says the defaults were chosen from variants; the likely sources are d and f, which differ in only 3 of 126 game outcomes. That is an inference, not verified.

Evidence that d and f lacked the fuel work: both have five games at fuel_max 100,000 with fuel_late > 0, and all five are losses (lanes A s0, lanes B s0/s1/s2, fracture B s2). So the 77 and 78 wins include fuel-starved lanes and fracture. The guard may fix these (upside) or change behaviour in unmeasured ways.

Deaths and margin: f 78/126, 16.8 deaths, +148 margin. d 77, 16.8, +150. A (main) 82, 17.4, +84. B's margin is larger, its win count is lower, and the 126-game s.d. is about 4.5 wins, so 78 vs 82 is noise.

B's gate max fuel: 95,804 (results/gate-s2-20261020-123630Z-linux.json). B's gear at r200: not available. The branch has no s2-*-metrics.jsonl, and metrics_s2.py was run on no committed B run. B also has no role emblem, so its gear would probably look like the nospend ablation (2.00 on A's code), not 2.32.

## Q2. The 3.0 gear line

Plain answer: A failed a pre-registered line (2.32 vs 3.0, equal to v4's 2.32 in the same games). It was not met and must not be rationalised. The line does not say A is bad, but it does say the spending module did not do what it was built to do: the SPEND_NOW mechanism "raise gear to the top-10 level" did not happen.

Soundness of 3.0:
- The line was calibrated from live data (top 10 3.8, v4 1.7) and tested in self-play where v4 already has 2.32 (reported 2.33 in several runs). It measures the wrong baseline. Against that baseline +0.68 needs roughly +30% gold spent, and A's own trace says unspent gold at r200 is only 24 to 36.
- The report shows the cause is income (2 of 6 champions earn any gold before round 220), not holding. A spending module cannot raise gear past income. The line asked a spending switch to fix an income problem, so it is badly specified.
- It is still the pre-registered rule, and it was read after the fact. Changing it now is exactly the move that turns a gate into a rationalisation.

Should failure block the upload? Yes, by the rules as written. A is not an admitted candidate. The lead may overrule only by logging an explicit waiver in PLAN.md ("line 3b waived, reason: unreachable by the module; live panel decides"). I would not do it on the strength of "82/126", because the h2h is a rejection gate only. The real risk is not the gear number but that the other evidence for A is thin (below).

## Q3. Why does turning spending off cost 21 wins with gear unchanged?

First a correction. Gear is not unchanged: the nospend ablation shows 2.00 vs 2.32 (A) and v4 at 2.32. Spending lifts A back to v4's level; it does not exceed it. The 21 wins are therefore not "gear".

Mechanisms, with the weakest link marked:
1. Draft/buy mismatch repair. The new core (Ryth, Ren, Varg, C&I) is melee-heavy, and v4's buy list gives Strength (the report names this gap). Spending-off keeps the new draft but also drops the Durability-Health first emblem and shop trips. The draft-only build scores 54/126, spend-off 61, full 82. Part of the "spending" gain is correcting the draft change, not a spending effect in its own right. This is confounded and cannot be separated from these runs.
2. Free healing. Standing on the spawn heals fully (GameRules l.332). A shop trip is also a heal, which would lower deaths (21.2 to 17.4). This is plausible but is a correlation: the nolock ablation has the lowest deaths (16.7) and loses 14 wins, so deaths are not the whole story.
3. Durability-Health emblem: -10% damage taken for the melee line (report's stated value, not verified against rules in this review).
4. Trips pull champions away from fights and the bag (a loss), the opposite sign. The net being positive in self-play against a v4 that does not do it says little about real teams that kill a recalling champion (recall is cancelled by damage, l.332).

Unknown: the same 21 wins would shrink against a v4-draft baseline. No build has "v4 draft + spending". That is the missing control.

## Q4. Fuel

A at 97.7k (gate) has 2.3k margin below the 100k kill, and the guideline is 80k. In A's h2h no game reached 100k (max 96,338; 11 games above 80k, fuel_late 0). Its ablations did hit 100k (nolock: lanes B in all 3 salts, all losses), so lanes fights at 100k exist in a neighbour build. The cause was never traced (report: "source not traced"). Real top-10 teams bring more units and more fights than self-play, so the peak is more likely to rise than fall. A turn that hits 100k presumably ends or faults the unit's turn, which is a silent unit loss at the worst time. Not safe to ship on a 2.3k margin untraced. Gate pass at 97.7k is a literal pass, but it breaks CLAUDE.md's 80k guideline ("stay under 80,000").

B's guard, from the commit diff:
- `P_S2_FUEL_FLOOR` 45,000: `s2_lock_turn` and `s2_tribute_gate` return "do nothing / gate off" when `fuel_remaining() < 45000`. So the S2 logic yields first, and the tribute proceeds ungated.
- `P_S2_FUEL_STOP` 35,000: the turn ends (return) before target resolution, banking and farming. A champion that reaches 35k left does nothing that turn: no move, no attack.
- Position queries cut: one `e.position()` per enemy instead of per altar (12k fuel saved in lanes fights).
- The existing `fuel_remaining() < 80000` init yield remains.
So under shortage B degrades to idling for that turn instead of suspending mid-action. Risks: an idling champion in a fight takes damage; the stop threshold is a guess (the 12:27 FAIL was the evidence); and the 95.8k gate max shows that fuel is still being spent above 60k before the stop triggers, where it sits relative to 100k is unknown. The stop fires after buying, locking and the interrupt checks, so most fuel is already spent at that point. Whether it fires in real games was not measured. None of this is a fix for the cause, it is a symptom cap.

## Q5. Lock and tribute-gate stall paths against real teams

Facts verified (GameRules): lock 10g, carry one (l.193); placing cancels channels there (l.195); three basic attacks clear it (l.195); a tribute against a locked altar is converted into an attack on the lock (l.199); damage does not interrupt a tribute but stun, death or a lock does (l.181, 229).

A (main):
- Gate returns false from `bank()` with no cap and no memory. Conditions: k >= 2 (bag > 10) and a ready CC enemy within 5; or k >= 2, any enemy lock seen this game (`s2_foe_locks` latches true forever once `lock_until` is set), and an enemy within 3 of the altar; or damage >= HP with no ally adjacent. Against top-10 teams that camp altars with Ren/Cassi nearby, a champion with 11+ gems can be refused every turn, return to farming (adding gems, so k only grows), and finally die with the bag. That is the stall path. There is no GATE_MAX equivalent. In self-play v4 places no locks and rarely sits on altars, so the h2h cannot reveal it. The ablation says the gate is worth +4 (noise), so the cost of removing it is low.
- Enemy-lock handling exists in v4 (3 attacks after 3 waits, `lock_wait`) and in A the same stall fix applies, but if the enemy re-locks, there is no cycle break.
- Locking: A buys one lock from the first shop visit (10 gold in the early gear race, 0.2 gear points per the report). Placing uses the action-free consumable, so no stall. Chasing a channel pulls a champion into the 2-ring if HP >= 50%, unguarded by bag size or enemy count (B guards bag < 20 gems; A does not).

B:
- Gate capped at 3 consecutive gated turns, then the tribute starts anyway, so a stall is bounded to 3 turns. The counter resets when the gate does not fire. Unknown whether `s2_gate_turns` resets on distinct episodes, minor.
- Lock from round 200 only: early gold goes to shards, which avoids A's early cost and matches the top-10 lock timing (unknown; not verified in this review).
- The tribute is not gated when fuel < 45k.
- A tribute that is attacked-lock (our carrier facing a lock placed by them): same v4 code path as A.

Both: the lock carrier "one at a time" and `s2_has_lock` in B is set true on buy and false on use or exception; if `can_buy` fails because already carrying, the flag is only set on success, fine. A reads the live consumable count each turn (fuel cost).

## Lenses

**Creativity.** Mostly safe: port the top-10 draft, buy locks, buy shards sooner. Bolder: lock the altar nearest our own bank while an enemy is not channelling (defensive lock) is not tested, and the shop trip could be timed to the heal-and-bank cycle (deliver bag, shop, return). Safer: ship only the draft change plus nothing else, as the control. The brief's "no denial order" reasoning for drafts is sound, but the A-B-A live test needs a draft-only arm to split effects.

**Restriction.** The framing excludes: (a) a v4-draft + spending arm, (b) lock timing as an experiment axis in A (B has it, A does not), (c) "do nothing" for the gate, which the ablation says is free to drop, (d) a gear target set per role rather than across the team.

**Causation.** Mechanism claimed: top-10 teams draft core, lock, spend, therefore they win. Steps: copy behaviours, assume the behaviours cause wins. The link resting on correlation is the whole thing: top-10 lock win rate is 16/31 (52%) per the brief, Nyx, the strongest team, buys none, so locks are not shown to cause wins. Self-play gain for locks (+14 in A) arises because v4 has no lock-clear skill for the first lock and locks cost it tributes; that exploits v4 specifically and may not exist against teams that clear locks.

**Limitations.** 126 games = 21 maps x 2 sides x 3 salts, deterministic per salt, so salts vary little: effective n is close to 21 map-sides x something small; s.d. about 4.5 wins; side effect is large (B's f: side A 44 wins, side B 34). The per-map draft exceptions were selected on the games they were scored on (the report says so). Panel is one opponent, v4 itself, which does not lock, rarely holds altars and is weaker than the live top 10 (68% self-play became 23% live). Fuel: see Q4. Crashes: faults 0 in all files; fuel exhaustion is the real failure mode. The gate is against starter-cpp only.

**Feasibility.** One live A-B-A panel is cheap: a 5-game series every ~10 minutes (6.1 series an hour per CLAUDE.md), so ~12 series (60 games, ~2 h) per arm is the measurable budget; only large effects (>20 points) are detectable. PLAN.md time left was not re-checked here. Because the lock is 10 gold per game, a 2-arm panel (A, B) plus v4 is about 6 hours at 6 series an hour for 12 series per arm. If the lead wants fewer arms, cut to one.

**Falsification.** A is dropped if the live panel win rate is not above v4's by more than noise, or if the gate or fuel causes a lost unit/bag in replays (a champion refused tribute for 10+ turns, or a turn at 100k). The lock claim is dropped if the lock arm does not reduce tributes against locking teams; the spend claim if gear at r200 stays at v4's level live.

## Fact checks

1. "Damage alone does not cancel a tribute; stun, death or a lock do" (brief): confirmed, GameRules l.181 and l.229.
2. "Top-10 place 6.4 locks a game" (brief): not re-derived. I did not open notes/top10-2026-10-20.md, so only the brief's own figure is cited. The 16/31 vs 52% cited there is consistent arithmetic (16/31 = 51.6%).
3. Report's "gear unchanged" claim for spending-off: wrong. The table has 2.00 vs 2.32.
4. Report's max fuel claim: the gate's 97,728 is the gate; A's h2h max is 96,338, B's d/f hit 100,000 in 5 games each. Confirmed from the JSONs.
5. h2h numbers recomputed from JSONs: A 82/126 and 17.4 deaths; f 78, 16.8; d 77, 16.8. They match the brief.

## Missing

- A control: v4 draft + A's spending and locks (separates draft from spend), and a draft-only arm live.
- A trace of the 97.7k peak (A) and of B's fuel stop firing rate; neither exists.
- A tribute-denied-turns metric per game in both builds; neither reports it.
- B's h2h on the committed defaults, B's locks placed, and B's gear at r200.
- A replay watch of a loss (not done in the cloud; CLAUDE.md requires /watch).
- A decision rule for what the live panel result must be to choose between A and B.
- Side asymmetry (A-side 44 vs B-side 34 for f) is unexplained.

## Verdict

- Build A as it stands: **drop** as an admitted candidate (it fails its pre-registered gear line). A named waiver is the only way to ship it, and I do not recommend it unless the two changes below are made.
- Build B as committed: **not uploadable yet**, because the committed defaults have no h2h result and no gear metric.
- **Upload with named changes** is the only upload I would accept: take B's committed structure (GATE_MAX cap, LOCK_FROM 200, fuel guard and stop) and run one 126-game h2h on it plus metrics_s2.py for locks placed and gear at r200. If it clears gate PASS, >= 45%, locks > 0, deaths within 10%, and gear (failing 3.0 there too is likely) is recorded openly as FAIL with a waiver, upload B. If A is preferred instead, change it first: add a GATE_MAX cap or switch the tribute gate off (ablation cost -4, noise), and trace and cut the 97.7k fuel peak below 90k. Either way the live panel is the decision and self-play is only a rejection gate.

Final: **upload neither as-is; upload B with named changes (h2h on committed defaults, gear recorded and waived in PLAN.md)**.
