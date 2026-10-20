# Rejected, shelved and untested ideas: test quality and retests

Scope: /home/claude/cambc-2026 at commit 4d3eda83. Every experiment in experiments/log.md (EXP-001 to EXP-092) ran on 13 or 14 October, on engine v0_0_1 and the pre-11:37 16 October layouts. No experiment in the repository postdates the engine patch. EXP-093 onward (v4.15 to v4.18b) exist only in the newer local copy and are absent here.

Old map numbers: 30 elkring, 31 packs, 40 interlock, 41 collision, 50 rows, 50b monolith, 71 corners, 80 expanse, 90 lanes. Collision, corners, interlock, lanes, packs and rushbait changed layout on 16 October.

Selection set: 72 cells (9 maps, 2 sides, opponents v2, starter-cpp, locker, tung-clone). Held-out set: 54 cells (opponents v4-8, v3d, v4-3a). All seven opponents are own-lineage or passive (0.4 to 0.9 of our deaths per game). The win-count rule (EXP-017 to EXP-028) counted one deterministic game per cell, and D-048 later recorded that such wins flip like noise.

Panel P for retests: foundry-v16 (top100-intel/data/repos, level with v4.14 per meta/SALVAGE.md) plus hunter, raider2, centre and locker (Table 2). Every retest uses current map files, cambc 2.2.5, both sides, and paired gems per 100 rounds plus deaths.

Grades: A means the test supports the rejection on the 14 October engine, and a single recheck on current maps suffices. B means the effect is plausible but the sample or the opponents limit the verdict. C means the test was confounded, inactive or never run.

## Table 1: ideas

