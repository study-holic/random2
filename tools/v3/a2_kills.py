"""Round-level (stride 13) kill events: killer type, victim type, isolation context. Used for a larger-sample duel (isolated kill) matrix.
usage: python -I a2_kills.py out.jsonl"""
import sys, os, json
sys.path.insert(0, 'C:/cambc-solo/tools/v3')
from a2_fights import stats_from_items, parse_key, load_meta, cheb
R = 'C:/cambc-solo/'
meta = load_meta()
out = open(sys.argv[1], 'w', encoding='utf8')
seen = set()
ng = 0
nev = 0
for dname, src in (('s13', 's13'), ('ours_s13', 'ours')):
    dp = R + 'data/v3/decoded/' + dname
    for fn in sorted(os.listdir(dp)):
        mid = fn[:-5]
        if mid in seen:
            continue
        seen.add(mid)
        mp = meta.get(mid)
        if mp is None:
            continue
        try:
            d = json.load(open(os.path.join(dp, fn)))
        except Exception as e:
            continue
        rounds = d['rounds']
        nch = len(rounds[0]['c'])
        cur = [{} for _ in range(nch)]
        snaps = []
        for s in rounds:
            for i, k in s.get('it', []):
                cur[i] = parse_key(k)
            snaps.append((s, [dict(x) for x in cur]) if s.get('it') or not snaps else (s, snaps[-1][1]))
        ng += 1
        for a in range(len(snaps) - 1):
            s0, it0 = snaps[a]
            s1, _ = snaps[a + 1]
            if s1['r'] - s0['r'] != 1 or s1.get('final'):
                continue
            c0, c1 = s0['c'], s1['c']
            dead = [i for i in range(nch) if c0[i][2] == 1 and c1[i][2] == 0]
            if not dead:
                continue
            for v in dead:
                tv = c0[v][0]
                ty = c0[v][1]
                if ty == 4 and (v - 1) in dead and c0[v - 1][1] == 3 and c0[v - 1][0] == tv:
                    continue  # partner counted with Cassi
                px = (c0[v][3], c0[v][4])
                killers = [w for w in range(nch) if c0[w][0] != tv and c1[w][9] > c0[w][9]]
                near = lambda i, rad: [w for w in range(nch) if c0[w][2] == 1 and w != i and cheb((c0[w][3], c0[w][4]), px) <= rad]
                enem6 = [w for w in near(v, 6) if c0[w][0] != tv]
                ally6 = [w for w in near(v, 6) if c0[w][0] == tv and not (c0[w][1] in (3, 4) and ty in (3, 4))]
                k = killers[0] if len(killers) == 1 else None
                if k is None and len(killers) > 1:
                    k = min(killers, key=lambda w: cheb((c0[w][3], c0[w][4]), px))
                kinfo = None
                if k is not None:
                    _, sk, fk, ptsk = stats_from_items(c0[k][1], it0[k])
                    kinfo = dict(i=k, ty=c0[k][1], hp=c0[k][5], mx=c0[k][6], str=round(sk, 1), foc=round(fk, 1), d=cheb((c0[k][3], c0[k][4]), px))
                _, sv, fv, ptsv = stats_from_items(ty, it0[v])
                rec = dict(game=mid, map=mp.get('map'), t=mp.get('t'), src=src, r=s0['r'], teams=mp.get('teams'), teamIds=mp.get('teamIds'), ratings=mp.get('rating'),
                           vteam=tv, v=dict(i=v, ty=ty, hp=c0[v][5], mx=c0[v][6], str=round(sv, 1), foc=round(fv, 1), gems=c0[v][7]), k=kinfo,
                           nk=len(killers), enem6=[(c0[w][1], c0[w][5], c0[w][6]) for w in enem6], ally6=[(c0[w][1], c0[w][5], c0[w][6]) for w in ally6])
                out.write(json.dumps(rec) + '\n')
                nev += 1
        if ng % 200 == 0:
            print(ng, 'games', nev, 'deaths', flush=True)
print('games', ng, 'death events', nev)
