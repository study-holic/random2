"""Isolated-duel kill matrix from round-level kill events (data/v3/kills_round_level.jsonl). A duel = victim has exactly one enemy UNIT within 6 tiles
and no ally within 6 (C&I counts as one unit); the killer is that unit."""
import sys, json, math, collections, csv
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
from a2_common import *
import numpy as np
from sklearn.linear_model import LogisticRegression
R_ = 'C:/cambc-solo/'
OUT = R_ + 'data/v3/matchups/'
ev = [json.loads(l) for l in open(R_ + 'data/v3/kills_round_level.jsonl', encoding='utf8')]
print('death events (round-level, %d games)' % len(set(e['game'] for e in ev)), len(ev))
tot = len(ev)
withk = [e for e in ev if e['k'] is not None]
print('with an identified killer champion', len(withk))
U = lambda t: 3 if t in (3, 4) else t
iso = []
for e in withk:
    en = e['enem6']
    if len(set(U(t) for t, _, _ in en)) != 1 or e['ally6']:
        continue
    if U(e['k']['ty']) != U(en[0][0]) or e['k']['d'] > 6:
        continue
    iso.append(e)
print('isolated duel kills', len(iso), 'games', len(set(e['game'] for e in iso)))
UN = [0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11]
UNAME = {0: 'Ryth', 1: 'Vesper', 2: 'Vendetta', 3: 'C&I', 5: 'Stellar', 6: 'Prism', 7: 'Marcella', 8: 'Willow', 9: 'Ren', 10: 'Varg', 11: 'Tavi'}
K = collections.defaultdict(int)
for e in iso:
    K[(U(e['k']['ty']), U(e['v']['ty']))] += 1
rows = []
for i, x in enumerate(UN):
    for y in UN[i:]:
        a, b = K[(x, y)], K[(y, x)]
        if x == y:
            a = K[(x, y)]
            b = a
            continue
        n = a + b
        if n == 0:
            continue
        lo, hi = wilson(a, n)
        rows.append(dict(a=UNAME[x], b=UNAME[y], a_kills=a, b_kills=b, n=n, a_share=a / n, lo=lo, hi=hi))
with open(OUT + 'duel_kill_matrix.csv', 'w', newline='') as f:
    w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
    w.writeheader()
    for r in rows:
        w.writerow(r)
print('pairs with data', len(rows), ' with n>=30:', sum(1 for r in rows if r['n'] >= 30), ' with n>=10:', sum(1 for r in rows if r['n'] >= 10))
for r in sorted(rows, key=lambda r: -r['n'])[:30]:
    print('%-9s v %-9s a_kills %3d b_kills %3d n=%3d share_a %.2f [%.2f, %.2f] %s' % (r['a'], r['b'], r['a_kills'], r['b_kills'], r['n'], r['a_share'], r['lo'], r['hi'], 'SIG' if (r['lo'] > 0.5 or r['hi'] < 0.5) else ''))
# BT with controls
Xs, ys, gs = [], [], []
for e in iso:
    k, v = e['k'], e['v']
    rt = e['ratings'] or [0, 0]
    kteam = 1 - e['vteam']
    gap = (rt[kteam] - rt[e['vteam']]) / 400.0
    for sgn, (p, q, rg) in ((1, (k, v, gap)), (0, (v, k, -gap))):
        x = np.zeros(11 + 3)
        x[UN.index(U(p['ty']))] += 1
        x[UN.index(U(q['ty']))] -= 1
        x[11] = math.log(max(p['hp'], 30) / max(q['hp'], 30))
        x[12] = (p['str'] - q['str']) / 50.0
        x[13] = rg
        Xs.append(x)
        ys.append(sgn)
        gs.append(e['game'])
Xs = np.array(Xs)
ys = np.array(ys)
mdl = LogisticRegression(C=2.0, fit_intercept=False, max_iter=3000).fit(Xs, ys)
games = sorted(set(gs))
gi = {g: i for i, g in enumerate(games)}
gidx = np.array([gi[g] for g in gs])
rng = np.random.default_rng(9)
bs = []
for _ in range(300):
    pick = rng.integers(0, len(games), len(games))
    w = np.bincount(pick, minlength=len(games))[gidx]
    keep = w > 0
    m = LogisticRegression(C=2.0, fit_intercept=False, max_iter=3000).fit(Xs[keep], ys[keep], sample_weight=w[keep].astype(float))
    bs.append(m.coef_[0])
bs = np.array(bs)
names = [UNAME[u] for u in UN] + ['log hp ratio', 'strength diff/50', 'rating gap/400']
c = mdl.coef_[0]
cm = c[:11].mean()
res = {}
print('\nBT strength in isolated duels (logit, centred; + = more likely to be the killer), controls HP ratio (round before death), strength, rating gap; n duels =', len(iso))
for j in range(14):
    val = c[j] - (cm if j < 11 else 0)
    lo, hi = np.percentile(bs[:, j], [2.5, 97.5]) - (cm if j < 11 else 0)
    ninv = int((np.abs(Xs[:, j]) > 0).sum() // 2) if j < 11 else len(iso)
    res[names[j]] = dict(coef=float(val), lo=float(lo), hi=float(hi), duels=ninv)
    print('%-18s %6.2f [%6.2f, %6.2f] duels %d' % (names[j], val, lo, hi, ninv))
json.dump(res, open(OUT + 'duel_kill_bt.json', 'w'), indent=1)
# by cohort: ours vs top
for nm, sel in (('top-10 mutual games', [e for e in iso if e['src'] == 's13' and 'xanarchy' not in (e['teams'] or [])]), ('xanarchy games', [e for e in iso if 'xanarchy' in (e['teams'] or [])])):
    print(nm, 'isolated duel kills', len(sel))

# ---- class-level view
CLS = {0: 'melee', 1: 'ranged', 2: 'melee', 3: 'C&I', 5: 'ranged', 6: 'ranged', 7: 'melee', 8: 'ranged', 9: 'melee', 10: 'melee', 11: 'ranged'}
cc = collections.Counter()
for e in iso:
    cc[(CLS[U(e['k']['ty'])], CLS[U(e['v']['ty'])])] += 1
print('\nclass-level isolated duel kills (killer class, victim class):', dict(cc))
for a_, b_ in (('ranged', 'melee'), ('C&I', 'melee'), ('C&I', 'ranged')):
    x, y_ = cc[(a_, b_)], cc[(b_, a_)]
    lo, hi = wilson(x, x + y_)
    print('%s kills %s: %d vs %d  share %.2f [%.2f, %.2f]' % (a_, b_, x, y_, x / (x + y_), lo, hi))
# HP-ratio check: share of killers who had more HP fraction
hp_adv = [(e['k']['hp'] / max(1, e['k']['mx'])) > (e['v']['hp'] / max(1, e['v']['mx'])) for e in iso]
print('killer had higher HP fraction in %.0f%% of isolated duel kills (n=%d)' % (100 * np.mean(hp_adv), len(iso)))
gear = [(e['k']['str'] > e['v']['str']) for e in iso if e['k']['str'] != e['v']['str']]
print('killer had higher strength in %.0f%%' % (100 * np.mean(gear)))
