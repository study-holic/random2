#!/bin/bash
# evalall.sh <bot> : full pool vs v19 v20 v16 v13, prints per-opponent and total wins
B=$1; ALL=$(ls maps | sed 's/.map26a//' | tr '\n' ' '); T=0
for O in v19 v20 v16 v13; do L=$(python3 pace.py $B $O $ALL | tail -1); echo "$L"; W=$(echo "$L" | sed 's/.*: \([0-9]*\)\/42.*/\1/'); T=$((T+W)); done
echo "TOTAL $B: $T/168"
