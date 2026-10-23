#!/bin/bash
# arena_b1.sh: B1 arena batch on engine 1.0.3 (cambc 3.0.5), release builds unless stated.
# Rows settled without the arena (KNOWN from the census constants dump, data/ENGINE-FACTS.json):
#   B1a sleep delay (camp_sleep_time 2), B1a camp_damage_multiplier and growth, B1b #7 ward range (ward_vision_radius 6),
#   B1b #10 team size (Cassi and Issac: one pick, two bodies), B1b #13 golem placement (six maps), group values incl. multipliers.
# Part 1 runs the probes that already exist; Part 2 lists the scenarios S1 adds to bots/arena-probe (tools/arena/mkprobe.mjs).
set -u
R=/home/claude/src/random2-main          # random2 checkout (read-only source)
WS=/home/claude/ws305                    # cambc 3.0.5 workspace
C=/home/claude/v305/bin/cambc
OUT=/home/claude/runs/arena
export SSL_CERT_FILE=/root/.ccr/ca-bundle.crt CAMBC_NO_UPDATE_CHECK=1
mkdir -p $OUT
log() { echo "$(date +%H:%M) $*" | tee -a $OUT/arena.log; }

copybot() { # copybot <src-name> <dst-name> [define-line]
  mkdir -p $WS/bots/$2; cp -r $R/bots/$1/. $WS/bots/$2/; rm -rf $WS/bots/$2/.cambc
  if [ $# -ge 3 ]; then f=$(ls $WS/bots/$2/src/*.cpp | head -1); { echo "$3"; cat "$f"; } > "$f.new" && mv "$f.new" "$f"; fi
}
runone() { # runone <botA> <botB> <map path> <outdir> [extra cambc args]
  mkdir -p "$4"; (cd $WS && $C run "$1" "$2" --map "$3" -o "$4" "${@:5}" > "$4/run.log" 2>&1); log "ran $1 vs $2 on $(basename $3) -> $4"
}

# ---- Part 1a. arena-probe on the 20 Oct probe maps (B1a rows: range, rewards, targeting, respawn, contest, adjacency, Monolith regen)
copybot arena-probe arena-probe
for m in $R/maps-arena/p*.map26a; do n=$(basename $m .map26a); runone arena-probe arena-probe $m $OUT/$n --release; done
mkdir -p /home/claude/runs/arena-an && cd $R
for p in p00_range_wolf p01_range_elk p02_range_monolith; do python3 -I tools/arena/an_range.py $OUT/$p > $OUT/$p/an.txt 2>&1; done
python3 -I tools/arena/an_rewards.py $OUT/p03_rewards > $OUT/p03_rewards/an.txt 2>&1
for p in p04_target4 p05_target5 p06_target6 p07_target7 p08_target8; do python3 -I tools/arena/an_target.py $OUT/$p > $OUT/$p/an.txt 2>&1; done
for d in $OUT/p*; do python3 -I tools/arena/ev.py $d > $d/ev.txt 2>&1; done

# ---- Part 1b. fuel overrun by build mode (B1a fuel row, B1b #3): burn 150k and 1.05M on round 50, debug and release
for mode in release debug; do
  for burn in 150000 1050000; do
    copybot fuelprobe fp-$mode-$burn "#define BURN_FUEL ${burn}ULL"
    extra=""; [ $mode = release ] && extra="--release"
    runone fp-$mode-$burn starter-cpp $WS/maps/00ambush.map26a $OUT/fuel-$mode-$burn $extra --rounds 120
    python3 -I $R/tools/arena/fuel.py $(ls $OUT/fuel-$mode-$burn/*.repl26a | head -1) > $OUT/fuel-$mode-$burn/fuel.txt 2>&1
  done
done

# ---- Part 1c. ms-lab: teleport crystal onto a wall (B1b #6), recall, capture timing (B1b #8)
copybot ms-lab ms-lab
for m in 01corridors 01boss-battle 00ambush; do runone ms-lab starter-cpp $WS/maps/$m.map26a $OUT/lab-$m --release --rounds 400; done

# ---- Part 1d. ms-elkprobe: wake scope (B1b #1), sleep timing and regeneration (B1a)
for mode in 0 1 2; do
  copybot ms-elkprobe elk-m$mode "#define MODE $mode"
  runone elk-m$mode starter-cpp $WS/maps/01corridors.map26a $OUT/elk-m$mode --release --rounds 300
done
log "part 1 complete"

# ---- Part 2. scenarios S1 adds to bots/arena-probe (each 2 to 6 games, both seats), one per open row:
#   B1b #1 wake scope        hit one camp of a 3-wolf group from range 3; log every member's awake state per round
#   B1b #2 first mover       both teams step towards one free tile each round for 20 rounds; log who occupies it
#   B1b #4 camp growth       idle until round 1,500, then read camp max_health and strength in vision; compare round 1
#   B1b #5 ranged move+attack Vesper: move then basic attack; Willow: attack, reset, attack; log can_attack and errors
#   B1b #8 capture timing    one champion arrives on a dead group's spawn tile; log rounds from arrival to reward
#   B1b #9 inventory caps    buy shards until buy_item_inventory_full or buy_item_stats_overflow; log counts
#   B1b #11 Marcella passive Marcella and an ally each tribute 10 gems; log both gold deltas
#   B1b #12 kill gold        Vesper kills an enemy champion; log gold delta (100 or 133)
#   B1b #13 golem rewards    clear a golem group on lost_world; log gems and gold paid against the group's census values
