"""Fight-outcome model. Instances: one per decisive fight, from the INITIATOR's perspective (we = side that dealt the first hit).
Label: initiator side killed more units than it lost. Features = what the initiator's bot sees at the first hit (alive champions within radius of the fight centre).
Splits: (a) time (first 70% of games by finish time train / last 30% test), (b) leave-teams-out (5 folds of teams)."""
import sys, json, math, collections, random
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
from a2_common import *
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import HistGradientBoostingClassifier
from sklearn.tree import DecisionTreeClassifier, export_text
from sklearn.metrics import roc_auc_score, log_loss, accuracy_score

OUT = R + 'data/v3/matchups/'
rows = load_fights()

FEATS = ['d_n4', 'd_n6', 'd_n8', 'd_hp6', 'd_mx6', 'd_hpfrac6', 'd_str6', 'd_burst6', 'd_burstpot6', 'd_pts6', 'd_gems6', 'd_stun6', 'd_ranged6', 'd_minhp', 'us_n6', 'them_n6']


def side_feats(pres, t, rad, own):
    ps = [p for p in pres if p['t'] == t and p['d'] <= rad]
    n = len(ps)
    hp = sum(max(0, p['hp']) for p in ps)
    mx = sum(p['mx'] for p in ps)
    st = sum(p['str'] for p in ps)
    burst = 0.0
    bpot = sum(p['foc'] * (SPELL[p['ty']][0] + SPELL[p['ty']][1]) for p in ps)
    pts = sum(sum(p['pts']) for p in ps)
    for p in ps:
        m0, m1 = SPELL[p['ty']]
        if own:
            burst += p['foc'] * (m0 * p['cd0'] + m1 * p['cd1'])
        else:
            burst += p['foc'] * (m0 + m1)
    gems = sum(p['gems'] for p in ps)
    stun = sum(p['stun'] for p in ps)
    rng = sum(1 for p in ps if p['ty'] in RANGED)
    minhp = min((max(0, p['hp']) for p in ps), default=0)
    return dict(n=n, hp=hp, mx=mx, str=st, burst=burst, bpot=bpot, pts=pts, gems=gems, stun=stun, rng=rng, minhp=minhp)


def build(r):
    t = r['init_team']
    pres = r['pres']
    out = {}
    for rad in (4, 6, 8):
        out[rad] = (side_feats(pres, t, rad, True), side_feats(pres, 1 - t, rad, False))
    u, v = out[6]
    f = {}
    f['d_n4'] = out[4][0]['n'] - out[4][1]['n']
    f['d_n6'] = u['n'] - v['n']
    f['d_n8'] = out[8][0]['n'] - out[8][1]['n']
    f['d_hp6'] = (u['hp'] - v['hp']) / 1000.0
    f['d_mx6'] = (u['mx'] - v['mx']) / 1000.0
    f['d_hpfrac6'] = (u['hp'] / u['mx'] if u['mx'] else 0) - (v['hp'] / v['mx'] if v['mx'] else 0)
    f['d_str6'] = (u['str'] - v['str']) / 100.0
    f['d_burst6'] = (u['burst'] - v['burst']) / 100.0
    f['d_burstpot6'] = (u['bpot'] - v['bpot']) / 100.0
    f['d_pts6'] = u['pts'] - v['pts']
    f['d_gems6'] = (u['gems'] - v['gems']) / 10.0
    f['d_stun6'] = u['stun'] - v['stun']
    f['d_ranged6'] = u['rng'] - v['rng']
    f['d_minhp'] = (u['minhp'] - v['minhp']) / 1000.0
    f['us_n6'] = u['n']
    f['them_n6'] = v['n']
    return f


def label(r):
    t = r['init_team']
    kd = kill_diff(r, t)
    if kd > 0:
        return 1
    if kd < 0:
        return 0
    return None


def dmg_label(r):
    t = r['init_team']
    a, b = r['dmg'][str(t)], r['dmg'][str(1 - t)]
    if a == b:
        return None
    return int(a > b)


