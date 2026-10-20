"""Compare unopposed runs per map (admission rule 2 of v5-s1-economy.md): round to 1,000 tributed gems, projected as
1000 / (mean over sides and salts of gems / last round); equals the round reached when every game reaches 1,000 at the same round. usage: python3 tools/s1/cmp.py base.json new.json [--md]"""
import json, sys, statistics
def load(p):
    d = json.load(open(p)); m = {}
    for r in d["results"]:
        g = r.get("gems", 0)
        x = g / max(1, r["round"])   # gem rate; the map's round to 1,000 = 1000 / mean rate
        m.setdefault(r["map"], {"r": [], "g": [], "d": []})
        m[r["map"]]["r"].append(x); m[r["map"]]["g"].append(g); m[r["map"]]["d"].append(r.get("deaths", 0))
    out = {k: {f: sum(v[f]) / len(v[f]) for f in v} for k, v in m.items()}
    for v in out.values(): v["r"] = min(1e5, 1000 / v["r"]) if v["r"] > 0 else 1e5
    return out
a, b = load(sys.argv[1]), load(sys.argv[2])
ks = set(a) & set(b); a = {k: a[k] for k in ks}; b = {k: b[k] for k in ks}
md = "--md" in sys.argv
print("| map | base r1000 | new r1000 | change | base gems | new gems | base deaths | new deaths |\n|---|---|---|---|---|---|---|---|" if md else
      "%-20s %7s %7s %7s %6s %6s %5s %5s" % ("map", "base", "new", "chg%", "gemsA", "gemsB", "dA", "dB"))
worst = []
for k in sorted(a):
    ra, rb = a[k]["r"], b[k]["r"]; ch = 100 * (rb - ra) / ra
    worst.append((ch, k))
    f = "| %s | %.0f | %.0f | %+.0f %% | %.0f | %.0f | %.1f | %.1f |" if md else "%-20s %7.0f %7.0f %+6.0f%% %6.0f %6.0f %5.1f %5.1f"
    print(f % (k, ra, rb, ch, a[k]["g"], b[k]["g"], a[k]["d"], b[k]["d"]))
ma, mb = statistics.median(v["r"] for v in a.values()), statistics.median(v["r"] for v in b.values())
worst.sort(reverse=True)
print("median %.0f -> %.0f (%+.1f %%; rule: <= -15 %%); worst map %s %+.0f %% (rule: <= +10 %%); total gems %.0f -> %.0f; deaths %.1f -> %.1f" % (
    ma, mb, 100 * (mb - ma) / ma, worst[0][1], worst[0][0], sum(v["g"] for v in a.values()), sum(v["g"] for v in b.values()),
    sum(v["d"] for v in a.values()) / len(a), sum(v["d"] for v in b.values()) / len(b)))
