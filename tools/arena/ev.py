"""Compact event view of an arena-probe replay: python tools/arena/ev.py <scenario dir> [side] -> non-L lines + L lines with changes."""
import sys, zipfile, glob
def lines(d, side):
    f = glob.glob(f"{d}/*.repl26a")[0]; z = zipfile.ZipFile(f)
    return [l.replace("\x00", "").strip() for l in z.read(f"team-{'ab'[side]}.stdout").decode().splitlines() if l.replace("\x00", "").strip()]
if __name__ == "__main__":
    d = sys.argv[1]; sides = [int(sys.argv[2])] if len(sys.argv) > 2 else [0, 1]
    for s in sides:
        last = {}
        for l in lines(d, s):
            t = l.split()
            if t[0] == 'L' and len(t) == 9:
                k = t[3]; v = tuple(t[6:9])
                if last.get(k) == v: continue
                last[k] = v
            print(l)
