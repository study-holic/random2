"""rollback_check.py: applies the rollback rule pre-registered 2026-10-23 08:24 (LEDGER) to v5r's ranked games.
Rule: Make Live on platform v3 (our v4) if v5r wins 40 or fewer of its first 100 ranked games, or wins 0 of its first 6
ranked games on road_of_riches. v5r's games are Xanarchy ranked games in groups created at or after --live (UTC).
Refreshes Xanarchy's ranked groups through the public API into --dir (default /home/claude/data/s2x), then prints the
verdict, the record so far, and the per-map record against v4's (data/s2/xanarchy_by_map.csv).
usage: python3 rollback_check.py [--live 2026-10-23T08:03:28Z] [--no-fetch]"""
import argparse, csv, os, subprocess, sys
from collections import defaultdict

ap = argparse.ArgumentParser()
ap.add_argument("--live", default="2026-10-23T08:03:28Z")
ap.add_argument("--dir", default="/home/claude/data/s2x")
ap.add_argument("--no-fetch", action="store_true")
a = ap.parse_args()
here = os.path.dirname(os.path.abspath(__file__))
if not a.no_fetch:
    subprocess.run([sys.executable, f"{here}/s2_index.py", "--top", "0", "--since", a.live, "--out", a.dir, "--workers", "4"],
                   check=True, stdout=subprocess.DEVNULL)
lad = {r["name"].lower(): r for r in csv.DictReader(open(f"{a.dir}/ladder.csv"))}
xid = lad["xanarchy"]["team_id"]
games, seen = [], set()
for r in csv.DictReader(open(f"{a.dir}/index.csv")):
    if r["kind"] != "ranked" or r["match_id"] in seen or xid not in (r["id_a"], r["id_b"]):
        continue
    seen.add(r["match_id"])
    s = "a" if r["id_a"] == xid else "b"
    won = (r["result_a"] == "win") == (s == "a")
    games.append(dict(created=r["created"], map=r["map"], win=int(won), score=r[f"score_{s}"], opp=r["team_b" if s == "a" else "team_a"]))
games.sort(key=lambda g: g["created"])
first = games[:100]
w = sum(g["win"] for g in first)
rr = [g for g in games if g["map"] == "road_of_riches"][:6]
print(f"v5r ranked games since {a.live}: {len(games)}; wins {sum(g['win'] for g in games)}; "
      f"Xanarchy rating {float(lad['xanarchy']['rating']):.0f}, rank {lad['xanarchy']['rank']}")
print(f"rule part 1: first {len(first)} games, {w} wins ({'complete' if len(first) == 100 else 'incomplete'}; rollback if <= 40 of 100)")
print(f"rule part 2: road_of_riches {sum(g['win'] for g in rr)} wins in {len(rr)} games ({'complete' if len(rr) == 6 else 'incomplete'}; rollback if 0 of 6)")
roll = (len(first) == 100 and w <= 40) or (len(rr) == 6 and sum(g["win"] for g in rr) == 0)
print("VERDICT:", "ROLL BACK to platform v3" if roll else ("keep v5r" if len(first) == 100 and len(rr) >= 6 else "keep v5r for now (rule incomplete)"))
v4 = {r["map"]: r for r in csv.DictReader(open("/home/claude/data/s2/xanarchy_by_map.csv")) if r["build"] == "v4/1.0.3"}
pm = defaultdict(lambda: [0, 0])
for g in games:
    pm[g["map"]][0] += g["win"]; pm[g["map"]][1] += 1
print("per map v5r W/G (v4 W/G on engine 1.0.3, ranked): " + ", ".join(
    f"{m} {x[0]}/{x[1]} ({v4[m]['wins']}/{v4[m]['games']})" if m in v4 else f"{m} {x[0]}/{x[1]}" for m, x in sorted(pm.items())))
