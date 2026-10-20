"""Part 4: how top teams' fights differ from ours. Uses the LEAN5 coefficients (fight_model_lean.json) to score each initiation."""
import sys, json, math, collections
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
import a2_model as M
from a2_common import *
import numpy as np
OUT = R + 'data/v3/matchups/'
mj = json.load(open(OUT + 'fight_model_lean.json'))
FE, W, B0 = mj['feats'], mj['coefs'], mj['intercept']
groups = collections.defaultdict(list)
for r in M.rows:
    if r['r0'] <= 3:
        continue
    f = M.build(r)
    if f['us_n6'] == 0 or f['them_n6'] == 0:
        continue
    z = B0 + sum(w * f[c] for w, c in zip(W, FE))
    p = 1 / (1 + math.exp(-z))
    kd = kill_diff(r, r['init_team'])
    if is_ours(r):
        o = r['teams'].index('xanarchy')
        key = 'ours initiates' if r['init_team'] == o else 'opponent initiates (vs us)'
    else:
        key = 'top-10 mutual (initiator)'
    groups[key].append(dict(p=p, kd=kd, f=f, r0=r['r0'], opener=r['init_kind']))
res = {}
print('%-28s %6s %6s %7s %7s %7s %7s %7s %7s %7s' % ('group', 'n', 'decis', 'W/(W+L)', 'meanP', 'p<.5', 'W|p<.5', 'W|p>=.7', 'd_pts', 'hpfrac'))
for k, L in groups.items():
    n = len(L)
    dec = [x for x in L if x['kd'] != 0]
    w = sum(1 for x in dec if x['kd'] > 0)
    lowp = [x for x in dec if x['p'] < 0.5]
    hip = [x for x in dec if x['p'] >= 0.7]
    row = dict(n=n, decisive=len(dec), win=w / max(1, len(dec)), meanP=float(np.mean([x['p'] for x in L])), share_p_lt_05=float(np.mean([x['p'] < 0.5 for x in L])),
               win_lowp=(sum(1 for x in lowp if x['kd'] > 0) / len(lowp) if lowp else None), n_lowp=len(lowp),
               win_hip=(sum(1 for x in hip if x['kd'] > 0) / len(hip) if hip else None), n_hip=len(hip),
               d_pts=float(np.mean([x['f']['d_pts6'] for x in L])), d_hpfrac=float(np.mean([x['f']['d_hpfrac6'] for x in L])),
               share_spell_open=float(np.mean([x['opener'] == 'spell' for x in L])), share_pts_neg=float(np.mean([x['f']['d_pts6'] < 0 for x in L])),
               share_n_adv=float(np.mean([x['f']['d_n6'] > 0 for x in L])), share_n_dis=float(np.mean([x['f']['d_n6'] < 0 for x in L])),
               mean_hp_frac_own=None)
    res[k] = row
    print('%-28s %6d %6d %7.3f %7.3f %7.3f %7s %7s %7.2f %7.3f' % (k, n, len(dec), row['win'], row['meanP'], row['share_p_lt_05'],
          ('%.2f(%d)' % (row['win_lowp'], row['n_lowp'])) if lowp else '-', ('%.2f(%d)' % (row['win_hip'], row['n_hip'])) if hip else '-', row['d_pts'], row['d_hpfrac']))
    print('   share opening with a spell %.2f; initiator numbers adv/disadv %.2f/%.2f; initiator gear deficit share %.2f' % (row['share_spell_open'], row['share_n_adv'], row['share_n_dis'], row['share_pts_neg']))
json.dump(res, open(OUT + 'ours_vs_top.json', 'w'), indent=1)
# per-game fight and death rates
byg = collections.defaultdict(lambda: dict(f=0, d=0, g=0, src=None))
for r in M.rows:
    if r['r0'] <= 3:
        continue
    k = 'ours' if is_ours(r) else 'top'
    byg[(k, r['game'])]['f'] += 1
    byg[(k, r['game'])]['d'] += len(r['deaths']['0']) + len(r['deaths']['1'])
