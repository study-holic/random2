"""Aggregate round-level game records (games_<tag>.jsonl from analyze_rounds.py) into side-level metrics and markdown tables.
Run: py -I tools/v3/aggregate.py <tag> > data/v3/decoded/tables_<tag>.txt
Cohorts: top10 = both teams in the current top 10 (set T1) ; micron = Micron's side in its games ; xan = xanarchy's side ; xanopp = xanarchy's opponents.
Expected score uses the 400-point logistic with the group's ratingBefore.
"""
import json, sys, statistics, collections, math
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
tag = sys.argv[1]
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
RANK = {t["teamId"]: t["rank"] for t in top}
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"; MIC = "695ddbdf-afd9-43bf-9988-2a98fdb0f750"
G = [json.loads(l) for l in open(f"{V3}/decoded/games_{tag}.jsonl", encoding="utf8")]
FAM = ["monster", "power", "durability", "treasure", "jungle", "home", "bruiser"]
STAT = {0: "str", 2: "foc", 4: "hp"}
TN = {0: 'Ryth', 1: 'Vesper', 2: 'Vendetta', 3: 'C&I', 4: 'C&I', 5: 'Stellar', 6: 'Prism', 7: 'Marcella', 8: 'Willow', 9: 'Ren', 10: 'Varg', 11: 'Tavi'}
med = lambda v: statistics.median(v) if v else None
f0 = lambda x: "-" if x is None else f"{x:.0f}"
f1 = lambda x: "-" if x is None else f"{x:.1f}"


def sides(g):
    out = []
    for t in (0, 1):
        tid = g["teamIds"][t]; o = g["teamIds"][1 - t]
        cohorts = []
        if RANK.get(tid, 999) <= 10 and RANK.get(o, 999) <= 10: cohorts.append("top10")
        if tid == MIC: cohorts.append("micron")
        if tid == XAN: cohorts.append("xan")
        if o == XAN: cohorts.append("xanopp")
        if not cohorts: continue
        ev = g["ev"]
        trib = [e for e in ev["tribute"] if e[1] == t]
        tr_rounds = sorted({e[0] for e in trib})
        bags = [e[4] for e in trib]
        items = [e for e in ev["item"] if e[2] == t]
        shards = [e for e in items if e[4] % 6 in (0, 2, 4)]
        ra, rb = g["rating"][t], g["rating"][1 - t]
        exp = 1 / (1 + 10 ** ((rb - ra) / 400)) if ra and rb else None
        out.append({"cohorts": cohorts, "team": g["teams"][t], "teamId": tid, "map": g["map"], "mapId": g["mapId"], "won": int(g["winner"] == t), "exp": exp,
                    "rounds": g["rounds"], "gems": g["sg"][t], "curve": {int(k): v for k, v in g["curve"][str(t)].items()}, "sg_at": {int(k): v[t] for k, v in g["sg_at"].items()},
                    "n_trib": len(trib), "trib_rounds": tr_rounds, "bags": bags, "first_trib": tr_rounds[0] if tr_rounds else None,
                    "deaths": sum(e[2] == t for e in ev["death"]), "kills": g["kills"][t], "dead": g["dead_rounds"][t], "recalls": sum(e[2] == t for e in ev["recall"]),
                    "lock_buy": sum(e[2] == t and e[4] == 2 for e in ev["cons_buy"]), "lock_use": sum(e[2] == t and e[4] == 2 for e in ev["cons_use"]),
                    "other_cons_buy": sum(e[2] == t and e[4] != 2 for e in ev["cons_buy"]), "captures": sum(e[2] == t for e in ev["capture"]),
                    "n_shards": sum(e[5] for e in shards), "first_shard": min((e[0] for e in shards), default=None),
                    "field_shards": sum(e[5] for e in shards if e[6]), "tavi_field": sum(e[5] for e in shards if e[6] and e[3] == 11), "tavi_shards": sum(e[5] for e in shards if e[3] == 11),
                    "draft": g["draft"][t], "items": [(e[0], e[3], e[4], e[5], e[6]) for e in items], "n": g["n"], "opp": g["teams"][1 - t], "oppId": o, "match": g["match"]})
    return out


