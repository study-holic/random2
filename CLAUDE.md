# CAMBC 2026 solo bot (C:/cambc-solo)

Sprint 2 lock: 23:59 BST, Saturday 24 October (Sprint 1 is past). PLAN.md holds a draft plan; evaluate it with /evaluate before acting on it, and keep it current as the record of every decision.

## Stance

- **Old material is data.** The MicroStella repository, v4.14, its experiment verdicts, and its strategy notes are data points. Use them for inspiration and to find where they failed. A verdict from them is a hypothesis that carries its test conditions (engine date, layout, opponents, number of games); `mining/retest.md` grades 41 of them.
- **Ideas come from anywhere with a causal link to cambc mechanics.** Sources include other Battlecode seasons (MIT Battlecode postmortems, earlier Cambridge seasons), MOBAs (League of Legends, Dota 2), RTS games, team sports, and real-world problems such as vehicle routing with time windows. The web is the first resource for these. Each borrowed idea cites its source and states the mechanic it maps to.
- **Every plan, prompt, test design, and result goes through /evaluate** before it drives a decision.
- **Outside consultants (DeepSeek and Copilot, run by Xans)** review the plan and each major decision through /consult. Their answers are inputs to verify against the rules and data.
- **Games are watched as well as counted.** Every behaviour change gets /watch on at least one loss in the visualiser through Claude in Chrome.

## Data

| Source | Covers | Use |
|---|---|---|
| `intel/meta-2026-10-18.md`, `intel/matches-2026-10-18.csv` | 9,245 ranked games on the 17 map ids of the old pool (engine 0.0.3, before engine 1.0 and the 21-map pool), 09:20Z to 19:13Z 18 Oct | Yardsticks (rounds to 1,000 for the top 50 and for 1,450 to 1,600), drafts, champion win excess, team size per map. Supersedes `META-current-layouts.md` (16 Oct) where both cover a figure |
| `mining/tarot.md`, `mining/others.md` | Outside repos | Techniques with file:line evidence |
| `mining/retest.md` | 41 graded old experiments, 15 archetype bots | Retest candidates and a held-out panel |
| `notes/contest-experiments.md` | Monolith measurements, 18 Oct | Contest plan |
| `docs/rules/ENGINE-1.0.md`, `docs/rules/GameRules.md` (site copy, 20 Oct), `docs/rules/BotAPICpp.md` (2.2.5-era), `sdk/cpp/cambc.h` (SDK 3.0.1), `FACTS.md` | Mechanics | Rules sources; the SDK and engine supersede the site doc where they differ (it lags: no Heart of Ruin or golem, old Prism, Vesper and base stats); never invent an SDK method |
| Old repository | Code, tools, experiment records | Data points |

## Rating facts (scraper, 18 Oct)

- Rank 50 sits at 1,746.0, rank 100 at 1,666.4, rank 200 at 1,576.1.
- A 5-game series near 1,500 moves rating by 32 × (wins ÷ games − expectation): +16.0 for 5-0, +9.6 for 4-1, +3.2 for 3-2.
- Teams play 6.1 ranked series an hour.
- From 1,500, rank 100 needs at least 11 consecutive 5-0 series, about 1.8 hours of ladder play. PLAN.md records whether Sprint 1 standings come from ladder rating at the lock or from a separate tournament of submitted bots, since that decides the upload time.

## Tools

