"""s2_index.py: S2 index from the platform's public API (no login).
Writes data/s2/ladder.csv, data/s2/groups.jsonl (raw group details), data/s2/index.csv (one row per match).
Targets: ladder top N (default 20), Xanarchy, MicroStella, plus --extra team names. Window: groups created after --since (UTC).
usage: python3 s2_index.py [--top 20] [--since 2026-10-21T05:30:00Z] [--extra name1,name2] [--near 0]
--near R also adds every team within R rating points of Xanarchy."""
import argparse, csv, json, os, sys, time, urllib.parse, urllib.request
sys.stdout.reconfigure(line_buffering=True)

B = "https://2026a.battlecode.co.uk/api/v1"
OUT = "/home/claude/data/s2"
os.makedirs(OUT, exist_ok=True)
ap = argparse.ArgumentParser()
ap.add_argument("--top", type=int, default=20)
ap.add_argument("--since", default="2026-10-21T05:30:00Z")
ap.add_argument("--extra", default="")
ap.add_argument("--near", type=float, default=0)
ap.add_argument("--sleep", type=float, default=0.15)
ap.add_argument("--kind", default="ranked", help="ranked | unranked | all")
a = ap.parse_args()
DRAFT = ["ryth", "vesper_vale", "vendetta", "cassi_and_issac", "stellar", "prism", "marcella", "willow", "ren", "varg", "tavi", "heart_of_ruin"]
nreq = 0


def get(path, params=None):
    global nreq
    url = f"{B}/{path}" + ("?" + urllib.parse.urlencode(params) if params else "")
    for attempt in range(5):
        try:
            nreq += 1
            with urllib.request.urlopen(urllib.request.Request(url, headers={"User-Agent": "xanarchy-s2-index"}), timeout=30) as r:
                time.sleep(a.sleep)
                return json.load(r)
        except urllib.error.HTTPError as e:
            if e.code in (429, 502, 503, 504):
                time.sleep(2 * (attempt + 1)); continue
            raise
        except Exception:
            time.sleep(2 * (attempt + 1))
    raise RuntimeError(f"failed {url}")


# 1. ladder
teams, page = [], 1
while True:
    d = get("ladder", {"page": page})
    rows = d.get("teams") or []
    if not rows:
        break
    teams += rows
    if len(teams) >= d.get("total", 0) or page > 60:
        break
    page += 1
with open(f"{OUT}/ladder.csv", "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["rank", "team_id", "name", "category", "rating", "matches", "wins", "losses", "language", "members", "picks_last50"])
    for t in teams:
        picks = ";".join(f'{p["champion"]}:{p["picks"]}' for p in t.get("mostPickedChampions") or [])
        w.writerow([t["rank"], t["teamId"], t["name"], t["category"], round(t["rating"], 1), t["matches"], t["wins"], t["losses"], t.get("language"), t.get("memberCount"), picks])
by_name = {t["name"].lower(): t for t in teams}
xan = by_name.get("xanarchy")
targets = {t["teamId"]: t["name"] for t in sorted(teams, key=lambda t: t["rank"])[: a.top]}
for nm in ["xanarchy", "microstella"] + [x.strip().lower() for x in a.extra.split(",") if x.strip()]:
    if nm in by_name:
        targets[by_name[nm]["teamId"]] = by_name[nm]["name"]
    else:
        print("not on ladder:", nm)
if a.near and xan:
    for t in teams:
        if abs(t["rating"] - xan["rating"]) <= a.near:
            targets[t["teamId"]] = t["name"]

# 2. group ids per target since the cutoff
gids = {}
for tid, nm in targets.items():
    before, n = None, 0
    while True:
        p = {"team": tid}
        if a.kind != "all":
            p["kind"] = a.kind
        if before:
            p["before"] = before
        d = get("match-groups", p)
        gs = d.get("groups") or []
        stop = False
        for g in gs:
            if g["createdAt"] < a.since:
                stop = True; break
            if g.get("status") == "finished":
                gids[g["id"]] = g["createdAt"]; n += 1
        if stop or not d.get("more") or not gs:
            break
        before = d.get("next")
    print(f"{nm[:28]:28s} groups since cutoff: {n}")

# 3. group details (cached)
cache = {}
gpath = f"{OUT}/groups.jsonl"
if os.path.exists(gpath):
    for line in open(gpath):
        try:
            g = json.loads(line); cache[g["group"]["id"]] = g
        except Exception:
            pass
with open(gpath, "a") as f:
    for gid in gids:
        if gid in cache:
            continue
        g = get(f"match-groups/{gid}")
        cache[gid] = g
        f.write(json.dumps(g) + "\n")

# 4. per-match index
rows = []
for gid, g in cache.items():
    grp = g["group"]
    if gid not in gids:
        continue
    gs = {s["side"]: s for s in grp["sides"]}
    for m in g.get("matches") or []:
        sd = {s["side"]: s for s in m.get("sides") or []}
        if set(sd) != {"a", "b"}:
            continue
        rows.append(dict(match_id=m["id"], group_id=gid, kind=m.get("kind"), created=m.get("createdAt"), started=m.get("startedAt"),
                         patch=(m.get("patchId") or "").split(":")[0], map=(m.get("map") or "")[2:].replace("-", "_"), rounds=m.get("rounds"),
                         end=m.get("endReason"), coinflip=m.get("coinflip"), summary=m.get("summaryAvailable"),
                         team_a=sd["a"]["teamName"], team_b=sd["b"]["teamName"], id_a=sd["a"]["teamId"], id_b=sd["b"]["teamId"],
                         score_a=sd["a"].get("score"), score_b=sd["b"].get("score"), result_a=sd["a"].get("result"),
                         draft_a=";".join(DRAFT[int(x)] if str(x).isdigit() and int(x) < len(DRAFT) else str(x) for x in sd["a"].get("draft") or []),
                         draft_b=";".join(DRAFT[int(x)] if str(x).isdigit() and int(x) < len(DRAFT) else str(x) for x in sd["b"].get("draft") or []),
                         rating_a=round(gs["a"].get("ratingBefore") or 0, 1), rating_b=round(gs["b"].get("ratingBefore") or 0, 1),
                         bot_error_a=sd["a"].get("botError"), bot_error_b=sd["b"].get("botError")))
rows.sort(key=lambda r: r["created"])
with open(f"{OUT}/index.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)
print(f"ladder teams {len(teams)}; targets {len(targets)}; groups {len(gids)}; matches {len(rows)}; requests {nreq}")
print("patches:", dict(__import__("collections").Counter(r["patch"] for r in rows)))
