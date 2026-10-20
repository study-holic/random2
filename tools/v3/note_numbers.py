"""Numbers for notes/v3-data-scope.md: per-map-id counts (engine 0.0.3 era) from cached group details, detail coverage by tier/era.
Run: py -I tools/v3/note_numbers.py
"""
import json, glob, collections, csv, sys, os
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]; R = {t["teamId"]: t["rank"] for t in top}
cur = {}
for r in csv.DictReader(open("C:/cambc-solo/intel/matches-2026-10-18.csv", encoding="utf8")): cur[r["map_id"]] = r["map_name"]
gl = [json.loads(l) for l in open(f"{V3}/groups_ranked.jsonl", encoding="utf8")]
gl = [g for g in gl if g["status"] == "finished"]
have = {os.path.basename(f)[:-5] for f in glob.glob(f"{V3}/raw/groups/*.json")}
era = lambda t: "v1" if t < "2026-10-16T07:30" else ("v2" if t < "2026-10-16T22:15" else "v3")
rk = lambda g: sorted(R.get(s["teamId"], 999) for s in g["sides"])
cov = collections.defaultdict(lambda: [0, 0])
for g in gl:
    e = era(g["createdAt"]); r = rk(g)
    for name, ok in (("all", True), ("both top-10", r[1] <= 10), ("both top-50", r[1] <= 50), ("any top-10", r[0] <= 10)):
        if ok:
            cov[(e, name)][0] += 1; cov[(e, name)][1] += g["id"] in have
print("coverage of group details (have/listed groups):")
for k in sorted(cov): print(" ", k, cov[k][1], "/", cov[k][0])
mp = collections.defaultdict(lambda: collections.Counter())
for line in open(f"{V3}/lists/index.jsonl", encoding="utf8"):
    r = json.loads(line)
    for t in r["tiers"]: mp[r["mapId"]][t] += 1
print("map id | name | T1 games | T2 games (partial) | T4 games (partial)")
for k, n in sorted(cur.items(), key=lambda x: x[1]):
    print(f"| {k[:8]} | {n} | {mp[k]['T1']} | {mp[k]['T2']} | {mp[k]['T4']} |")
t1 = [mp[k]["T1"] for k in cur]; print("T1 per map min/median/max", min(t1), sorted(t1)[len(t1)//2], max(t1), "sum", sum(t1))
