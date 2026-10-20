"""w12: per-death engagement context for our champions in decoded ladder games (extract_pilot.mjs, stride 1).
usage: py -I results/w12-deaths.py <decoded.json> <our_team 0|1> [label]
Per death: allies/enemies alive within 4 (Chebyshev) at the last alive snapshot and at fight start, HP at fight start,
who hit first (our side vs enemy champion within the 8 rounds before the first enemy-champion hit on the victim),
gems carried, and what the champion was doing (dist to own spawn trend over the 6 rounds before the fight).
"""
import json, sys, collections
T = ['Ryth', 'Vesper', 'Vendetta', 'Cassi', 'Issac', 'Stellar', 'Prism', 'Marcella', 'Willow', 'Ren', 'Varg', 'Tavi']
d = json.load(open(sys.argv[1], encoding='utf8'))
us = int(sys.argv[2]); label = sys.argv[3] if len(sys.argv) > 3 else ''
snaps = {}
for s in d['rounds']:
    snaps.setdefault(s['r'], s)
R = sorted(snaps)
c0 = snaps[R[0]]['c']; n = len(c0)
team = [c[0] for c in c0]
sp = {}
for t in (0, 1):
    xs = [c[3] for c in c0 if c[0] == t]; ys = [c[4] for c in c0 if c[0] == t]
    sp[t] = (sum(xs) / len(xs), sum(ys) / len(ys))
cheb = lambda a, b: max(abs(a[0] - b[0]), abs(a[1] - b[1]))
# enemy-champion hits on each champion, by round; our hits on enemies by round
hit_on = collections.defaultdict(list)   # victim -> [(round, actor, dmg, seq)]
for k, a in enumerate(d['actions']):
    r, actor = a[0], a[1]
    for h in a[9]:
        if h[0] == 'c' and team[h[1]] != team[actor]:
            hit_on[h[1]].append((r, actor, h[2], k))
deaths = []
for ra, rb in zip(R, R[1:]):
    A, B = snaps[ra]['c'], snaps[rb]['c']
    for i in range(n):
        if team[i] == us and A[i][2] and not B[i][2]:
            deaths.append((rb, i))
rows = []
for rd, i in deaths:
    last = snaps[rd - 1]['c'] if rd - 1 in snaps else None
    if last is None: continue
    me = last[i]; p = (me[3], me[4])
    # fight start: earliest enemy-champion hit on i such that hits are chained with gaps <= 6 rounds back from death
    hs = [h for h in hit_on[i] if rd - 40 <= h[0] <= rd]
    fs = None
    if hs:
        fs = hs[-1][0]
        for h in reversed(hs):
            if fs - h[0] <= 6: fs = h[0]
            else: break
    champ_dmg = sum(h[2] for h in hs if fs is not None and h[0] >= fs)
    killers = sorted({T[c0[h[1]][1]] for h in hs if fs is not None and h[0] >= fs})
    r0 = fs if fs is not None else rd - 3
    s0 = snaps.get(r0, snaps[rd - 1])['c']
    hp0 = s0[i][5]; mhp = s0[i][6]
    p0 = (s0[i][3], s0[i][4])
    al4 = sum(1 for j in range(n) if j != i and team[j] == us and s0[j][2] and cheb((s0[j][3], s0[j][4]), p0) <= 4)
    en4 = sum(1 for j in range(n) if team[j] != us and s0[j][2] and cheb((s0[j][3], s0[j][4]), p0) <= 4)
    en8 = sum(1 for j in range(n) if team[j] != us and s0[j][2] and cheb((s0[j][3], s0[j][4]), p0) <= 8)
    al4L = sum(1 for j in range(n) if j != i and team[j] == us and last[j][2] and cheb((last[j][3], last[j][4]), p) <= 4)
    en4L = sum(1 for j in range(n) if team[j] != us and last[j][2] and cheb((last[j][3], last[j][4]), p) <= 4)
    # who hit first: any hit by our side on an enemy champion in [r0-8, first enemy hit seq)
    first = None
    if fs is not None:
        k0 = min(h[3] for h in hs if h[0] == fs)
        ours = [k for k, a in enumerate(d['actions'][max(0, k0 - 400):k0], start=max(0, k0 - 400))
                if team[a[1]] == us and a[0] >= r0 - 8 and any(h[0] == 'c' and team[h[1]] != us for h in a[9])]
        first = 'us' if ours else 'them'
    # doing: trend of distance to our spawn over 6 rounds before fight start
    sb = snaps.get(r0 - 6, s0)['c'][i]
    dsp = lambda c: cheb((c[3], c[4]), sp[us])
    dse = lambda c: cheb((c[3], c[4]), sp[1 - us])
    trend = dsp(s0[i]) - dsp(sb)
    near_camp = min((cheb(p0, (cx, cy)) for (ct, cx, cy) in d['camps'] if cx >= 0), default=99)
    if trend >= 2: doing = 'advancing'
    elif trend <= -2: doing = 'retreating'
    elif near_camp <= 2: doing = 'at-camp'
    else: doing = 'holding'
    rows.append(dict(r=rd, champ=T[me[1]], x=p0[0], y=p0[1], hp0=hp0, hpf=round(hp0 / mhp, 2), gems=s0[i][7], al4=al4, en4=en4, en8=en8,
                     al4L=al4L, en4L=en4L, fs=fs, first=first, champ_dmg=champ_dmg, killers=killers, doing=doing,
                     dspawn=dsp(s0[i]), denemy=dse(s0[i]), near_camp=near_camp))
print(f'# {label} {sys.argv[1].split("/")[-1][:8]} us=team{us} deaths={len(rows)} finalsg={snaps[R[-1]]["sg"]}')
print('round champ  x  y  hp0 hpf gems al4 en4 en8 | al4L en4L | first  doing      dSp dEn camp killers')
for w in rows:
    print(f"{w['r']:5} {w['champ']:8}{w['x']:3}{w['y']:3} {w['hp0']:5} {w['hpf']:4} {w['gems']:4} {w['al4']:3} {w['en4']:3} {w['en8']:3} | {w['al4L']:4} {w['en4L']:4} | {str(w['first']):5} {w['doing']:10} {w['dspawn']:3} {w['denemy']:3} {w['near_camp']:3} {','.join(w['killers'])}")
c = collections.Counter
print('alone (al4==0):', sum(w['al4'] == 0 for w in rows), ' outnumbered (en4>al4+1... en4>al4):', sum(w['en4'] > w['al4'] for w in rows),
      ' no-champ-hit:', sum(w['fs'] is None for w in rows))
print('first:', c(w['first'] for w in rows), ' doing:', c(w['doing'] for w in rows))
print('hpf<0.5 at fight start:', sum(w['hpf'] < 0.5 for w in rows), ' gems>0:', sum(w['gems'] > 0 for w in rows),
      ' enemy-half (dSp>dEn):', sum(w['dspawn'] > w['denemy'] for w in rows))
json.dump(rows, open(sys.argv[1].replace('.json', '') .split('/')[-1] + '.w12deaths.json', 'w')) if False else None
print('JSON', json.dumps(rows))
