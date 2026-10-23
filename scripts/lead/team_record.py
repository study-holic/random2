"""team_record.py TEAM_ID SINCE_ISO_UTC [--until ISO]: ranked record of one team from the public API (no login).
Prints games, wins, win rate, pace, per-map record, and the opponents' mean rating; writes data/s2/record-<since>.csv."""
import json, sys, time, urllib.parse, urllib.request, collections, csv

B = "https://2026a.battlecode.co.uk/api/v1"
tid, since = sys.argv[1], sys.argv[2]
until = sys.argv[sys.argv.index("--until") + 1] if "--until" in sys.argv else "9999"
def get(path, p=None):
    url = f"{B}/{path}" + ("?" + urllib.parse.urlencode(p) if p else "")
    for k in range(4):
        try:
            with urllib.request.urlopen(url, timeout=30) as r:
                time.sleep(0.1); return json.load(r)
        except Exception:
            time.sleep(2 * (k + 1))
    raise RuntimeError(url)
gids, before = [], None
while True:
    p = {"team": tid, "kind": "ranked"}
    if before: p["before"] = before
    d = get("match-groups", p); gs = d.get("groups") or []
    stop = False
    for g in gs:
        if g["createdAt"] < since: stop = True; break
        if g["status"] == "finished" and g["createdAt"] < until: gids.append(g["id"])
    if stop or not d.get("more") or not gs: break
    before = d["next"]
rows = []
for gid in gids:
    g = get(f"match-groups/{gid}")
    sides = {s["side"]: s for s in g["group"]["sides"]}
    me = "a" if sides["a"]["teamId"] == tid else "b"; op = "b" if me == "a" else "a"
    for m in g.get("matches") or []:
        sd = {s["side"]: s for s in m.get("sides") or []}
        if set(sd) != {"a", "b"}: continue
        rows.append(dict(group=gid, created=m["createdAt"], map=(m.get("map") or "")[2:].replace("-", "_"), rounds=m.get("rounds") or 0,
                         won=int(sd[me].get("result") == "win"), score=sd[me].get("score") or 0, opp_score=sd[op].get("score") or 0,
                         opp=sd[op]["teamName"], opp_rating=sides[op].get("ratingBefore") or 0, draft=";".join(map(str, sd[me].get("draft") or []))))
n = len(rows); w = sum(r["won"] for r in rows)
print(f"groups {len(gids)} games {n} wins {w} ({w/max(1,n):.3f}); pace {sum(100*r['score']/max(1,r['rounds']) for r in rows)/max(1,n):.1f}; "
      f"opp rating {sum(r['opp_rating'] for r in rows)/max(1,n):.0f}")
pm = collections.defaultdict(lambda: [0, 0])
for r in rows: pm[r["map"]][0] += r["won"]; pm[r["map"]][1] += 1
print("per map (wins/games):", ", ".join(f"{m} {a}/{b}" for m, (a, b) in sorted(pm.items(), key=lambda kv: kv[1][0] / kv[1][1])))
with open(f"/home/claude/data/s2/record-{since[:13].replace(':','')}.csv", "w", newline="") as f:
    if rows:
        wr = csv.DictWriter(f, fieldnames=list(rows[0])); wr.writeheader(); wr.writerows(rows)
