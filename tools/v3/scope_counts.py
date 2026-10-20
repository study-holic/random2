"""Q2: counts per top-50 team from the cached ranked listings (data/v3/groups_ranked.jsonl) and map-id coverage from cached group details.
Writes data/v3/scope_counts.json and prints markdown tables. Run: py -I tools/v3/scope_counts.py
"""
import json, os, glob, collections, sys, datetime
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
R = {t["teamId"]: t for t in top}
gs = [json.loads(l) for l in open(f"{V3}/groups_ranked.jsonl", encoding="utf8")]
fin = [g for g in gs if g["status"] == "finished"]
per = collections.defaultdict(lambda: {"groups": 0, "vs50": 0, "vs10": 0, "first": "9", "last": "0", "wins": 0})
for g in fin:
    ids = [s["teamId"] for s in g["sides"]]
    for s in g["sides"]:
        if s["teamId"] not in R: continue
        p = per[s["teamId"]]; o = next(x for x in g["sides"] if x is not s)
        p["groups"] += 1; p["first"] = min(p["first"], g["createdAt"]); p["last"] = max(p["last"], g["createdAt"])
        if o["teamId"] in R: p["vs50"] += 1
        if o["teamId"] in R and R[o["teamId"]]["rank"] <= 10: p["vs10"] += 1
        p["wins"] += s["wins"] > o["wins"]
rows = []
for t in top:
    p = per[t["teamId"]]
    rows.append((t["rank"], t["teamId"][:8], t["name"], round(t["rating"]), t["matches"], p["groups"], p["groups"] * 5, p["vs50"], p["vs10"], p["first"][:10], p["last"][5:16]))
print("| # | id | team | rating | ladder matches | ranked groups | games | groups vs top-50 | vs top-10 | first | last |")
print("|---|---|---|---|---|---|---|---|---|---|---|")
for r in rows: print("| " + " | ".join(str(x) for x in r) + " |")
both50 = sum(1 for g in fin if all(s["teamId"] in R for s in g["sides"]))
both10 = sum(1 for g in fin if all(s["teamId"] in R and R[s["teamId"]]["rank"] <= 10 for s in g["sides"]))
any10 = sum(1 for g in fin if any(s["teamId"] in R and R[s["teamId"]]["rank"] <= 10 for s in g["sides"]))
tg = sum(r[5] for r in rows)
out = {"listed_groups": len(gs), "finished_groups": len(fin), "error_groups": len(gs) - len(fin), "team_group_sum": tg, "distinct_groups": len(fin),
       "both_top50": both50, "both_top10": both10, "any_top10": any10, "games_distinct": len(fin) * 5,
       "by_day": dict(sorted(collections.Counter(g["createdAt"][:10] for g in fin).items()))}
# map ids + patches from cached details
det = glob.glob(f"{V3}/raw/groups/*.json")
mp = collections.Counter(); names = {}; patch = collections.defaultdict(lambda: [None, None, 0]); rounds = []; endr = collections.Counter(); nm = 0; ids = set()
for f in det:
    G = json.load(open(f, encoding="utf8")); ids.add(G["group"]["id"])
    for m in G["matches"]:
        if m.get("status") != "finished": continue
        nm += 1; mp[m["mapId"]] += 1; names[m["mapId"]] = m["map"]; rounds.append(m["rounds"]); endr[m["endReason"]] += 1
        pa = (m.get("patchId") or "none")[:5]; r = patch[pa]; t = m["createdAt"]
        r[0] = t if r[0] is None or t < r[0] else r[0]; r[1] = t if r[1] is None or t > r[1] else r[1]; r[2] += 1
out.update({"detail_groups": len(ids), "detail_games": nm, "maps": {k: [names[k], v] for k, v in mp.most_common()},
            "patches": dict(patch), "endReason": dict(endr), "rounds_median": sorted(rounds)[len(rounds) // 2] if rounds else None,
            "rounds_mean": sum(rounds) / len(rounds) if rounds else None})
json.dump(out, open(f"{V3}/scope_counts.json", "w", encoding="utf8"), indent=1, ensure_ascii=False)
print(json.dumps({k: v for k, v in out.items() if k not in ("maps",)}, ensure_ascii=False, indent=1))
