# Consult 19:05 BST 19 Oct: current strategy review + wider strategies (ChatGPT, DeepSeek, Copilot)

Attachment: `consult/pack/xanarchy-context-v4.txt` (situation, faults F1-F6 with evidence, experiments, real-team data, evidence against our lean, per-map economics, full rules, Bot API index). Sent through Claude in Chrome at Xans's request ("send in current consultation on strategies and also consultation on the use of wider strategies ... be exhaustive and also ask it to come up with more sound strategies").

Two messages per consultant, in order: Part 1 (current strategy), then Part 2 (wider and new strategies) after Part 1's answer arrives.

## Part 1 (shared text; each consultant gets its angle line at the top)

The attached file is the complete, current context of our Cambridge Battlecode 2026 bot: our ladder situation, six diagnosed faults with evidence, every experiment since this morning with its result, decoded data from 515 of our ranked games and 1,011 top-10 games, per-map camp economics, the full rules and the C++ API. Please read all of it before answering. You are an outside reviewer: be critical and concrete, quote the rule line or table row behind each claim, and say where you are unsure.

QUESTION 1: is our diagnosis right, and what is missing from it? For each of F1-F6 in §3, say whether the evidence supports it as a cause of lost games, how much of the gap to top-10 income (0.98 vs 1.54 gems a round, §5) you think it explains, and what evidence would refute it. Then name the causes we have not listed that the Appendix B tables point to (for example the tribute-count gap on eight, fragment, packs, pilgrimage and swirl with equal or larger bags; our 41 deaths a game on swirl).

QUESTION 2: what goes into the next version, and in what order? Options we see, in no particular order:
(a) gem-per-round target scoring (travel, clear time, capture, share, bank trip) replacing distance-greedy targeting;
(b) monolith siege from range 4 on the maps that have monoliths;
(c) damage-first item builds; Focus stacking for casters; Tavi field buying;
(d) engagement rules: no lone even fights, regroup, fight-outcome prediction before committing;
(e) tribute cadence: smaller, more frequent tributes, or tribute timing against enemy positions;
(f) denial: altar locks, tribute interrupts, camp-respawn blocking by standing on spawn tiles;
(g) drafting and counter-drafting;
(h) per-map strategy modes chosen from the map's economics (Appendix C);
(i) an option not listed;
(j) none of these.

REQUESTS:
1) Rank (a)-(j) by expected rating gain per day of work over the 5 days left, with reasons tied to named rules and table rows.
2) Name each option's failure mode and the result that should make us drop it (fixed before testing).
3) Say how to test against real teams given 250 unranked games an hour, deterministic local games, and a no-op flip rate of 7 %; how many games per decision.
4) Say which fact in the file, if wrong, would change your ranking.

Constraints: 5 days to the lock; one person plus AI coding agents; 14 cores; 100,000 fuel a turn; 8 bytes of shared team memory plus 1,024 bytes of draft memory; no shared vision; no persistence between matches.

## Part 2 (shared text; sent after Part 1's answer)

Part 2: wider strategies. Be exhaustive; we want quantity and quality, then your own new ideas.

A) Strategies from other contexts. For each source below, list every technique you know that could transfer, and for each one state: the source (name the game/season/team or paper where you can), the exact cambc rule or mechanic it maps onto (quote the rule line), how a bot would implement it within 100,000 fuel a turn, 8 shared bytes and 1,024 draft bytes with no shared vision, the expected effect on gems per round or deaths, and the result that would falsify it.
 1. Past Battlecode seasons (MIT Battlecode 2010-2025 postmortems; earlier Cambridge seasons): communication schemes under tiny shared memory, micro (kiting, focus fire, target selection, retreat), macro (economy timing, scouting), pathing.
 2. MOBAs (League of Legends, Dota 2): jungle timers and camp stacking/blocking, objective trading, rotations, vision control, wave/tempo management, power spikes from items, team-fight positioning and engage/disengage rules, split pushing, "recall timing", counter-picking and draft phases (bans/picks).
 3. RTS (StarCraft II, Age of Empires, BWAPI/SSCAIT bot competitions): kiting under attack cooldowns, focus fire and overkill avoidance, concave/surround, army trading by value, timing attacks, economy-vs-army balance, threat/influence maps, build orders as opening books.
 4. Team sports: zone vs man marking, pressing triggers, set plays, possession and tempo, substitution and fatigue analogues.
 5. Operations research: vehicle routing with time windows (camp respawn timers as windows), orienteering/prize-collecting routing, scheduling, queueing at altars.
 6. Game theory: sequential drafting as an extensive-form game, minimax counter-picks, mixed strategies against unknown opponents, exploitation vs safety.
 7. Machine learning and adaptive control inside a match: opponent modelling from the revealed draft and from observed behaviour, Bayesian updating of enemy damage and aggression, bandits over micro policies, offline-trained small models (logistic/decision trees) embedded as constants, all within the fuel and memory limits.

