import sys, collections
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
from a2_common import *
rows = load_fights()
print(len(rows))


def pres_n(r, t, rad):
    return sum(1 for p in r['pres'] if p['t'] == t and p['d'] <= rad)


# clean 1v1 within 8
c = collections.Counter()
mat = collections.defaultdict(lambda: [0, 0, 0, 0])  # a kills b, b kills a, neither, both
for r in rows:
    p0 = [p for p in r['pres'] if p['t'] == 0]
    p1 = [p for p in r['pres'] if p['t'] == 1]
    if len(p0) == 1 and len(p1) == 1 and r['dmg']['0'] > 0 and r['dmg']['1'] > 0:
        a, b = p0[0]['ty'], p1[0]['ty']
        d0 = len(r['deaths']['0']) > 0
        d1 = len(r['deaths']['1']) > 0
        c[(d0, d1)] += 1
print('clean 1v1 exchange outcomes (t0 died, t1 died):', c)
# numbers table by present within 6
tab = collections.defaultdict(lambda: [0, 0, 0])
for r in rows:
    for t in (0, 1):
        nu, nt = pres_n(r, t, 6), pres_n(r, 1 - t, 6)
        kd = kill_diff(r, t)
        k = (min(nu, 4), min(nt, 4))
        tab[k][0 if kd > 0 else (1 if kd < 0 else 2)] += 1
for k in sorted(tab):
    w, l, tie = tab[k]
    print(k, 'win', w, 'loss', l, 'tie', tie, 'win/(w+l)=%.3f' % (w / max(1, w + l)))
# initiator
ini = collections.Counter()
for r in rows:
    t = r['init_team']
    kd = kill_diff(r, t)
    ini[(r['src'], 'win' if kd > 0 else ('loss' if kd < 0 else 'tie'))] += 1
print(ini)
