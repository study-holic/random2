"""Pick the pilot replays from the 429 bundles already on disk (intel/copied/cambc-top30-intel/data/replays, downloaded 15-16 Oct
when /bundle was still public). Both teams must be in today's top 50 (data/v3/top50.json), map id in the 18 Oct current-17 set.
Preference: lowest best-rank, then lowest worst-rank, at most 2 per map id. Writes data/v3/pilot/pilot_list.json.
Run: py -I tools/v3/select_pilot.py
"""
import json, os, csv, collections, sys
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
R = "C:/cambc-solo"; B = f"{R}/intel/copied/cambc-top30-intel/data"
top = json.load(open(f"{R}/data/v3/top50.json", encoding="utf8"))["top50"]
RK = {t["teamId"]: (t["rank"], t["name"]) for t in top}
cur = {r["map_id"] for r in csv.DictReader(open(f"{R}/intel/matches-2026-10-18.csv", encoding="utf8"))}
idx = {}
for l in open(f"{B}/replay_index.jsonl", encoding="utf8"):
    d = json.loads(l); idx[d["match"]] = d
zips = {f[:-4] for f in os.listdir(f"{B}/replays") if f.endswith(".zip")}
cands, gcache = [], {}
for m in zips:
    d = idx[m]; ranks = sorted(RK.get(t, (999, ""))[0] for t in d["teamIds"])
    if ranks[1] > 50: continue
    gp = f"{B}/group_detail/{d['group']}.json"
    if not os.path.exists(gp): continue
    G = gcache.setdefault(d["group"], json.load(open(gp, encoding="utf8")))
    mm = next(x for x in G["matches"] if x["id"] == m)
    if mm["mapId"] not in cur: continue
    cands.append((ranks[0], ranks[1], m, mm["mapId"], mm["map"], mm.get("patchId"), mm["rounds"], d["start"][:16], d["teams"]))
cands.sort()
per, pick = collections.Counter(), []
for c in cands:
    if per[c[3]] >= 2: continue
    per[c[3]] += 1; pick.append(c)
    if len(pick) == 20: break
print(len(cands), "eligible;", len(pick), "picked")
for c in pick: print(c)
json.dump([{"match": c[2], "mapId": c[3], "map": c[4], "rounds": c[6], "start": c[7], "teams": c[8], "ranks": [c[0], c[1]],
            "zip": f"{B}/replays/{c[2]}.zip"} for c in pick], open(f"{R}/data/v3/pilot/pilot_list.json", "w", encoding="utf8"), indent=1, ensure_ascii=False)
json.dump([{"match": c[2], "mapId": c[3], "map": c[4], "rounds": c[6], "start": c[7], "teams": c[8], "ranks": [c[0], c[1]],
            "zip": f"{B}/replays/{c[2]}.zip"} for c in cands], open(f"{R}/data/v3/pilot/legacy_candidates.json", "w", encoding="utf8"), ensure_ascii=False)
