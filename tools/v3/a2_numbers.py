import sys, collections, json, math
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
from a2_common import *
import numpy as np
rows = load_fights()
OUT = R + 'data/v3/matchups/'
rows = [r for r in rows if r['r0'] > 3]
top = [r for r in rows if not is_ours(r)]
ours = [r for r in rows if is_ours(r)]
print('fights (r0>3): top-top', len(top), 'games', len(set(r['game'] for r in top)), '| ours', len(ours), 'games', len(set(r['game'] for r in ours)))


def pn(r, t, rad):
    return sum(1 for p in r['pres'] if p['t'] == t and p['d'] <= rad)


def outcome(r, t):
    kd = kill_diff(r, t)
    return 'W' if kd > 0 else ('L' if kd < 0 else 'T')


def numbers_table(rs, label):
    # initiator perspective, bins of (initiator present within 6, other present within 6)
    tab = collections.defaultdict(lambda: collections.Counter())
    for r in rs:
        t = r['init_team']
        a, b = pn(r, t, 6), pn(r, 1 - t, 6)
        if a == 0 or b == 0:
            continue
        tab[(min(a, 3), min(b, 3))][outcome(r, t)] += 1
    print('\n##', label, '(initiator present within 6 tiles vs other; caps at 3; W/L/T = initiator kills more / fewer units / equal)')
    res = {}
    for k in sorted(tab):
        c = tab[k]
        w, l, tt = c['W'], c['L'], c['T']
        lo, hi = wilson(w, w + l)
        print(k, 'fights', w + l + tt, 'W', w, 'L', l, 'T', tt, 'W/(W+L)=%.2f [%.2f,%.2f]' % (w / max(1, w + l), lo, hi))
        res[str(k)] = dict(W=w, L=l, T=tt)
    return res


nt = numbers_table(top, 'top-10 mutual')
no = numbers_table(ours, 'ours (xanarchy games)')


def init_stats(rs, label, focal=None):
    # focal: function r->team index of the side of interest, or None -> both sides
    c = collections.Counter()
    adv = []
    hpf = []
    gems = []
    dph = collections.defaultdict(list)
    for r in rs:
        t = r['init_team']
        a, b = pn(r, t, 6), pn(r, 1 - t, 6)
        if a == 0 or b == 0:
            continue
        who = 'init'
        c['fights'] += 1
        c['adv+' if a > b else ('adv=' if a == b else 'adv-')] += 1
        adv.append(a - b)
        ps = [p for p in r['pres'] if p['t'] == t and p['d'] <= 6]
        hpf.append(sum(max(0, p['hp']) for p in ps) / max(1, sum(p['mx'] for p in ps)))
    return c, np.mean(adv), np.mean(hpf)


for lbl, rs in (('top-10 mutual', top),):
    c, ma, mh = init_stats(rs, lbl)
    print('\ninitiator numbers (present<=6):', lbl, dict(c), 'mean advantage %.2f' % ma, 'mean hp frac %.2f' % mh)

# ours: separate our initiations from theirs
our_side = lambda r: r['teams'].index('xanarchy')
c_us = collections.Counter()
c_th = collections.Counter()
for r in ours:
    o = our_side(r)
    t = r['init_team']
    a, b = pn(r, t, 6), pn(r, 1 - t, 6)
    if a == 0 or b == 0:
        continue
    tgt = c_us if t == o else c_th
    tgt['fights'] += 1
    tgt['adv+' if a > b else ('adv=' if a == b else 'adv-')] += 1
    tgt['W' if kill_diff(r, t) > 0 else ('L' if kill_diff(r, t) < 0 else 'T')] += 1
    tgt['sumadv'] += a - b
    ps = [p for p in r['pres'] if p['t'] == t and p['d'] <= 6]
    tgt['hpfrac'] += sum(max(0, p['hp']) for p in ps) / max(1, sum(p['mx'] for p in ps))
print('\nours as initiator', dict(c_us), '\nopponent as initiator vs us', dict(c_th))
json.dump(dict(top=nt, ours=no, ours_init=dict(c_us), opp_init=dict(c_th)), open(OUT + 'fight_numbers.json', 'w'), indent=1)

# top-10 initiator stats by outcome and adv
tt = collections.Counter()
for r in top:
    t = r['init_team']
    a, b = pn(r, t, 6), pn(r, 1 - t, 6)
    if a == 0 or b == 0:
        continue
    cat = 'adv+' if a > b else ('adv=' if a == b else 'adv-')
    tt[(cat, outcome(r, t))] += 1
print('\ntop initiator adv x outcome', sorted(tt.items()))
tu = collections.Counter()
for r in ours:
    o = our_side(r)
    t = r['init_team']
    a, b = pn(r, t, 6), pn(r, 1 - t, 6)
    if a == 0 or b == 0:
        continue
    cat = 'adv+' if a > b else ('adv=' if a == b else 'adv-')
    tu[('us' if t == o else 'them', cat, outcome(r, t))] += 1
print('\nours-games initiator x adv x outcome', sorted(tu.items()))

# damage per hit by champion (basic attacks) top vs ours-side vs ours-opp
dph = {'top': collections.defaultdict(list), 'us': collections.defaultdict(list), 'opp': collections.defaultdict(list)}
spell = {'top': collections.defaultdict(list), 'us': collections.defaultdict(list), 'opp': collections.defaultdict(list)}
mxhp = {'top': collections.defaultdict(list), 'us': collections.defaultdict(list), 'opp': collections.defaultdict(list)}
strs = {'top': collections.defaultdict(list), 'us': collections.defaultdict(list), 'opp': collections.defaultdict(list)}
for grp, rs in (('top', top), ('ours', ours)):
    for r in rs:
        o = our_side(r) if grp == 'ours' else None
        for t in ('0', '1'):
            key = 'top' if grp == 'top' else ('us' if int(t) == o else 'opp')
            for p in r['parts'][t]:
                if p['dph'] is not None:
                    dph[key][p['ty']].append(p['dph'])
        for p in r['pres']:
            if p['inf']:
                key = 'top' if grp == 'top' else ('us' if p['t'] == o else 'opp')
                mxhp[key][p['ty']].append(p['mx'] / BASE_H[p['ty']] if False else p['mx'])
                strs[key][p['ty']].append(p['str'])
tabout = {}
print('\nbasic-attack damage per hit (mean, n hits-fights) and max HP of fighters at fight start, strength est: top / us / opp')
for ty in range(12):
    row = []
    for key in ('top', 'us', 'opp'):
        d = dph[key][ty]
        m = mxhp[key][ty]
        s = strs[key][ty]
        row.append((round(float(np.mean(d)), 1) if d else None, len(d), round(float(np.mean(m)), 0) if m else None, round(float(np.mean(s)), 1) if s else None, len(m)))
    tabout[NAMES[ty]] = row
    print(NAMES[ty], row)
json.dump(tabout, open(OUT + 'fight_dph.json', 'w'), indent=1)
