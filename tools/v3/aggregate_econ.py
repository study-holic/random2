"""Tables from data/v3/decoded/econ.jsonl: our gem income and champion-round allocation against the opponent and against the top-10 reference, per map id.
Run: py -I tools/v3/aggregate_econ.py > data/v3/decoded/econ_tables.txt
"""
import json, sys, statistics, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
G = [json.loads(l) for l in open(f"{V3}/decoded/econ.jsonl", encoding="utf8")]
CATS = ["dead", "spawn", "altar", "gem camps", "gold camps", "transit"]
mean = lambda v: statistics.mean(v) if v else None
med = lambda v: statistics.median(v) if v else None
fm = lambda x, d=1: "-" if x is None else f"{x:.{d}f}"


def side_rows(g, which):
    """which: 'us' / 'opp' for our games; 'both' for top10 games (each side one row)."""
    ts = [g["teamIds"].index(XAN)] if (g["cohort"] == "ours" and which == "us") else ([1 - g["teamIds"].index(XAN)] if (g["cohort"] == "ours" and which == "opp") else [0, 1])
    rows = []
    for t in ts:
        n = g["n"]; R = max(1, g["rounds"]); tot = n * R if False else sum(g["cnt"][t].values())
        rows.append({"map": g["map"], "mapId": g["mapId"], "sg": g["sg"][t], "rate": g["sg"][t] / R, "col": g["collected"][t] / R, "lost": g["lost"][t], "deaths": g["deaths"][t],
                     "trib": g["tributes"][t], "bag": g["mean_bag"][t], "first": g["first_trib"][t], "rounds": g["rounds"], "won": int(g["sg"][t] > g["sg"][1 - t]),
                     "share": {c: g["cnt"][t].get(c, 0) / tot for c in CATS} if tot else {c: 0 for c in CATS}, "n": n,
                     "gc": g["cnt"][t].get("gem camps", 0), "col_abs": g["collected"][t], "recall_share": 5 * g.get("recalls", [0, 0])[t] / tot if tot else 0, "recalls": g.get("recalls", [0, 0])[t]})
    return rows


US = [r for g in G if g["cohort"] == "ours" for r in side_rows(g, "us")]
OPP = [r for g in G if g["cohort"] == "ours" for r in side_rows(g, "opp")]
TOP = [r for g in G if g["cohort"] == "top10" for r in side_rows(g, "both")]
maps = sorted({r["map"] for r in US})
print(f"games: ours {len(US)}, top-10 mutual {len(TOP) // 2}; our wins {sum(r['won'] for r in US)}/{len(US)}")


def agg(rs):
    if not rs: return None
    return {"n": len(rs), "rate": mean([r["rate"] for r in rs]), "col": mean([r["col"] for r in rs]), "trib": mean([r["trib"] for r in rs]), "bag": med([r["bag"] for r in rs if r["bag"]]),
            "first": med([r["first"] for r in rs if r["first"]]), "deaths": mean([r["deaths"] for r in rs]), "lost": mean([r["lost"] for r in rs]), "won": mean([r["won"] for r in rs]),
            "rounds": med([r["rounds"] for r in rs]), "share": {c: mean([r["share"][c] for r in rs]) for c in CATS}}


