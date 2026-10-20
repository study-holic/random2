"""Q1: fetch the live ladder (all pages) and write data/v3/top50.json (rank, teamId, name, rating, matches).
Raw pages cached at data/v3/raw/ladder/<stamp>-pN.json. Run: py -I tools/v3/fetch_ladder.py
"""
import json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *

stamp = now().replace(":", "").replace("-", "")
teams, page = [], 1
while True:
    d = cached_json(f"ladder?page={page}&pageSize=100&category=all", f"{RAW}/ladder/{stamp}-p{page}.json", refresh=True)
    teams += d["teams"]
    if len(d["teams"]) < 100 or page > 30:
        break
    page += 1
top = [{k: t[k] for k in ("rank", "teamId", "name", "rating", "matches", "wins", "losses", "memberCount", "language")} for t in teams[:50]]
json.dump({"retrievedAt": now(), "nTeams": len(teams), "top50": top}, open(f"{V3}/top50.json", "w", encoding="utf8"), ensure_ascii=False, indent=1)
json.dump({"retrievedAt": now(), "teams": [{k: t[k] for k in ("rank", "teamId", "name", "rating")} for t in teams]},
          open(f"{V3}/ladder_ratings.json", "w", encoding="utf8"), ensure_ascii=False)
print(now(), "teams", len(teams), "requests", stats)
for t in top:
    print(t["rank"], t["teamId"][:8], t["name"], round(t["rating"], 1), t["matches"])
