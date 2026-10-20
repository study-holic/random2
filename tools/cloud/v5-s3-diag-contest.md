Read tools/cloud/v5-common.md first and follow it. Your copy is bots/s3. Module: **DIAGNOSTICS FIRST, then CONTEST, then the OPENER.**

## 1. Diagnostics (first hour)
Push results/v5-s3-diag.md to this session's branch as soon as it is done; the other sessions use it.
- **Gem funnel per game** for v4 in the real top-10 games (data/real/v4-panel/v4top-*, decoded as in tools/README-decode.md) and in self-play vs v3b-e1. Columns: gems acquired from camps, gems tributed, gems destroyed by monster deaths, gems lost to champion kills, and gems carried at round 2,000. ChatGPT's test: if v4 acquires about 550 and banks 499, the problem is production; if it acquires about 950, the problem is conversion.
- **v4's deaths against the top 10, by cause:** monster or champion, by location and by round band, using the step0 method (tools/step0/, notes/step0-deaths.md). Claude's deciding fact: if about 80 % are champion kills, fights rise in priority.
- **Lock awareness:** does v4 detect an enemy altar lock, and what does a v4 champion do at a locked altar? Read the code and trace one case.
- **Time use:** the share of champion-rounds v4 spends walking, clearing, capturing, tributing, recalling, dead and idle, in the top-10 games.

## 2. Contest (v4 scores 0 gems against every real team)
- Contest's monolith group has ONE spawn tile, 200 gems and 300 gold, and respawns in 150 rounds (data/maps21.json). The map's only gem sources are the monolith and kills.
- The top 10 score 822 gems from 4.1 tributes a game, about 200 a tribute, so they bank whole monoliths. A 200-gem tribute channels for ceil(200/10) = 20 turns. A stun, the carrier's death, or a newly placed lock cancels it; damage does not.
- Plays, each behind a switch and each with a trace:
  - (a) **interdiction:** carry a lock (10 gold); when an enemy carrier channels at an altar, place the lock and/or stun or kill the carrier, since killing it transfers 200 gems;
  - (b) **our own monolith:** the squad kills it with a tank swap (the highest-eHP ally stands nearest; swap below 35 % HP), one capturer holds the tile, and the 200-gem carrier is escorted to the safest altar;
  - (c) a gold tiebreak only if (a) and (b) both fail.
- Admission on contest: more than 0 gems a game vs v3b-e1 and vs v4, with fewer deaths than v4's.

## 3. Ability-first opener (after s2 settles the draft)
For the core champions (Stellar, Cassi & Issac, Ren, Varg, Ryth): when a fight is committed (an enemy within cast range and ours to take), cast the stun or burst ability before basic attacks, on the focus target (Cassi first, since both die together; then the biggest bag; then the lowest effective HP). Never cast at camps while an enemy is within 8 tiles. Measure the share of fights opened with an ability (top 10: 85 %; us: 35-43 %).

Use the admission rule in v5-common.md for parts 2 and 3.
