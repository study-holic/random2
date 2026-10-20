"""HP/rating-adjusted champion strength in fights (Bradley-Terry style) and ours-vs-top investment comparison."""
import sys, json, math, collections
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
from a2_common import *
import numpy as np
from sklearn.linear_model import LogisticRegression
rows = [r for r in load_fights() if r['r0'] > 3]
OUT = R + 'data/v3/matchups/'
# ---------- 1v1 duels (participants), exchange, decisive; both orientations, antisymmetric features, no intercept
Xr, yr, gr, info = [], [], [], []
for r in rows:
    P0, P1 = r['parts']['0'], r['parts']['1']
    if len(P0) != 1 or len(P1) != 1:
        continue
    a, b = P0[0], P1[0]
    if not (a['dealt'] > 0 and b['dealt'] > 0):
        continue
    d0, d1 = len(r['deaths']['0']) > 0, len(r['deaths']['1']) > 0
    if d0 == d1:
        continue
    pa = next(p for p in r['pres'] if p['i'] == a['i'])
    pb = next(p for p in r['pres'] if p['i'] == b['i'])
    rt = r['ratings'] or [0, 0]
    for (x, y_, px, py, dx, dy, ex, ey, tx) in ((a, b, pa, pb, d0, d1, rt[0], rt[1], 0), (b, a, pb, pa, d1, d0, rt[1], rt[0], 1)):
        v = np.zeros(12 + 4)
        v[x['ty']] += 1
        v[y_['ty']] -= 1
        v[12] = math.log(max(px['hp'], 50) / max(py['hp'], 50))
        v[13] = (px['str'] - py['str']) / 50.0
        v[14] = (ex - ey) / 400.0
        v[15] = 1.0 if r['init_team'] == tx else -1.0
        Xr.append(v)
        yr.append(0 if dx else 1)  # x wins if x did not die
        gr.append(r['game'])
Xr = np.array(Xr)
yr = np.array(yr)
print('decisive 1v1 exchange duels (fights):', len(yr) // 2)
mdl = LogisticRegression(C=2.0, fit_intercept=False, max_iter=3000).fit(Xr, yr)
coef = mdl.coef_[0]
games = sorted(set(gr))
gi = {g: i for i, g in enumerate(games)}
gidx = np.array([gi[g] for g in gr])
rng = np.random.default_rng(5)
bs = []
for _ in range(300):
    pick = rng.integers(0, len(games), len(games))
    w = np.bincount(pick, minlength=len(games))[gidx]
    keep = w > 0
    m = LogisticRegression(C=2.0, fit_intercept=False, max_iter=3000).fit(Xr[keep], yr[keep], sample_weight=w[keep].astype(float))
    bs.append(m.coef_[0])
bs = np.array(bs)
names = NAMES + ['log hp ratio', 'strength diff/50', 'rating gap/400', 'initiator']
cnt = Xr[:, :12]
out = {}
print('type strength (logit, centred; positive = more likely to survive a 1v1 exchange duel), HP/strength/rating/initiator controlled')
cen = coef[:12] - coef[:12].mean()
for j in range(16):
    n_inv = int((np.abs(Xr[:, j]) > 0).sum() // 2) if j < 12 else len(yr) // 2
    lo, hi = np.percentile(bs[:, j], [2.5, 97.5])
    val = coef[j] - (coef[:12].mean() if j < 12 else 0)
    lo -= (coef[:12].mean() if j < 12 else 0)
    hi -= (coef[:12].mean() if j < 12 else 0)
    out[names[j]] = dict(coef=float(val), lo=float(lo), hi=float(hi), duels=n_inv)
    print('%-18s %6.2f [%6.2f, %6.2f]  duels %d' % (names[j], val, lo, hi, n_inv))
json.dump(out, open(OUT + 'duel_strength.json', 'w'), indent=1)

# ---------- ours vs top: investment and stats of fighters at fight start
def sidekey(r, t):
    if is_ours(r):
        return 'us' if r['teams'][t] == 'xanarchy' else 'opp'
    return 'top'


inv = {k: collections.defaultdict(list) for k in ('top', 'us', 'opp')}
allp = {k: [] for k in ('top', 'us', 'opp')}
seen = set()
for r in rows:
    for p in r['pres']:
        if not p['inf']:
            continue
        k = sidekey(r, p['t'])
        allp[k].append((p['ty'], p['pts'][0], p['pts'][1], p['pts'][2], p['mx'], p['str'], p['foc'], r['r0']))
print('\nfighter loadouts at fight start (shard-equivalent points: strength/focus/health), mean over fighter-fights; n = fighter-fights')
res = {}
for k in ('top', 'us', 'opp'):
    a = np.array([(x[1], x[2], x[3], x[4], x[5], x[6], x[7]) for x in allp[k]])
    res[k] = dict(n=len(a), str_pts=float(a[:, 0].mean()), foc_pts=float(a[:, 1].mean()), hp_pts=float(a[:, 2].mean()), maxhp=float(a[:, 3].mean()), strength=float(a[:, 4].mean()), focus=float(a[:, 5].mean()), round=float(a[:, 6].mean()))
    print(k, {kk: round(v, 1) for kk, v in res[k].items()})
# same, restricted to fights starting in round 100-400 (time-matched)
for lo_, hi_ in ((0, 200), (200, 400), (400, 2000)):
    print('rounds', lo_, hi_, end=': ')
    for k in ('top', 'us', 'opp'):
        a = np.array([(x[1], x[2], x[3], x[4]) for x in allp[k] if lo_ <= x[7] < hi_])
        print(k, 'n=%d str %.1f foc %.1f hp %.1f maxhp %.0f' % (len(a), a[:, 0].mean(), a[:, 1].mean(), a[:, 2].mean(), a[:, 3].mean()), end=' | ')
    print()
json.dump(res, open(OUT + 'loadouts.json', 'w'), indent=1)
