Claude (claude.ai), Opus 5.5 Extra, answer saved 2026-10-20 ~10:56 BST

I would put the window into B, and release C, G (altar control, below), and the gold-spending half of F first, since each takes a few hours.
I would put the window into B, and release C, G (altar control, below), and the gold-spending half of F first, since each takes a few hours. Rank 20 is out of reach on these numbers. Rank 20 (1,897) sits 73 points below rank 10 (1,970), so a rank-20 bot takes roughly 35 to 40 per cent of games from teams ranked 2 to 10. v4 takes 1 per cent. The climb itself also takes time. A bot whose strength exceeds its rating by 500 points needs about 55 ranked series, roughly 9 hours, to come within 50 points. A realistic target is parity with the 1,453–1,771 panel, which puts the rating near 1,600.

Allocation
Session 1 works on B for the whole window, scoped as in point 1.
Session 2 releases C within the first 6 hours, then builds G, then F’s gold-spending and recall rules.
Session 3 first settles the fact in point 4 from the 90 top-10 replays. It then runs v4 unopposed on each map to find the round at which it reaches 1,000 gems.
If that round comes later than the round at which the top 10 finish against v4, farming speed is the binding constraint and B stays first.
If it comes earlier, the gap comes from contact, and A and G move ahead of B.
After that, the session builds the monolith plays in point 5, then ability-first openers for the champions C settles on.
Release rule. Accept a build when a 30-minute live run (125 games against the top 10) shows a gem share above the live build’s and deaths per game below it. Win rate against the top 10 sits at 1 per cent and cannot register a change. Release each passing build at once so the rating climbs during development. The last substantive build needs to be live by about Friday 04:00.
1. Ranking
B ranks first. v4’s own change bounds what death reduction is worth. Against the panel, v4 cut 14.9 deaths a game and gained 116 gems, which is at most 8 gems per death avoided. At that rate, matching the top 10’s 6.0 deaths and keeping the 62 gems a game lost to kills puts v4 near 350 gems against their roughly 880. The remaining gap is farming rate, and B is the option that changes it. The pre-registered rule points the same way. A win rate below 30 per cent does not by itself separate economy from fighting, but the bound above does. Within B, I would build in this order:
Pathing puts a large cost on tiles within 3, 4, and 5 of awake wolves, elk, and monoliths. Monsters caused 48 per cent of the previous version’s deaths, against 22 per cent for the top 10.
Camp fights use the tank swap in point 3.
The group kills each camp, one capturer holds the tile, and the others leave for the next camp. The monolith is the exception (point 5).
Champions never wait on a spawn tile. Under rule 2, a champion waiting on a tile for a respawn holds that respawn back. Count the rounds v4 spends on tiles of camps whose timer reads zero.
Camp order, tribute bag threshold, and retreat HP are tuned per map offline, using unopposed farming speed as the metric. Twelve cores at 1 second a game give about 43,000 games an hour. An unopposed metric also avoids the self-play bias that the corners results show.
C ranks second and releases first. The top 10 draft Stellar 96 per cent of the time, Cassi & Issac 95, Ren 72, Varg 70, and Ryth 67. v4 opens with Vendetta on 14 of 21 maps, a champion the top 10 take 3 per cent of the time. As written, the rule bars a team from repeating its own pick, so both teams can field the core in every game. The arena duels give no case for a counter-pick. Heart of Ruin beats Stellar but loses to Cassi & Issac. Prism beats Stellar above 21 gems but loses to Ren and Varg. All of those winners are core members. Test the draft change with v4’s code unchanged, first locally across 21 maps and then in one 30-minute live run.
G, altar control, ranks third. It covers four things: placing locks, clearing enemy locks with three basic attacks, starting a tribute channel when no enemy champion stands within reach, and interrupting enemy channels. The last item comes out of A. The top 10 place 6.4 locks a game and v4 places none. 43 per cent of the previous version’s deaths fell within 8 tiles of an altar. Check first whether v4 detects an enemy lock. A bot that carries a full bag to a locked altar and waits there produces this death pattern. Each lock costs 10 gold and costs the opponent three attack turns.
F ranks fourth. At round 200, the top 10 have 3.8 gear points per champion and v4 has 1.7.
If v4 holds unspent gold, buying at once toward completed emblems takes an hour. If lifetime gold counts earnings, spending leaves the tiebreak unchanged, so there is no reason to hold gold.
If v4 lacks the gold, the gap belongs to B.
A recall below an HP threshold trades a 60-round death, 50 gold, and the bag for 5 turns and the walk back. This works when the channel starts outside threat range.
A ranks fifth. The fight gap is measured: 35–43 per cent ability openers against 85, and 62 gems lost to kills a game against 3. Ability logic is champion-specific, so it waits for C’s draft decision. Self-play tunes micro against v4’s own micro, and the corners results show self-play and the live panel disagreeing. The bound in B also caps what fewer deaths deliver.
D ranks sixth. Holding enemy spawn tiles parks one of three to five champions on the enemy side, where the top 10 already kill v4 22.7 times a game. Rule 1 is symmetric. An aggro pull works when our champions stand farther from awake monsters than the enemy does, and v4’s monster-death share says it does not manage that distance today. The one form worth building is holding a monolith tile at timer zero to choose the respawn round.
E ranks seventh. The four contact maps are 4 of 21, about 19 per cent of games. The first tiebreak is tributed gems. On contest the panel scores 325 and the top 10 score 822, so avoidance loses every game there. B’s pathing reaches the 45–56 deaths a game on these maps without map-specific work.
H ranks last. The data identifies specific gaps, and C, G, and F each take hours.
2. Most likely failure modes
Option	Failure mode
A	Self-play tunes the micro against v4’s own habits, and the live panel shows no change in deaths.
B	The tuned parameters assume camps that the top 10 reach first, and they give nothing on maps outside the 21 if the final pool adds maps.
C	The core champions arrive without the ability logic that gives them their value, and the new draft scores below the Vendetta draft in v4’s hands.
D	The tile holder dies alone on the enemy side, and the camp respawns next to whichever v4 champions remain.
E	The plans fit four maps, and none of them yields a tributed gem on contest.
F	A recall starts inside threat range, damage interrupts it, and the champion dies after channelling for 5 turns.
G	Locks land on altars the opponent does not use, and the threat gate holds full bags back long enough that v4 tributes less.
H	The gap persists.
3. Idea from another game

