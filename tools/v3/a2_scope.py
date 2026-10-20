import sys, collections
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
from a2_common import *
import numpy as np
rows = load_fights()
g = {}
for r in rows:
    g.setdefault(r['game'], r)
top = [r for r in g.values() if not is_ours(r)]
ours = [r for r in g.values() if is_ours(r)]
for nm, L in (('top-10 mutual', top), ('ours', ours)):
    ts = sorted(r['t'] for r in L if r['t'])
    teams = set(t for r in L for t in (r['teamIds'] or []))
    rt = [x for r in L for x in (r['ratings'] or [])]
    print(nm, 'games with >=1 fight', len(L), 'teams', len(teams), 'time', ts[0], '->', ts[-1], 'mean rating %.0f' % np.mean(rt), 'maps', len(set(r['map'] for r in L)))
fl = [r for r in rows]
print('fights total', len(fl), '| r0>3', sum(1 for r in fl if r['r0'] > 3))
oo = [r for r in g.values() if is_ours(r)]
# our rating vs opponent rating
d = []
for r in oo:
    o = r['teams'].index('xanarchy')
    d.append((r['ratings'][o], r['ratings'][1 - o]))
d = np.array(d)
print('ours: our rating mean %.0f, opponent mean %.0f' % (d[:, 0].mean(), d[:, 1].mean()))
mp = collections.Counter(r['map'] for r in fl if r['r0'] > 3)
print(mp.most_common())
sz = collections.Counter()
for r in fl:
    if r['r0'] <= 3: continue
    sz[(len(r['parts']['0']) + len(r['parts']['1']))] += 1
print('participants per fight', sorted(sz.items()))
