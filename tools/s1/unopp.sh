#!/bin/bash
# Unopposed farming speed for one bot (vs idle, 21 maps x 2 sides x salts 0,1,2 = 126 games).
# usage: bash tools/s1/unopp.sh <bot> [P_X=v ...]   (with defines: builds bots/<bot> as a variant of s1 first)
set -e
cd "$(dirname "$0")/../.."
b=$1; shift
if [ $# -gt 0 ]; then python3 tools/s1/variant.py "$b" "$@"; fi
python3 scripts/mapgate.py "$b" all --jobs 3 --salts ${SALTS:-0,1,2,3,4,5} --panel idle --out results/s1/u-$b.json > results/s1/u-$b.log 2>&1
python3 tools/s1/diag.py results/s1/u-$b.json | tail -1
