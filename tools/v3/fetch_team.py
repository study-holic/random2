"""List every ranked group of one team (kind=ranked), fetch the group details, write data/v3/lists/team_<label>.txt (finished match ids, newest first)
and data/v3/lists/team_<label>_index.jsonl. Raw pages/details cached under data/v3/raw/ (shared with the top-50 crawl).
Usage: py -I tools/v3/fetch_team.py <teamId> <label>
"""
import json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
tid, label = sys.argv[1], sys.argv[2]
cur, n, groups = None, 0, {}
while True:
    n += 1
    d = cached_json(f"match-groups?team={tid}&scope=all&kind=ranked" + (f"&before={cur}" if cur else ""), f"{RAW}/listings/ranked/{tid}/{n:04d}.json")
    for g in d.get("groups", []): groups[g["id"]] = g
    cur = d.get("next")
    if not d.get("groups") or not d.get("more") or not cur: break
rows = []
for g in sorted(groups.values(), key=lambda g: g["createdAt"], reverse=True):
    if g["status"] != "finished": continue
    G = cached_json(f"match-groups/{g['id']}", f"{RAW}/groups/{g['id']}.json")
    for m in G["matches"]:
        if m.get("status") == "finished":
            rows.append({"match": m["id"], "group": g["id"], "mapId": m["mapId"], "map": m["map"], "rounds": m["rounds"], "t": m["createdAt"], "patch": (m.get("patchId") or "")[:5],
                         "teams": [s["teamName"] for s in m["sides"]], "teamIds": [s["teamId"] for s in m["sides"]]})
open(f"{V3}/lists/team_{label}.txt", "w", newline="\n").write("\n".join(r["match"] for r in rows) + "\n")
with open(f"{V3}/lists/team_{label}_index.jsonl", "w", encoding="utf8", newline="\n") as f:
    for r in rows: f.write(json.dumps(r, ensure_ascii=False) + "\n")
print(len(groups), "groups", len(rows), "games; patches", {p: sum(r["patch"] == p for r in rows) for p in {r["patch"] for r in rows}}, stats)
