"""Ablation, calibration, policy table, per-source, composition test, and C++ export for the fight model (kill label, initiator perspective)."""
import sys, json, math, collections, random
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
import a2_model as M
from a2_common import *
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import roc_auc_score, log_loss, accuracy_score

OUT = R + 'data/v3/matchups/'
X, y, meta = M.prep(M.rows, M.label)
FE = M.FEATS
print('instances', len(y), 'base', y.mean().round(3), 'games', len(set(m['game'] for m in meta)))
# composition features: counts of each type (initiator side minus other) within 6
comp = np.zeros((len(y), 12))
k = 0
for r in M.rows:
    l = M.label(r)
    if l is None or r['r0'] <= 3:
        continue
    f = M.build(r)
    if f['us_n6'] == 0 or f['them_n6'] == 0:
        continue
    t = r['init_team']
    for p in r['pres']:
        if p['d'] <= 6:
            comp[k, p['ty']] += 1 if p['t'] == t else -1
    k += 1
assert k == len(y)
teams = sorted(set(t for m in meta for t in (m['teams'] or [])))
rnd = random.Random(7)
rnd.shuffle(teams)
fold = {t: i % 5 for i, t in enumerate(teams)}
src_ours = np.array([m['ours'] for m in meta])


def cv(cols, extra=None, C=1.0, ret=False):
    idx = [FE.index(c) for c in cols]
    A = X[:, idx]
    if extra is not None:
        A = np.hstack([A, extra])
    P = []
    Y = []
    S = []
    aucs = []
    for kf in range(5):
        trk = np.array([all(fold[t] != kf for t in (m['teams'] or [])) for m in meta])
        tek = np.array([any(fold[t] == kf for t in (m['teams'] or [])) for m in meta])
        mu, sd = A[trk].mean(0), A[trk].std(0) + 1e-9
        mdl = LogisticRegression(C=C, max_iter=3000).fit((A[trk] - mu) / sd, y[trk])
        p = mdl.predict_proba((A[tek] - mu) / sd)[:, 1]
        aucs.append(roc_auc_score(y[tek], p))
        P.append(p)
        Y.append(y[tek])
        S.append(src_ours[tek])
    P = np.concatenate(P)
    Y = np.concatenate(Y)
    S = np.concatenate(S)
    res = dict(auc=float(np.mean(aucs)), auc_sd=float(np.std(aucs)), acc=float(accuracy_score(Y, P > 0.5)), ll=float(log_loss(Y, np.clip(P, 1e-4, 1 - 1e-4))), n=len(Y))
    if ret:
        return res, P, Y, S
    return res


print('\n== ablation (leave-teams-out, LR, kill label)')
steps = [
    ('const (initiator wins)', None),
    ('d_n6', ['d_n6']),
    ('d_n6 + d_hp6  [count+HP baseline]', ['d_n6', 'd_hp6']),
    ('+ d_hpfrac6', ['d_n6', 'd_hp6', 'd_hpfrac6']),
    ('+ d_minhp', ['d_n6', 'd_hp6', 'd_hpfrac6', 'd_minhp']),
    ('+ d_str6', ['d_n6', 'd_hp6', 'd_hpfrac6', 'd_minhp', 'd_str6']),
    ('+ d_burst6 (own true cooldowns)', ['d_n6', 'd_hp6', 'd_hpfrac6', 'd_minhp', 'd_str6', 'd_burst6']),
    ('+ d_burstpot6 (all spells ready)', ['d_n6', 'd_hp6', 'd_hpfrac6', 'd_minhp', 'd_str6', 'd_burstpot6']),
    ('HP terms + d_pts6 (gear points) [LEAN4]', ['d_hp6', 'd_hpfrac6', 'd_minhp', 'd_pts6']),
    ('LEAN4 + d_str6', ['d_hp6', 'd_hpfrac6', 'd_minhp', 'd_pts6', 'd_str6']),
    ('LEAN4 + d_burstpot6', ['d_hp6', 'd_hpfrac6', 'd_minhp', 'd_pts6', 'd_burstpot6']),
    ('LEAN4 + d_n6', ['d_n6', 'd_hp6', 'd_hpfrac6', 'd_minhp', 'd_pts6']),
    ('count+HP+d_pts6', ['d_n6', 'd_hp6', 'd_pts6']),
    ('+ stun,ranged,gems', ['d_n6', 'd_hp6', 'd_hpfrac6', 'd_minhp', 'd_str6', 'd_pts6', 'd_burstpot6', 'd_stun6', 'd_ranged6', 'd_gems6']),
    ('all 14', FE),
]
LEAN = ['d_hp6', 'd_hpfrac6', 'd_minhp', 'd_pts6']
abl = {}
for nm, cols in steps:
    if cols is None:
        print('%-40s acc=%.3f (predict initiator wins)' % (nm, max(y.mean(), 1 - y.mean())))
        continue
    r_ = cv(cols)
    abl[nm] = r_
    print('%-40s auc=%.3f+-%.3f acc=%.3f ll=%.3f n=%d' % (nm, r_['auc'], r_['auc_sd'], r_['acc'], r_['ll'], r_['n']))
