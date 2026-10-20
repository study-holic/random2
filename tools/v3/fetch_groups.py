"""Q2/Q4: fetch /match-groups/{id} details (5 games each: map id, rounds, endReason, drafts, scores, replayUrl) for the
ranked groups in data/v3/groups_ranked.jsonl, in priority order, until --deadline (UTC ISO). Raw JSON cached at
data/v3/raw/groups/<id>.json; groups already in data/groups/ (earlier collector) are copied, not refetched.
Order: tier0 both sides top-10 (all eras); random sample (seeded) of the rest; then by engine era (0.0.3 first), inside an era both-top-50, any top-10, the rest; newest first inside a cell.
Usage: py -I tools/v3/fetch_groups.py --deadline 2026-10-19T00:10:00Z   (CAMBC_GAP sets seconds between requests)
"""
import json, os, sys, glob, shutil, random, argparse
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import *

ap = argparse.ArgumentParser()
ap.add_argument("--deadline", required=True)
ap.add_argument("--sample", type=int, default=800)
a = ap.parse_args()
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
R = {t["teamId"]: t["rank"] for t in top}
gs = [json.loads(l) for l in open(f"{V3}/groups_ranked.jsonl", encoding="utf8")]
gs = [g for g in gs if g["status"] == "finished"]
os.makedirs(f"{RAW}/groups", exist_ok=True)
for f in glob.glob(f"{ROOT}/data/groups/*.json"):
    d = f"{RAW}/groups/{os.path.basename(f)}"
    if not os.path.exists(d):
        shutil.copyfile(f, d)
have = lambda g: os.path.exists(f"{RAW}/groups/{g['id']}.json")
rk = lambda g: sorted(R.get(s["teamId"], 999) for s in g["sides"])
gs.sort(key=lambda g: g["createdAt"], reverse=True)
t0 = [g for g in gs if rk(g)[1] <= 10]
rest = [g for g in gs if rk(g)[1] > 10]
random.Random(7).shuffle(rest)
smp, rest = rest[:a.sample], rest[a.sample:]
rest.sort(key=lambda g: g["createdAt"], reverse=True)
# engine era first (0.0.3 groups are created from 2026-10-16T22:15Z), then tier, then newest first
era = lambda g: 0 if g["createdAt"] >= "2026-10-16T22:15" else (1 if g["createdAt"] >= "2026-10-16T07:30" else 2)
tier = lambda g: 0 if rk(g)[1] <= 50 else (1 if rk(g)[0] <= 10 else 2)
order = t0 + smp + sorted(rest, key=lambda g: (era(g), tier(g)))  # stable sort keeps newest-first inside each cell
todo = [g for g in order if not have(g)]
print(now(), "groups", len(gs), "have", len(gs) - len(todo), "todo", len(todo), "tier0", len(t0), flush=True)
n = 0
for g in todo:
    if now() >= a.deadline:
        break
    try:
        cached_json(f"match-groups/{g['id']}", f"{RAW}/groups/{g['id']}.json")
    except Exception as e:
        print(now(), "ERROR", g["id"], type(e).__name__, e, flush=True)
        continue
    n += 1
    if n % 200 == 0:
        print(now(), "fetched", n, stats, flush=True)
print(now(), "finished; fetched", n, "remaining", len(todo) - n, stats, flush=True)
