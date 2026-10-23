"""s2_select.py: match ids for the laptop summary fetch (s2_metrics.py fetch).
Picks, on engine 1.0.3 ranked games with a summary: per map, the most recent --per-map games between two TOP20 teams,
then --uk-per-map games involving a UK top-8 team outside the top 20, then the --xan-max most recent Xanarchy losses (v5r first by recency, then v4).
Writes data/s2/fetch_ids.csv (match_id, map, reason, team_a, team_b) and prints counts.
usage: python3 s2_select.py [--per-map 8] [--uk-per-map 2] [--patch 1.0.3]"""
import argparse, csv
from collections import Counter, defaultdict

D = "/home/claude/data/s2"
ap = argparse.ArgumentParser()
ap.add_argument("--per-map", type=int, default=8)
ap.add_argument("--uk-per-map", type=int, default=2)
ap.add_argument("--patch", default="1.0.3")
ap.add_argument("--xan-max", type=int, default=100, help="most recent Xanarchy losses kept")
a = ap.parse_args()
lad = list(csv.DictReader(open(f"{D}/ladder.csv")))
top20 = {r["team_id"] for r in lad if int(r["rank"]) <= 20}
uk8 = {r["team_id"] for r in sorted((r for r in lad if r["category"] == "uk"), key=lambda r: int(r["rank"]))[:8]}
xan = {r["team_id"] for r in lad if r["name"].lower() == "xanarchy"}
rows, seen = [], set()
for r in csv.DictReader(open(f"{D}/index.csv")):
    if r["kind"] == "ranked" and r["patch"] == a.patch and r["summary"] == "True" and r["match_id"] not in seen:
        seen.add(r["match_id"]); rows.append(r)
rows.sort(key=lambda r: r["created"], reverse=True)
out, used = [], set()
bymap = defaultdict(list)
for r in rows:
    bymap[r["map"]].append(r)
for m, rs in sorted(bymap.items()):
    tt = [r for r in rs if r["id_a"] in top20 and r["id_b"] in top20][: a.per_map]
    uk = [r for r in rs if ({r["id_a"], r["id_b"]} & (uk8 - top20)) and r not in tt][: a.uk_per_map]
    for r, why in [(r, "top20-vs-top20") for r in tt] + [(r, "uk8") for r in uk]:
        out.append(dict(match_id=r["match_id"], map=m, reason=why, team_a=r["team_a"], team_b=r["team_b"])); used.add(r["match_id"])
for r in rows:
    if r["match_id"] in used:
        continue
    for s in ("a", "b"):
        if r[f"id_{s}"] in xan:
            won = (r["result_a"] == "win") == (s == "a")
            if not won and sum(1 for o in out if o["reason"] == "xanarchy-loss") < a.xan_max:
                out.append(dict(match_id=r["match_id"], map=r["map"], reason="xanarchy-loss", team_a=r["team_a"], team_b=r["team_b"]))
                used.add(r["match_id"])
with open(f"{D}/fetch_ids.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(out[0])); w.writeheader(); w.writerows(out)
print(f"{len(out)} ids: {dict(Counter(o['reason'] for o in out))}; maps {len({o['map'] for o in out})}; "
      f"top20-vs-top20 maps under {a.per_map}: {sorted(m for m in bymap if sum(1 for o in out if o['map'] == m and o['reason'] == 'top20-vs-top20') < a.per_map)}")
