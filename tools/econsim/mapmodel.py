"""Map model for econsim: grid, BFS fields, groups, altars, reachability (walk / dash / crystal)."""
import json, math, os, collections as C
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
INF = 10**6
HP = {0: 1350, 1: 2400, 2: 120, 3: 120, 4: 20000, 5: 5000}
ST = {0: 20, 1: 60, 2: 0, 3: 0, 4: 200, 5: 120}
NM = {0: 'wolf', 1: 'elk', 2: 'gem', 3: 'gold', 4: 'mono', 5: 'golem'}
D8 = [(a, b) for a in (-1, 0, 1) for b in (-1, 0, 1) if (a or b)]

def mirror_fn(m):
    w, h, sy = m['w'], m['h'], m['sym']
    if sy == 2: return lambda x, y: (w - 1 - x, h - 1 - y)
    if sy == 0: return lambda x, y: (w - 1 - x, y)
    return lambda x, y: (x, h - 1 - y)

def load_all(path=None):
    D = json.load(open(path or os.path.join(ROOT, 'data', 'maps21.json')))
    for m in D.values():
        mir = mirror_fn(m); G = []
        for g in m['grp']:
            G.append(g); mc = [[t, *mir(x, y)] for t, x, y in g['camps']]
            if sorted(map(tuple, mc)) != sorted(map(tuple, g['camps'])): G.append({'info': g['info'], 'camps': mc})
        m['grp'] = G
    return D

class MapModel:
    def __init__(self, name, m, mode='walk'):
        self.mode = mode; self.name = name; self.m = m; w, h = m['w'], m['h']; self.w, self.h = w, h
        self.champs = m['champs']; mir = mirror_fn(m)
        self.wall = [[False] * w for _ in range(h)]; self.hole = [[False] * w for _ in range(h)]
        for x, y in m['walls']: self.wall[y][x] = True
        for x, y in m['holes']: self.hole[y][x] = True
        x0, y0, x1, y1 = m['spawnA']
        self.spA = self.nearest_free(((x0 + x1) // 2, (y0 + y1) // 2))
        self.spB = self.nearest_free(mir(*self.spA))
        self.altars = [tuple(a) for a in m['altars']]
        self.cache = {}
        self.dA = self.bfs(self.spA); self.dB = self.bfs(self.spB)
        # groups
        self.groups = []
        for g in m['grp']:
            gems, gold, resp, n = g['info']; cs = g['camps']
            cx = sum(c[1] for c in cs) / len(cs); cy = sum(c[2] for c in cs) / len(cs)
            a = min(cs, key=lambda c: (c[1] - cx) ** 2 + (c[2] - cy) ** 2); anc = (a[1], a[2])
            span = max(max(abs(c[1] - d[1]) for d in cs for _ in [0]) for c in cs) if len(cs) > 1 else 0
            span = max(max(abs(c[1] - d[1]), abs(c[2] - d[2])) for c in cs for d in cs)
            self.groups.append(dict(gems=gems, gold=gold, resp=resp, anchor=anc, span=span,
                hp=sum(HP[c[0]] for c in cs), st=sum(ST[c[0]] for c in cs), mono=any(c[0] == 4 for c in cs),
                comp='+'.join(f"{k}{NM[t]}" for t, k in sorted(C.Counter(c[0] for c in cs).items()))))
        for i, g in enumerate(self.groups):
            g['id'] = i; g['d'] = self.bfs(g['anchor'])
            g['dA'] = self.dA.get(g['anchor'], INF); g['dB'] = self.dB.get(g['anchor'], INF)
            # reach by walking from our spawn
            g['walk'] = g['dA'] < INF
        # our altars: nearer to A than B
        al = [(a, self.dA.get(a, INF), self.dB.get(a, INF)) for a in self.altars]
        mine = [a for a, da, db in al if da <= db and da < INF] or [min(al, key=lambda t: t[1])[0]]
        self.my_altars = mine
        self.adist = {a: self.bfs(a) for a in mine}
        self.dash_reach = self.reach_set('dash'); self.cry_reach = self.reach_set('crystal')

    def free(self, x, y): return 0 <= x < self.w and 0 <= y < self.h and not self.wall[y][x] and not self.hole[y][x]
    def nearest_free(self, p):
        for r in range(0, 20):
            for dx in range(-r, r + 1):
                for dy in range(-r, r + 1):
                    if max(abs(dx), abs(dy)) == r and self.free(p[0] + dx, p[1] + dy): return (p[0] + dx, p[1] + dy)
        return p
    def bfs(self, src, mode=None):
        mode = mode or self.mode
        """Distances in king moves; source may be a camp/altar tile (blocked terrain is expanded from the source only)."""
        key = (src, mode)
        if key in self.cache: return self.cache[key]
        d = {src: 0}; q = C.deque([src])
        while q:
            x, y = q.popleft(); dd = d[(x, y)] + 1
            for a, b in D8:
                nx, ny = x + a, y + b
                if (nx, ny) not in d and self.free(nx, ny):
                    d[(nx, ny)] = dd; q.append((nx, ny))
            if mode == 'dash':
                for a in range(-2, 3):
                    for b in range(-2, 3):
                        if max(abs(a), abs(b)) == 2:
                            nx, ny = x + a, y + b
                            mx, my = x + (a // 2 if a % 2 == 0 else (1 if a > 0 else -1)), y + (b // 2 if b % 2 == 0 else (1 if b > 0 else -1))
                            if (nx, ny) not in d and self.free(nx, ny) and 0 <= mx < self.w and 0 <= my < self.h and not self.wall[my][mx]:
                                d[(nx, ny)] = dd; q.append((nx, ny))
            elif mode == 'crystal':
                for a in range(-3, 4):
                    for b in range(-3, 4):
                        nx, ny = x + a, y + b
                        if (nx, ny) not in d and self.free(nx, ny):
                            d[(nx, ny)] = dd + 2; q.append((nx, ny))
        self.cache[key] = d
        return d
    def reach_set(self, mode):
        d = self.bfs(self.spA, mode)
        return [g['anchor'] in d or any((g['anchor'][0] + a, g['anchor'][1] + b) in d for a, b in D8) for g in self.groups]

if __name__ == '__main__':
    D = load_all()
    for n, m in D.items():
        M = MapModel(n, m); un = [g for g in M.groups if not g['walk']]
        print(n, len(M.groups), 'unreach', len(un), 'dash+', sum(1 for g, r in zip(M.groups, M.dash_reach) if not g['walk'] and r), 'cry+', sum(1 for g, r in zip(M.groups, M.cry_reach) if not g['walk'] and r), 'altars', len(M.my_altars))
