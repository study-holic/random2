# Cloud parity test (20 Oct 2026, 06:57 UTC)

## Environment
- Linux 6.18 container, 4 cores, 15 GB RAM; Python 3.13.16, Node 22.22.0
- cambc 3.0.1 (pip, already installed), engine 1.0.0, `sdk_version = "3.0.1"`, maps set da4707523852 (21)
- Toolchain: the first build tried to download wasi-sdk 33.0, cmake 3.31.8 and ninja 1.12.1 from github.com and failed with
  `io: invalid peer certificate: UnknownIssuer`. The CLI's Rust downloader does not trust the agent proxy CA. Workaround: fetch the same
  three release archives with curl and unpack them into `~/.cache/cambc/` as `wasi-sdk-33/`, `cmake-3.31.8-linux-x86_64/` and
  `ninja-1.12.1-linux-x86_64/ninja`. The builds then pass. A setup script for each new container should do the same.

## Timings
- Build (release, cold cache): starter-cpp 6 s, v3b 50 s
- Gate (42 games, `--jobs 3`): 42.8 s of games, 43 s wall, about 1.0 s per game (3 in parallel, so about 3.1 s of CPU per game)
- Laptop reference: 68.2 s with `--jobs 2`, about 1.6 s per game

## Comparison
Cloud `results/gate-v3b-cloud-0657*.json` against laptop `results/gate-v3b-0657*.json`, matched by (map, side), 42 against 42 games.
Fields compared: win, round, gems, gems_opp, faults, faults_opp, fuel_max, fuel_max_r1, fuel_draft_max, codes, kills, deaths, gold, gold_opp, engine, nch.
Differences: **0**. Same verdict on both: FAIL, 36/42, the same 6 losses and the same 6 games with fuelExhausted after round 1.

Note: gate.py names output by local `%H%M`. The cloud clock is UTC, so this run reused the laptop's name `0657` (BST) and overwrote it.
The laptop files were restored from git and the cloud ones renamed `-cloud-`.

## Verdict: IDENTICAL
