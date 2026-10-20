import sys, random
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
import a2_model as M
from a2_common import *
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import roc_auc_score
X, y, meta = M.prep(M.rows, M.label)
FE = M.FEATS
pot = []
cdfrac = []
k = 0
for r in M.rows:
    l = M.label(r)
    if l is None or r['r0'] <= 3:
        continue
    f = M.build(r)
    if f['us_n6'] == 0 or f['them_n6'] == 0:
        continue
    t = r['init_team']
    own = sum(p['foc'] * (sum(SPELL[p['ty']])) for p in r['pres'] if p['d'] <= 6 and p['t'] == t)
    oth = sum(p['foc'] * (sum(SPELL[p['ty']])) for p in r['pres'] if p['d'] <= 6 and p['t'] != t)
    pot.append((own - oth) / 100.0)
    # fraction of own spell-capable bodies with a ready spell
    cap = [p for p in r['pres'] if p['d'] <= 6 and p['t'] == t and sum(SPELL[p['ty']]) > 0]
    cdfrac.append(sum(1 for p in cap if (SPELL[p['ty']][0] > 0 and p['cd0']) or (SPELL[p['ty']][1] > 0 and p['cd1'])) / len(cap) if cap else 1.0)
pot = np.array(pot)
cdfrac = np.array(cdfrac)
teams = sorted(set(t for m in meta for t in (m['teams'] or [])))
rnd = random.Random(7)
rnd.shuffle(teams)
fold = {t: i % 5 for i, t in enumerate(teams)}


def cv(A):
    aucs = []
    for kf in range(5):
        trk = np.array([all(fold[t] != kf for t in (m['teams'] or [])) for m in meta])
        tek = np.array([any(fold[t] == kf for t in (m['teams'] or [])) for m in meta])
        mu, sd = A[trk].mean(0), A[trk].std(0) + 1e-9
        mdl = LogisticRegression(max_iter=3000).fit((A[trk] - mu) / sd, y[trk])
        aucs.append(roc_auc_score(y[tek], mdl.predict_proba((A[tek] - mu) / sd)[:, 1]))
    return np.mean(aucs)


base = [FE.index(c) for c in ['d_n6', 'd_hp6', 'd_hpfrac6', 'd_minhp', 'd_str6']]
A0 = X[:, base]
print('base5 (no burst)', cv(A0))
print('base5 + burst (own true cooldowns, enemy ready)', cv(np.hstack([A0, X[:, [FE.index('d_burst6')]]])))
print('base5 + burst potential (all ready both sides)', cv(np.hstack([A0, pot[:, None]])))
print('base5 + share of own spell bodies with a ready spell', cv(np.hstack([A0, cdfrac[:, None]])))
print('base5 + potential + cd fraction', cv(np.hstack([A0, pot[:, None], cdfrac[:, None]])))
# opening action type of the initiator
ik = np.array([1.0 if r['init_kind'] == 'spell' else 0.0 for r in M.rows if M.label(r) is not None and r['r0'] > 3 and M.build(r)['us_n6'] > 0 and M.build(r)['them_n6'] > 0])
print('share of fights opened with a spell', ik.mean(), 'win rate if spell-open %.3f, basic-open %.3f' % (y[ik == 1].mean(), y[ik == 0].mean()))

# ---- what drives the burst gain: composition vs Focus items vs wealth
ptsd = []
basef = []
k = 0
for r in M.rows:
    l = M.label(r)
    if l is None or r['r0'] <= 3:
        continue
    f = M.build(r)
    if f['us_n6'] == 0 or f['them_n6'] == 0:
        continue
    t = r['init_team']
    row = []
    for j in range(3):
        row.append(sum(p['pts'][j] for p in r['pres'] if p['d'] <= 6 and p['t'] == t) - sum(p['pts'][j] for p in r['pres'] if p['d'] <= 6 and p['t'] != t))
    ptsd.append(row)
    from a2_fights import BASE
    bf = sum(BASE[p['ty']][4] * (SPELL[p['ty']][0] + SPELL[p['ty']][1]) for p in r['pres'] if p['d'] <= 6 and p['t'] == t) - sum(BASE[p['ty']][4] * (SPELL[p['ty']][0] + SPELL[p['ty']][1]) for p in r['pres'] if p['d'] <= 6 and p['t'] != t)
    basef.append(bf / 100.0)
ptsd = np.array(ptsd, float)
basef = np.array(basef)
print('base5 + composition-only burst (base Focus, no shards)', cv(np.hstack([A0, basef[:, None]])))
print('base5 + total shard points diff (wealth proxy)', cv(np.hstack([A0, ptsd.sum(1)[:, None]])))
print('base5 + focus shard points diff', cv(np.hstack([A0, ptsd[:, [1]]])))
print('base5 + str/focus/health points diff (3)', cv(np.hstack([A0, ptsd])))
print('base5 + burstpot + points diff (3)', cv(np.hstack([A0, pot[:, None], ptsd])))