r_ = cv(LEAN + ['d_n6'], extra=comp)
print('%-40s auc=%.3f+-%.3f acc=%.3f ll=%.3f' % ('LEAN4 + d_n6 + champion composition (12)', r_['auc'], r_['auc_sd'], r_['acc'], r_['ll']))
abl['LEAN6+comp'] = r_
# strength-only and hp-only singles
for nm, cols in (('only d_hpfrac6', ['d_hpfrac6']), ('only d_minhp', ['d_minhp']), ('only d_hp6', ['d_hp6'])):
    r_ = cv(cols)
    print('%-40s auc=%.3f acc=%.3f' % (nm, r_['auc'], r_['acc']))
# by source
res, P, Y, S = cv(LEAN, ret=True)
for nm, mask in (('top-10 mutual games', ~S), ('ours (xanarchy) games', S)):
    print('LEAN4 on', nm, 'n=%d auc=%.3f acc=%.3f base=%.3f' % (mask.sum(), roc_auc_score(Y[mask], P[mask]), accuracy_score(Y[mask], P[mask] > 0.5), Y[mask].mean()))
res2, P2, Y2, S2 = cv(['d_n6', 'd_hp6'], ret=True)
for nm, mask in (('top-10 mutual games', ~S2), ('ours (xanarchy) games', S2)):
    print('B2 count+HP on', nm, 'n=%d auc=%.3f acc=%.3f' % (mask.sum(), roc_auc_score(Y2[mask], P2[mask]), accuracy_score(Y2[mask], P2[mask] > 0.5)))
# ---- time split: first 70% of games (by finish time) train, last 30% test
tmap = {}
for m in meta:
    tmap.setdefault(m['game'], m['t'])
gs_ = sorted(tmap, key=lambda g: tmap[g])
cut = tmap[gs_[int(0.7 * len(gs_))]]
trt = np.array([m['t'] < cut for m in meta])
tet = ~trt
print('\n== time split: train games before', cut, '(n=%d instances), test after (n=%d); test base rate %.3f' % (trt.sum(), tet.sum(), y[tet].mean()))
tsplit = {}
for nm, cols in (('d_n6 + d_hp6 [count+HP]', ['d_n6', 'd_hp6']), ('count+HP+hpfrac+minhp', ['d_n6', 'd_hp6', 'd_hpfrac6', 'd_minhp']), ('LEAN4', LEAN), ('all 14 features', FE)):
    idx_ = [FE.index(c) for c in cols]
    A_ = X[:, idx_]
    mu_, sd_ = A_[trt].mean(0), A_[trt].std(0) + 1e-9
    m_ = LogisticRegression(max_iter=3000).fit((A_[trt] - mu_) / sd_, y[trt])
    p_ = m_.predict_proba((A_[tet] - mu_) / sd_)[:, 1]
    tsplit[nm] = dict(auc=float(roc_auc_score(y[tet], p_)), acc=float(accuracy_score(y[tet], p_ > 0.5)), ll=float(log_loss(y[tet], np.clip(p_, 1e-4, 1 - 1e-4))))
    print('%-28s auc=%.3f acc=%.3f ll=%.3f' % (nm, tsplit[nm]['auc'], tsplit[nm]['acc'], tsplit[nm]['ll']))
print('constant (initiator wins) acc on test %.3f' % max(y[tet].mean(), 1 - y[tet].mean()))
# calibration (pooled leave-teams-out), 8 quantile bins
qs = np.quantile(P, np.linspace(0, 1, 9))
cal = []
for i in range(8):
    m = (P >= qs[i]) & ((P <= qs[i + 1]) if i == 7 else (P < qs[i + 1]))
    cal.append((round(float(P[m].mean()), 2), round(float(Y[m].mean()), 2), int(m.sum())))
print('calibration LEAN4 pooled (mean pred, observed win, n):', cal)
# policy table
print('\n== policy: initiate only if P(win) >= thr  (pooled leave-teams-out predictions; rows = decisive fights where the initiator was the focal side)')
pol = []
for thr in (0.0, 0.5, 0.6, 0.7, 0.8, 0.9):
    m = P >= thr
    m2 = P2 >= thr
    pol.append(dict(thr=thr, share=float(m.mean()), win=float(Y[m].mean()), n=int(m.sum()), share_b2=float(m2.mean()), win_b2=float(Y2[m2].mean()), n_b2=int(m2.sum())))
    print('thr %.1f: LEAN4 engages %.0f%% (n=%d) of fights, wins %.1f%% of them | count+HP engages %.0f%%, wins %.1f%%' % (thr, 100 * m.mean(), m.sum(), 100 * Y[m].mean(), 100 * m2.mean(), 100 * Y2[m2].mean()))
