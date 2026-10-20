import sys, collections, math, json, random
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
from a2_common import *
import numpy as np
rows = load_fights()
OUT = R + 'data/v3/matchups/'
# --- 1v1 participant fights, exchange
duel = collections.defaultdict(lambda: dict(n=0, a_kill=0, b_kill=0, none=0, both=0, net=[], games=set(), dealt_a=0, dealt_b=0))
cnt11 = collections.Counter()
for r in rows:
    P0, P1 = r['parts']['0'], r['parts']['1']
    if len(P0) != 1 or len(P1) != 1:
        continue
    a, b = P0[0], P1[0]
    ex = a['dealt'] > 0 and b['dealt'] > 0
    cnt11[ex] += 1
    if not ex:
        continue
    d0 = len(r['deaths']['0']) > 0
    d1 = len(r['deaths']['1']) > 0
    # orient so that first type index <= second (unordered) ; a-side is the lower type id
    for (x, y, dx, dy, ddx, ddy) in ((a, b, d0, d1, a['dealt'], b['dealt']), (b, a, d1, d0, b['dealt'], a['dealt'])):
        if x['ty'] > y['ty']:
            continue
        key = (x['ty'], y['ty'])
        c = duel[key]
        if x['ty'] == y['ty'] and x is b:
            continue
        c['n'] += 1
        c['games'].add(r['game'])
        if dy and not dx:
            c['a_kill'] += 1
        elif dx and not dy:
            c['b_kill'] += 1
        elif dx and dy:
            c['both'] += 1
        else:
            c['none'] += 1
        c['dealt_a'] += ddx
        c['dealt_b'] += ddy
print('1v1 participants: exchange', cnt11)
rowsout = []
for (x, y), c in sorted(duel.items()):
    dec = c['a_kill'] + c['b_kill']
    lo, hi = wilson(c['a_kill'], dec)
    tot = c['dealt_a'] + c['dealt_b']
    rowsout.append(dict(a=NAMES[x], b=NAMES[y], fights=c['n'], games=len(c['games']), a_kills=c['a_kill'], b_kills=c['b_kill'], neither=c['none'], both=c['both'],
                        a_share_decisive=(c['a_kill'] / dec if dec else None), lo=lo, hi=hi, dmg_share_a=(c['dealt_a'] / tot if tot else None)))
json.dump(rowsout, open(OUT + 'fight_duel_matrix.json', 'w'))
import csv
with open(OUT + 'fight_duel_matrix.csv', 'w', newline='') as f:
    w = csv.DictWriter(f, fieldnames=list(rowsout[0].keys()))
    w.writeheader()
    for r in rowsout:
        w.writerow(r)
big = [r for r in rowsout if r['a_kills'] + r['b_kills'] >= 10]
print('cells', len(rowsout), 'with >=10 decisive', len(big), 'with >=30 fights', sum(1 for r in rowsout if r['fights'] >= 30))
for r in sorted(big, key=lambda r: -(r['a_kills'] + r['b_kills']))[:40]:
    print(r['a'], 'v', r['b'], 'fights', r['fights'], 'a_kills', r['a_kills'], 'b_kills', r['b_kills'], 'neither', r['neither'], 'a_share %.2f [%.2f,%.2f]' % (r['a_share_decisive'], r['lo'], r['hi']), 'dmg_share_a %.2f' % r['dmg_share_a'])
