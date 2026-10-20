# Watch (decoded): three v2 ladder losses, 01:50 BST 19 Oct

Replays downloaded with `cambc matches download <id>` (signed-in CLI) into `data/ladder/replays/`, decoded with `node tools/v3/extract_pilot.mjs --stride 1` into `data/ladder/decoded/`. The platform visualiser froze the Chrome tab (CPU loaded), so these are trace-based watches.
Enums (sdk/cpp/cambc.h): champion_type 0 Ryth, 1 Vesper, 2 Vendetta, 3 Cassi, 4 Issac, 5 Stellar, 6 Prism, 7 Marcella, 8 Willow, 9 Ren, 10 Varg, 11 Tavi (differs from champion_draft, where 3 = C&I, 4 = Stellar, 8 = Ren, 9 = Varg, 10 = Tavi). camp_type 0 wolf, 1 elk, 2 gem cluster, 3 gold cluster, 4 monolith.

## 1. Stampede vs I Am The Senate (1,422), v2 team 0, lost 0-1,080 at round 1,106 (`1a16c489`)

- Camps: 28 gold clusters (0 gems), 6 elks, 6 gem clusters. The elks and gem clusters form the only gem source: 6 × 36 + 6 × 24 = 360 gems a capture, split among the capturers.
- **Our champions carried 0 gems at every sampled round.** Gold rose to 500-900 each: the bot farmed gold clusters all game.
- The enemy tributed in 360-gem lumps (240 by r553, 360, 720, 1,080) after taking the big group three times, dying 11 times to its elks along the way (15 deaths, 4 to us).
- Cause: target choice never commits to the one gem group (the former teammate's "contested rule" report describes the same failure in their code). Sent to builder w10.

## 2. Contest vs Hidden Gem Thieves (1,415), v2 team 1, lost 0-1,000 at round 1,528 (`06490b46`)

- Camps: 2 monoliths (200 gems, 150-round respawn) and 20 gold clusters. The monoliths are the only gems.
- **v2 died 58 times, the enemy 8.** Enemy kills: Tavi 23, Ryth 9, Varg 6, Vesper 6. Our deaths cluster at (22-31, 22-31), the centre, one or two at a time (r60 Vendetta, r78 C&I, r190 Vendetta, r232 Ryth, r299 Vendetta, r339 Varg, r364 Ryth, ...): we trickle into a contested area and get picked off.
- The enemy took a monolith every ~130-250 rounds from r890 (200, 400, 600, 800, 1,000).

## 3. Monolith vs Micron (1,500), v2 team 1, lost 482-1,018 at round 450 (`4408dbb4`)

- Level at r149 (212-212). Then **Micron 0 deaths, v2 10 deaths, 0 kills**. Micron's Ren made 4 kills. Our first four deaths (r129-140) were in our own half near (29-39, 0-9).
- Micron's champions carry small bags (22-44 gems) and tribute often; from r149 to r450 they scored 806 gems (2.7 a round), we scored 270.

## Candidate causes for v3

1. Objective choice ignores map economics (stampede: the only gem group never taken; gold clusters valued as an alternative).
2. Engagement: champions enter contested areas alone and feed (contest 58 deaths; monolith 0-10). Fights are lost before they start.
Both are execution faults, not draft faults (Micron's draft is one pick from ours).
