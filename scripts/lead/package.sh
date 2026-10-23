#!/bin/bash
# package.sh: builds /home/claude/out/cambc-lead-<stamp>.zip for the random2 repo (no replays, no SDK, no binaries).
# Layout inside the zip mirrors repo paths: project/lead/, scripts/lead/, bots/census/, tools/microstella/pace.py, data/lead/, results/lead/.
set -eu
H=/home/claude
STAMP=${1:-$(date +%Y%m%d-%H%M)}
P=$H/out/pkg-$STAMP
mkdir -p $P/project/lead $P/scripts/lead $P/bots/census/src $P/tools/microstella $P/data/lead/readers $P/results/lead
cp $H/state/CONTEXT.md $H/state/LEDGER.md $P/project/lead/
[ -f $H/state/HANDOFF.md ] && cp $H/state/HANDOFF.md $P/project/lead/
cp $H/scripts/*.py $H/scripts/*.sh $P/scripts/lead/
cp $H/bots/census/src/main.cpp $P/bots/census/src/
cp $H/tools/microstella/pace.py $P/tools/microstella/
cp $H/data/ENGINE-FACTS.json $H/data/census-groups.json $H/data/designs-1.0.2.json $H/data/designs-1.0.3.json $P/data/lead/
cp $H/data/readers/dump-1.0.3.json $P/data/lead/readers/
cp $H/data/MAPS.csv $H/data/MAP-GROUPS.csv $P/data/lead/
mkdir -p $P/data/lead/renders && cp $H/data/renders/*.png $P/data/lead/renders/
cp -r $H/data/readers/reach $P/data/lead/readers/
for d in $H/runs/*/; do n=$(basename $d); [ -f $d/summary.json ] || continue; mkdir -p $P/results/lead/$n; cp $d/summary.json $d/games.csv $P/results/lead/$n/ 2>/dev/null || true; done
mkdir -p $P/results/lead/arena && cp $H/runs/arena/arena.log $H/runs/arena/an_range.txt $H/runs/arena/an_target.txt $P/results/lead/arena/ 2>/dev/null || true
(cd $P && zip -q -r $H/out/cambc-lead-$STAMP.zip .)
ls -la $H/out/cambc-lead-$STAMP.zip
