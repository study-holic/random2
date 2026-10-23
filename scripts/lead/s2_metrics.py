"""s2_metrics.py: per-team, per-match metrics from cambc summary JSON (the format of `cambc run --statistics --json`,
`cambc summary`, and `cambc matches summary --json`).

Two uses:
  python3 s2_metrics.py local <dir with *.json>                -> metrics.csv in that dir (validation on local games)
  python3 s2_metrics.py fetch <ids.csv> [--out matchdb] [--keep] [--cambc PATH] -> runs `cambc matches summary <id> --json -o <out>/<id>.json`
     for every match_id in ids.csv (needs `cambc login` once), appends rows to <out>/metrics.csv, deletes each JSON unless --keep.
     Resume-safe: ids already in metrics.csv are skipped. Stops after 5 consecutive download failures and prints the last error.
Columns: match_id, map, rounds, team (A|B), team_name, opp_name, won, gems_submitted, submissions, round_250, round_500, round_1000,
  gems_from_camps, gems_from_kills, gems_lost_to_champions, gems_lost_to_camps, deaths_by_champions, deaths_by_camps, kills,
  dead_rounds (60 per death, capped at game end), gold_earned, gold_spent, items_bought, locks_bought, crystals_bought, wards,
  dmg_to_champions, dmg_to_monsters, dmg_from_champions, dmg_from_monsters, picks.
"""
import csv, json, os, shutil, subprocess, sys, sysconfig, collections


def find_cambc():
    """cambc from --cambc PATH, the CAMBC variable, PATH, or the Scripts folders of this Python (pip --user on Windows
    installs cambc.exe in %APPDATA%\\Python\\Python3xx\\Scripts, which is often off PATH)."""
    if "--cambc" in sys.argv:
        return sys.argv[sys.argv.index("--cambc") + 1]
    if os.environ.get("CAMBC"):
        return os.environ["CAMBC"]
    hit = shutil.which("cambc")
    if hit:
        return hit
    dirs = [sysconfig.get_path("scripts")]
    for scheme in ("nt_user", "posix_user", "osx_framework_user"):
        try:
            dirs.append(sysconfig.get_path("scripts", scheme))
        except KeyError:
            pass
    for d in dirs:
        for name in ("cambc.exe", "cambc"):
            if d and os.path.isfile(os.path.join(d, name)):
                return os.path.join(d, name)
    sys.exit("cambc not found. Pass --cambc <path to cambc.exe>, or run: python -m pip install --user cambc==3.0.5")

COLS = ["match_id", "map", "rounds", "team", "team_name", "opp_name", "won", "gems_submitted", "submissions", "round_250", "round_500",
        "round_1000", "gems_from_camps", "gems_from_kills", "gems_lost_to_champions", "gems_lost_to_camps", "deaths_by_champions",
        "deaths_by_camps", "kills", "dead_rounds", "gold_earned", "gold_spent", "items_bought", "locks_bought", "crystals_bought",
        "wards", "dmg_to_champions", "dmg_to_monsters", "dmg_from_champions", "dmg_from_monsters", "picks"]