B) Micro and matchups (we want this exhaustively). There are 11 champions with 2 abilities each (see the kits in Appendix A) and 3-5 champions a side. (i) Enumerate the micro opportunities the rules create, per champion kit and per pairing (e.g. ranged walk-or-attack kiting against melee, stun-into-burst, attack-reset weaving, stun held for channels, fighting at a camp's edge so monsters hit the enemy, Cassi & Issac dying together, bush vision, Tavi's 2.2 × Focus stun at range 4, dash escapes, heal/shield timing, turn-order priority). (ii) For the drafts in the file, say which micro matters most in which matchup, and estimate by how much. (iii) Propose how a bot should switch micro at run time to counter the enemy draft and enemy behaviour (what to write in draft memory, what each champion should estimate during the match, what to share in 8 bytes). (iv) Propose an experiment design to measure each micro's value per matchup (we can build small arena maps and run thousands of deterministic duels locally).

C) Your own new strategies: at least 10 sound strategies not listed anywhere in the file or above, each grounded in a quoted rule line, with expected effect and a falsifier.

D) Finally, rank your top 15 ideas from A-C by expected rating gain per day of work given 5 days.

## Angle lines

- ChatGPT: "Take the angle of a game-AI designer and machine-learning engineer: architecture for adaptability (utility AI, state machines, opponent modelling, online adaptation within tight compute)."
- DeepSeek: "Take the rules angle: which rule interactions are most exploitable and least used by the teams in the data; quote rule lines."
- Copilot: "Take the code-and-data angle: which modules to write first in this C++ codebase, what to extract from the decoded replays, and how to measure."

## Answers

(Collected 20 Oct by the consultation runner. Full DeepSeek text: answers/1905-deepseek-part1.md and part2.md, condensed copies; ChatGPT text was read in the browser but the full 58,645 + 83,209 character answers were not copied to disk (see answers/1905-chatgpt-part1.md and part2.md, which hold the read sections only). Copilot: skipped, the page demanded authentication. Tags are relative to consult/pack/header-v4.md: new / agrees / contradicts.)

### DeepSeek, Part 1 (rules angle)
Ranked by rating per day:
1. Tribute cadence + altar-first routing + bag safety (options e / M1-M3): agrees with option (e) as listed, but new framing: tempo (first tribute round 69 vs 86) and bag-to-altar gap are the biggest cause, not target scoring. Contradicts the header's "bank at about 36" policy.
2. Turn-flow reorder, enemy cast before camp cast, dash/reset/heal (F4/F6/M4): agrees with F4 and F6; new: the 33 of 60 victims who never cast are an ordering bug.
3. Per-map draft table, Stellar in (g): agrees with the header's Stellar data.
4. (a) gem scoring, with a survival term: partly contradicts F1 as the main cause (F1 only explains the gold-camp family; on swirl/fragment the cause is deaths and bag loss).
5. (b) contest siege only: agrees; contradicts F2 as a gap driver (contest is 6.6% of games).
6. (h) per-map modes in draft memory: new use of the 1,024 draft bytes.
7. (c) builds: reads F3 as "durability/jungle emblems and power shards, not no-defence"; contradicts the damage-only direction after the addendum.
8. (d) engagement rules only as "do not initiate outnumbered fights"; (f) denial last.
Rule checks: "Monsters hit at range 3 ... never chase, attack the nearest champion until undamaged for 2 turns": checked true (GameRules.md:189, 206). "Captures resolve before channels" true (231). "Any occupied spawn tile delays the entire group" true (193). "Melee champions can walk and attack in the same turn" true (159). Monolith row 20,000/200/3/300/200/150 true (185). Durability 10% and Jungle 30% true (300, 302). Claim that "bank at 36 loses the capture-then-tribute freebie" is unverifiable as stated; line 232 says tribute length uses the bag when started, completion tributes the whole bag including gems gained since (checked: true), which is a different effect (see Part 3 note).

