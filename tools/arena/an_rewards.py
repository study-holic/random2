"""Rewards: per side, each gems/gold change of a champion with the round, plus group state changes and deaths."""
import sys, glob
sys.path.insert(0, 'tools/arena'); from ev import lines
d = sys.argv[1] if len(sys.argv) > 1 else glob.glob('matches/arena/p03_*')[0]
for side in (0, 1):
    last = {}; ev = []; gi = {}
    for l in lines(d, side):
        t = l.split()
        if t[0] == 'GI': gi[t[2]] = t[3:]
        if t[0] == 'G' and t[4] in '12' and t[5] in ('0', '50', '40', '60'): ev.append(f"G{t[3]}:{t[4]}@{t[1]}")
        if t[0] == 'L' and len(t) == 9:
            k = t[3]; v = (int(t[7]), int(t[8]))
            if k in last and last[k] != v: ev.append(f"k{k}:+{v[0]-last[k][0]}gem+{v[1]-last[k][1]}g@{t[1]}")
            last[k] = v
        if t[0] == 'L' and t[-1] == 'dead': ev.append(f"k{t[3]}:dead@{t[1]}")
    print('side', side, 'groups(gold gems respawn n x y)', {k: v for k, v in gi.items() if side == 0})
    print(' ', ' '.join(ev))
