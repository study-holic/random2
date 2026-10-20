#!/usr/bin/env bash
# w14 sweep 1b: the rest of World B (both fight to the death), bots/w14d.
cd /c/cambc-solo
run() { python tools/micro/lab.py run --tag "$1" --polA "$2" --polB "$3" --picks all --jobs 2; }
for v in 4 8 16 32 256 512 4096; do run B_v$v $((v|1)) 1; done
echo SWEEP1B DONE