print("\n### E1 gem income per round by map id (tributed gems per round of game length; collected = bag increments per round)")
print("| map | n | our win | rounds | tributed/round us | opp | top10 | collected/round us | opp | top10 | tributes us | opp | top10 | bag us | opp | top10 | first tribute us | opp | top10 | deaths us | opp |")
print("|" + "---|" * 21)
for m in maps + ["ALL"]:
    f = (lambda rs: [r for r in rs if m == "ALL" or r["map"] == m])
    u, o, t = agg(f(US)), agg(f(OPP)), agg(f(TOP))
    if not u: continue
    flag = "†" if u["n"] < 30 else ""
    print(f"| {m} | {u['n']}{flag} | {100 * u['won']:.0f}% | {fm(u['rounds'], 0)} | {fm(u['rate'], 2)} | {fm(o['rate'], 2)} | {fm(t['rate'], 2) if t else '-'} | {fm(u['col'], 2)} | {fm(o['col'], 2)} | {fm(t['col'], 2) if t else '-'} | "
          f"{fm(u['trib'])} | {fm(o['trib'])} | {fm(t['trib']) if t else '-'} | {fm(u['bag'], 0)} | {fm(o['bag'], 0)} | {fm(t['bag'], 0) if t else '-'} | {fm(u['first'], 0)} | {fm(o['first'], 0)} | {fm(t['first'], 0) if t else '-'} | {fm(u['deaths'])} | {fm(o['deaths'])} |")

print("\n### E2 where champion-rounds go (share of all champion-rounds, %; us / opponent / top-10 reference)")
print("| map | n | " + " | ".join(f"{c} us/opp/top10" for c in CATS) + " |")
print("|" + "---|" * (len(CATS) + 2))
for m in maps + ["ALL"]:
    f = (lambda rs: [r for r in rs if m == "ALL" or r["map"] == m])
    u, o, t = agg(f(US)), agg(f(OPP)), agg(f(TOP))
    if not u: continue
    cells = [f"{100 * u['share'][c]:.0f} / {100 * o['share'][c]:.0f} / {100 * t['share'][c]:.0f}" if t else f"{100 * u['share'][c]:.0f} / {100 * o['share'][c]:.0f} / -" for c in CATS]
    print(f"| {m} | {u['n']}{'†' if u['n'] < 30 else ''} | " + " | ".join(cells) + " |")

print("\n### E3 largest income gaps (tributed/round, top10 minus us) and (opponent minus us), per map")
rows = []
for m in maps:
    u = agg([r for r in US if r["map"] == m]); o = agg([r for r in OPP if r["map"] == m]); t = agg([r for r in TOP if r["map"] == m])
    if u and t: rows.append((m, t["rate"] - u["rate"], o["rate"] - u["rate"], u["rate"], t["rate"], u["n"]))
for r in sorted(rows, key=lambda r: -r[1]): print(f"| {r[0]} | n={r[5]} | us {r[3]:.2f} | top10 {r[4]:.2f} | gap {r[1]:.2f} ({100 * r[1] / r[4]:.0f}% of top10) | opp-us {r[2]:.2f} |")


print(); print("### E4 income chain per map (sums over games): collected gems per 100 champion-rounds at gem camps; tributed / collected; recall channel estimate = 5 rounds per teleport-to-spawn, as % of champion-rounds; recalls per game")
print("| map | n | collected per 100 gem-camp champion-rounds us / opp / top10 | tributed per collected gem us / opp / top10 | recall % us / opp / top10 | recalls per game us / opp / top10 |")
print("|---|---|---|---|---|---|")
for m in maps + ["ALL"]:
    f = (lambda rs: [r for r in rs if m == "ALL" or r["map"] == m])
    U, O, T = f(US), f(OPP), f(TOP)
    if not U: continue
    def chain(rs):
        if not rs: return ("-", "-", "-", "-")
        gc = sum(r["gc"] for r in rs); col = sum(r["col_abs"] for r in rs); sg = sum(r["sg"] for r in rs)
        return (f"{100 * col / gc:.1f}" if gc else "-", f"{sg / col:.2f}" if col else "-", f"{100 * mean([r['recall_share'] for r in rs]):.1f}", f"{mean([r['recalls'] for r in rs]):.1f}")
    cu, co, ct = chain(U), chain(O), chain(T)
    print(f"| {m} | {len(U)}{'†' if len(U) < 30 else ''} | {cu[0]} / {co[0]} / {ct[0]} | {cu[1]} / {co[1]} / {ct[1]} | {cu[2]} / {co[2]} / {ct[2]} | {cu[3]} / {co[3]} / {ct[3]} |")