for k in ('top', 'ours'):
    L = [v for (kk, g), v in byg.items() if kk == k]
    print(k, 'games with fights', len(L), 'fights per game %.1f' % np.mean([v['f'] for v in L]), 'deaths-in-fights per game %.1f' % np.mean([v['d'] for v in L]))
# our deaths vs their deaths in our games, by initiator
c = collections.Counter()
for r in M.rows:
    if r['r0'] <= 3 or not is_ours(r):
        continue
    o = r['teams'].index('xanarchy')
    c['our_unit_deaths'] += r['units_lost'][str(o)] if 'units_lost' in r else len(r['deaths'][str(o)])
    c['their_unit_deaths'] += r['units_lost'][str(1 - o)] if 'units_lost' in r else len(r['deaths'][str(1 - o)])
    c['our_gems_lost'] += r['gems_lost'][str(o)] if 'gems_lost' in r else sum(g for _, _, g in r['deaths'][str(o)])
    c['their_gems_lost'] += r['gems_lost'][str(1 - o)] if 'gems_lost' in r else sum(g for _, _, g in r['deaths'][str(1 - o)])
print('ours games: deaths', dict(c))

# ---- initiator champion type and opener kind (decisive fights, initiator perspective), top-10 mutual and ours (incl. opponents' initiations)
print('\n== initiator champion type: decisive fights W/(W+L), top-10 mutual | all fights in ours games (either side initiates)')
it = {'top': collections.defaultdict(lambda: [0, 0]), 'ours': collections.defaultdict(lambda: [0, 0])}
ok = {'top': collections.defaultdict(lambda: [0, 0]), 'ours': collections.defaultdict(lambda: [0, 0])}
for r in M.rows:
    if r['r0'] <= 3:
        continue
    f = M.build(r)
    if f['us_n6'] == 0 or f['them_n6'] == 0:
        continue
    kd = kill_diff(r, r['init_team'])
    if kd == 0:
        continue
    g = 'ours' if is_ours(r) else 'top'
    it[g][r['init_ty']][0 if kd > 0 else 1] += 1
    ok[g][r['init_kind']][0 if kd > 0 else 1] += 1
for ty in range(12):
    a, b = it['top'][ty], it['ours'][ty]
    n1, n2 = sum(a), sum(b)
    print('%-9s top: n=%4d W/(W+L)=%s | ours-games: n=%4d W/(W+L)=%s' % (NAMES[ty], n1, ('%.2f' % (a[0] / n1)) if n1 else '-', n2, ('%.2f' % (b[0] / n2)) if n2 else '-'))
for g in ('top', 'ours'):
    for k, v in ok[g].items():
        print('opener', g, k, 'n=%d' % sum(v), 'W/(W+L)=%.3f' % (v[0] / sum(v)))
# gems per death
gd = collections.defaultdict(lambda: [0, 0])
for r in M.rows:
    if r['r0'] <= 3:
        continue
    for t in ('0', '1'):
        key = 'top' if not is_ours(r) else ('us' if r['teams'][int(t)] == 'xanarchy' else 'opp')
        gd[key][0] += sum(g_ for (_, _, g_) in r['deaths'][t])
        gd[key][1] += len(r['deaths'][t])
print('gems carried by dying bodies (bag at fight start): ', {k: (v[0], v[1], round(v[0] / max(1, v[1]), 1)) for k, v in gd.items()})
# policy gain estimate on our initiations
L = groups['ours initiates']
dec = [x for x in L if x['kd'] != 0]
lo = [x for x in dec if x['p'] < 0.5]
w = sum(1 for x in lo if x['kd'] > 0)
print('ours initiates: decisive %d, with P<0.5: %d (win %d, lose %d) -> net unit change if skipped: %+d units over %d games' % (len(dec), len(lo), w, len(lo) - w, (len(lo) - w) - w, len(set(r['game'] for r in M.rows if is_ours(r)))))