def prep(rows, labf):
    X, y, meta = [], [], []
    for r in rows:
        l = labf(r)
        if l is None:
            continue
        if r['r0'] <= 3:
            continue  # spawn-adjacent opening hits
        f = build(r)
        if f['us_n6'] == 0 or f['them_n6'] == 0:
            continue
        X.append([f[k] for k in FEATS])
        y.append(l)
        meta.append(dict(game=r['game'], t=r['t'], teams=r['teamIds'], src=r['src'], ours=is_ours(r), map=r['map'], init_team=r['init_team'], rating=r['ratings']))
    return np.array(X, float), np.array(y), meta


def fit_eval(Xtr, ytr, Xte, yte, cols, kind='lr'):
    idx = [FEATS.index(c) for c in cols]
    A, B = Xtr[:, idx], Xte[:, idx]
    mu, sd = A.mean(0), A.std(0) + 1e-9
    if kind == 'lr':
        m = LogisticRegression(C=1.0, max_iter=2000).fit((A - mu) / sd, ytr)
        p = m.predict_proba((B - mu) / sd)[:, 1]
        return p, (m, mu, sd)
    if kind == 'gb':
        m = HistGradientBoostingClassifier(max_depth=3, max_iter=80, learning_rate=0.06, min_samples_leaf=40, l2_regularization=1.0).fit(A, ytr)
        return m.predict_proba(B)[:, 1], (m, None, None)
    if kind == 'tree':
        m = DecisionTreeClassifier(max_depth=3, min_samples_leaf=40).fit(A, ytr)
        return m.predict_proba(B)[:, 1], (m, None, None)


def metrics(y, p):
    p = np.clip(p, 1e-4, 1 - 1e-4)
    return dict(n=len(y), base=float(y.mean()), auc=float(roc_auc_score(y, p)) if len(set(y)) > 1 else float('nan'), acc=float(accuracy_score(y, p > 0.5)), ll=float(log_loss(y, p)))


def calib(y, p, bins=5):
    qs = np.quantile(p, np.linspace(0, 1, bins + 1))
    res = []
    for i in range(bins):
        m = (p >= qs[i]) & (p <= qs[i + 1] if i == bins - 1 else p < qs[i + 1])
        if m.sum():
            res.append((float(p[m].mean()), float(y[m].mean()), int(m.sum())))
    return res


