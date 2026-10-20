# Contest experiments (cloud session, 18 October 21:00 to 21:20 BST, cambc 2.2.5)

Data points from 6 local games. Each figure names its opponent and side.

## Map facts (probe of map id 3a5c2468)

- 50 x 50, spawns at (4,4) and (45,45), 2 altars at the centre ((25,24) and (24,25)), 40 walls, 26 bushes.
- 2 monoliths, each 200 gems, 300 gold, 150-round respawn: (17,31) at distance 27 from our spawn and 28 from theirs, and (32,18), the mirror.
- 12 gold groups (20 gold camps, 50-round respawn): 4 groups worth 105 gold near the centre and 8 worth 35 gold nearer the edges. No gem camps, no wolves, no Elk.
- Ladder (scraper, 18 Oct): winners reach 1,000 gems in a median of 1,602 rounds (top 50, n 124); 56.3 kills a game on pre-patch data.

## Results

| Bot | Opponent | Side | Result | Gems (ours/theirs) | Kills/deaths | Gold (ours/theirs) |
|---|---|---|---|---|---|---|
| v1 (no contest plan) | foundry-v16 | A | win on gold | 0/0 | 32/41 | 10,385/10,175 |
| ct0: v4.14 draft, chip the monolith | foundry-v16 | A | loss | 0/400 | 11/80 | 1,570/18,625 |
| ct3: Ryth, Marcella, Willow, Stellar, Varg, chip | foundry-v16 | A | loss | 0/400 | 7/69 | 1,335/18,045 |
| ct3 with a finish rule (stage at 5 to 6 tiles below 3,000 health, kill with 3 healthy allies) | foundry-v16 | A | loss | 0/200 | 10/42 | |
| same | foundry-v16 | B | loss on gold | 0/0 | 2/87 | |
| same | v4.14 | A | loss | 0/400 | 8/22 | |
| same | v4.14 | B | loss on gold | 0/0 | 9/36 | |

## Monolith health trace (ct3 vs foundry-v16, side A, logged every 25 rounds)

20,000 at the start; 16,330 at round 75; 9,567 at 350; 4,134 at 700; 758 at 825; dead at 925 and captured by foundry-v16. Respawned by 1,175 at 18,396 (already hit); 1,600 by 1,800; captured by foundry-v16 again near 1,950.

## Readings (interpretations, to test)

1. The monolith keeps its damage between visits, so chipping over several trips works.
2. The team that chips does the work and the opponent takes the capture: both panel bots captured monoliths that our champions had worn down, including v4.14, whose target choice skips monoliths; its kill probably comes from `fight_any`, which attacks any camp within 4 tiles (unverified).
3. Deaths rose from 41 to 69 to 87 a game: champions under the monolith (200 Strength a hit) are easy kills for passing enemies, and kills feed the enemy's gold, which also decides 0-to-0 games.
4. The League of Legends Baron is the nearest analogy: a team starts Baron only with vision of the area and the enemy jungler accounted for, and the last hit (smite) decides the steal. Candidate cambc version: wards around the monolith, chip only while no enemy is seen within 8 tiles, keep the burst (stuns and damage spells) for the final 1,500 health, and hold the spawn tiles with every healthy champion for the 5-round capture.

## Draft file

`candidates/map_contest_v2.cpp` is the last version tested (the finish rule). It is a starting point with known losses, built for v1's hooks; `CONTEST_TRACE 1` in a `contest_cfg.h` prints the monolith trace.
