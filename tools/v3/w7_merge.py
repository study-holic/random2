"""Merge mapgate JSONs (results lists) into one, replacing cells by later files' (map, opp, side, salt) rows.
usage: py -I tools/v3/w7_merge.py out.json in1.json in2.json ..."""
import json, sys
out, ins = sys.argv[1], sys.argv[2:]
rows, salts, maps = {}, set(), []
base = None
for f in ins:
    d = json.load(open(f))
    base = base or d
    for r in d["results"]:
        if "error" in r and (r["map"], r["opp"], r["side"], r["salt"]) in rows: continue
        rows[(r["map"], r["opp"], r["side"], r["salt"])] = r
    salts |= set(d["salts"])
    maps += [m for m in d["maps"] if m not in maps]
m = dict(base); m["results"] = list(rows.values()); m["salts"] = sorted(salts); m["maps"] = maps; m["games"] = len(rows)
m["merged_from"] = ins; m.pop("per_map", None)
json.dump(m, open(out, "w"), indent=1)
print(out, len(rows), "rows, salts", sorted(salts))
