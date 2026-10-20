"""Queue an unranked real-team panel for the LIVE bot (unranked groups always play the team's live bot).
Opponents: ONLY the current top 10 in data/unranked/top10.csv (Xans, 20 Oct: no unranked games against anyone else).
Maps: four fixed sets of five (20 of the 21 pool maps; big_world left out as in the v3b panel), so panels pair by team and map.
Usage: python tools/v3/queue_panel.py <label> [--sets ABCD] [--teams 1-10]
Appends rows to data/unranked/groups.csv; skips labels already queued; stops at the first refusal (quota)."""
import csv, json, os, subprocess, sys, time
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CAMBC = os.environ.get("CAMBC") or (os.path.expandvars(r"%APPDATA%\Python\Python314\Scripts\cambc.exe") if os.name == "nt" else "cambc")
SETS = {"A": "00eight,00contest,00ambush,00collision,00corners",
        "B": "01corridors,01broken_curve,01gold_rush,01boss_battle,01concentrated_wealth",
        "C": "00interlock,00elkring,01hooks,00monolith,00expanse",
        "D": "00lanes,00fragment,01elk_arena,01fracture,01islands"}
label = sys.argv[1]
sets = sys.argv[sys.argv.index("--sets") + 1] if "--sets" in sys.argv else "ABCD"
lo, hi = map(int, (sys.argv[sys.argv.index("--teams") + 1] if "--teams" in sys.argv else "1-10").split("-"))
top = [r for r in csv.DictReader(open(os.path.join(ROOT, "data/unranked/top10.csv"), encoding="utf8")) if lo <= int(r["rank"]) <= hi]
assert top and all(int(r["rank"]) <= 10 for r in top), "top-10 opponents only"
path = os.path.join(ROOT, "data/unranked/groups.csv")
done = {r["label"] for r in csv.DictReader(open(path, encoding="utf8"))}
for s in sets:
    for t in top:
        lab = f"{label}-t{int(t['rank']):02d}-{s}"
        if lab in done:
            continue
        out = subprocess.run([CAMBC, "match-groups", "create", t["team_id"], "--maps", SETS[s], "--json"], capture_output=True, text=True)
        try:
            gid = json.loads(out.stdout)["groupId"]
        except Exception:
            err = (out.stdout + out.stderr).strip()[:200]
            if "eligible" in err:   # the team has no bot open to unranked games: skip it
                print("SKIP", lab, err); continue
            print("STOPPED at", lab, err); sys.exit(1)
        with open(path, "a", encoding="utf8", newline="") as f:
            csv.writer(f).writerow([lab, label, t["name"], SETS[s], gid, time.strftime("%H:%M")])
        print(lab, t["name"], gid)
