#!/bin/bash
# S0 baseline queue: engine 1.0.3 (cambc 3.0.5, ws305) first, then engine 1.0.2 (cambc 3.0.4, ws).
set -u
C5=/home/claude/v305/bin/cambc; C4=/usr/local/bin/cambc
B=/home/claude/scripts/baseline.py
run() { python3 $B --cambc "$1" --ws "$2" --set "$3" --a "$4" --b "$5" --jobs 2 >> /home/claude/runs/queue.log 2>&1; echo "done $3 $(date +%H:%M)" >> /home/claude/runs/queue.log; }
mkdir -p /home/claude/runs
run $C5 /home/claude/ws305 f1-v4-e103   v4     starter-cpp
run $C5 /home/claude/ws305 f1-v5r-e103  v5r    starter-cpp
run $C5 /home/claude/ws305 f1-v22-e103  ms-v22 starter-cpp
run $C5 /home/claude/ws305 h2h-v5r-v4-e103  v5r    v4
run $C5 /home/claude/ws305 h2h-v5r-v22-e103 v5r    ms-v22
run $C5 /home/claude/ws305 h2h-v22-v4-e103  ms-v22 v4
run $C4 /home/claude/ws    f1r-v4-e102  v4     starter-repo
run $C4 /home/claude/ws    f1-v4-e102   v4     starter-cpp
run $C5 /home/claude/ws305 f1r-v4-e103  v4     starter-repo
run $C5 /home/claude/ws305 f1r-v5r-e103 v5r    starter-repo
run $C5 /home/claude/ws305 f1r-v22-e103 ms-v22 starter-repo
echo "QUEUE COMPLETE $(date +%H:%M)" >> /home/claude/runs/queue.log
