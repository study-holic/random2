"""Map id history from older group details on disk: per map id, name, first/last ranked and any-kind appearance, counts.
Usage: python -I mapids_hist.py <detailsDir> [<detailsDir>...] <out.json>"""
import json, glob, sys, os
dirs, out = sys.argv[1:-1], sys.argv[-1]
H = {}
for d in dirs:
    for f in glob.glob(os.path.join(d, "*.json")):
        try: G = json.load(open(f, encoding="utf8"))
        except Exception: continue
        kind = G["group"]["kind"]
        for m in G["matches"]:
            if m.get("status") != "finished" or not m.get("mapId"): continue
            h = H.setdefault(m["mapId"], {"name": m["map"], "n": 0, "nRanked": 0, "first": None, "firstRanked": None, "last": None})
            t = m.get("finishedAt") or m["createdAt"]; h["n"] += 1
            h["first"] = min(h["first"] or t, t); h["last"] = max(h["last"] or t, t)
            if kind == "ranked": h["nRanked"] += 1; h["firstRanked"] = min(h["firstRanked"] or t, t)
json.dump(H, open(out, "w", encoding="utf8"), indent=1)
for k, h in sorted(H.items(), key=lambda kv: kv[1]["name"]): print(k[:8], h["name"], h["nRanked"], h["firstRanked"], h["last"])