| Idea | Experiment and source | Test (opponents, maps, games, engine date) | Result | Grade | Retest (maps) |
|---|---|---|---|---|---|
| Willow heal composition (C&I, Prism, Stellar, Tavi, Willow; or C&I, Varg, Vendetta, Willow) | none; idea at strategy/ideas-2026-10-14.md:121, strategy/top2-micro-2026-10-14.md:50, docs/opponents/mite.md:34 | No local test exists in experiments/log.md, strategy/ or results/. No bot casts Willow's heal: Willow appears in code only as the last fallback in bots/tung-clone/src/draft.cpp:12. The parent round-robin (scratchpad rr_test, sets 0 to 2 in deliver/draft.cpp) contains no Willow. | Ladder: 69 per cent (n 55, 8 teams) and 69 per cent (n 32) on current layouts (scratchpad meta/current_layout_meta.txt:28, :76); rows 69 and 79 per cent (top100-intel/dossiers/TOP10-BRIEF.md:43, :71). Willow z -3.6 on ambush and -3.4 on corners (meta/META-current-layouts.md:59). | C | Implement heal on cooldown 8 onto the most-hurt melee within range 4 (mite.md:54), then draft-only A/B vs P on rows, elkring, monolith, lanes_v2, interlock_v2; keep corners_v2 and ambush as negative controls |
| Willow and Varg sustain duo that never recalls | strategy/strategic-audit-2-2026-10-14.md:47, :90 | Never run ("no prior entry") | Arithmetic only: heal 37 per round plus shield 31 per round against a pack's 60 per round | C | Probe heal and shield numbers on v0_0_2 first, then a two-body duo on packs_v2 and rows vs P |
| Value-per-cycle group scoring (S-03) | experiments/log.md:625; strategy/candidates.md:72 | vs starter, 4 maps, 13 Oct, before the deadlock fixes of EXP-004 to EXP-008 | 59.6 vs v1 97.6 g/100r; log flags "retest after EXP-008", never done | C | v5-base knob, selection plus held-out, then P on rows, expanse, elkring, lanes_v2 |
| Three allies on an Elk, two elsewhere | experiments/log.md:626 | vs starter, 4 maps, deadlock era | 38.7 g/100r | C | One knob on elkring, expanse and monolith vs P |
| Designated capturer, one-capturer rule (E21) | experiments/log.md:627; strategy/DECISIONS.md:22 | vs starter, 4 maps, deadlock era | 37.6 g/100r; D-016 demoted the rule without a clean test | C | Kill-shared, capture-single knob on elkring, packs_v2, rows vs P; verify against Emperor Tung replays first |
| Tank rotation at 50 per cent health (E22) | EXP-043, experiments/log.md:266 | 72 selection cells, 14 Oct | +0.77 g/100r, failed on deaths +0.31; opponents rarely damage tanks | C | Elk and pack maps (elkring, rows, packs_v2) vs hunter and foundry-v16, where tanks take champion damage |
| Focus rules: carried gems, nearest (focus-gems, focus-near) | EXP-038, EXP-039, experiments/log.md:296, :290 | 72 selection cells; 59 and 52 cells identical | -0.14 and -0.40 g/100r | C | Focus fire matters in champion fights: test vs hunter, raider2, foundry-v16 on corners_v2, interlock_v2, collision_v2 |
| Engage minimum health 50 per cent, chase 1 tile, save CC | EXP-042, EXP-040, EXP-044, experiments/log.md:272, :284, :260 | 72 selection cells vs passive opponents | -0.99, -0.36, -1.07 g/100r, CI spans zero | C | Same fight panel and maps as the focus row |
| flee3, ret30, ret35, health-first tanks (selection PASS, never held out) | EXP-036, EXP-048, EXP-049, EXP-068, experiments/log.md:308, :236, :230, :130 | 72 selection cells only | +0.64, +0.57, +0.88, +2.05; ret35 deaths +0.29 | C | Held-out plus P on corners_v2, collision_v2, interlock_v2, packs_v2 (the maps with the most deaths) |
| Threat layer, regroup mode alone | EXP-072, experiments/log.md:115 | 6 games vs hunter plus starter cells, 14 Oct | Regroup alone 3 of 6 wins vs base 1 of 6, deaths -1.8 per game, uncontested pace kept; never swept alone | C | Regroup knob alone: selection, held-out, then hunter, raider2, foundry-v16 on corners_v2, collision_v2, interlock_v2 |
| Tactics v1 (focus, stun then burst, spacing, support, release) | EXP-088, experiments/log.md:20 | raider2 testbed, 6 encounters (30, 31, 40), 14 Oct | Deaths 10.7 to 13.3; 1 to 2 bot errors per game in 9 games | C | Fix the errors, then judge rules a to e singly vs raider2 and foundry-v16 on packs_v2, interlock_v2, elkring |
| Rows squad fallback | EXP-081, experiments/log.md:62 | Selection cells | Hook was dead code (wait_tank never reached), so the idea never ran | C | Reimplement at the real idle point; rows and packs_v2 vs P |
| Opening assignment v1 and margin 10 | EXP-079, EXP-080, experiments/log.md:73, :67 | Selection plus held-out; 50b deltas were fuel drift (EXP-082) | Hurt 71 and 50b vs contesting opponents; gated form shipped as v4.14 | C | Rerun ungated on current layouts with identity checked: corners_v2, monolith, expanse, lanes_v2 vs P |
| Ambush opening on close maps | EXP-020, experiments/log.md:441 | Only map30 qualified: 2 games vs v2, 2 vs v4-2, 14 Oct | Wipe worked (5 kills, 0 deaths by round 73) but games 100 to 150 rounds longer | C | elkring and the new ambush and rushbait_v2 maps vs foundry-v16 and tung-clone |
| Contested-altar choice | EXP-023, experiments/log.md:430 | 12 games vs v2, starter, v4-2; opponents never lock except locker | 9 to 27 per cent slower on 30 and 50 A | C | collision_v2 and lanes_v2 vs locker and foundry-v16 |
| Altar bank walk | EXP-027, experiments/log.md:383 | 12 games vs v2 under the win-count rule | 6 of 12 vs 9 of 12; map71 flips | C | Paired gems on corners_v2, collision_v2, rows vs raider2 and foundry-v16 |
| Cassi and Issac protection (pair retreat at 50 per cent, no solo start) | EXP-028, experiments/log.md:371 | 12 games vs v2 under the win-count rule | 7 of 12; helped map50 and 80 B | C | Pair deaths are a top ladder kill target (top2-micro:49): rows, packs_v2, interlock_v2 vs hunter, raider2, foundry-v16 |
| v4.1 bundle (contested avoidance, stuck, race bank, bag protection, Health first) | EXP-017, experiments/log.md:522 | 18 games vs v2, 18 vs v4g, 10 vs starter; five changes at once | 11 of 18; per-change split never run beyond v4.2's two | C | Split bag protection and race banking into knobs; P on collision_v2, corners_v2 |
| Monolith capture (S-11) | strategy/candidates.md:33 | Never run | Ladder: one top-30 team wins 5 of 6 with it (AUTONOMOUS-SPRINT:25) | C | monolith vs P, late swing from about round 290 |
| Opportunistic carrier kill (S-17), Vesper draft, wards, teleports, set_priority | strategy/candidates.md:85; top2-micro:50; MICROSTELLA_AUDIT_2026-10-16.md:31 | Never run | Ladder shows no gain for wards, teleports or locks (AUTONOMOUS-SPRINT:30) | C | Carrier kill and Vesper first: corners_v2, interlock_v2 vs P |
| Centre control v1 | EXP-075, experiments/log.md:95; D-058 | Selection +9.03; held-out 12 active cells (71, 40), 14 Oct | Held-out -9.98, deaths +0.96; 6 to 15 deaths vs v4-8, v4-3a | B | corners_v2 and interlock_v2 vs foundry-v16 and hunter; layouts changed |
| Centre v2, v3, v4 (advantage gate, synchronised stack, release) | EXP-076 to EXP-078, experiments/log.md:90, :85, :80 | 12 held-out centre cells each | Deaths 7.5, 4.9, 4.5; v3 and v4 stop fighting and farming (2 and 3 of 12 wins) | B | Only after centre v1 is retested; same maps |
| Hostility-gated threat response, two tiers | EXP-086, EXP-087, experiments/log.md:35, :30 | 16 fight cells plus 6 hunter games plus starter | Wins 5 and 9 vs 11; corners A misfire on farmers | B | Keep parked; retest only with a fight evaluator, vs foundry-v16 on corners_v2 |
| Kite, chase4, vision fix, micro trio (held out) | EXP-056 to EXP-058, EXP-060, experiments/log.md:192, :186, :189, :176 | 54 held-out cells vs own versions | Trio -4.97, wins 41 to 29, deaths -0.41 | B | Fewer deaths is the target on the ladder: corners_v2, collision_v2 vs hunter, raider2, foundry-v16 |
| Shop recall after far tributes (hold) | EXP-064, EXP-069, experiments/log.md:153, :125 | PASS both sets; corners B deaths +0.6 vs v4-3a | +5.09 and +3.31 held-out | B | corners_v2, expanse, monolith vs P; fix the corners B deaths |
| Field shop for Tavi (hold) | EXP-065, experiments/log.md:148 | 54 held-out, 31 cells identical | +0.55, null | B | Pair with Power-first build (EXP-091 next step) on rows, collision_v2 |
| Power-first build | EXP-091, experiments/log.md:10 | 8 games vs starter (50, 41, 90, 71) | rows -7.2, collision +5.3; gate not met | B | With field shop, vs foundry-v16 on rows, collision_v2, lanes_v2 |
| Solo wolf packs | EXP-092, experiments/log.md:5 | 6 games vs starter (50, 31, 41) | rows -19.6 and -31.6, 2 to 3 deaths | B | Varg-anchor variant (audit 4C) on rows and packs_v2 vs P |
| S-07 build (Monster, Jungle Health, Power) | EXP-010, experiments/log.md:579 | 16 vs starter, 16 vs v2, 13 Oct | -1.2 per cent g/100r; Monster-only retest never run | B | Monster-only to round 250 on elkring, collision_v2, interlock_v2 |
| v3-quick and fight-back rule v3a | EXP-011, experiments/log.md:558; strategy/DECISIONS.md:35 | 16 vs v2, 16 vs starter, 8 vs tung-clone, 13 Oct | 2 of 16 vs v2; v3a 7 of 16, 5.7 deaths | B | Low priority; superseded by v4 squads |
| Race finish calculator | EXP-022, experiments/log.md:453 | 6 games vs v2, 6 vs v4-2 | Rule hardly fired; no race was close | B | Close races only: vs foundry-v16 on lanes_v2, expanse, eight |
| Lone guard and death-loop breaker; v4.5b | EXP-024, EXP-025, experiments/log.md:419, :408 | 16 games vs v2 per variant, win-count rule | Rows deaths 9 to 18; Monolith guard worked | B | Monolith guard alone on monolith vs P |
| Two stacks, bank at 72 | EXP-019, experiments/log.md:478 | 12 vs v2, 12 vs v4-2 | 143.9 g/100r, 12.6 kills per game | B | elkring, packs_v2 vs foundry-v16 |
| Methodical ambush stack | EXP-016, experiments/log.md:500 | 12 vs v2, 12 vs v4g | 64.9 g/100r; wipe lopsided on map50 only | B | Opening only (EXP-020 row) |
| Single stack all game | EXP-015, experiments/log.md:511 | 12 vs v2, 12 vs v4g | 80.8 g/100r: whole team walks to bank after each group | A | None needed |
| Force check before Elk fights | EXP-029, experiments/log.md:360 | 72 selection cells | -15.65 [-22.08, -9.58]; v2 -9.12 | A | Recheck on elkring only |
| Bank at 72 gems | EXP-033, experiments/log.md:326 | 72 selection cells | -8.83, CI excludes zero | A | EOQ banking (audit section 8) instead, rows and expanse |
| Bank at 22 gems | EXP-032, experiments/log.md:332 | 72 selection cells | -2.38, CI spans zero | B | Fold into EOQ banking test |
| Cluster wait 15 and 40 | EXP-066, EXP-067, experiments/log.md:142, :136 | 72 selection cells | -6.03 and -6.80, CI excludes zero | A | None needed |
| Bank-first threat mode | EXP-071, EXP-072, experiments/log.md:120, :115 | 8 smoke games plus hunter | Uncontested pace -48 to -66 per cent on corners | A | None needed |
| Rows sticky approach | EXP-089, experiments/log.md:15 | Selection plus held-out | +0.51 and -0.14; effect about 2 g/100r | A | None needed |
| Flee radius 6 | EXP-037, experiments/log.md:302 | 72 selection cells | -0.76, deaths +0.08 | B | Fight panel only, low priority |

