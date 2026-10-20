#!/bin/bash
# Unopposed run on chosen maps (vs idle, both sides, salts 0,1,2). usage: bash tools/s1/maps.sh <bot> "<maps>" [P_X=v ...]
set -e
cd "$(dirname "$0")/../.."
b=$1; m=$2; shift 2
if [ $# -gt 0 ]; then python3 tools/s1/variant.py "$b" "$@"; fi
python3 scripts/mapgate.py "$b" $m --jobs 3 --salts ${SALTS:-0,1,2,3,4,5} --panel idle --out results/s1/m-$b.json > results/s1/m-$b.log 2>&1
