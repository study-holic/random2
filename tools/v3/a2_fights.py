# A2: fight extraction from full-action decodes (stride 1). Output: one JSON per fight.
# usage: python -I a2_fights.py out.jsonl dir1:src dir2:src ...
import json, os, sys, collections
R = 'C:/cambc-solo/'
BASE = {0: (1200, 100, 50, 5, 100, 10), 1: (900, 100, 45, 5, 100, 10), 2: (800, 100, 70, 5, 100, 10), 3: (700, 71.25, 30, 3.5, 70, 15),
        4: (900, 95, 40, 4.75, 100, 10), 5: (750, 75, 35, 3.5, 100, 10), 6: (700, 75, 35, 3.5, 80, 10), 7: (1200, 100, 45, 5, 50, 10),
        8: (700, 50, 45, 3, 105, 5), 9: (900, 100, 40, 5, 100, 10), 10: (1000, 100, 50, 5, 100, 10), 11: (750, 75, 35, 5, 100, 10)}
G = 5  # idle rounds that end a fight


def cheb(a, b):
    return max(abs(a[0] - b[0]), abs(a[1] - b[1]))


def stats_from_items(typ, items):
    h0, hg, s0, sg, f0, fg = BASE[typ]
    pts = [0, 0, 0]
    for idx, cnt in items.items():
        fam = idx // 6
        slot = idx % 6
        st = slot // 2
        emb = slot % 2
        if not emb:
            pts[st] += cnt
        elif fam == 1:
            pts[st] += 6 * cnt
        elif fam == 6:
            for k in range(3):
                pts[k] += cnt * (5 if k == st else 1)
        else:
            pts[st] += 5 * cnt
    return h0 + hg * pts[2], s0 + sg * pts[0], f0 + fg * pts[1], pts


def parse_key(k):
    out = {}
    if k:
        for t in k.split(','):
            a, b = t.split(':')
            out[int(a)] = int(b)
    return out


def load_meta():
    meta = {}
    for fn in ('games_s13.jsonl', 'games_ours.jsonl'):
        for l in open(R + 'data/v3/decoded/' + fn, encoding='utf8'):
            g = json.loads(l)
            meta.setdefault(g['match'], g)
    return meta


