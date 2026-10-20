"""Q2 (unranked volume): density of unranked groups per top-50 team at several time points, by asking for the 25 groups before a synthetic cursor.
Raw responses cached under data/v3/raw/unranked_probe/. Run: py -I tools/v3/probe_unranked.py
"""
import json, os, sys, datetime
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
pts = ["2026-10-18T22:00:00", "2026-10-17T12:00:00", "2026-10-16T12:00:00", "2026-10-15T12:00:00", "2026-10-14T12:00:00"]
res = []
for t in [top[i] for i in (0, 1, 4, 9, 24, 49)]:
    for p in pts:
        cur = f"{p}.000000Z_00000000-0000-0000-0000-000000000000"
        d = cached_json(f"match-groups?team={t['teamId']}&scope=all&kind=unranked&before={cur}", f"{RAW}/unranked_probe/{t['rank']:02d}_{p[:13]}.json")
        gs = d.get("groups", [])
        if len(gs) >= 2:
            a = datetime.datetime.fromisoformat(gs[0]["createdAt"].replace("Z", "+00:00")); b = datetime.datetime.fromisoformat(gs[-1]["createdAt"].replace("Z", "+00:00"))
            span = (a - b).total_seconds() / 3600
            res.append((t["rank"], t["name"], p, len(gs), round(span, 2), round((len(gs) - 1) / span, 1) if span > 0 else None, gs[0]["createdAt"][:16]))
        else:
            res.append((t["rank"], t["name"], p, len(gs), None, None, None))
for r in res: print(r)