def run(labf, name):
    X, y, meta = prep(rows, labf)
    print('\n=====', name, 'instances', len(y), 'base rate (initiator wins)', y.mean().round(3), 'games', len(set(m['game'] for m in meta)))
    games = sorted(set(m['game'] for m in meta), key=lambda g: next(m['t'] for m in meta if m['game'] == g))
    tmap = {}
    for m in meta:
        tmap.setdefault(m['game'], m['t'])
    gs = sorted(tmap, key=lambda g: tmap[g])
    cut = tmap[gs[int(0.7 * len(gs))]]
    tr = np.array([m['t'] < cut for m in meta])
    te = ~tr
    print('time split cut', cut, 'train', tr.sum(), 'test', te.sum())
    sets = {
        'initiator-wins(const)': ['us_n6'],  # placeholder replaced below
    }
    base_models = {
        'B1 count (d_n6)': (['d_n6'], 'lr'),
        'B2 count+HP (d_n6,d_hp6)': (['d_n6', 'd_hp6'], 'lr'),
        'B3 count+HP+hpfrac': (['d_n6', 'd_hp6', 'd_hpfrac6'], 'lr'),
        'M1 full LR': (FEATS, 'lr'),
        'M2 full GB depth3': (FEATS, 'gb'),
        'M3 tree depth3 (count+HP+burst)': (['d_n6', 'd_hp6', 'd_burst6', 'd_str6', 'd_hpfrac6'], 'tree'),
    }
    res = {}
    print('--- time split (train earlier 70% games, test later 30%)')
    print('const (always predict initiator wins): test base %.3f' % y[te].mean(), ' acc=%.3f' % max(y[te].mean(), 1 - y[te].mean()))
    for nm, (cols, kind) in base_models.items():
        p, _ = fit_eval(X[tr], y[tr], X[te], y[te], cols, kind)
        mt = metrics(y[te], p)
        res[nm] = mt
        print('%-34s n=%d auc=%.3f acc=%.3f ll=%.3f' % (nm, mt['n'], mt['auc'], mt['acc'], mt['ll']))
    # team split
    teams = sorted(set(t for m in meta for t in (m['teams'] or [])))
    rnd = random.Random(7)
    rnd.shuffle(teams)
    fold = {t: i % 5 for i, t in enumerate(teams)}
    print('--- leave-teams-out (5 folds; train games contain no fold team; test games contain >=1 fold team), teams', len(teams))
    agg = collections.defaultdict(list)
    allp = {}
    for k in range(5):
        trk = np.array([all(fold[t] != k for t in (m['teams'] or [])) for m in meta])
        tek = np.array([any(fold[t] == k for t in (m['teams'] or [])) for m in meta])
        for nm, (cols, kind) in base_models.items():
            p, _ = fit_eval(X[trk], y[trk], X[tek], y[tek], cols, kind)
            agg[nm].append(metrics(y[tek], p))
            allp.setdefault(nm, []).append((y[tek], p))
    for nm in base_models:
        a = agg[nm]
        yy = np.concatenate([q[0] for q in allp[nm]])
        pp = np.concatenate([q[1] for q in allp[nm]])
        print('%-34s auc=%.3f (folds %s) acc=%.3f ll=%.3f  pooled n=%d' % (nm, np.mean([x['auc'] for x in a]), ','.join('%.2f' % x['auc'] for x in a), np.mean([x['acc'] for x in a]), np.mean([x['ll'] for x in a]), len(yy)))
    # calibration of M1 on time split
    p, (m, mu, sd) = fit_eval(X[tr], y[tr], X[te], y[te], FEATS, 'lr')
    print('calibration M1 time split (mean pred, observed, n):', [(round(a, 2), round(b, 2), n) for a, b, n in calib(y[te], p)])
    p2, _ = fit_eval(X[tr], y[tr], X[te], y[te], ['d_n6', 'd_hp6'], 'lr')
    print('calibration B2 time split:', [(round(a, 2), round(b, 2), n) for a, b, n in calib(y[te], p2)])
    # final fit on everything for export
    mu = X.mean(0)
    sd = X.std(0) + 1e-9
    mm = LogisticRegression(C=1.0, max_iter=2000).fit((X - mu) / sd, y)
    w = mm.coef_[0] / sd
    b = mm.intercept_[0] - float((mm.coef_[0] * mu / sd).sum())
    coefs = dict(zip(FEATS, [float(x) for x in w]))
    print('M1 raw-unit coefficients:', {k: round(v, 3) for k, v in coefs.items()}, 'intercept', round(b, 3))
    # B2 coefficients
    idx = [FEATS.index(c) for c in ['d_n6', 'd_hp6']]
    A = X[:, idx]
    mu2, sd2 = A.mean(0), A.std(0)
    m2 = LogisticRegression(C=1.0, max_iter=2000).fit((A - mu2) / sd2, y)
    w2 = m2.coef_[0] / sd2
    b2 = m2.intercept_[0] - float((m2.coef_[0] * mu2 / sd2).sum())
    print('B2 coefficients: d_n6 %.3f d_hp6 %.3f intercept %.3f' % (w2[0], w2[1], b2))
    return dict(name=name, n=len(y), base=float(y.mean()), coefs=coefs, intercept=float(b), b2=dict(d_n6=float(w2[0]), d_hp6=float(w2[1]), intercept=float(b2)), time=res)


if __name__ == '__main__':
    r1 = run(label, 'kill label (initiator kills more units than it loses; decisive fights only)')
    r2 = run(dmg_label, 'damage label (initiator deals more damage; all fights)')
    json.dump(dict(kill=r1, dmg=r2, feats=FEATS), open(OUT + 'fight_model.json', 'w'), indent=1)
