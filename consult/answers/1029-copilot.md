Copilot (pasted by Xans), answer to consult/1029-v5-direction.md Prompt 2, received 10:58 BST 20 Oct. Condensed by the lead. Copilot's own figures and wording are kept, and its long tables are shortened.

## Recommendation
Build **G: a "safe tempo" release** that combines the narrow, high-confidence parts of C, F, B and A:
- replace the Vendetta-heavy drafts with map-class lookups based on the established core;
- recall and shop on explicit gold and risk thresholds;
- assign one champion to each camp group and follow a fixed opening route;
- bank carriers before they enter contact;
- cast an appropriate ability before committing to champion combat.

Do not attempt any of these before Friday:
- a general fight simulator;
- a dynamic route optimiser;
- a denial system;
- a rewrite of all four contact maps.

If it must pick a single listed option: **B** (economy and routing). "Your preregistered rule was correct: 23 % means the main deficit is structural. Do not overturn it because '85 % ability opening' is visually persuasive." "Economy first" should not mean spending 40 hours on an elaborate optimiser.

**Rules conflict (it flagged this):** the engine fact sheet says damage does not interrupt a tribute. Only a stun, death or an altar lock does. Recall is still interrupted by damage.

## Ranking (incremental vs v4 on a 1,453-1,771 panel; effects not additive)
| Rank | Option | Panel effect | Elo | Confidence |
|---|---|---|---|---|
| 1 | G safe-tempo package | +10 to +18 pp | +80 to +170 | Medium |
| 2 | B economy and routing | +6 to +13 | +50 to +120 | Medium |
| 3 | F gear curve and early recall | +3 to +8 | +25 to +70 | Medium-low |
| 4 | C draft overhaul | +2 to +7 | +15 to +60 | Medium-low |
| 5 | A fight module | +1 to +7 by Friday | +10 to +60 | Low-medium |
| 6 | E four contact-map plans | +1 to +4 overall | +10 to +35 | Medium |
| 7 | D denial | -2 to +3 | -15 to +25 | Low |
| 8 | H none | -3 to 0 | ladder drift | High |

## Key points per option
**G.** Draft by pick count:
- 5 picks: C&I, Stellar, Ren, Varg, Ryth.
- 4 picks: C&I, Stellar, Ren, then Ryth or Varg.
- 3 picks: C&I, Stellar, Ren.

Fixed opening routes for rounds 250-350, with one capturer per group, explicit bank and shop points, and carrier safety.

The loop it describes: assigned camps → earlier gold → earlier gear → safer clearing → bigger banked bags → fewer resets. It cites top-10 figures: 15 groups by round 300 against 10-11, gear 3.8 against 1.7, and 41 gems per tribute against 24-30.
- *Trap:* the champion oscillates between intents. Fix: an intent state with a commitment horizon and explicit pre-emption rules (enum Intent {RouteToCamp, ClearCamp, CaptureCamp, Bank, RecallShop, Fight, Retreat}, with expiry and priority).

**B.**
- *Evidence:*
  - the 23 % breach;
  - tributes rose but gems lost to kills did not improve;
  - the good maps (expanse, islands) show basic micro converts income;
  - extra allies don't speed a capture;
  - respawns give predictable time windows.
- *Scope:* offline shortest paths; 2-4 candidate opening routes per champion and map; simulate them uncontested and choose a non-overlapping set; encode them in the plan file; then a cheap online score:

  S = (E[banked gems] + λ·E[gold]) / (travel + clear + capture + bank) − ρ·contact risk − κ·lateness.
- *Trap:* optimising captured gems instead of tributed gems. Track tributed gems per round, carried-gem-rounds, distance to a safe altar while carrying, gems destroyed and transferred, and idle or duplicated capture rounds. Invalidate stale camp timers.

