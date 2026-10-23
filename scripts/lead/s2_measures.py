"""s2_measures.py: S2 measures from per-team metrics (s2_metrics.py output).
Groups: top-20 vs top-20 ranked games from the laptop fetch (winners and losers), and local reference sets in which our bot
holds seat A in '<map>-A' and seat B in '<map>-B' (runs/<set>/stats/metrics.csv). Rates are per 100 rounds of the game;
milestones are the round a team's tributed total reached 250, 500, and 1,000 (median over the team-games that reached it,
with the share that reached it). Prints one table; writes data/s2/S2-MEASURES.csv.
usage: python3 s2_measures.py <fetched metrics.csv> [<ids csv with a 'set' or 'reason' column>]"""
import csv, os, statistics, sys, collections

fetched = sys.argv[1]
ids = sys.argv[2] if len(sys.argv) > 2 else None
kind = {}
if ids:
    for r in csv.DictReader(open(ids)):
        kind[r["match_id"]] = r.get("reason") or r.get("set") or ""

def fnum(x):
    try:
        return float(x)
    except (TypeError, ValueError):
        return None

RATE = ["gems_submitted", "gems_from_camps", "gems_from_kills", "gems_lost", "deaths", "deaths_by_champions", "kills",
        "dead_rounds", "gold_earned", "gold_spent", "items_bought", "locks_bought", "wards", "dmg_to_monsters",
        "dmg_to_champions", "dmg_from_champions", "dmg_from_monsters"]

def derive(r):
    rd = max(1.0, fnum(r["rounds"]) or 1.0)
    r = dict(r)
    r["gems_lost"] = (fnum(r["gems_lost_to_champions"]) or 0) + (fnum(r["gems_lost_to_camps"]) or 0)
    r["deaths"] = (fnum(r["deaths_by_champions"]) or 0) + (fnum(r["deaths_by_camps"]) or 0)
    out = {k: 100.0 * (fnum(r[k]) or 0) / rd for k in RATE}
    out["rounds"] = rd
    for m in ("round_250", "round_500", "round_1000"):
        out[m] = fnum(r[m])
    out["bodies"] = len([p for p in (r.get("picks") or "").split(";") if p])
    out["won"] = int(fnum(r["won"]) or 0)
    return out

groups = collections.OrderedDict()
rows = list(csv.DictReader(open(fetched)))
for r in rows:
    k = kind.get(r["match_id"], "")
    if ids and not k.startswith("top20"):
        continue
    d = derive(r)
    groups.setdefault("top20 all", []).append(d)
    groups.setdefault("top20 winners" if d["won"] else "top20 losers", []).append(d)

LOCAL = [("v5r vs starter", "f1-v5r-e103"), ("v5r vs v4", "h2h-v5r-v4-e103"), ("v5r-meta vs v5r", "d1-meta-v5r"),
         ("v5r-meta vs starter", "d1-meta-starter")]
for name, s in LOCAL:
    p = f"/home/claude/runs/{s}/stats/metrics.csv"
    if not os.path.exists(p):
        continue
    for r in csv.DictReader(open(p)):
        if r["team"] == r["match_id"][-1]:
            groups.setdefault(name, []).append(derive(r))

COLS = ["n", "win", "rounds", "gems_submitted", "gems_from_camps", "gems_from_kills", "gems_lost", "deaths",
        "deaths_by_champions", "kills", "dead_rounds", "gold_earned", "gold_spent", "items_bought", "locks_bought", "wards",
        "dmg_to_monsters", "dmg_to_champions", "dmg_from_champions", "dmg_from_monsters", "bodies",
        "round_250", "round_500", "round_1000", "reach_1000"]
table = []
for g, ds in groups.items():
    row = {"group": g, "n": len(ds), "win": round(sum(d["won"] for d in ds) / len(ds), 3)}
    for k in COLS[2:]:
        if k == "reach_1000":
            row[k] = round(sum(1 for d in ds if d["round_1000"]) / len(ds), 3)
        elif k.startswith("round_"):
            v = [d[k] for d in ds if d[k]]
            row[k] = round(statistics.median(v)) if v else ""
        else:
            row[k] = round(statistics.median(d[k] for d in ds), 2)
    table.append(row)
out = "/home/claude/data/s2/S2-MEASURES.csv"
with open(out, "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=["group"] + COLS); w.writeheader(); w.writerows(table)
short = {"gems_submitted": "gems", "gems_from_camps": "camps", "gems_from_kills": "kills_g", "gems_lost": "lost",
         "deaths": "deaths", "deaths_by_champions": "d_champ", "kills": "kills", "dead_rounds": "dead_r",
         "gold_earned": "gold", "gold_spent": "spent", "items_bought": "items", "locks_bought": "locks", "wards": "wards",
         "dmg_to_monsters": "dmg_mon", "dmg_to_champions": "dmg_ch", "dmg_from_champions": "dmg_in_ch",
         "dmg_from_monsters": "dmg_in_mon", "round_250": "r250", "round_500": "r500", "round_1000": "r1000",
         "reach_1000": "reach1k", "bodies": "bodies", "rounds": "rounds", "n": "n", "win": "win"}
print("medians per team-game; rates per 100 rounds")
print(f"{'group':22s}" + "".join(f"{short[c]:>9s}" for c in COLS))
for row in table:
    print(f"{row['group']:22s}" + "".join(f"{str(row[c]):>9s}" for c in COLS))
print(f"-> {out}")
