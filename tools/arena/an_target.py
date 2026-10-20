"""Targeting: per round, each side's distance to the centre camp and HP lost at that round's camp phase; 'A'=A attacked that turn."""
import sys, glob
sys.path.insert(0, 'tools/arena'); from ev import lines
def timeline(d, c):
    T = {}
    for side in (0, 1):
        prev = None
        for l in lines(d, side):
            t = l.split()
            if t[0] == 'A' and len(t) > 4 and t[4] == 'atk': T.setdefault(int(t[1]), {}).setdefault(side, {})['atk'] = 1
            if t[0] == 'L' and len(t) == 9:
                r, x, y, hp = int(t[1]), int(t[4]), int(t[5]), int(t[6])
                e = T.setdefault(r, {}).setdefault(side, {}); e['d'] = max(abs(x - c[0]), abs(y - c[1])); e['hit'] = (prev - hp) if prev is not None else 0; e['hp'] = hp; prev = hp
            if t[0] == 'L' and t[-1] == 'dead': T.setdefault(int(t[1]), {}).setdefault(side, {})['dead'] = 1
    return T
if __name__ == "__main__":
    for n in range(4, 9):
        d = glob.glob(f'matches/arena/p0{n}_*')[0]; T = timeline(d, (15 + n, 15))
        row = []
        for r in sorted(T):
            if r < 38 or r > 90: continue
            a, b = T[r].get(0, {}), T[r].get(1, {})
            f = lambda e: ('X' if e.get('dead') else f"d{e.get('d')}{'-'+str(e['hit']) if e.get('hit') else ''}{'A' if e.get('atk') else ''}")
            row.append(f"{r}:{f(a)}/{f(b)}")
        print(n, ' '.join(row))
