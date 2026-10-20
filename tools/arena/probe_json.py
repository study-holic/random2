"""Raw data for tests 1-4 (bots/arena-probe on maps-arena/p*.map26a) -> results/arena-1020-probe.json."""
import glob, json, os, sys
sys.path.insert(0, 'tools/arena')
from ev import lines
from an_range import run as range_rows
from an_target import timeline
out = {'range': {}, 'target': {}, 'events': {}}
for n, name, W in ((0, 'wolf', 31), (1, 'elk', 33), (2, 'monolith', 35)):
    out['range'][name] = {str(s): rows for s, rows in range_rows(glob.glob(f'matches/arena/p0{n}_*')[0], (16, 2), W).items()}   # (round, distance, damage)
for n in range(4, 9):
    T = timeline(glob.glob(f'matches/arena/p0{n}_*')[0], (15 + n, 15))
    out['target'][f'p0{n}'] = {r: v for r, v in T.items() if 38 <= r <= 90}
for d in sorted(glob.glob('matches/arena/p*')):
    if os.path.basename(d)[:3] in ('p03', 'p09', 'p10', 'p11', 'p12', 'p13'):
        keep = []; last = {}
        for side in (0, 1):
            for l in lines(d, side):
                t = l.split()
                if t[0] in ('GI', 'G', 'A') or 'error' in l: keep.append(f'{side}|{l}'); continue
                if t[0] == 'L':
                    k = (side, t[3]); v = tuple(t[4:])
                    if last.get(k) != v: keep.append(f'{side}|{l}'); last[k] = v
        out['events'][os.path.basename(d)] = keep
json.dump(out, open('results/arena-1020-probe.json', 'w'))
print('ok', {k: len(v) for k, v in out.items()}, os.path.getsize('results/arena-1020-probe.json'))
