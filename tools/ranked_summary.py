"""Our ranked record by patch, map and draft, from tools/v3/fetch_team.py output (data/v3/lists/team_xanarchy_index.jsonl). Run from the repo root: py -I -X utf8 tools/ranked_summary.py"""
import json, glob, collections, statistics as st
TID = "6c59147e-576f-465f-8d00-bdde9ed12f22"
rows = [json.loads(l) for l in open("data/v3/lists/team_xanarchy_index.jsonl", encoding="utf8")]
groups = {}
out = []
for r in rows:
    g = groups.get(r["group"]) or json.load(open(f"data/v3/raw/groups/{r['group']}.json", encoding="utf8")); groups[r["group"]] = g
    m = next(x for x in g["matches"] if x["id"] == r["match"])
    us = next(s for s in m["sides"] if s["teamId"] == TID); them = next(s for s in m["sides"] if s["teamId"] != TID)
    out.append(dict(t=m["createdAt"], patch=r["patch"], map=m["map"], res=us["result"], us=us["score"], them=them["score"], rounds=m["rounds"], end=m["endReason"], opp=them["teamName"], odraft=them["draft"], udraft=us["draft"]))
json.dump(out, open("data/v3/lists/team_xanarchy_results.json", "w"))
by = collections.defaultdict(list)
for o in out: by[o["patch"]].append(o)
for p in ["0.0.3", "1.0.0", "1.0.1", "1.0.2"]:
    L = by[p]; w = sum(o["res"] == "win" for o in L)
    print(f"{p}: {w}/{len(L)} = {100*w/len(L):.0f}%  gems us {st.mean(o['us'] for o in L):.0f} them {st.mean(o['them'] for o in L):.0f}  first {min(o['t'] for o in L)[:16]}")
new = [o for o in out if o["patch"] in ("1.0.1", "1.0.2")]
print("\nsince 1.0.1, n", len(new))
print(collections.Counter((o["res"], o["end"]) for o in new).most_common())
lost = [o for o in new if o["res"] == "loss"]
print("losses: median them", st.median(o["them"] for o in lost), "median us", st.median(o["us"] for o in lost), "median rounds", st.median(o["rounds"] for o in lost))
print("losses where they reach 1000:", sum(o["them"] >= 1000 for o in lost), "/", len(lost))
pm = collections.defaultdict(lambda: [0, 0, [], []])
for o in new:
    x = pm[o["map"]]; x[0] += o["res"] == "win"; x[1] += 1; x[2].append(o["us"]); x[3].append(o["them"])
print("\nmap: wins/games, mean gems us / them")
for k, x in sorted(pm.items(), key=lambda kv: kv[1][0] / kv[1][1]):
    print(f"  {k:22s} {x[0]:3d}/{x[1]:3d}  {st.mean(x[2]):5.0f} / {st.mean(x[3]):5.0f}")
print("\nopponent picks", collections.Counter(c for o in new for c in o["odraft"]).most_common())
print("our picks", collections.Counter(c for o in new for c in o["udraft"]).most_common())
