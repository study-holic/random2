"""Tables for results/v5r.md: per-map head-to-head and unopposed rows for chosen maps.
usage: python3 tools/s1/v5r_tables.py h2h <run.json> [null.json] [maps...] | unopp <base.json> <new.json> [maps...]"""
import json, sys
NEW = ["packs", "pilgrimage", "quadrants", "reverse_spiral", "rows", "rushbait_v2", "shortcut", "split", "stampede", "starfish", "starvation", "swirl", "the_farm", "yellow_brick_road"]
def load(p): return json.load(open(p))["results"]
mode = sys.argv[1]
if mode == "h2h":
    R = load(sys.argv[2]); N = load(sys.argv[3]) if len(sys.argv) > 3 and sys.argv[3].endswith(".json") else None
    maps = [m for m in sys.argv[4 if N else 3:]] or NEW
    print("| map | wins/6 | deaths/game | gem margin/game |" + (" null wins/6 | null deaths |" if N else ""))
    print("|---|---|---|---|" + ("---|---|" if N else ""))
    for m in maps:
        x = [r for r in R if r["map"] == m]
        if not x: continue
        row = "| %s | %d | %.1f | %+.0f |" % (m, sum(r["win"] for r in x), sum(r["deaths"] for r in x) / len(x), sum(r["gems"] - r["gems_opp"] for r in x) / len(x))
        if N:
            y = [r for r in N if r["map"] == m]
            row += " %d | %.1f |" % (sum(r["win"] for r in y), sum(r["deaths"] for r in y) / max(1, len(y)))
        print(row)
else:
    A, B = load(sys.argv[2]), load(sys.argv[3]); maps = sys.argv[4:] or NEW
    print("| map | base gems (A/B) | new gems (A/B) | base last round | new last round |\n|---|---|---|---|---|")
    for m in maps:
        a = sorted([r for r in A if r["map"] == m], key=lambda r: (r["salt"], r["side"])); b = sorted([r for r in B if r["map"] == m], key=lambda r: (r["salt"], r["side"]))
        f = lambda L: " / ".join(str(r.get("gems", 0)) for r in L)
        g = lambda L: " / ".join(str(r.get("round", 0)) for r in L)
        print("| %s | %s | %s | %s | %s |" % (m, f(a), f(b), g(a), g(b)))
