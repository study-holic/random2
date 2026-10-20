"""Kills per game by map id from older group details that still carry platform statistics (ranked games only).
Usage: python -I kills_hist.py <detailsDir> <out.json>"""
import json, glob, sys, os
d, out = sys.argv[1:3]; K = {}
for f in glob.glob(os.path.join(d, "*.json")):
    try: G = json.load(open(f, encoding="utf8"))
    except Exception: continue
    if G["group"]["kind"] != "ranked": continue
    for m in G["matches"]:
        st = m.get("statistics")
        if m.get("status") != "finished" or not st or not st.get("champions"): continue
        k = K.setdefault(m["mapId"], {"name": m["map"], "n": 0, "kills": 0, "deaths": 0, "first": None, "last": None})
        t = m.get("finishedAt") or m["createdAt"]
        k["n"] += 1; k["kills"] += sum(c.get("kills", 0) for c in st["champions"]); k["deaths"] += sum(c.get("deaths", 0) for c in st["champions"])
        k["first"] = min(k["first"] or t, t); k["last"] = max(k["last"] or t, t)
json.dump(K, open(out, "w", encoding="utf8"), indent=1)
for mid, k in sorted(K.items(), key=lambda kv: kv[1]["name"]): print(mid[:8], k["name"], k["n"], round(k["kills"] / k["n"], 2), round(k["deaths"] / k["n"], 2), k["first"][:16], k["last"][:16])
