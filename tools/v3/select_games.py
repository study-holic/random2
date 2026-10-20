"""Build match-id lists per sampling tier from cached group details (data/v3/raw/groups) and the top-50 table.
Tiers (finished ranked games only; era = engine patch of the match, from patchId):
  T1  both teams top-10, engine 0.0.3          T2  both teams top-50, engine 0.0.3
  T3  >=1 team top-10, engine 0.0.3            T4  any top-50 team, engine 0.0.3
Writes data/v3/lists/<tier>.txt (one match id per line, newest first) plus data/v3/lists/index.jsonl (match, group, mapId, rounds, tier, ranks, teams, t).
Run: py -I tools/v3/select_games.py
"""
import json, os, glob, sys, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
R = {t["teamId"]: t["rank"] for t in json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]}
os.makedirs(f"{V3}/lists", exist_ok=True)
rows = []
for f in glob.glob(f"{V3}/raw/groups/*.json"):
    G = json.load(open(f, encoding="utf8")); g = G["group"]
    rk = sorted(R.get(s["teamId"], 999) for s in g["sides"])
    if rk[0] > 50: continue
    for m in G["matches"]:
        if m.get("status") != "finished" or not (m.get("patchId") or "").startswith("0.0.3"): continue
        tiers = ["T4"] + (["T3"] if rk[0] <= 10 else []) + (["T2"] if rk[1] <= 50 else []) + (["T1"] if rk[1] <= 10 else [])
        rows.append({"match": m["id"], "group": g["id"], "mapId": m["mapId"], "map": m["map"], "rounds": m["rounds"], "t": m["createdAt"], "ranks": rk,
                     "teams": [s["teamName"] for s in m["sides"]], "tiers": tiers})
rows.sort(key=lambda r: r["t"], reverse=True)
with open(f"{V3}/lists/index.jsonl", "w", encoding="utf8", newline="\n") as fh:
    for r in rows: fh.write(json.dumps(r, ensure_ascii=False) + "\n")
cnt = collections.Counter()
for t in ("T1", "T2", "T3", "T4"):
    sel = [r for r in rows if t in r["tiers"]]
    open(f"{V3}/lists/{t}.txt", "w", newline="\n").write("\n".join(r["match"] for r in sel) + "\n")
    cnt[t] = (len(sel), sum(r["rounds"] for r in sel))
print({t: {"games": n, "mean_rounds": round(rr / n) if n else None} for t, (n, rr) in cnt.items()})
