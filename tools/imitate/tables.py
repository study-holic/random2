"""Binned decision rates from tools/imitate/policies.py output: python3 -I tools/imitate/tables.py policies.json"""
import json, sys, collections
T = {tuple(k.split("|")): v for k, v in json.load(open(sys.argv[1])).items()}
LAB = dict(hp=["<0.3", "0.3-0.5", "0.5-0.7", "0.7-0.9", ">=0.9"], bag=["0", "1-9", "10-19", "20-39", "40-79", "80+"],
           ds=["3-5", "6-10", "11-20", "21-30", "31+"], en8=["0", "1", "2", "3+"], da=["0-1", "2-3", "4-7", "8-15", "16+"],
           ratio=["<0.5", "0.5-0.8", "0.8-1.25", "1.25-2", ">=2"], gd=["<-3", "-3..-1", "-1..1", "1..3", ">=3"], al4=["0", "1", "2+"], dn=["0", "1", "2", "3", "4"])
DIMS = dict(recall=["hp", "bag", "ds", "en8"], bank=["bag", "da", "en8"], engage=["ratio", "gd", "al4", "dn"])
def table(kind, rowdim, coldim=None, sides=("v4", "top10"), filt=None, minn=200):
    dims = DIMS[kind]; ri = dims.index(rowdim); ci = dims.index(coldim) if coldim else None
    agg = collections.defaultdict(lambda: [0, 0])
    for k, (n, e) in T.items():
        if k[0] != kind or k[1] not in sides: continue
        b = list(map(int, k[2:]))
        if filt and not all(b[dims.index(d)] in vs for d, vs in filt.items()): continue
        agg[(k[1], b[ri], b[ci] if ci is not None else 0)][0] += n; agg[(k[1], b[ri], b[ci] if ci is not None else 0)][1] += e
    cols = LAB[coldim] if coldim else ["all"]
    print(f"\n{kind}: P(event) by {rowdim}" + (f" x {coldim}" for _ in [0]).__next__() if coldim else f"\n{kind}: P(event) by {rowdim}", ("filter " + str(filt)) if filt else "")
    print("| " + rowdim + " | " + " | ".join(f"{s} {c}" for s in sides for c in cols) + " |")
    print("|---" * (1 + len(sides) * len(cols)) + "|")
    for r, rl in enumerate(LAB[rowdim]):
        cells = []
        for s in sides:
            for c in range(len(cols)):
                n, e = agg.get((s, r, c), [0, 0])
                cells.append(f"{100 * e / n:.1f}% ({n})" if n >= minn else (f"({n})" if n else "-"))
        print(f"| {rl} | " + " | ".join(cells) + " |")
if __name__ == "__main__":
    table("recall", "hp", None, ("v4", "top10", "pantheon"))
    table("recall", "hp", "en8")
    table("recall", "hp", "bag", filt=None)
    table("recall", "ds", None, ("v4", "top10", "pantheon"), filt={"hp": [0, 1]})
    table("bank", "bag", "da")
    table("bank", "bag", None, ("v4", "top10", "pantheon"), filt={"da": [0]})
    table("bank", "en8", None, ("v4", "top10"), filt={"da": [0], "bag": [2, 3, 4, 5]})
    table("engage", "ratio", "al4")
    table("engage", "gd", None, ("v4", "top10", "pantheon"))
    table("engage", "dn", None, ("v4", "top10", "pantheon"))
