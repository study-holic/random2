"""Ablation rows for unopposed runs: python3 tools/s1/abl.py base.json name=run.json ...  (markdown table)
Columns: median round to 1,000 (1000 / mean gem rate per map), maps under 2,000, total gems over maps (mean per map-side),
deaths per game, fuel-exhausted turns per game, and the change vs base (median; worst and best map)."""
import json, statistics, sys
def load(p):
    d = json.load(open(p)); m = {}
    for r in d["results"]:
        x = m.setdefault(r["map"], {"rate": [], "g": [], "d": [], "f": []})
        x["rate"].append(r.get("gems", 0) / max(1, r["round"])); x["g"].append(r.get("gems", 0)); x["d"].append(r.get("deaths", 0)); x["f"].append(r.get("fuel_late", 0))
    out = {}
    for k, x in m.items():
        rt = sum(x["rate"]) / len(x["rate"])
        out[k] = {"r": 1000 / rt if rt > 0 else 1e5, "g": sum(x["g"]) / len(x["g"]), "d": sum(x["d"]) / len(x["d"]), "f": sum(x["f"]) / len(x["f"])}
    return out
base = load(sys.argv[1])
print("| run | median r1000 | maps < 2,000 | gems (sum of map means) | deaths/game | fuel-out turns/game | median vs base | worst map vs base | best map vs base |")
print("|---|---|---|---|---|---|---|---|---|")
for arg in ["base=" + sys.argv[1]] + sys.argv[2:]:
    name, p = arg.split("=", 1); b = load(p); ks = sorted(set(b) & set(base))
    med = statistics.median(b[k]["r"] for k in ks); mb = statistics.median(base[k]["r"] for k in ks)
    ch = sorted(((100 * (b[k]["r"] - base[k]["r"]) / base[k]["r"]), k) for k in ks if base[k]["r"] < 1e5)
    print("| %s | %.0f | %d | %.0f | %.1f | %.1f | %+.1f %% | %s %+.0f %% | %s %+.0f %% |" % (name, med, sum(1 for k in ks if b[k]["r"] <= 2000), sum(b[k]["g"] for k in ks),
        sum(b[k]["d"] for k in ks) / len(ks), sum(b[k]["f"] for k in ks) / len(ks), 100 * (med - mb) / mb, ch[-1][1], ch[-1][0], ch[0][1], ch[0][0]))
