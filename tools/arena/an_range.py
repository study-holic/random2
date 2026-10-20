"""Range test: for each own turn, Chebyshev distance to the camp and HP lost since the previous turn (taken at round start)."""
import sys, collections
sys.path.insert(0, 'tools/arena'); from ev import lines
def run(d, camp, W, H=31):
    out = {}
    for side in (0, 1):
        cx, cy = camp if side == 0 else (W - 1 - camp[0], H - 1 - camp[1])
        prev = None; awake = False; rows = []
        for l in lines(d, side):
            t = l.split()
            if t[0] == 'C': awake = t[8] == '1'
            if t[0] == 'L' and len(t) == 9:
                r, x, y, hp = int(t[1]), int(t[4]), int(t[5]), int(t[6])
                dist = max(abs(x - cx), abs(y - cy))
                if prev is not None and dist <= 8: rows.append((r, dist, prev - hp))
                prev = hp
        out[side] = rows
    return out
if __name__ == "__main__":
    import glob
    for n, name, W in ((0, 'wolf', 31), (1, 'elk', 33), (2, 'monolith', 35)):
        d = glob.glob(f'matches/arena/p0{n}_*')[0]; res = run(d, (16, 2), W)
        agg = collections.defaultdict(lambda: [0, 0, set()])
        for side, rows in res.items():
            for r, dist, dmg in rows:
                a = agg[dist]; a[0] += 1; a[1] += dmg > 0; dmg and a[2].add(dmg)
        print(name, ' '.join(f"d{k}:{v[1]}/{v[0]}{sorted(v[2])}" for k, v in sorted(agg.items())))
        for side, rows in res.items(): print('  side', side, ' '.join(f"{r}:{dist}:{dmg}" for r, dist, dmg in rows if dmg or 30 <= r <= 60))