def process(path, mid, meta, src):
    d = json.load(open(path))
    rounds = d['rounds']
    A = d['actions']
    nch = len(rounds[0]['c'])
    snaps = {}
    item_at = {}
    cur = [{} for _ in range(nch)]
    last_items = None
    for s in rounds:
        if s.get('it'):
            for i, k in s['it']:
                cur[i] = parse_key(k)
            last_items = [dict(x) for x in cur]
        elif last_items is None:
            last_items = [dict(x) for x in cur]
        item_at[s['r']] = last_items
        if s['r'] not in snaps:
            snaps[s['r']] = s
    team = [c[0] for c in rounds[0]['c']]
    typ = [c[1] for c in rounds[0]['c']]
    camps = [(c[1], c[2]) for c in d['camps'] if c[1] >= 0]
    pos = [None] * nch
    hp = [0] * nch
    alive = [1] * nch
    lastcast = [[-99, -99] for _ in range(nch)]
    cur_r = -1
    events = []
    ev_ctx = []
    mon_hits = collections.defaultdict(list)
    for a in A:
        r, act, kind, fx, fy, tx, ty, gems, gold, hits, slot, atype, cb, ca = a
        if r != cur_r and r in snaps:
            s = snaps[r]
            for i, c in enumerate(s['c']):
                pos[i] = (c[3], c[4])
                hp[i] = c[5]
                alive[i] = c[2]
            cur_r = r
        if pos[act] is None:
            continue
        if kind == 4:
            pos[act] = (tx, ty)
        elif kind == 5 and fx >= 0:
            pos[act] = (fx, fy)
        elif kind == 6 and atype == 6:
            pos[act] = (tx, ty)
        prior_cast = tuple(tuple(x) for x in lastcast)  # cooldown state before this action (no leakage of the opening cast)
        if kind == 6 and slot in (0, 1):
            lastcast[act][slot] = r
        if not hits:
            continue
        chits = [h for h in hits if h[0] == 'c']
        evl = []
        for h in chits:  # direct hits first, so a retaliation never precedes the attack that caused it
            v = h[1]
            if v != act and team[v] != team[act]:
                evl.append((r, act, v, h[2], 'atk' if kind == 5 else 'spell', atype if kind == 6 else -1))
        for h in chits:
            if h[1] == act:
                rens = [x[1] for x in chits if x[1] != act and team[x[1]] != team[act] and typ[x[1]] == 9]
                if len(rens) == 1:
                    evl.append((r, rens[0], act, h[2], 'retal', -1))
        if evl:
            ci_ = len(ev_ctx)
            ev_ctx.append((tuple(pos), tuple(hp), tuple(alive), prior_cast))
            for e_ in evl:
                events.append(e_ + (ci_,))
        for h in chits:
            hp[h[1]] -= h[2]
            if hp[h[1]] <= 0:
                alive[h[1]] = 0
        mh = sum(1 for h in hits if h[0] == 'm')
        if mh:
            mon_hits[act].append((r, mh))
    fights = []
    active = []

    def dist_to(parts, p, posv):
        return min((cheb(posv[p], posv[q]) for q in parts if posv[q] is not None), default=99)

    for (r, a, v, dmg, kind, abil, ci) in events:
        posv, hpv, alv, lc = ev_ctx[ci]
        cands = []
        for f in active:
            if f['closed'] or r - f['last'] > G:
                continue
            if a in f['parts'] or v in f['parts'] or dist_to(f['parts'], a, posv) <= 4 or dist_to(f['parts'], v, posv) <= 4:
                cands.append(f)
        if not cands:
            f = dict(parts=set(), last=r, first=r, ev=[], closed=False, dead={0: set(), 1: set()}, hpe={})
            active.append(f)
            fights.append(f)
        else:
            f = cands[0]
            for g in cands[1:]:
                f['parts'] |= g['parts']
                f['ev'] += g['ev']
                f['ev'].sort(key=lambda e: e[6])
                f['first'] = min(f['first'], g['first'])
                g['closed'] = True
                for t in (0, 1):
                    f['dead'][t] |= g['dead'][t]
                f['hpe'].update(g['hpe'])
                if g in fights:
                    fights.remove(g)
        f['parts'] |= {a, v}
        f['last'] = r
        f['ev'].append((r, a, v, dmg, kind, abil, ci))
        hv = f['hpe'].get(v, hpv[v]) - dmg
        f['hpe'][v] = hv
        if hv <= 0 or not alv[v]:
            f['dead'][team[v]].add(v)
        for t in (0, 1):
            pt = [p for p in f['parts'] if team[p] == t]
            if pt and all(p in f['dead'][t] for p in pt):
                f['closed'] = True
        active = [x for x in active if not x['closed'] and r - x['last'] <= G]
    out = []
    mp = meta.get(mid, {})
    maxr = max(snaps)
    for f in fights:
        evs = f['ev']
        r0 = evs[0][0]
        r1 = evs[-1][0]
        posv, hpv, alv, lc = ev_ctx[evs[0][6]]
        a0, v0 = evs[0][1], evs[0][2]
        if evs[0][4] == 'retal':  # the attack itself did no HP damage; the retaliation victim is the real initiator
            a0, v0 = v0, a0
        c0 = ((posv[a0][0] + posv[v0][0]) / 2, (posv[a0][1] + posv[v0][1]) / 2)
        s = snaps.get(r0) or snaps[max(k for k in snaps if k <= r0)]
        itm = item_at.get(r0) or item_at[max(k for k in item_at if k <= r0)]
        ks = [k for k in snaps if k >= r1 + 2]
        sa = snaps[min(ks)] if ks else snaps[maxr]
        parts = f['parts']
        dealt = collections.Counter()
        taken = collections.Counter()
        nh = collections.Counter()
        nsp = collections.Counter()
        hits_atk = collections.defaultdict(list)
        for (r, a, v, dmg, kind, abil, ci) in evs:
            dealt[a] += dmg
            taken[v] += dmg
            nh[a] += 1
            if kind == 'spell':
                nsp[a] += 1
            if kind == 'atk':
                hits_atk[a].append(dmg)
        pres = []
        for i in range(nch):
            if posv[i] is None:
                continue
            dd = cheb(posv[i], c0)
            if (dd <= 8 and alv[i]) or i in parts:
                st = s['c'][i]
                mh_, sg, fc, pts = stats_from_items(typ[i], itm[i] if i < len(itm) else {})
                pres.append(dict(i=i, t=team[i], ty=typ[i], d=round(dd, 1), hp=hpv[i], mx=st[6], gems=st[7], gold=st[8], nit=st[11], stun=st[12],
                                 str=round(sg, 1), foc=round(fc, 1), pts=pts, cd0=int(r0 - lc[i][0] >= 8), cd1=int(r0 - lc[i][1] >= 8), inf=int(i in parts)))
        res = {}
        for t in (0, 1):
            deaths = []
            for p in parts:
                if team[p] == t:
                    st0 = s['c'][p]
                    sta = sa['c'][p]
                    died = (p in f['dead'][t]) or (st0[2] == 1 and sta[2] == 0)
                    if died:
                        deaths.append((p, typ[p], st0[7]))
            res[t] = deaths
        dcamp = min((cheb(c0, cp) for cp in camps), default=99)
        mh_n = 0
        for p in parts:
            for (rr, m) in mon_hits.get(p, []):
                if r0 - 1 <= rr <= r1 + 1:
                    mh_n += m
        rec = dict(game=mid, map=mp.get('map'), mapId=mp.get('mapId'), src=src, t=mp.get('t'), r0=r0, r1=r1, dur=r1 - r0 + 1, nev=len(evs),
                   c0=c0, init_team=team[a0], init_ty=typ[a0], init_kind=evs[0][4], teams=mp.get('teams'), ratings=mp.get('rating'),
                   winner=mp.get('winner'), teamIds=mp.get('teamIds'), pres=pres,
                   parts={t: [dict(i=p, ty=typ[p], dealt=dealt[p], taken=taken[p], hits=nh[p], spells=nsp[p],
                                   dph=(sum(hits_atk[p]) / len(hits_atk[p]) if hits_atk[p] else None)) for p in sorted(parts) if team[p] == t] for t in (0, 1)},
                   deaths={t: res[t] for t in (0, 1)}, dmg={t: sum(dealt[p] for p in parts if team[p] == t) for t in (0, 1)},
                   units_lost={t: len(set(3 if ty in (3, 4) else ty for (_, ty, _) in res[t])) for t in (0, 1)},
                   gems_lost={t: sum(g for (_, _, g) in res[t]) for t in (0, 1)},
                   camp_d=dcamp, mon_hits=mh_n)
        out.append(rec)
    return out


if __name__ == '__main__':
    meta = load_meta()
    outp = sys.argv[1]
    dirs = sys.argv[2:]
    done = set()
    n = 0
    with open(outp, 'w', encoding='utf8') as fo:
        for ds in dirs:
            dpath, src = ds.rsplit(':', 1)
            for fn in sorted(os.listdir(dpath)):
                mid = fn[:-5]
                if mid in done:
                    continue
                try:
                    recs = process(os.path.join(dpath, fn), mid, meta, src)
                except Exception as e:
                    print('ERR', fn, repr(e))
                    continue
                done.add(mid)
                for rc in recs:
                    fo.write(json.dumps(rc) + '\n')
                n += 1
                if n % 50 == 0:
                    print(n, 'games', flush=True)
    print('games', n)
