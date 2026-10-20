"""S2 economy simulator, no enemies. Squads (k champions together) farm camp groups and tribute gems.
Policies: greedy, value, conveyor, team, orient.  See notes/econsim.md for assumptions."""
from mapmodel import INF


class P:  # parameters (all swappable)
    def __init__(s, **kw):
        s.dpr = 50          # damage per champion per round on camps
        s.hp_budget = 700   # max damage a champion may absorb clearing a group (feasibility)
        s.reach = 1         # rounds to step onto a spawn tile after the kill
        s.cap_rounds = 5
        s.T = 900
        s.bank = 30
        s.recall = False
        s.recall_ticks = 5
        s.policy = 'value'
        s.split = None      # squad sizes
        s.scenario = 'all'  # all | half
        s.allow_mono = False
        s.gw = 0.0          # gem-equivalents per gold
        s.gem_scale = 1.0   # multiplier on gem rewards (swap engine values)
        s.beam_w = 6
        s.beam_d = 3
        for k, v in kw.items():
            setattr(s, k, v)


def ceil_div(a, b):
    return -(-int(a) // int(b))


class Sim:
    def __init__(self, M, p):
        self.M, self.p = M, p
        n = M.champs
        sizes = p.split or [n]
        assert sum(sizes) == n
        for g in M.groups:
            g['kmin'] = self.kmin(g)
            g['hp0'] = g['hp'] + (g['span'] // 2) * p.dpr
        cand = []
        for g in M.groups:
            if g['d'].get(M.spA) is None:
                continue
            if p.scenario == 'half' and M.dA.get(g['anchor'], INF) > M.dB.get(g['anchor'], INF):
                continue
            if g['mono'] and not p.allow_mono:
                continue
            if g['gems'] * p.gem_scale + p.gw * g['gold'] <= 0:
                continue
            if g['kmin'] > n:
                continue
            cand.append(g)
        self.cand = cand
        self.st = {g['id']: dict(hp=g['hp0'], ready=0, present=[], prog=None, empty=0, claim=[], claim_free=0) for g in cand}
        self.squads = [dict(id=i, bags=[0] * k, k=k, pos=M.spA, state='idle', tgt=None, ta=0, te=0) for i, k in enumerate(sizes)]
        self.total = 0
        self.gold = 0
        self.cat = dict(travel=0, clear=0, capture=0, tribute=0, wait=0)
        self.tributes = 0
        self.log = []

    def kmin(self, g):
        p = self.p
        for k in range(1, 6):
            cl = ceil_div(g['hp'], k * p.dpr)
            if g['st'] * cl / k <= p.hp_budget:
                return k
        return 99

    def dist(self, pos, g):  # steps to be in range
        d = g['d'].get(pos)
        return INF if d is None else max(0, d - self.p.reach)

    def trip(self, g):
        """round-trip overhead (rounds) of a tribute for a farm at group g."""
        M, p = self.M, self.p
        best = INF
        for a in M.my_altars:
            da = M.adist[a].get(g['anchor'], INF)
            if da >= INF:
                continue
            walk = 2 * da
            if p.recall:
                sp = M.adist[a].get(M.spA, INF)
                sg = g['d'].get(M.spA, INF)
                walk = min(walk, p.recall_ticks + sp + sg)
            best = min(best, walk)
        return best + ceil_div(p.bank, 10)

    def claimed_k(self, g, sq):
        return sum(o['k'] for o in self.st[g['id']]['claim'] if o is not sq)

    def est(self, sq, g, t, pos, over=None):
        p = self.p
        s = self.st[g['id']]
        tr = self.dist(pos, g)
        if tr >= INF:
            return None
        arrive = t + tr
        ck = self.claimed_k(g, sq)
        excl = p.policy in ('value', 'conveyor', 'orient')
        avail = s['ready'] if over is None or g['id'] not in over else over[g['id']]
        keff = sq['k']
        if excl and ck > 0:
            if ck < g['kmin'] <= ck + sq['k']:
                keff = sq['k'] + ck
                avail = min(avail, arrive)  # join a squad that needs help
            else:
                avail = max(avail, s['claim_free'])
        elif not excl:
            keff = sq['k'] + ck
        if keff < g['kmin']:
            return None
        start = max(arrive, avail)
        clear = ceil_div(g['hp0'], keff * p.dpr)
        done = start + clear + p.reach + p.cap_rounds
        return dict(arrive=arrive, start=start, done=done, keff=keff, clear=clear)

    def gain(self, g):
        return g['gems'] * self.p.gem_scale + self.p.gw * g['gold']

    def val(self, sq, g, e, t):
        dur = (e['done'] - t) + (self.gain(g) / max(1, sq['k'])) * self.trip(g) / self.p.bank
        return self.gain(g) / dur

    def choose(self, sq, t):
        p = self.p
        opts = []
        for g in self.cand:
            e = self.est(sq, g, t, sq['pos'])
            if e:
                opts.append((g, e))
        if not opts:
            return None
        if p.policy == 'greedy':
            alive = [(g, e) for g, e in opts if e['start'] <= e['arrive'] + 1]
            if alive:
                return min(alive, key=lambda ge: (ge[1]['arrive'], -ge[0]['gems']))[0]
            return min(opts, key=lambda ge: (ge[1]['start'], -ge[0]['gems']))[0]
        if p.policy == 'orient':
            return self.beam(sq, t)
        return max(opts, key=lambda ge: self.val(sq, ge[0], ge[1], t))[0]

    def beam(self, sq, t0):
        p = self.p
        beams = [(0.0, 0.0, t0, sq['pos'], {}, None)]
        best = (-1, None)
        for depth in range(p.beam_d):
            nb = []
            for gain, tripc, t, pos, over, f in beams:
                for g in self.cand:
                    e = self.est(sq, g, t, pos, over)
                    if not e:
                        continue
                    gg = self.gain(g)
                    ng = gain + gg
                    ntr = tripc + (gg / max(1, sq['k'])) * self.trip(g) / p.bank
                    o2 = dict(over)
                    o2[g['id']] = e['done'] + g['resp']
                    sc = ng / max(1, (e['done'] - t0) + ntr)
                    nb.append((sc, ng, ntr, e['done'], g['anchor'], o2, f if f is not None else g))
            nb.sort(key=lambda x: -x[0])
            nb = nb[:p.beam_w]
            if nb and nb[0][0] > best[0]:
                best = (nb[0][0], nb[0][6])
            beams = [(b[1], b[2], b[3], b[4], b[5], b[6]) for b in nb]
        return best[1]

    def decide(self, sq, t):
        p, M = self.p, self.M
        if max(sq['bags']) >= p.bank:
            best = None
            for a in M.my_altars:
                da = M.adist[a].get(sq['pos'], INF)
                if da < INF:
                    c = (max(0, da - 1), a)
                    best = c if best is None or c < best else best
                sp = M.adist[a].get(M.spA, INF)
                if p.recall and sp < INF and sq['pos'] != M.spA:
                    c = (p.recall_ticks + max(0, sp - 1), a)
                    best = c if best is None or c < best else best
            if best:
                sq['state'] = 'tmove'
                sq['ta'] = t + best[0]
                sq['pos'] = best[1]
                sq['tgt'] = None
                return
        g = self.choose(sq, t)
        if g is None:
            fl = sq.get('follow')
            lead = [fl] if fl is not None and fl['tgt'] is not None else [o for o in self.squads if o is not sq and o['tgt'] is not None]
            if lead and self.dist(sq['pos'], lead[0]['tgt']) < INF:
                g = lead[0]['tgt']
        if g is None:
            sq['state'] = 'idle'
            self.cat['wait'] += sq['k']
            return
        e = self.est(sq, g, t, sq['pos'])
        sq['state'] = 'move'
        sq['tgt'] = g
        sq['ta'] = t + self.dist(sq['pos'], g)
        s = self.st[g['id']]
        s['claim'].append(sq)
        if e:
            s['claim_free'] = max(s['claim_free'], e['done'] + g['resp'])
        sq['pos'] = g['anchor']

    def leave(self, sq, g):
        s = self.st[g['id']]
        if sq in s['present']:
            s['present'].remove(sq)
        if sq in s['claim']:
            s['claim'].remove(sq)
        sq['tgt'] = None
        sq['state'] = 'idle'
        sq['pos'] = g['anchor']

    def can_release(self, g, s):
        # release only if the rest of the squad can still take some next group
        q = s['present'][0]
        if q['k'] > 1:
            fake = dict(q)
            fake['k'] = q['k'] - 1
            fake['pos'] = g['anchor']
            fake['id'] = -1
            sv = self.st[g['id']]['claim']
            full = dict(q)
            full['pos'] = g['anchor']
            full['id'] = -2
            opts = [(h, self.est(full, h, 0, g['anchor'])) for h in self.cand if h is not g]
            opts = [(h, e) for h, e in opts if e]
            if not opts:
                return False
            h = max(opts, key=lambda he: self.val(full, he[0], he[1], 0))[0]
            return bool(self.est(fake, h, 0, g['anchor']))
        return len(s['present']) > 1

    def release(self, g, s):
        """conveyor: one capturer (the member with the smallest bag) stays, everyone else goes on."""
        pr = s['present']
        q = pr[0]
        if q['k'] > 1:
            i = min(range(q['k']), key=lambda j: q['bags'][j])
            cap = dict(id=len(self.squads), follow=q, bags=[q['bags'][i]], k=1, pos=g['anchor'], state='at', tgt=g, ta=0, te=0)
            q['bags'] = [b for j, b in enumerate(q['bags']) if j != i]
            q['k'] -= 1
            self.squads.append(cap)
            pr.append(cap)
            s['claim'].append(cap)
        else:
            cap = q
        for o in list(pr):
            if o is not cap:
                self.leave(o, g)

    def run(self):
        p = self.p
        for t in range(p.T):
            for sq in list(self.squads):
                if sq['state'] == 'idle':
                    self.decide(sq, t)
            for sq in list(self.squads):
                if sq['state'] == 'move' and sq['ta'] <= t:
                    sq['state'] = 'at'
                    self.st[sq['tgt']['id']]['present'].append(sq)
                elif sq['state'] == 'tmove' and sq['ta'] <= t:
                    sq['state'] = 'trib'
                    sq['te'] = t + ceil_div(max(sq['bags']), 10)
                if sq['state'] == 'trib' and sq['te'] <= t:
                    self.total += sum(sq['bags'])
                    self.tributes += 1
                    sq['bags'] = [0] * sq['k']
                    sq['state'] = 'idle'
                    self.decide(sq, t)
            for sq in self.squads:
                if sq['state'] in ('move', 'tmove'):
                    self.cat['travel'] += sq['k']
                elif sq['state'] == 'trib':
                    self.cat['tribute'] += sq['k']
            for g in self.cand:
                s = self.st[g['id']]
                pr = s['present']
                if not pr:
                    s['empty'] += 1
                    if s['empty'] >= 2 and s['prog'] is None:
                        s['hp'] = g['hp0']
                    if s['prog'] is not None:
                        s['prog'] = -p.reach  # capture progress resets when the region is empty
                    continue
                s['empty'] = 0
                K = sum(q['k'] for q in pr)
                if t < s['ready']:
                    self.cat['wait'] += K
                    continue
                if s['hp'] > 0:
                    if K >= g['kmin']:
                        s['hp'] -= K * p.dpr
                        self.cat['clear'] += K
                        if s['hp'] <= 0:
                            s['prog'] = -p.reach
                            if p.policy == 'conveyor' and K > 1 and self.can_release(g, s):
                                self.release(g, s)
                    else:
                        self.cat['wait'] += K
                    continue
                K = sum(q['k'] for q in s['present'])
                s['prog'] = (s['prog'] if s['prog'] is not None else -p.reach) + 1
                self.cat['capture'] += K
                if s['prog'] >= p.cap_rounds:
                    gems = int(g['gems'] * p.gem_scale)
                    self.gold += g['gold']
                    mem = [(q, i) for q in s['present'] for i in range(q['k'])]
                    sh, rem = divmod(gems, len(mem))
                    for j, (q, i) in enumerate(mem):
                        q['bags'][i] += sh + (rem if j == 0 else 0)
                    roots = set()
                    for q in s['present']:
                        r = q
                        while r.get('follow') is not None:
                            r = r['follow']
                        roots.add(r['id'])
                    self.log.append((t, g['id'], sorted(roots)))
                    s['ready'] = t + g['resp']
                    s['hp'] = g['hp0']
                    s['prog'] = None
                    s['claim_free'] = s['ready']
                    for q in list(s['present']):
                        self.leave(q, g)
        return self.result()

    def result(self):
        tot = sum(self.cat.values()) or 1
        return dict(gpr=self.total / self.p.T, gold_pr=self.gold / self.p.T, tributes=self.tributes,
                    share={k: v / tot for k, v in self.cat.items()}, total=self.total)
