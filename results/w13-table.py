"""w13: per-map table, base (v2) vs candidate panel runs, pace against the top-50 yardstick, and the head-to-head.

python results/w13-table.py results/v2-panel.json results/w13-scoring.json [results/w13-h2h.json]
"""
import json, sys

TOP50 = {"ambush": 594, "collision": 695, "contest": 1602, "corners": 536, "eight": 640, "elkring": 577, "expanse": 513,
         "fragment": 772, "interlock": 663, "lanes": 468, "monolith": 447, "packs": 528, "pilgrimage": 844, "rows": 386,
         "rushbait-v2": 660, "stampede": 686, "swirl": 600}


def med(xs):
    xs = sorted(xs)
    return xs[len(xs) // 2] if xs else None


def per_map(path):
    d = json.load(open(path))
    ns = len(d["salts"])
    out = {}
    for mp in d["maps"]:
        g = [r for r in d["results"] if r["map"] == mp]
        out[mp] = {
            "w": sum(r["win"] for r in g) / ns,
            "s": sum(r["win"] for r in g if r["opp"] in ("x-v4-14", "x-foundry-v16")) / ns,
            "medR": med([r["round"] for r in g if r["win"] and "round" in r]),
            "gemD": sum(r.get("gems", 0) - r.get("gems_opp", 0) for r in g) / len(g),
            "deaths": sum(r.get("deaths", 0) for r in g) / len(g),
            "faults": sum(r.get("faults", 0) for r in g) + sum(1 for r in g if "error" in r),
            "fuel_late": sum(r.get("fuel_late", 0) for r in g),
        }
    return out


b, c = per_map(sys.argv[1]), per_map(sys.argv[2])
h = None
if len(sys.argv) > 3:
    hd = json.load(open(sys.argv[3]))
    h = {}
    for mp in hd["maps"]:
        g = [r for r in hd["results"] if r["map"] == mp]
        h[mp] = (sum(r["win"] for r in g), len(g), sum(r.get("gems", 0) - r.get("gems_opp", 0) for r in g) / len(g))
print("| map | v2 W/12 | w13 W/12 | v2 strong/4 | w13 strong/4 | v2 medR | w13 medR | top-50 | v2 gemD | w13 gemD | v2 deaths | w13 deaths | h2h W | h2h gemD |")
print("|---|---|---|---|---|---|---|---|---|---|---|---|---|---|")
fell = 0; tw = th = 0; tb = tc = 0.0
for mp in c:
    x, y = b.get(mp), c[mp]
    hh = h.get(mp) if h else None
    if hh:
        tw += hh[0]; th += hh[1]
    tb += x["w"]; tc += y["w"]
    if x["medR"] and y["medR"] and y["medR"] < x["medR"]:
        fell += 1
    print(f"| {mp} | {x['w']:.2f} | {y['w']:.2f} | {x['s']:.2f} | {y['s']:.2f} | {x['medR']} | {y['medR']} | {TOP50.get(mp)} | "
          f"{x['gemD']:.0f} | {y['gemD']:.0f} | {x['deaths']:.1f} | {y['deaths']:.1f} | "
          f"{(str(hh[0]) + '/' + str(hh[1])) if hh else '-'} | {(round(hh[2])) if hh else '-'} |")
print(f"\ntotal wins v2 {tb:.2f}, w13 {tc:.2f} of {12 * len(c)}; median win round fell on {fell} of {len(c)} maps; "
      f"h2h {tw}/{th}" + (f" = {100 * tw / th:.0f}%" if th else ""))
print("faults", sum(v["faults"] for v in c.values()), "fuel_late", sum(v["fuel_late"] for v in c.values()),
      "(v2 fuel_late", sum(v["fuel_late"] for v in b.values()), ")")
