"""Map-controlled pair interactions: logit(P(A wins)) = elo offset + side*map + champion*map (ridge) + pair (ridge). Cluster bootstrap by group."""
import numpy as np, pandas as pd, json
from scipy.optimize import minimize
R = 'C:/cambc-solo/'
df = pd.read_csv(R + 'intel/matches-2026-10-18.csv')
CH = ['Ryth', 'Vesper', 'Vendetta', 'CassiIssac', 'Stellar', 'Prism', 'Marcella', 'Willow', 'Ren', 'Varg', 'Tavi']
ci = {c: i for i, c in enumerate(CH)}


def vec(s):
    v = np.zeros(11, bool)
    for c in s.split('+'):
        v[ci[c]] = True
    return v


A = np.stack([vec(s) for s in df.draft_a])
B = np.stack([vec(s) for s in df.draft_b])
n = len(df)
off = np.log(10) / 400 * (df.rating_a.values - df.rating_b.values)
y = (df.winner == 'a').values.astype(float)
maps = pd.factorize(df.map_id)[0]
nm = maps.max() + 1
gid = pd.factorize(df.group_id)[0]
ng = gid.max() + 1
dA, dB = A & ~B, B & ~A
dX = dA.astype(float) - dB.astype(float)
side = np.zeros((n, nm))
side[np.arange(n), maps] = 1
mc = np.hstack([dX[:, [i]] * side for i in range(11)])
pidx = [(i, j) for i in range(11) for j in range(i + 1, 11)]
Pm = np.stack([(dA[:, i] & dB[:, j]).astype(float) - (dB[:, i] & dA[:, j]).astype(float) for i, j in pidx], 1)
X = np.hstack([side, dX, mc, Pm])
l2 = np.concatenate([np.full(nm, 0.5), np.full(11, 20.0), np.full(11 * nm, 20.0), np.full(55, 8.0)])


def fit(Xm, offm, ym, wts=None):
    p = Xm.shape[1]
    w_ = np.ones(len(ym)) if wts is None else wts

    def f(w):
        z = offm + Xm @ w
        pr = 1 / (1 + np.exp(-z))
        ll = -(w_ * (ym * np.log(pr + 1e-12) + (1 - ym) * np.log(1 - pr + 1e-12))).sum() + 0.5 * (l2 * w ** 2).sum()
        g = Xm.T @ (w_ * (pr - ym)) + l2 * w
        return ll, g
    return minimize(f, np.zeros(p), jac=True, method='L-BFGS-B').x


w0 = fit(X, off, y)
pair0 = w0[-55:]
rng = np.random.default_rng(21)
B_ = 150
bs = np.zeros((B_, 55))
for b in range(B_):
    cnt = np.bincount(rng.integers(0, ng, ng), minlength=ng)[gid].astype(float)
    bs[b] = fit(X, off, y, cnt)[-55:]
sd = bs.std(0)
rows = []
from scipy.stats import norm
for q, (i, j) in enumerate(pidx):
    z = pair0[q] / sd[q] if sd[q] > 0 else 0
    p = 2 * (1 - norm.cdf(abs(z)))
    rows.append(dict(a=CH[i], b=CH[j], games=int(np.abs(Pm[:, q]).sum()), logit=float(pair0[q]), pp=float(25 * pair0[q]), lo_pp=float(25 * np.percentile(bs[:, q], 2.5)), hi_pp=float(25 * np.percentile(bs[:, q], 97.5)), p=float(p)))
d = pd.DataFrame(rows).sort_values('p')
m = len(d)
thr = 0
for rank, pv in enumerate(d.p.values, 1):
    if pv <= 0.05 * rank / m:
        thr = rank
d['bh05'] = [r <= thr for r in range(1, m + 1)]
d.to_csv(R + 'data/v3/matchups/draft_pairs_mapctrl.csv', index=False)
print(d.head(12).to_string())
print('cells', m, 'raw CI excludes 0:', int(((d.lo_pp > 0) | (d.hi_pp < 0)).sum()), 'BH:', int(d.bh05.sum()), 'expected false positives at 5%:', 0.05 * m)
