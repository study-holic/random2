"""Per map, the factors with the largest shortfall of our games against the top-10 mutual games (ratio us / top-10, 1.0 = equal; lower-is-better factors inverted).
Factors: tributed gems per round, collected gems per round, tributes per game, first tribute (inverted), share of champion-rounds dead (inverted, +1 pp smoothing),
share at gem camps, collected per gem-camp champion-round, tributed per collected gem. Inputs: data/v3/decoded/econ.jsonl. Run: py -I tools/v3/pace_gaps.py
"""
import json, sys, statistics, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
G = [json.loads(l) for l in open(f"{V3}/decoded/econ.jsonl", encoding="utf8")]
mean = statistics.mean; med = statistics.median


def rows(cohort, side):
    out = collections.defaultdict(list)
    for g in G:
        if g["cohort"] != cohort: continue
        ts = [0, 1] if cohort == "top10" else ([g["teamIds"].index(XAN)] if side == "us" else [])
        for t in ts:
            R = max(1, g["rounds"]); tot = sum(g["cnt"][t].values()) or 1
            out[g["map"]].append({"rate": g["sg"][t] / R, "col": g["collected"][t] / R, "trib": g["tributes"][t], "first": g["first_trib"][t], "dead": g["cnt"][t].get("dead", 0) / tot,
                                  "gc": g["cnt"][t].get("gem camps", 0) / tot, "gcr": g["cnt"][t].get("gem camps", 0), "col_abs": g["collected"][t], "sg": g["sg"][t]})
    return out


US = rows("ours", "us"); TOP = rows("top10", "both")


def factors(rs):
    gcr = sum(r["gcr"] for r in rs); col = sum(r["col_abs"] for r in rs); sg = sum(r["sg"] for r in rs)
    f = [r["first"] for r in rs if r["first"]]
    return {"tributed per round": mean(r["rate"] for r in rs), "collected per round": mean(r["col"] for r in rs), "tributes per game": mean(r["trib"] for r in rs),
            "first tribute (earlier is better)": med(f) if f else None, "dead share (lower is better)": mean(r["dead"] for r in rs) * 100,
            "share at gem camps": mean(r["gc"] for r in rs) * 100, "collected per gem-camp champion-round": col / gcr if gcr else None, "tributed per collected gem": sg / col if col else None}


print("| map | n ours | tributed/round ours vs top-10 | three largest shortfalls (ratio ours / top-10; 1.0 = equal) |")
print("|---|---|---|---|")
for m in sorted(US):
    if m not in TOP: continue
    u, t = factors(US[m]), factors(TOP[m])
    ratios = {}
    for k in u:
        a, b = u[k], t[k]
        if a is None or b is None: continue
        if k.startswith("first"): r = (b + 1) / (a + 1)
        elif k.startswith("dead"): r = (b + 1) / (a + 1)
        else: r = a / b if b else None
        if r is not None: ratios[k] = r
    worst = sorted(ratios.items(), key=lambda kv: kv[1])[:3]
    print(f"| {m} | {len(US[m])}{'†' if len(US[m]) < 30 else ''} | {u['tributed per round']:.2f} vs {t['tributed per round']:.2f} | " + "; ".join(f"{k} {v:.2f}" for k, v in worst) + " |")
