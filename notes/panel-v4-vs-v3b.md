# v4 vs v3b, real-team unranked panel (160 paired games; v4 queued 20 Oct 09:47-09:48 BST, finished 09:49-09:52; v3b queued 06:12-06:16)

Source: `data/unranked/matches.jsonl` (32 `v4-<team>-*` groups; the `v4top-*` groups are excluded), `data/unranked/sumj/` (`cambc matches summary`), `tools/v3/panel_v4_vs_v3b.py` (new; prints every table). Engine 1.0.0 both. Same 8 teams, same 20 maps, one game per (team, map), so rating adjustment cancels in the pairs. All cells are n<=20 per team or n<=8 per map, flagged with a dagger in the text below. Gems and deaths are per-game means. "Lost to champion kills" = sum of `gemLoss` entries with context champion, owner = our team (my reading of the event owner; not checked against a replay).

## Live check
All 160 v4 games finished 08:49:37Z-08:51:50Z (09:49-09:52 BST), after the 09:46 BST upload. The draft is a weak check: 152 of 160 pairs have the same draft as v3b; the 8 that differ are all 00contest (v4 2.3.9.0.10, v3b 4.10.5.3.0), which shows the new per-map draft is live. Verdict: time-based, plus the contest draft.

## 1. Overall (data, n=160 each)
| bot | W-L | win rate (Wilson 95 %) | gems us / them | tributes us | gems tributed per round | deaths us / game | gems lost to champion kills (us / them) | draws (equal gems) |
|---|---|---|---|---|---|---|---|---|
| v3b | 29-131 | 18.1 % | 383 / 781 | 19.3 | 0.24 | 37.9 | 69 / 31 | 10 |
| v4 | 37-123 | 23.1 % (17.2-30.2) | 499 / 782 | 25.3 | 0.31 | 23.0 | 74 / 29 | 12 |

Paired (team, map): loss->win 15, win->loss 7, unchanged 138. McNemar exact two-sided p = 0.13 (22 discordant pairs; not significant). Mean paired gem margin +114 a game (v4 minus v3b); mean paired deaths change -14.9 (sd 25.1). Mean rounds per game: v3b 1,614, v4 1,607.

## 2. Per team (data; n=20 each †)
| team | v3b W-L | v4 W-L | L->W | W->L | gems us/them v3b | gems us/them v4 | deaths v3b | deaths v4 | tributes v3b | tributes v4 |
|---|---|---|---|---|---|---|---|---|---|---|
| Micron | 2-18 | 5-15 | 4 | 1 | 329/758 | 510/798 | 37.8 | 22.2 | 17.1 | 25.9 |
| Deer Park | 3-17 | 3-17 | 1 | 1 | 384/812 | 461/831 | 35.6 | 20.1 | 20.0 | 26.1 |
| tvojtatko | 5-15 | 9-11 | 4 | 0 | 446/722 | 586/651 | 42.2 | 25.0 | 22.8 | 31.3 |
| 777 | 13-7 | 11-9 | 1 | 3 | 611/524 | 677/584 | 33.6 | 17.7 | 29.4 | 31.3 |
| Snowman Workshop | 2-18 | 6-14 | 4 | 0 | 400/850 | 521/791 | 40.9 | 26.5 | 18.1 | 22.6 |
| hackingnoises | 2-18 | 3-17 | 1 | 0 | 428/792 | 567/775 | 34.0 | 21.4 | 20.8 | 27.6 |
| Tozo's Disciples | 1-19 | 0-20 | 0 | 1 | 309/900 | 388/927 | 43.2 | 28.4 | 15.4 | 20.7 |
| umamusume | 1-19 | 0-20 | 0 | 1 | 161/888 | 285/902 | 36.0 | 22.6 | 10.7 | 17.0 |

Gems lost to champion kills per team (us, v3b / v4): Micron 68/77, Deer Park 47/81, tvojtatko 73/66, 777 45/45, Snowman 81/78, hackingnoises 86/104, Tozo 56/61, uma 94/79 (no systematic change).

