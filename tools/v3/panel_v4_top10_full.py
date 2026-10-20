"""v4 top 2-10 baseline, sets A-D (180 games). Writes data/real/v4-panel/baseline-v4top.csv, prints tables. Run: py -I tools/v3/panel_v4_top10_full.py"""
import sys, json, csv
sys.path.insert(0, "C:/cambc-solo/tools/v3"); sys.stdout.reconfigure(encoding="utf-8")
import panel_v4_vs_v3b as P
side = {}; grp = {}
for l in open(P.U + "/matches.jsonl", encoding="utf8"):
    m = json.loads(l)
    if m["label"].startswith("v4top"): side[(m["opponent"], m["map"])] = m["our_side"]; grp[(m["opponent"], m["map"])] = m["label"]
B = P.load("v4top")
with open("C:/cambc-solo/data/real/v4-panel/baseline-v4top.csv", "w", encoding="utf8", newline="") as f:
    w = csv.writer(f); w.writerow("group team map side result gems_us gems_them deaths_us deaths_them tributes_us tributes_them rounds lost_us lost_them match".split())
    for k, r in sorted(B.items(), key=lambda kv: (grp[kv[0]], kv[0][1])):
        w.writerow([grp[k], k[0], k[1], side[k], r["res"], r["gu"], r["gt"], r["du"], r["dt"], r["tru"], r["trt"], r["rounds"], r["lossu"], r["losst"], r["match"]])
def agg(rs):
    n = len(rs); w = sum(r["res"] == "win" for r in rs); a = lambda k: sum(r[k] for r in rs) / n; R = sum(r["rounds"] for r in rs)
    return f"{w}-{n-w} | {a('gu'):.0f} / {a('gt'):.0f} | {sum(r['gu'] for r in rs)/R:.2f} / {sum(r['gt'] for r in rs)/R:.2f} | {a('du'):.1f} / {a('dt'):.1f} | {a('tru'):.1f} / {a('trt'):.1f} | {a('lossu'):.0f} / {a('losst'):.0f} | {a('rounds'):.0f}"
H = "| group | n | W-L | gems us / them | gems per round us / them | deaths us / them | tributes us / them | gems lost to kills us / them | mean rounds |\n|---|---|---|---|---|---|---|---|---|"
print("games", len(B), "draws", sum(r["gu"] == r["gt"] for r in B.values()), "tiebreak wins", sum(r["gu"] == r["gt"] and r["res"] == "win" for r in B.values()))
print(H); print(f"| all | {len(B)} | {agg(list(B.values()))} |")
print("TEAM"); print(H)
for t in sorted({k[0] for k in B}, key=lambda t: grp[[k for k in B if k[0] == t][0]]):
    ks = [k for k in B if k[0] == t]; print(f"| {grp[ks[0]][6:9]} {t} | {len(ks)} | {agg([B[k] for k in ks])} |")
print("MAP"); print(H)
for mp in sorted({k[1] for k in B}):
    ks = [k for k in B if k[1] == mp]; print(f"| {mp} | {len(ks)}† | {agg([B[k] for k in ks])} |")
print("wins:", [(grp[k], k[1], B[k]["gu"], B[k]["gt"]) for k in B if B[k]["res"] == "win"])
print("00 maps", agg([B[k] for k in B if k[1].startswith("00")]), "| 01 maps", agg([B[k] for k in B if k[1].startswith("01")]))