def metrics(d, match_id="", names=None):
    m = d["match"]; rounds = int(m.get("roundsCompleted") or 0)
    team_of = {c["id"]: c["team"] for c in d["roster"]["champions"]}
    picks = collections.defaultdict(list)
    for c in sorted(d["roster"]["champions"], key=lambda c: c["draftPick"]):
        picks[c["team"]].append(c["name"])
    T = {t: collections.Counter() for t in ("teamA", "teamB")}
    for c in d["champions"]:
        t = c["team"]; mt = c["metrics"]
        T[t]["kills"] += int(mt["kills"]); T[t]["gold_earned"] += int(mt["earnedGold"]); T[t]["gold_spent"] += int(mt["spentGold"])
        T[t]["dmg_to_champions"] += float(mt["championDamageDone"]); T[t]["dmg_to_monsters"] += float(mt["monsterDamageDone"])
        T[t]["dmg_from_champions"] += float(mt["championDamageTaken"]); T[t]["dmg_from_monsters"] += float(mt["monsterDamageTaken"])
        T[t]["items_bought"] += sum(int(x.get("quantity") or 1) for x in (c.get("itemsBought") or []) if isinstance(x, dict))
    subs = collections.defaultdict(list)
    for ev in d["events"]:
        k = ev["kind"]; own = (ev.get("owner") or {}).get("team")
        for e in ev.get("entries", []):
            r = (e.get("timestamp") or {}).get("round", rounds)
            v = int(float(e.get("value") or 0)) if str(e.get("value", "")).lstrip("-").replace(".", "", 1).isdigit() else 0
            if k == "gemCollection":
                T[own]["gems_from_camps" if e.get("context") == "campGroup" else "gems_from_kills"] += v
            elif k == "gemLoss":
                T[own]["gems_lost_to_champions" if e.get("context") == "champion" else "gems_lost_to_camps"] += v
            elif k == "gemSubmission":
                subs[own].append((r, v)); T[own]["submissions"] += 1
            elif k == "death":
                T[own]["deaths_by_champions" if e.get("context") == "champion" else "deaths_by_camps"] += 1
                T[own]["dead_rounds"] += max(0, min(60, rounds - r))
            elif k == "buyConsumable":
                nm = ((e.get("target") or {}).get("name") or "").lower()
                T[own]["locks_bought" if "lock" in nm else "crystals_bought"] += 1
            elif k == "wardPlacement":
                T[own]["wards"] += 1
    gems = {t["team"]: int(t["submittedGems"]) for t in d["teams"]}
    out = []
    for t, o in (("teamA", "teamB"), ("teamB", "teamA")):
        cum, marks = 0, {}
        for r, v in sorted(subs[t]):
            cum += v
            for th in (250, 500, 1000):
                if cum >= th and th not in marks:
                    marks[th] = r
        row = dict(match_id=match_id, map=(names or {}).get("map", ""), rounds=rounds, team=t[-1], team_name=(names or {}).get(t, ""),
                   opp_name=(names or {}).get(o, ""), won=int(m.get("outcome") == t), gems_submitted=gems.get(t, 0),
                   round_250=marks.get(250, ""), round_500=marks.get(500, ""), round_1000=marks.get(1000, ""), picks=";".join(picks[t]))
        for k in COLS:
            if k not in row:
                v = T[t][k]; row[k] = round(v) if isinstance(v, float) else v
        out.append(row)
    return out


def main():
    mode = sys.argv[1]
    if mode == "local":
        d0 = sys.argv[2]; rows = []
        for f in sorted(os.listdir(d0)):
            if f.endswith(".json"):
                rows += metrics(json.load(open(os.path.join(d0, f))), f[:-5])
        with open(os.path.join(d0, "metrics.csv"), "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=COLS); w.writeheader(); w.writerows(rows)
        print(f"{len(rows)} rows -> {os.path.join(d0, 'metrics.csv')}")
        return
    ids_csv = sys.argv[2]
    cambc = find_cambc()
    print(f"cambc: {cambc}")
    out = sys.argv[sys.argv.index("--out") + 1] if "--out" in sys.argv else "matchdb"
    keep = "--keep" in sys.argv
    os.makedirs(out, exist_ok=True)
    mpath = os.path.join(out, "metrics.csv")
    done = set()
    if os.path.exists(mpath):
        done = {r["match_id"] for r in csv.DictReader(open(mpath))}
    ids = list(csv.DictReader(open(ids_csv)))
    new = not os.path.exists(mpath)
    fh = open(mpath, "a", newline=""); w = csv.DictWriter(fh, fieldnames=COLS)
    if new:
        w.writeheader()
    fails = 0
    for i, r in enumerate(ids):
        mid = r["match_id"]
        if mid in done:
            continue
        jp = os.path.join(out, f"{mid}.json")
        p = subprocess.run([cambc, "matches", "summary", mid, "--json", "-o", jp], capture_output=True, text=True)
        if p.returncode != 0 or not os.path.exists(jp):
            fails += 1; print(f"[{i+1}/{len(ids)}] {mid} failed: {(p.stderr or p.stdout).strip()[:200]}")
            if fails >= 5:
                print("5 consecutive failures; stopping."); break
            continue
        fails = 0
        names = {"map": r.get("map", ""), "teamA": r.get("team_a", ""), "teamB": r.get("team_b", "")}
        for row in metrics(json.load(open(jp)), mid, names):
            w.writerow(row)
        fh.flush()
        if not keep:
            os.remove(jp)
        if (i + 1) % 50 == 0:
            print(f"[{i+1}/{len(ids)}] ok")
    fh.close()
    print(f"metrics in {mpath}")


if __name__ == "__main__":
    main()