S = [s for g in G for s in sides(g)]
print(f"games decoded: {len(G)}; side records: {len(S)}; check_ok {sum(g['check_ok'] for g in G)}/{len(G)}; cohorts " + str({c: sum(c in s['cohorts'] for s in S) for c in ('top10', 'micron', 'xan', 'xanopp')}))


def row(rs):
    n = len(rs)
    if not n: return None
    rt = lambda m: [r["curve"][m] for r in rs if m in r["curve"]]
    return {"n": n, "win": sum(r["won"] for r in rs) / n, "exp": statistics.mean(r["exp"] for r in rs if r["exp"] is not None) if any(r["exp"] is not None for r in rs) else None,
            **{f"r{m}": (med(rt(m)), len(rt(m))) for m in (250, 500, 750, 1000)},
            **{f"g{R}": med([r["sg_at"][R] for r in rs]) for R in (100, 200, 300, 400, 500)},
            "trib": statistics.mean(r["n_trib"] for r in rs), "bag": med([b for r in rs for b in r["bags"]]), "nbag": sum(len(r["bags"]) for r in rs),
            "first_trib": med([r["first_trib"] for r in rs if r["first_trib"] is not None]), "deaths": statistics.mean(r["deaths"] for r in rs), "dead": statistics.mean(r["dead"] for r in rs),
            "recalls": statistics.mean(r["recalls"] for r in rs), "lock_buy": statistics.mean(r["lock_buy"] for r in rs), "lock_use": statistics.mean(r["lock_use"] for r in rs),
            "shards": statistics.mean(r["n_shards"] for r in rs), "first_shard": med([r["first_shard"] for r in rs if r["first_shard"] is not None]), "rounds": med([r["rounds"] for r in rs]),
            "captures": statistics.mean(r["captures"] for r in rs)}


def cell(rr, k):
    v = rr[k]
    return f"{f0(v[0])} ({v[1]})" if isinstance(v, tuple) else v


def table(title, groups, keys):
    print(f"\n### {title}")
    print("| " + " | ".join(["group", "n"] + [k for k, _ in keys]) + " |")
    print("|" + "---|" * (len(keys) + 2))
    for name, rs in groups:
        r = row(rs)
        if not r: continue
        flag = "†" if r["n"] < 30 else ""
        cells = []
        for k, fmt in keys:
            v = r[k]
            if isinstance(v, tuple): cells.append(f"{f0(v[0])} ({v[1]})")
            elif k == "win": cells.append(f"{100 * v:.0f}%" + (f" (exp {100 * r['exp']:.0f}%)" if r["exp"] is not None else ""))
            else: cells.append(fmt(v))
        print(f"| {name} | {r['n']}{flag} | " + " | ".join(cells) + " |")


KEYS = [("win", None), ("r250", None), ("r500", None), ("r750", None), ("r1000", None), ("g300", f0), ("trib", f1), ("bag", f0), ("first_trib", f0), ("deaths", f1), ("dead", f0), ("recalls", f1), ("lock_buy", f1), ("lock_use", f1), ("shards", f1), ("first_shard", f0)]
maps = sorted({s["map"] for s in S})
for coh, title in (("top10", "Top-10 mutual games (T1), per map"), ("xan", "xanarchy, per map"), ("micron", "Micron, per map")):
    table(f"{title} (medians of rounds to N gems with games reaching in brackets; g300 = median submitted gems at round 300; trib = tributes per game; bag = median gems per tribute; dead = own champion-rounds dead per game; lock_* = altar-lock purchases and uses per game)", [(m, [s for s in S if coh in s["cohorts"] and s["map"] == m]) for m in maps] + [("ALL", [s for s in S if coh in s["cohorts"]])], KEYS)
teams10 = [t["name"] for t in top[:10]]
table("Top-10 teams, all maps (T1 games only)", [(t["name"], [s for s in S if "top10" in s["cohorts"] and s["teamId"] == t["teamId"]]) for t in top[:10]], KEYS)
table("xanarchy opponents (games vs xanarchy)", [(n, [s for s in S if "xanopp" in s["cohorts"] and s["team"] == n]) for n in sorted({s["team"] for s in S if "xanopp" in s["cohorts"]})], KEYS)
json.dump([{k: v for k, v in s.items() if k != "items"} for s in S], open(f"{V3}/decoded/sides_{tag}.json", "w", encoding="utf8"))