## Table 2: archetype sparring bots in bots/

| Bot | Behaviour encoded |
|---|---|
| hunter | Draft Ryth, Stellar, Prism, Ren, Tavi; hunt mode roams after the last seen enemy champion (9.4 kills per game vs v4-13 on 71, 30, 40; weak on 90). EXP-070. |
| raider2 | Two raiders (Prism or Ren plus Tavi or Varg) patrol the enemy half and hunt; the rest farm their own half. Reproduces own-half raids (10.7 deaths per game for v4-14). EXP-088. |
| locker | v2-structure farmer with a Power build; keeps one altar lock in stock, locks any unlocked altar within 2 tiles, and routes around locked altars (bot.cpp:165-290). EXP-023. |
| centre | Centre v4: v4-13 plus centre control on maps with a central altar and 2+ gem clusters within 10 (old 71, 40); holds the centre five-strong, kills entrants, release and relaxed burst rules. EXP-078. |
| centre-v2 | Centre v1 plus local-advantage gate (P_CENTRE_ADV) and re-centre leash. EXP-076. |
| centre-v3 | Synchronised stack: stack tile, engage as a stack, stun-range exposure, isolated rule. EXP-077. Centre v1 lives only in git history (D-058). |
| tact | v4-14 with tactics.cpp compiled in and P_TACTICS off, so it plays as v4.14. |
| tact-on | tact with P_TACTICS=1: focus fire, stun then burst, ranged spacing, support, instant release; logs bot errors. EXP-088. |
| tung-clone | Emperor Tung model: Vendetta, Varg, C&I, Ryth, Tavi; fixed squads, own-half targeting, tank and ranged roles, one capturer per group; never fights champions. EXP-009. |
| v5-stack | All champions in one squad all game; every non-Monolith group a target; wins fights (16.4 kills vs v2), slow race. EXP-015. |
| v5-ambush | v5-stack with kAmbush: the stack attacks the enemy's first groups from round 1, then farms as a stack. EXP-016. |
| v5-two | Two stacks by champion type (Vendetta, Issac, Cassi; the rest), bank at 72, no capture wait. EXP-019. |
| v4-amb | v4-2 plus an ambush opening when spawns are close (only old elkring qualified), then v4.2 squads. EXP-020. |
| v3-quick | Pure farmer: never attacks champions, own half first, abandons contested waits. EXP-011. |
| probe, x-rejected-early-v2 | Measurement bot and an early regressed variant; neither is a sparring archetype. |

All archetypes reuse the v2 or v4 action layer written against the v0_0_1 SDK. SALVAGE.md confirms that v4.14 builds under cambc 2.2.5; the others are unverified.
