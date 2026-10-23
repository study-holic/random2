"""baseline.py: paired head-to-head over every map in a workspace, both sides, release builds.

usage: python3 baseline.py --cambc BIN --ws WORKSPACE --set NAME --a BOT --b BOT [--jobs 2] [--maps m1,m2]
writes runs/NAME/games.csv (one row per game) and runs/NAME/summary.json; prints a summary of 15 lines or fewer.
Bot A plays side teamA on the first game of each map and teamB on the second.
"""
import argparse, csv, json, os, subprocess, sys, time, collections

ap = argparse.ArgumentParser()
ap.add_argument("--cambc", required=True)
ap.add_argument("--ws", required=True)
ap.add_argument("--set", required=True)
ap.add_argument("--a", required=True)
ap.add_argument("--b", required=True)
ap.add_argument("--jobs", type=int, default=2)
ap.add_argument("--maps", default="")
ap.add_argument("--runs", default="/home/claude/runs")
ap.add_argument("--reparse", action="store_true", help="skip run-batch and re-read existing stats")
args = ap.parse_args()

ws = os.path.abspath(args.ws)
out = os.path.join(args.runs, args.set)
os.makedirs(os.path.join(out, "stats"), exist_ok=True)
os.makedirs(os.path.join(out, "replays"), exist_ok=True)
def mapname(f):
    return f.split(".")[0][2:].replace("-", "_")


maps = sorted(f for f in os.listdir(os.path.join(ws, "maps")) if f.endswith(".map26a"))
if args.maps:
    want = set(args.maps.split(","))
    maps = [m for m in maps if mapname(m) in want]


rows = []
for m in maps:
    n = mapname(m)
    for side, (x, y) in (("A", (args.a, args.b)), ("B", (args.b, args.a))):
        st = os.path.join(out, "stats", f"{n}-{side}.json")
        rp = os.path.join(out, "replays", f"{n}-{side}.repl26a")
        rows.append((n, side, x, y, os.path.join(ws, "maps", m), rp, st))
manifest = os.path.join(out, "manifest.txt")
with open(manifest, "w") as f:
    for n, side, x, y, mp, rp, st in rows:
        f.write(f"{ws}/bots/{x} {ws}/bots/{y} {mp} {rp} --statistics {st} --json\n")

env = dict(os.environ, SSL_CERT_FILE="/root/.ccr/ca-bundle.crt", CAMBC_NO_UPDATE_CHECK="1")
t0 = time.time()
rc = "reparse"
if not args.reparse:
  with open(os.path.join(out, "batch.log"), "w") as log:
    rc = subprocess.call([args.cambc, "run-batch", manifest, "--release", "--json", "--jobs", str(args.jobs), "--timeout", "900"],
                         cwd=ws, stdout=log, stderr=subprocess.STDOUT, env=env)
wall = time.time() - t0


def team_of(entry, champ_team):
    for k in ("team", "owner", "champion", "actor", "subject", "target"):
        v = entry.get(k)
        if isinstance(v, str) and v.startswith("team"):
            return v
        if isinstance(v, dict):
            if isinstance(v.get("team"), str):
                return v["team"]
            if v.get("kind") == "champion" and v.get("id") in champ_team:
                return champ_team[v["id"]]
    return "?"


games, missing = [], []
engine = set()
for n, side, x, y, mp, rp, st in rows:
    if not os.path.exists(st):
        missing.append(f"{n}-{side}")
        continue
    d = json.load(open(st))
    mt = d["match"]
    engine.add(mt.get("engineVersion"))
    champ_team = {c["id"]: c["team"] for c in d["roster"]["champions"]}
    gems = {t["team"]: int(t["submittedGems"]) for t in d["teams"]}
    deaths = collections.Counter(); kills = collections.Counter()
    for c in d["champions"]:
        deaths[c["team"]] += int(c["metrics"]["deaths"]); kills[c["team"]] += int(c["metrics"]["kills"])
    fuel = collections.defaultdict(list); faults = collections.Counter()
    for ev in d["events"]:
        k = ev.get("kind")
        if k not in ("fuelExhausted", "botFault"):
            continue
        owner_team = (ev.get("owner") or {}).get("team")
        for e in ev.get("entries", []):
            t = owner_team if owner_team else team_of(e, champ_team)
            if k == "fuelExhausted":
                fuel[t].append(e.get("timestamp", {}).get("round"))
            else:
                faults[t] += 1
    ta = "teamA" if side == "A" else "teamB"   # the team bot A played
    tb = "teamB" if side == "A" else "teamA"
    win = mt["outcome"]
    games.append(dict(map=n, a_side=side, a_bot=args.a, b_bot=args.b, a_gems=gems.get(ta, 0), b_gems=gems.get(tb, 0),
                      a_win=int(win == ta), rounds=mt.get("roundsCompleted"), a_deaths=deaths[ta], b_deaths=deaths[tb],
                      a_kills=kills[ta], b_kills=kills[tb], a_fuel_exh=len(fuel[ta]), b_fuel_exh=len(fuel[tb]),
                      a_fuel_rounds=";".join(str(r) for r in fuel[ta][:8]), b_fuel_rounds=";".join(str(r) for r in fuel[tb][:8]),
                      unattributed_fuel=len(fuel["?"]), a_faults=faults[ta], b_faults=faults[tb],
                      diag_faults=json.dumps(d["diagnostics"].get("botFaults", {})), engine=mt.get("engineVersion")))

with open(os.path.join(out, "games.csv"), "w", newline="") as f:
    if games:
        w = csv.DictWriter(f, fieldnames=list(games[0].keys())); w.writeheader(); w.writerows(games)

N = len(games)
wins = sum(g["a_win"] for g in games)
losses = [f'{g["map"]}/{g["a_side"]} {g["a_gems"]}-{g["b_gems"]}' for g in games if not g["a_win"]]
summ = dict(set=args.set, a=args.a, b=args.b, engine=sorted(e for e in engine if e), games=N, missing=missing, a_wins=wins,
            a_gems_mean=round(sum(g["a_gems"] for g in games) / max(N, 1), 1), b_gems_mean=round(sum(g["b_gems"] for g in games) / max(N, 1), 1),
            a_deaths_mean=round(sum(g["a_deaths"] for g in games) / max(N, 1), 2), b_deaths_mean=round(sum(g["b_deaths"] for g in games) / max(N, 1), 2),
            a_fuel_exh=sum(g["a_fuel_exh"] for g in games), b_fuel_exh=sum(g["b_fuel_exh"] for g in games),
            unattributed_fuel=sum(g["unattributed_fuel"] for g in games),
            a_faults=sum(g["a_faults"] for g in games), b_faults=sum(g["b_faults"] for g in games),
            a_fuel_games=[f'{g["map"]}/{g["a_side"]}@{g["a_fuel_rounds"]}' for g in games if g["a_fuel_exh"]][:12],
            losses=losses, batch_rc=rc, wall_s=round(wall))
json.dump(summ, open(os.path.join(out, "summary.json"), "w"), indent=1)
print(json.dumps({k: v for k, v in summ.items()}, separators=(",", ":"))[:3000])
