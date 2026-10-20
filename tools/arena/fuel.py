"""Fuel per turn from a replay bundle: python tools/arena/fuel.py <bundle> -> draft, round-1 and later maxima, exhausted count."""
import sys, zipfile, struct
def fuel(f, side=0):
    b = zipfile.ZipFile(f).read(f"team-{'ab'[side]}.fuel"); n = (len(b) - 13) // 8
    v = struct.unpack(f'<{n}Q', b[13:13 + 8 * n]); return v[8:13], v[13:]
if __name__ == "__main__":
    for side in (0, 1):
        d, g = fuel(sys.argv[1], side); print(side, 'draft', d, 'game first', g[:5], 'max', max(g), 'exhausted', sum(x >= 100000 for x in g))
