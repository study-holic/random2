"""Q2: page through /match-groups?team=<id>&scope=all&kind=<kind> (25 groups per page, newest first) for every
top-50 team until the listing ends or passes --since. Raw pages cached at data/v3/raw/listings/<kind>/<teamId>/<NNNN>.json,
so a rerun resumes from disk. Writes data/v3/groups_<kind>.jsonl (de-duplicated by group id, one line per group).
Usage: py -I tools/v3/fetch_listings.py ranked [--since 2026-10-12] [--max-pages N] [--teams 1-50]
Rate limit: CAMBC_GAP seconds between requests (default 0.26 = under 4/s).
"""
import json, os, sys, argparse
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *

ap = argparse.ArgumentParser()
ap.add_argument("kind", choices=["ranked", "unranked"])
ap.add_argument("--since", default="2026-10-12T00:00:00")
ap.add_argument("--max-pages", type=int, default=400)
ap.add_argument("--teams", default="1-50")
a = ap.parse_args()
lo, hi = map(int, a.teams.split("-"))
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
out = {}
for t in top:
    if not (lo <= t["rank"] <= hi):
        continue
    tid, cur, n, oldest = t["teamId"], None, 0, None
    while n < a.max_pages:
        n += 1
        q = f"match-groups?team={tid}&scope=all&kind={a.kind}" + (f"&before={cur}" if cur else "")
        d = cached_json(q, f"{RAW}/listings/{a.kind}/{tid}/{n:04d}.json")
        gs = d.get("groups", [])
        for g in gs:
            out[g["id"]] = g
        if gs:
            oldest = gs[-1]["createdAt"]
        cur = d.get("next")
        if not gs or not d.get("more") or not cur or (oldest and oldest < a.since):
            break
    print(now(), t["rank"], t["name"], "pages", n, "oldest", oldest, "groups so far", len(out), stats, flush=True)
with open(f"{V3}/groups_{a.kind}.jsonl", "w", encoding="utf8") as f:
    for g in sorted(out.values(), key=lambda g: g["createdAt"]):
        f.write(json.dumps(g, ensure_ascii=False) + "\n")
print("done", len(out), stats)
