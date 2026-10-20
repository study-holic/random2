"""cmpcells: candidate vs base mapgate JSON, per map, averaged over salts (PLAN.md changed-cell rule).

python scripts/cmpcells.py base.json cand.json [--maps a,b]

Per cell (map, opp, side): win rate and gem margin averaged over that run's salts.
Per map: dW = sum over cells of (cand win rate - base win rate), in games of 12; dGem = summed gem-margin change;
dStrong = the same dW over the cells against x-v4-14 and x-foundry-v16.
Admit when dW >= 1.5, dGem > 0 and dStrong >= 0 (both runs with at least 2 salts); with single-salt runs the
threshold is dW >= 2 (a no-op change scored +3 on expanse and +2 on lanes with one sample, 22:16).
"""
import json, sys
from collections import defaultdict

STRONG = {"x-v4-14", "x-foundry-v16"}


def load(path):
    d = json.load(open(path))
    cells = defaultdict(lambda: [0.0, 0.0, 0])
    for r in d["results"]:
        k = (r["map"], r["opp"], r["side"])
        cells[k][0] += r["win"]
        cells[k][1] += r.get("gems", 0) - r.get("gems_opp", 0)
        cells[k][2] += 1
    return {k: (w / n, g / n) for k, (w, g, n) in cells.items()}, len(d.get("salts", [0]))


def main():
    b, nb = load(sys.argv[1])
    c, nc = load(sys.argv[2])
    only = None
    if "--maps" in sys.argv:
        only = set(sys.argv[sys.argv.index("--maps") + 1].split(","))
    multi = nb >= 2 and nc >= 2
    thr = 1.5 if multi else 2.0
    maps = sorted({k[0] for k in c if k in b and (only is None or k[0] in only)})
    print(f"salts: base {nb}, cand {nc}; admit when dW >= {thr}, dGem > 0, dStrong >= 0")
    print(f"{'map':<12} {'base':>5} {'cand':>5} {'dW':>5} {'dStrong':>7} {'dGem':>7} admit")
    admitted, tot = [], 0.0
    for m in maps:
        keys = [k for k in c if k[0] == m and k in b]
        bw = sum(b[k][0] for k in keys)
        cw = sum(c[k][0] for k in keys)
        dw = cw - bw
        ds = sum(c[k][0] - b[k][0] for k in keys if k[1] in STRONG)
        dg = sum(c[k][1] - b[k][1] for k in keys)
        ok = dw >= thr and dg > 0 and ds >= 0
        tot += dw
        if ok:
            admitted.append(m)
        print(f"{m:<12} {bw:>5.2f} {cw:>5.2f} {dw:>+5.2f} {ds:>+7.2f} {dg:>+7.0f} {'ADMIT' if ok else '-'}")
    print(f"total dW {tot:+.2f}; admitted: {','.join(admitted) or 'none'}")


if __name__ == "__main__":
    main()
