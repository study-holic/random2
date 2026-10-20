import sys, random
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
import a2_model as M
from a2_common import *
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import roc_auc_score
X, y, meta = M.prep(M.rows, M.label)
FE = M.FEATS
gap = np.array([((m['rating'][m['init_team']] - m['rating'][1 - m['init_team']]) / 400.0) if m['rating'] else 0.0 for m in meta])
print('instances', len(y), 'mean |gap| in Elo points %.0f' % (400 * np.abs(gap).mean()), 'corr(gap, d_pts6) %.2f' % np.corrcoef(gap, X[:, FE.index('d_pts6')])[0, 1])
teams = sorted(set(t for m in meta for t in (m['teams'] or [])))
rnd = random.Random(7)
rnd.shuffle(teams)
fold = {t: i % 5 for i, t in enumerate(teams)}


def cv(A, ret_coef=False):
    aucs = []
    for kf in range(5):
        trk = np.array([all(fold[t] != kf for t in (m['teams'] or [])) for m in meta])
        tek = np.array([any(fold[t] == kf for t in (m['teams'] or [])) for m in meta])
        mu, sd = A[trk].mean(0), A[trk].std(0) + 1e-9
        mdl = LogisticRegression(max_iter=3000).fit((A[trk] - mu) / sd, y[trk])
        aucs.append(roc_auc_score(y[tek], mdl.predict_proba((A[tek] - mu) / sd)[:, 1]))
    mu, sd = A.mean(0), A.std(0) + 1e-9
    mdl = LogisticRegression(max_iter=3000).fit((A - mu) / sd, y)
    return np.mean(aucs), mdl.coef_[0] / sd


idx = [FE.index(c) for c in ['d_hp6', 'd_hpfrac6', 'd_minhp', 'd_pts6']]
A0 = X[:, idx]
a, c0 = cv(A0)
print('LEAN4 auc %.3f coef' % a, np.round(c0, 3))
a, c1 = cv(np.hstack([A0, gap[:, None]]))
print('LEAN4 + rating gap/400 auc %.3f coef' % a, np.round(c1, 3), '(pts coef shrink %.0f%%)' % (100 * (1 - c1[3] / c0[3])))
a, _ = cv(gap[:, None])
print('rating gap alone auc %.3f' % a)
# split by source
src = np.array([m['ours'] for m in meta])
for nm, mask in (('top-10 mutual', ~src), ('ours', src)):
    mu, sd = A0[mask].mean(0), A0[mask].std(0)
    m_ = LogisticRegression(max_iter=3000).fit((A0[mask] - mu) / sd, y[mask])
    print(nm, 'n=%d' % mask.sum(), 'coef per raw unit', np.round(m_.coef_[0] / sd, 3), 'base', y[mask].mean().round(3))

# ---- robustness: subsets and cross-population transfer
rowsel = []
for r in M.rows:
    l = M.label(r)
    if l is None or r['r0'] <= 3:
        continue
    f = M.build(r)
    if f['us_n6'] == 0 or f['them_n6'] == 0:
        continue
    rowsel.append(r)
assert len(rowsel) == len(y)
nocamp = np.array([(r['camp_d'] > 4 and r['mon_hits'] == 0) for r in rowsel])
nocontest = np.array([r['map'] != 'contest' for r in rowsel])


def cv_mask(A, mask):
    aucs = []
    for kf in range(5):
        trk = mask & np.array([all(fold[t] != kf for t in (m['teams'] or [])) for m in meta])
        tek = mask & np.array([any(fold[t] == kf for t in (m['teams'] or [])) for m in meta])
        if tek.sum() < 30 or len(set(y[tek])) < 2:
            continue
        mu, sd = A[trk].mean(0), A[trk].std(0) + 1e-9
        mdl = LogisticRegression(max_iter=3000).fit((A[trk] - mu) / sd, y[trk])
        aucs.append(roc_auc_score(y[tek], mdl.predict_proba((A[tek] - mu) / sd)[:, 1]))
    return np.mean(aucs), int(mask.sum())


B2 = X[:, [FE.index('d_n6'), FE.index('d_hp6')]]
for nm, mask in (('all', np.ones(len(y), bool)), ('no camp within 4 tiles and no monster hits by participants', nocamp), ('excluding contest map (3a5c2468)', nocontest)):
    a1, n1 = cv_mask(A0, mask)
    a2, _ = cv_mask(B2, mask)
    print('subset %-62s n=%d LEAN4 auc %.3f | count+HP auc %.3f | base rate %.3f' % (nm, n1, a1, a2, y[mask].mean()))
# transfer
for trn, tsn, trm, tsm in (('top-10 -> ours', 'ours', ~src, src), ('ours -> top-10', 'top', src, ~src)):
    for nm, A in (('LEAN4', A0), ('count+HP', B2)):
        mu, sd = A[trm].mean(0), A[trm].std(0) + 1e-9
        mdl = LogisticRegression(max_iter=3000).fit((A[trm] - mu) / sd, y[trm])
        p = mdl.predict_proba((A[tsm] - mu) / sd)[:, 1]
        print('transfer', trn, nm, 'auc %.3f acc %.3f (n test %d)' % (roc_auc_score(y[tsm], p), ((p > 0.5) == y[tsm]).mean(), tsm.sum()))