Raid tank swaps from MMOs such as World of Warcraft map onto rule 1, nearest-champion targeting.

In a camp fight, the champion with the most effective HP stands nearest to each monster, and the others attack from farther out.
When the tank drops below a set HP fraction, the next champion steps one tile closer and draws the monsters’ attacks on the following round. The tank then steps back.
WoW swaps on accumulated threat. Rule 1 swaps on distance, which the bot computes exactly each turn.

The same rule applies to contested camps. When both teams stand near awake monsters, keeping every ally farther from them than the nearest enemy puts the camp’s damage on the enemy. Monster deaths per game measure the effect.

4. The fact that would most change this

The fact is the 48/50 split between monster and champion deaths. It comes from the previous version playing against the panel. v4 then cut 14.9 deaths a game. If most of that cut was monster deaths, v4’s 22.7 deaths a game against the top 10 are mostly champion kills. With a champion share near 80 per cent, three things change:

B’s first item, threat-aware pathing, yields little.
The 62 gems and 50 gold per kill become the main cost of a death.
A moves from fifth to third, behind C.

The 90 top-10 replays answer this within the hour. I would check it before session 1 starts.

5. Game-changing idea

Yes. The contest row shows how the top 10 tribute monoliths. They score 822 gems from 4.1 tributes a game, which is 200 gems per tribute. The gem sources on that map are the monolith and kills, and v4 champions there carry nothing. Each monolith therefore reaches the altar as one 200-gem bag. That channel lasts ceil(200/10) = 20 turns, any damage restarts it, and killing the carrier gives the killer 200 gems and 50 gold. Two plays follow:

Offence. Place a lock in advance on the altar the carrier uses (10 gold), then land one hit on the carrier inside each 20-turn window. Each interruption keeps 200 gems off their score, and a kill moves those gems to a v4 champion.
Our own captures. If the monolith group has more than one spawn tile, every available ally stands on one. Allies split the reward, so four allies carry 50 gems each and channel 5 turns each in parallel. If the group has one tile, the split is impossible, and the 200-per-tribute figure reflects the map more than a gap in the top 10’s code.

The numbers say the top 10 use altar denial (6.4 locks a game). They also show the top 10 tributing whole monoliths as single bags. v4 dies 48.7 times a game on contest, so this play depends on G’s channel logic and B’s pathing being in place first. It is the largest single swing the rules allow, and it applies on every map with a monolith. It does not close the overall gap on its own.







## Follow-up (correction)

The ranking stays B, C, G, F, A, D, E, H.
Drafting the final wording and checking word limits.

Drafting the final wording and checking word limits.

The ranking stays B, C, G, F, A, D, E, H. Two recommendations change.

G’s case strengthens. A lock placed during an enemy channel now cancels it and blocks further tributes at that altar until three basic attacks remove it, all for 10 gold. Our tribute gate should respond to enemy stuns, enemy locks within reach of the altar, and kill threat. Ordinary damage no longer matters to it.

Point 5 keeps its target and changes its tool. A lock, a stun, or a kill cancels the 20-turn, 200-gem monolith channel on contest, and a hit does not. The offence therefore starts with a champion who reaches the altar mid-channel and places a lock.

F’s recall advice is unchanged. A’s interrupt item now depends on stun abilities, so C’s draft test should record which champions carry them. Teleport crystals do not appear in the brief and affect nothing above.