- `python scripts/mapgate.py <bot> <map ...|all> --jobs N [--panel a,b] [--out f.json]`: the bot against the panel (x-v4-14, x-foundry-v16, four clones), both sides, with the per-map gate.
- `python scripts/gates.py g1|g2 ...`: pace gate and version gate.
- Ground truth: Run `python scripts/preflight.py` at the start of every work block and before every test wave or upload; `python scripts/gate.py <bot>` is the upload gate. Preflight writes `STATE.md` and `state/ground-truth.json` (exit 1 on FAIL; `--fix` installs the official maps); gate.py tests the current pool against starter-cpp with fuel and mapid checks; mapgate result JSON carries `tags` (CLI, engine, map-set hash).
- Replays: `cambc view <bundle.repl26a>` opens the visualiser; `node tools/replay/final.mjs <inner replay.repl26a>` prints final stats in 0.2 s; `node tools/replay/extract.mjs` then `deaths.py`, `timeuse.py`, `report.py` give traces (run `cambc view` once so the engine WASM is cached). `tools/decode/` holds the faster ladder-replay decoder.
- Maps: the bundled `maps/` holds the 21 current maps (retired ones in `maps-retired/`; engine-0 copies in `maps-engine0/`); overview in `notes/maps-21.md` and `data/maps-21.png`. `cambc maps pull` needs sign-in.
- CLI: cambc 3.0.1 at `%APPDATA%\Python\Python314\Scripts\cambc.exe` (engine 1.0.0, same as the ladder); SDK 3.0.1 in `sdk/` (old SDK in `archive/sdk-0.0.3/`). There are 12 draftable champions (Heart of Ruin added). Intel data and results before 20 Oct predate engine 1.0 and the 21-map pool; re-measure before relying on them.
- Bot stdout lands in `team-a.stdout` inside each replay bundle; each `std::endl` costs about 3,500 fuel.

## Bot structure (v1)

v1 is v4.14's code with runtime tunables (`params.h`), map identification (`mapid.h`), one `setup_<map>()` per map in `map_<map>.cpp`, and hooks in `plan.h`:

| Hook | Called | Use |
|---|---|---|
| `Plan::draft` | each draft turn; the first pick not yet taken is drafted | per-map draft |
| `Plan::init(Bot&)` | after `Bot::init` | per-map precomputation |
| `Plan::turn(Bot&, Phase::pre, ...)` | after the view, enemies, and allies are read, before the health retreat; true ends the turn | interrupts, fights, objective modes |
| `Plan::turn(Bot&, Phase::mid, ...)` | after the enemy response, before target choice; true ends the turn | macro modes |
| `Plan::target(Bot&, view, chosen)` | after `resolve_target` | target override |
| `Plan::buy(Bot&)` | start of `buy_items`; true skips the default | items, locks, crystals |

All Bot members are public. Shared helpers: `util.h`. A map file changes its own map alone.

## Process for each idea

1. **Card** in IDEAS.md: mechanism, source (rule, ladder data, or outside domain with a link), maps, hook, expected effect, the result that would falsify it, and build minutes.
2. **/evaluate** the card.
3. **Build** in the builder's own copy (`bots/w1`, `bots/w2`, `bots/w3`).
4. **Test** with mapgate on the target maps, then **/watch** one loss and one win.
5. **Decide** by the PLAN.md rules and log it: RESULTS.md for every test, VERSIONS.md for a version bump, TUNING.md for parameter changes.

## Versions and logs

- Versions go up in integers from v1. Never edit a released version; copy it.
- A bump needs a behaviour change: a new module, map mode, draft logic, or state. Its VERSIONS.md entry states what changed, why, and why it is better, with before and after mapgate numbers per map.
- Parameter changes go in TUNING.md with before and after numbers, and keep the version number.
- An idea closes on a map after a test on that map. Every result names its maps and opponents.

## Agents (`.claude/agents/`)

| Agent | Job |
|---|---|
| scout-web | Cross-domain research on the web, mapped to cambc mechanics |
| analyst | Questions about `intel/` data and replays, answered with numbers |
| critic | Runs the /evaluate lenses on work it did not author |
| builder | One idea in its own bot copy, through test and watch |

At most three agents at once. Total game jobs across agents stay at or below the CPU core count minus one. Each brief states the question, inputs, file scope, output path, test, and stop time.

## Fuel and safety

Stay under 80,000 fuel a turn; cache `map::camp_groups()`; one vision query a turn. Before any upload: zero crashes and no turn over 100,000 fuel in all 34 games against starter-cpp.