## 3. Per map (data; n=7-8 each †)
| map id | v3b W-L | v4 W-L | L->W | W->L | gems us/them v3b | gems us/them v4 | deaths v3b | deaths v4 | tributes v3b | tributes v4 |
|---|---|---|---|---|---|---|---|---|---|---|
| 00ambush | 1-6 | 2-5 | 1 | 0 | 711/946 | 754/894 | 14.0 | 12.6 | 17.7 | 19.1 |
| 00collision | 2-6 | 3-5 | 1 | 0 | 442/890 | 568/906 | 19.6 | 21.6 | 17.0 | 25.2 |
| 00contest | 0-8 | 0-8 | 0 | 0 | 0/325 | 0/325 | 119.6 | 51.5 | 0.0 | 0.0 |
| 00corners | 3-5 | 3-5 | 0 | 0 | 680/868 | 720/860 | 11.0 | 10.6 | 46.0 | 45.0 |
| 00eight | 1-6 | 1-6 | 0 | 0 | 444/889 | 564/988 | 26.1 | 31.6 | 16.3 | 19.7 |
| 00elkring | 0-7 | 1-6 | 1 | 0 | 526/1010 | 567/964 | 19.6 | 17.4 | 14.7 | 15.4 |
| 00expanse | 5-3 | 5-3 | 1 | 1 | 859/752 | 922/757 | 4.4 | 2.2 | 56.9 | 62.8 |
| 00fragment | 1-7 | 0-8 | 0 | 1 | 213/900 | 236/882 | 64.9 | 53.6 | 16.2 | 22.0 |
| 00interlock | 3-5 | 2-6 | 1 | 2 | 649/880 | 588/918 | 29.2 | 28.0 | 26.2 | 29.9 |
| 00lanes | 1-7 | 2-6 | 1 | 0 | 770/952 | 787/933 | 11.6 | 11.0 | 48.1 | 52.2 |
| 00monolith | 2-5 | 1-6 | 0 | 1 | 720/944 | 756/995 | 17.7 | 15.6 | 49.3 | 50.9 |
| 01big_world | 3-4 | 3-4 | 0 | 0 | 743/750 | 876/820 | 10.6 | 4.7 | 36.3 | 44.6 |
| 01boss_battle | 3-5 | 3-5 | 1 | 1 | 0/0 | 0/0 | 21.1 | 1.0 | 0 | 0 |
| 01broken_curve | 1-7 | 1-7 | 0 | 0 | 149/779 | 485/864 | 76.8 | 25.4 | 5.9 | 18.0 |
| 01concentrated_wealth | 1-7 | 2-6 | 1 | 0 | 344/738 | 438/742 | 34.2 | 27.6 | 21.9 | 27.1 |
| 01corridors | 1-6 | 1-6 | 0 | 0 | 190/707 | 276/603 | 94.3 | 16.4 | 7.4 | 14.6 |
| 01elk_arena | 0-7 | 1-6 | 1 | 0 | 144/896 | 205/901 | 27.0 | 16.1 | 2.1 | 3.1 |
| 01fracture | 0-8 | 2-6 | 2 | 0 | 216/740 | 365/686 | 49.9 | 19.5 | 8.8 | 24.2 |
| 01gold_rush | 0-8 | 0-8 | 0 | 0 | 67/823 | 432/898 | 73.9 | 45.4 | 5.0 | 28.8 |
| 01hooks | 1-7 | 0-8 | 0 | 1 | 175/968 | 148/984 | 58.6 | 55.6 | 5.9 | 5.1 |
| 01islands | 0-7 | 4-3 | 4 | 0 | 70/717 | 910/570 | 0.4 | 7.4 | 2.0 | 22.3 |

Groups: 00 maps (old pool) v3b 19-65, v4 20-64 (84 games); 01 maps (new) v3b 10-66, v4 17-59 (76 games). 01boss_battle: 0-0 gems in all 8 games both times, 3 tiebreak wins each (outcome is not skill evidence). v4 deaths over 45 a game: 01hooks 55.6, 00fragment 53.6, 00contest 51.5, 01gold_rush 45.4.

## 4. PLAN-v4 Draft 3 admission check (data vs rule)
| rule | threshold | v4 result | verdict |
|---|---|---|---|
| panel win rate | kill if < 30 % | 37 of 160 = 23.1 % (Wilson 17.2-30.2 %) | **fails the rule as written** (upper CI bound just touches 30 %) |
| deaths a game | <= 25 | 23.0 (v3b 37.9) | passes |

Rating adjustment: the 8 opponents and 20 maps are identical to the v3b panel, so the v4-v3b comparison needs none. I did not compute a rating-based expected win rate for v4 (our rating at the time, 1,398 at 09:46 per STATE.md, is a ladder figure from a different pool of opponents).

## 5. Interpretation (labelled, not data)
- v4 beats v3b on every secondary measure (gems +116 a game, deaths -15, tributes +6) and by 8 wins, but the win difference is not significant (p = 0.13). Most of the gain sits on new maps: 01islands +4 wins (70 -> 910 gems, plausibly mapid/reachability), 01fracture +2; 01broken_curve and 01corridors deaths fell by 51 and 78 a game without a win. The 11 old maps are flat (19 -> 20 wins).
- The rule says kill. It was fixed against a v3b-like floor; v4 is above v3b but below 30 %. Whether "kill" means rolling back is the lead's call; the data do not support v3b as the better bot.
- Still broken: 00contest (0 gems, 51.5 deaths; the new draft did not help), 00fragment, 01hooks, 01gold_rush (deaths 45-56 a game; gold_rush gems 67 -> 432 yet 0-8). Tozo and uma still take 0-20.
- 777 slipped 13-7 -> 11-9 (00fragment, 00monolith, 01hooks); one-game flips sit inside the engine's fuel-chaotic noise (a no-op change flipped 9 wins and 6 losses of 204 cells locally, PLAN.md).
