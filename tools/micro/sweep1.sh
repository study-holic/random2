#!/usr/bin/env bash
# w14 sweep 1: all 11 picks; world A = v2 retreat on both sides, world B = both fight to the death (bit 1).
cd /c/cambc-solo
run() { python tools/micro/lab.py run --tag "$1" --polA "$2" --polB "$3" --picks all --jobs 2; }
run A_base 0 0
run B_base 1 1
for v in 1 2 4 8 16 32 64 128 256 512 1024 4096; do run A_v$v $v 0; done
for v in 2 4 8 16 32 64 128 256 512 4096; do run B_v$v $((v|1)) 1; done
echo SWEEP1 DONE
