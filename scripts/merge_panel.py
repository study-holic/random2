"""Merge mapgate result files: rows for the maps in each later file replace that map's rows in the base.
usage: python scripts/merge_panel.py <base.json> <override.json> [override2.json ...] --out <merged.json>
Used to rebuild a full 17-map base after some maps change (e.g. the six re-laid maps of 16 Oct), so cmpcells
compares like with like. per_map summaries are taken from whichever file supplied that map's rows.
"""
import argparse, json

ap = argparse.ArgumentParser()
ap.add_argument("base")
ap.add_argument("overrides", nargs="+")
ap.add_argument("--out", required=True)
a = ap.parse_args()

merged = json.load(open(a.base))
for path in a.overrides:
    o = json.load(open(path))
    maps = {r["map"] for r in o["results"]}
    merged["results"] = [r for r in merged["results"] if r["map"] not in maps] + o["results"]
    for m in maps:
        if m in o.get("per_map", {}):
            merged.setdefault("per_map", {})[m] = o["per_map"][m]
    print(f"{path}: replaced {len(maps)} maps ({', '.join(sorted(maps))})")
merged["merged_from"] = [a.base] + a.overrides
merged["games"] = len(merged["results"])
json.dump(merged, open(a.out, "w"), indent=1)
print(f"wrote {a.out}: {merged['games']} games, {len({r['map'] for r in merged['results']})} maps")
