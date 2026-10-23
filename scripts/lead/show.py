"""show.py [sets...]: one compact block per finished baseline set."""
import json, sys, glob, os
paths = sorted(glob.glob("/home/claude/runs/*/summary.json"), key=os.path.getmtime)
want = set(sys.argv[1:])
for p in paths:
    d = json.load(open(p))
    if want and d["set"] not in want:
        continue
    print(f'{d["set"]} e{",".join(d["engine"])}: {d["a"]} {d["a_wins"]}/{d["games"]} vs {d["b"]}; gems {d["a_gems_mean"]}-{d["b_gems_mean"]}; '
          f'deaths {d["a_deaths_mean"]}-{d["b_deaths_mean"]}; fuelExh {d["a_fuel_exh"]}-{d["b_fuel_exh"]} (unattr {d["unattributed_fuel"]}); faults {d["a_faults"]}-{d["b_faults"]}')
    if d["losses"]:
        print("   losses:", ", ".join(d["losses"][:14]) + (" ..." if len(d["losses"]) > 14 else ""))
    if d["a_fuel_games"]:
        print("   A fuel:", ", ".join(d["a_fuel_games"][:8]))