### DeepSeek, Part 2
Top 15 order: cadence + altar-first; 8-byte protocol + turn-flow reorder; spawn-tile respawn block (C2); both-ways micro + kiting; objective trading + enemy-carry beacon; bait the camp (C7); opening book in draft memory; Stellar-first + Marcella anti-stun draft; orienteering with survival term; Vesper gold specialist; recall-cancel press; jungle timers; Tavi field Health buy; defensive altar lock; respawn-window push.
Tags: spawn-tile block new as a priority (header lists it only inside option f); bait the camp (monsters hit the enemy) new; defensive altar lock new (we have 0 lock placements); carry exactly 1 gem to a locked altar new; Marcella anti-stun draft new; kiting contradicts our duel lab (kiting about 0); orienteering with survival agrees with (a) and explains the 38% local rejection.
Rule checks: "Priority, then draft order" and "set_priority takes effect from the next round" true (127-128); "Teams alternate: the starting team swaps each round" true (126); altar lock "blocks opposing tributes but allows its owner's team" true (241 region); "Attempts to tribute against a locked altar are implicitly converted to auto attacks" true (241); recall "interrupted by damage" true (272, 373); tribute "damage does not stop it" true (272); "Prism gains 0.2 Focus per carried gem" true (550); Vesper +20% gold, Treasure +20 points true (518); "Marcella immune to stuns" true (644); Ren retaliates 0.2 x Focus true (738); "Health bought away from spawn raises maximum health without healing" true as quoted (676); DeepSeek's inference that this is a free pre-recall HP buffer is unverifiable. DeepSeek's C13 respawn "60 owner turn-end ticks" true (361). Claim that "a stationary enemy for 2 rounds is probably recalling" is unverifiable.

### ChatGPT, Part 1 (game-AI / ML angle; read: executive verdict and ranking table only)
Rank: (a) economic target scoring (gems per productive round) 1; (i) new capture and banking controller 2; (b) contest monolith siege 3; (e) adaptive tribute cadence 4; (d) selective engagement evaluation 5; (h) per-map modes 6; (c) champion-specific builds 7; (g) draft 8; (f) denial 9; (j) 10.
Tags: (a) first agrees with the header's option (a), later contradicted by our 38% local result; "capture controller" state machine (missing capture completion step) new; warns not to abandon Health or copy top-team builds (agrees with the addendum); "28/102 shows safety can destroy throughput" agrees. Names three problems: gem acquisition, conversion to score, catastrophic time loss. It declines to give numeric Elo.
Rule checks: no rule quotations in the sections read; nothing to check.

### ChatGPT, Part 2 (read: headings and final ranking)
Ranked shortlist: 1 short-horizon fight evaluation with hold-vs-flee (new framing of option d, aimed at the failed 28/102 retreat controller); 2 anti-Stellar micro; 3 objective feasibility gates before economic scoring (new; answers the 38% result); 4 focus-fire and kill-secure targeting; 5 attack-reset / spell / shield action optimiser; 6 capture completion and payout control; 7 banking by survival to channel completion; 8 Stellar-aware draft; 9 combat-triggered rotations; 10 opening books; 11 damage/Health breakpoints; 12 camp clear efficiency; 13 selective ward and threat reporting; 14 Tavi hybrid build with free respec; 15 contest-only monolith siege (contradicts its own Part 1 rank 3, and DeepSeek's contest-only keep).
New ideas by heading (C1-C15): capture-to-tribute pipelining (checked: true in principle, GameRules.md:232 tribute length uses the starting bag, completion pays the whole bag); capture payout assignment as risk transfer; bush occupancy probing at no movement cost (rule at 163-167: a blocked step reveals the tile and keeps the allowance: checked true); dead-time item progression (dead champions retain shopping turns, line 361: true); continue tributing under non-stunning damage (line 272: true); terminal-score override; gold-tiebreak policy (line 34: tie goes to lifetime gold, kills, then Team B: true); respawn-tile evacuation; teleport-crystal shortcuts (line 159: range 3, ignore terrain: true); camp-group geometry by cardinal connectivity (line 193-194: true); initiative-aware channel protection (priority rules true); health purchases heal at spawn (unverified wording).
