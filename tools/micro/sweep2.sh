#!/usr/bin/env bash
# w14 sweep 2 (bots/w14t: v2's lone-flee rule, team fights, race retreat, focus, heal, adaptive table).
cd /c/cambc-solo
r1() { python tools/micro/lab.py run --tag "$1" --polA "$2" --polB "$3" --picks all --jobs 2 --botA w14t --botB w14t; }
tm() { python tools/micro/lab.py team --tag "$1" --polA "$2" --polB "$3" --size "$4" --jobs 2 --bot w14t; }
r1 T1_base 0 0
r1 T1_race 16384 0
r1 T1_nr 1 0
r1 T1_adapt 32768 0
tm T2_base 0 0 2
tm T2_nr 1 0 2
tm T2_race 16384 0 2
tm T2_focus 2048 0 2
tm T2_heal 8192 0 2
tm T2_ec 2 0 2
tm T2_adapt 32768 0 2
tm T3_base 0 0 3
tm T3_race 16384 0 3
tm T3_focus 2048 0 3
tm T3_adapt 32768 0 3
echo SWEEP2 DONE
