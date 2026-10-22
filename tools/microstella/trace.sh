#!/bin/bash
# trace.sh <botA> <botB> <map> <outdir>  -> extracts team-a/team-b stdout T lines
A=$1; B=$2; M=$3; O=$4; mkdir -p $O
cambc run bots/$A bots/$B --map maps/$M.map26a --release --log-limit 50000000 > $O/run.log 2>&1
grep -E "wins" $O/run.log
P=$(grep "replay bundle" $O/run.log | awk '{print $3}')
cd $O && unzip -q -o "$P" && grep -ao "T r[0-9]* .*" team-a.stdout > ta.txt; grep -ao "T r[0-9]* .*" team-b.stdout > tb.txt; wc -l ta.txt tb.txt
