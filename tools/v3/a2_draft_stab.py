import numpy as np, pandas as pd
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
E = 1 / (1 + 10 ** ((df.rating_b.values - df.rating_a.values) / 400))
y = (df.winner == 'a').values.astype(float)
e = y - E
dA, dB = A & ~B, B & ~A


def cells(mask):
    S = np.zeros((11, 11))
    N = np.zeros((11, 11))
    for k in np.where(mask)[0]:
        for i in np.where(dA[k])[0]:
            for j in np.where(dB[k])[0]:
                S[i, j] += e[k]
                N[i, j] += 1
                S[j, i] -= e[k]
                N[j, i] += 1
    return S, N


t = pd.to_datetime(df.finished_at)
med = t.median()
splits = {'cohort a (top50) vs b (1450-1600)': (df.sample_a.values == 1, df.sample_b.values == 1), 'first half vs second half (time)': ((t < med).values, (t >= med).values)}
iu = [(i, j) for i in range(11) for j in range(i + 1, 11)]
for nm, (m1, m2) in splits.items():
    S1, N1 = cells(m1)
    S2, N2 = cells(m2)
    x1 = np.array([S1[i, j] / N1[i, j] if N1[i, j] >= 30 else np.nan for i, j in iu])
    x2 = np.array([S2[i, j] / N2[i, j] if N2[i, j] >= 30 else np.nan for i, j in iu])
    ok = ~np.isnan(x1) & ~np.isnan(x2)
    r = np.corrcoef(x1[ok], x2[ok])[0, 1]
    same = np.mean(np.sign(x1[ok]) == np.sign(x2[ok]))
    # restrict to cells with |x1|>=0.04
    big = ok & (np.abs(x1) >= 0.04)
    sb = np.mean(np.sign(x1[big]) == np.sign(x2[big])) if big.sum() else float('nan')
    print(nm, ': cells with n>=30 in both halves', ok.sum(), 'corr %.2f' % r, 'sign agreement %.0f%%' % (100 * same), '| cells with |excess|>=4pp in first: %d, sign agreement %.0f%%' % (big.sum(), 100 * sb))

# ---- per-cohort cells with cluster-bootstrap CI for the pooled top cells
gid = pd.factorize(df.group_id)[0]
rng = np.random.default_rng(11)
import json
pooled = pd.read_csv(R + 'data/v3/matchups/draft_pairs.csv')
top_cells = pooled.sort_values('p').head(10)
out = []
for _, row in top_cells.iterrows():
    i, j = ci[row.a], ci[row.b]
    rec = dict(a=row.a, b=row.b, pooled=row.exc_pp, pooled_n=int(row.games))
    for nm, m in (('top50', df.sample_a.values == 1), ('mid', df.sample_b.values == 1)):
        ks = np.where(m & dA[:, i] & dB[:, j])[0]
        ks2 = np.where(m & dB[:, i] & dA[:, j])[0]
        vals = list(e[ks]) + list(-e[ks2])
        gs = list(gid[ks]) + list(gid[ks2])
        vals = np.array(vals)
        gs = np.array(gs)
        n = len(vals)
        if n < 10:
            rec[nm] = (n, None, None, None)
            continue
        ug = np.unique(gs)
        idx = {u: np.where(gs == u)[0] for u in ug}
        bs = []
        for _ in range(500):
            pick = rng.choice(ug, len(ug))
            ii = np.concatenate([idx[u] for u in pick])
            bs.append(vals[ii].mean())
        rec[nm] = (n, 100 * vals.mean(), 100 * np.percentile(bs, 2.5), 100 * np.percentile(bs, 97.5))
    out.append(rec)
    print(rec['a'], 'v', rec['b'], 'pooled %.1f (n=%d)' % (rec['pooled'], rec['pooled_n']), '| top50 n=%s exc=%s [%s,%s]' % tuple(None if v is None else (round(v, 1) if isinstance(v, float) else v) for v in rec['top50']),
          '| mid n=%s exc=%s [%s,%s]' % tuple(None if v is None else (round(v, 1) if isinstance(v, float) else v) for v in rec['mid']))
json.dump(out, open(R + 'data/v3/matchups/draft_pairs_cohort.json', 'w'), indent=1, default=float)
