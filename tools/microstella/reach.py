import sys,re,collections
def load(path):
    s=open(path,'rb').read().decode('latin1')
    lines=[l.rstrip('\r').lstrip('\x00') for l in s.split('\n')]
    m=re.search(r'MAP (\d+)x(\d+)',s); W,H=int(m.group(1)),int(m.group(2))
    rows=[l for l in lines if len(l)>=W and re.fullmatch(r'[.#"oabAM ]+',l[:W])]
    rows=rows[-H:]
    groups=[]
    for l in lines:
        g=re.match(r'GROUP (\d+) gems (\d+)(?: dist (-?\d+))? :(.*)',l)
        if g: groups.append((int(g.group(1)),int(g.group(2)),[(k,(int(x),int(y))) for k,x,y in re.findall(r'(\w+)@\((-?\d+), (-?\d+)\)',g.group(4))]))
    altars=[tuple(int(x) for x in re.findall(r'\d+',l)) for l in lines if l.startswith('ALTAR')]
    spawnA=[tuple(int(x) for x in re.findall(r'\d+',l)) for l in lines if l.startswith('SPAWN_A')]
    return W,H,rows,groups,altars,spawnA
def main(path):
    W,H,rows,groups,altars,spawnA=load(path)
    assert len(rows)==H, (len(rows),H)
    camp=set(); 
    for _,_,cs in groups: 
        for _,p in cs: camp.add(p)
    def passable(x,y): return 0<=x<W and 0<=y<H and rows[y][x] in '."abA'
    def bfs(starts, blocked_extra=set()):
        dist={s:0 for s in starts}; q=collections.deque(starts)
        while q:
            x,y=q.popleft()
            for dx in (-1,0,1):
                for dy in (-1,0,1):
                    n=(x+dx,y+dy)
                    if n in dist or not passable(*n) or n in camp: continue
                    dist[n]=dist[(x,y)]+1; q.append(n)
        return dist
    R0=bfs(spawnA)
    # one TP hop: from any R0 tile to a passable tile within chebyshev 3 not in R0
    hop=set()
    for (x,y),d in R0.items():
        for dx in range(-3,4):
            for dy in range(-3,4):
                n=(x+dx,y+dy)
                if n not in R0 and passable(*n) and n not in camp: hop.add(n)
    R1=bfs(list(hop)) if hop else {}
    def near(p,R): return any((p[0]+dx,p[1]+dy) in R for dx in (-1,0,1) for dy in (-1,0,1))
    g0=[g for g in groups if any(near(p,R0) for _,p in g[2])]
    g1=[g for g in groups if g not in g0 and any(near(p,R1) for _,p in g[2])]
    gx=[g for g in groups if g not in g0 and g not in g1]
    print(f'{path.split("/")[-2]}: walk {len(g0)} groups {sum(g[1] for g in g0)} gems | 1-TP {len(g1)} groups {sum(g[1] for g in g1)} gems | unreachable {len(gx)} ({sum(g[1] for g in gx)} gems)')
    print('  altars walk:',[a for a in altars if near(a,R0)],' 1-TP:',[a for a in altars if not near(a,R0) and near(a,R1)])
    if g1: print('  1-TP groups:',[(g[0],g[1]) for g in g1])
for p in sys.argv[1:]: main(p)