**F.**
- Recall for productive resets: enough gold for the next planned purchase, no high-value camp nearby, the channel likely to complete, the next route connects from spawn, and gems banked first. Breakpoints of 60, 90 or 150 gold.
- *Trap:* recall loops. Add a cooldown after an interrupt, a safe-channel predicate, reason codes, a diagnostic for recalls per 100 rounds, and "walk out of threat first". Shopping while dead reduces the need to recall.

**C.**
- Usage data is confounded by team quality: Stellar, C&I and Ren need spell timing.
- Ship: stop Vendetta-first, use the core by default, and keep 1-2 tested exceptions. Exclude Heart of Ruin (no system-level evidence) and Prism (it conflicts with safe banking and loses to Ren and Varg).
- *Trap:* draft-policy mismatch. Assert, per champion, its range, first ability, purchase curve, retreat threshold and camp-clear behaviour.

**A.**
- Only the opener timing and carrier protection are safe for Friday.
- Rule: in an irreversible fight, use the best legal ability before a basic attack; stun a visible tribute or recall; target the lowest effective turns-to-kill; broadcast one focus target, with expiry, in shared memory.
- *Trap:* "ability first" becomes "ability always" (casting on camps before contact, dashing into monster range, every champion choosing a different target).

**E.**
- Turning four 0 % maps into 25 % maps is worth at most about 4.8 points overall.
- contest: gold and monolith control decide it; avoid monolith feeding.
- fragment and hooks: territory partition and contact avoidance.
- gold_rush: treat the central monolith as a later phase.
- *Trap:* overfitting 8 games a map.

**D.**
- Net denial = enemy value delayed − our farming forgone − death risk − carrier risk. Use monster aggro only as a movement preference during fights.
- *Trap:* an indefinite blockade that succeeds but loses the game. Needs a hold horizon, a minimum denied value, an escape route and telemetry.

## Cross-domain idea
Vehicle routing with time windows plus receding-horizon control:
- champions are vehicles and camps are jobs;
- respawns are time windows and the 5-round capture is service time;
- altars and spawn are depots, and carried gems are cargo.

Make a short plan, commit briefly, observe, replan. Precompute BFS distances, clear times by class, and beam-searched route candidates of 3-6 groups at draft time. At runtime, re-evaluate only on events, with beam width 4-8 and depth 2-3. A claimed job leaves the other champions' candidate sets, which gives one capturer per group.

## Fact that would change the ranking
The 23 % panel win rate. If the true value were above about 35 %, it would move the cheap parts of A and C ahead of B. The 1-89 top-10 result matters less for ordering.

## Testing
Three jobs:
1. Arena and unit tests of rule correctness and state transitions (one capturer, route advances, recall not restarted, tribute behaviour as the engine does it, claims expire, occupied-spawn delay, and so on).
2. Local macro benchmarks against v4, a passive farmer and perturbation bots (rush, contest the first camp, occupy a planned spawn, altar camp, monolith rush). Local games only *reject* (< 45 % vs v4); 60 % proves nothing.
3. Real switchback: v4 baseline → candidate → v4 baseline, with the same teams and maps.

Against the top 10 use continuous endpoints:
- tributed gem margin;
- tributed gems per round;
- deaths per champion-round;
- groups by round 200 and 300;
- gear at round 200;
- the share of games reaching 250, 500 and 750 gems.

Admit G at ≥ +20 % tributed gems per round with ≥ −15 % deaths, and no catastrophic regression on more than two maps.

Ablations: BASE, C, F, B1 (claims plus fixed openings), B2 (plus respawn-aware continuation), and G.

60-hour split across the three sessions:
1. routing and claims;
2. draft, gear and carrier safety;
3. tests, a minimal opener, the focus byte, fuel and the gate.

Cut order: denial, dynamic routing, a general fight model, map-specific combat, adaptation. Never cut: one capturer, fixed openings, carrier safety, the draft fix, telemetry, rollback, the full-map gate.

Bottom line: "The Friday build should be a camp-tempo and survival system, not a combat AI."
