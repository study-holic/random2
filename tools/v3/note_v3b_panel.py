"""Tables for notes/unranked-v3b.md from data/unranked/games_v3b.tsv and games.tsv (v2). Run: py -I tools/v3/note_v3b_panel.py"""
import csv, statistics as st, collections, sys, json
sys.stdout.reconfigure(encoding="utf-8")
U = "C:/cambc-solo/data/unranked"
G = list(csv.DictReader(open(f"{U}/games_v3b.tsv", encoding="utf8"), delimiter="\t"))
for g in G:
    for k in g:
        if k not in ("label", "opp", "map", "match", "side", "res", "draft"):
            g[k] = int(g[k]) if g[k] != "" else None
    g["win"] = g["res"] == "win"
V2 = list(csv.DictReader(open(f"{U}/games.tsv", encoding="utf8"), delimiter="\t"))
pr = {r["key"]: (r["name"], float(r["rating_1800"])) for r in csv.DictReader(open(f"{U}/panel.csv"))}
mean = lambda v: st.mean(v) if v else float("nan")
med = lambda v: st.median(v) if v else float("nan")
def row(name, rs):
    n = len(rs); w = sum(r["win"] for r in rs); R = sum(r["rounds"] for r in rs)
    return (f"| {name} | {n}{'†' if n < 30 else ''} | {w}-{n - w} | {mean([r['g_us'] for r in rs]):.0f} / {mean([r['g_th'] for r in rs]):.0f} | {med([r['rounds'] for r in rs]):.0f} | "
            f"{sum(r['bag_us'] for r in rs) / R:.2f} / {sum(r['bag_th'] for r in rs) / R:.2f} | {mean([r['tr_us'] for r in rs]):.1f} / {mean([r['tr_th'] for r in rs]):.1f} | "
            f"{mean([r['d_us'] for r in rs]):.1f} / {mean([r['d_th'] for r in rs]):.1f} |")
H = "| group | games | W-L | gems us / them | median rounds | tributed per round us / them | tributes us / them | deaths us / them |\n|---|---|---|---|---|---|---|---|"
print("ALL"); print(H); print(row("all", G))
print("TEAM"); print(H)
for k, (nm, rt) in pr.items(): print(row(f"{nm} ({rt})", [g for g in G if g["opp"] == k]))
print("MAP"); print(H)
for m in sorted({g["map"] for g in G}): print(row(m, [g for g in G if g["map"] == m]))
# v2 vs v3b
KEPT = ["ambush", "collision", "contest", "corners", "eight", "elkring", "expanse", "fragment", "interlock", "lanes", "monolith"]
v2 = {(r["opponent"], r["map"]): r for r in V2}
print("PAIRED"); print("| map | pairs | v2 W-L | v3b W-L | v2 gems us-them | v3b gems us-them | v2 tribute/rd | v3b tribute/rd | v2 deaths us | v3b deaths us |\n|---|---|---|---|---|---|---|---|---|---|")
tot = collections.defaultdict(list)
for m in KEPT:
    ps = [(v2[(g["opp"], m)], g) for g in G if g["map"] == "00" + m and (g["opp"], m) in v2]
    if not ps: continue
    a = [p[0] for p in ps]; b = [p[1] for p in ps]
    w2 = sum(r["platform_result"] == "win" for r in a); w3 = sum(r["win"] for r in b)
    print(f"| 00{m} | {len(ps)}† | {w2}-{len(ps) - w2} | {w3}-{len(ps) - w3} | {mean([int(r['gems_us']) - int(r['gems_th']) for r in a]):.0f} | {mean([r['g_us'] - r['g_th'] for r in b]):.0f} | "
          f"{mean([float(r['rate_us']) for r in a]):.2f} | {mean([r['bag_us'] / r['rounds'] for r in b]):.2f} | {mean([int(r['deaths_us']) for r in a]):.1f} | {mean([r['d_us'] for r in b]):.1f} |")
    tot["a"] += a; tot["b"] += b
a, b = tot["a"], tot["b"]; n = len(a)
w2 = sum(r["platform_result"] == "win" for r in a); w3 = sum(r["win"] for r in b)
print(f"| all 11 | {n} | {w2}-{n - w2} | {w3}-{n - w3} | {mean([int(r['gems_us']) - int(r['gems_th']) for r in a]):.0f} | {mean([r['g_us'] - r['g_th'] for r in b]):.0f} | {mean([float(r['rate_us']) for r in a]):.2f} | {mean([r['bag_us'] / r['rounds'] for r in b]):.2f} | {mean([int(r['deaths_us']) for r in a]):.1f} | {mean([r['d_us'] for r in b]):.1f} |")
print("PER-TEAM paired"); 
for k in pr:
    ps = [(v2[(k, g["map"][2:])], g) for g in G if g["opp"] == k and (k, g["map"][2:]) in v2]
    print(k, len(ps), "v2 wins", sum(p[0]["platform_result"] == "win" for p in ps), "v3b wins", sum(p[1]["win"] for p in ps))
print("DRAFTS", collections.Counter(g["draft"] for g in G).most_common(6))
print("sanity: res vs gems mismatches", sum(1 for g in G if g["win"] != (g["g_us"] > g["g_th"]) and g["g_us"] != g["g_th"]), "faults", sum(g["faults"] for g in G), "draws", sum(g["g_us"] == g["g_th"] for g in G))