# opponents' observed decision: all fights
# final fit on all for export (LEAN6)
idx = [FE.index(c) for c in LEAN]
A = X[:, idx]
mu, sd = A.mean(0), A.std(0)
mdl = LogisticRegression(C=1.0, max_iter=3000).fit((A - mu) / sd, y)
w = mdl.coef_[0] / sd
b = float(mdl.intercept_[0] - (mdl.coef_[0] * mu / sd).sum())
print('\nLEAN4 coefficients (raw units):', {c: round(float(v), 4) for c, v in zip(LEAN, w)}, 'intercept', round(b, 4))
# bootstrap CI of coefficients by game
games = sorted(set(m['game'] for m in meta))
gi = {g: i for i, g in enumerate(games)}
gidx = np.array([gi[m['game']] for m in meta])
rng = np.random.default_rng(3)
bs = []
for _ in range(200):
    pick = rng.integers(0, len(games), len(games))
    cnt = np.bincount(pick, minlength=len(games))
    wts = cnt[gidx]
    keep = wts > 0
    mm = LogisticRegression(C=1.0, max_iter=3000).fit(((A - mu) / sd)[keep], y[keep], sample_weight=wts[keep].astype(float))
    bs.append(mm.coef_[0] / sd)
bs = np.array(bs)
ci = {c: (round(float(np.percentile(bs[:, i], 2.5)), 3), round(float(np.percentile(bs[:, i], 97.5)), 3)) for i, c in enumerate(LEAN)}
print('bootstrap 95% CI by game:', ci)
# composition logit (champion value in fights) with LEAN6 controls
A_c = X[:, [FE.index(c) for c in LEAN + ['d_n6']]]
C_ = np.hstack([A_c, comp])
mu2, sd2 = C_.mean(0), C_.std(0) + 1e-9
cm = LogisticRegression(C=1.0, max_iter=3000).fit((C_ - mu2) / sd2, y)
wc = cm.coef_[0] / sd2
# bootstrap
bsc = []
for _ in range(200):
    pick = rng.integers(0, len(games), len(games))
    cnt = np.bincount(pick, minlength=len(games))
    wts = cnt[gidx]
    keep = wts > 0
    mm = LogisticRegression(C=1.0, max_iter=3000).fit(((C_ - mu2) / sd2)[keep], y[keep], sample_weight=wts[keep].astype(float))
    bsc.append(mm.coef_[0] / sd2)
bsc = np.array(bsc)
compres = {}
for j in range(12):
    col = len(LEAN) + 1 + j
    n_present = int((np.abs(comp[:, j]) > 0).sum())
    compres[NAMES[j]] = dict(coef=float(wc[col]), lo=float(np.percentile(bsc[:, col], 2.5)), hi=float(np.percentile(bsc[:, col], 97.5)), fights_present=n_present)
    print('type %-9s extra logit per body (initiator side minus other) %.3f [%.3f, %.3f]  fights present %d' % (NAMES[j], wc[col], np.percentile(bsc[:, col], 2.5), np.percentile(bsc[:, col], 97.5), n_present))
json.dump(dict(time_split=tsplit, feats=LEAN, coefs=[float(v) for v in w], intercept=b, ci=ci, ablation=abl, policy=pol, calibration=cal, composition=compres,
               n=len(y), base=float(y.mean())), open(OUT + 'fight_model_lean.json', 'w'), indent=1)
# C++ header
hdr = '''// Fight-outcome precognition, generated by tools/v3/a2_model2.py from data/v3/fights.jsonl (%d decisive fights, initiator perspective).
// p = sigmoid(B0 + sum_i W_i * x_i); x_i = (initiator side) minus (other side), summed over alive champions within 6 tiles of the fight centre
// (midpoint of attacker and first target) in the bot's own vision. p = P(initiator kills more units than it loses | someone dies).
// pts (gear points) per champion = (max_health-base_health)/health_growth + (strength-base_strength)/strength_growth + (focus-base_focus)/focus_growth,
// base values from champion::base_stats(). Cassi and Issac each count as one body in every sum.
#pragma once
#include <cmath>
namespace fightmodel {
constexpr float B0 = %.5ff;
constexpr float W_HP6 = %.5ff;      // x = sum of current health / 1000
constexpr float W_HPFRAC6 = %.5ff;  // x = sum(health)/sum(max_health) of the side
constexpr float W_MINHP = %.5ff;    // x = lowest current health on the side / 1000
constexpr float W_PTS6 = %.5ff;     // x = sum of pts (gear points, see above)
inline float p_win(float d_hp6k, float d_hpfrac, float d_minhpk, float d_pts) {
  float z = B0 + W_HP6 * d_hp6k + W_HPFRAC6 * d_hpfrac + W_MINHP * d_minhpk + W_PTS6 * d_pts;
  return 1.0f / (1.0f + std::exp(-z));
}
}  // namespace fightmodel
''' % (len(y), b, w[0], w[1], w[2], w[3])
open(OUT + 'fightmodel.h', 'w').write(hdr)
print('wrote header')
