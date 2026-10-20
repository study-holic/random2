"""gate: the automated release gate on the CURRENT pool (state/ground-truth.json), against starter-cpp.

python scripts/gate.py <bot> [--jobs N] [--skip-build]

PASS only if: release build ok; every game won; 0 faults; no turn over 100,000 fuel; no fuelExhausted after round 1;
if the bot prints mapid=<name> then every pool map maps to its own name; local maps/ match the recorded pool and the engine
played matches the recorded engine. Writes results/gate-<bot>-<HHMM>.json (+ the mapgate JSON beside it). Exit 1 on FAIL.
"""
import argparse, json, os, subprocess, sys, time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CAMBC = os.path.expandvars(r"%APPDATA%\Python\Python314\Scripts\cambc.exe")
LIMIT = 100000


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bot")
    ap.add_argument("--jobs", type=int, default=2)
    ap.add_argument("--skip-build", action="store_true")
    a = ap.parse_args()
    gt = json.load(open(os.path.join(ROOT, "state", "ground-truth.json")))
    pool = gt["pool"]
    hm = time.strftime("%H%M")
    base = os.path.join(ROOT, "results", f"gate-{a.bot}-{hm}")
    why = []

    if not a.skip_build:
        p = subprocess.run([CAMBC, "build", "--release", a.bot], cwd=ROOT, capture_output=True, text=True, encoding="utf8", errors="replace")
        if p.returncode:
            why.append("release build failed: " + (p.stdout + p.stderr).strip()[-200:])
    maps = []
    for n in pool:
        for c in (n, n.replace("_", "-")):
            if os.path.exists(os.path.join(ROOT, "maps", c + ".map26a")):
                maps.append(c); break
        else:
            why.append(f"pool map {n} missing from maps/ (run scripts/preflight.py --fix)")
    res = None
    if not why:
        mg = base + "-mapgate.json"
        subprocess.call([sys.executable, os.path.join(ROOT, "scripts", "mapgate.py"), a.bot, *maps, "--panel", "starter-cpp",
                         "--salts", "0", "--jobs", str(a.jobs), "--out", mg], cwd=ROOT, stdout=subprocess.DEVNULL)
        if not os.path.exists(mg):
            why.append("mapgate produced no result")
        else:
            res = json.load(open(mg))
    summary = {}
    if res:
        R = res["results"]
        t = res["tags"]
        norm = lambda n: n.replace("-", "_")
        lost = [f"{r['map']}/{r['side']}" for r in R if not r["win"]]
        faults = [f"{r['map']}/{r['side']}:{r.get('faults', 0)}" for r in R if r.get("faults") or "error" in r]
        over = [f"{r['map']}/{r['side']}:{r['fuel_max']}" for r in R if (r.get("fuel_max") or 0) > LIMIT]
        nofuel = [r["map"] for r in R if r.get("fuel_max") is None]
        exh = [f"{r['map']}/{r['side']}" for r in R if r.get("fuel_r2")]
        ids = {r["mapid"] for r in R if r.get("mapid")}
        bad_id = [f"{r['map']}->{r['mapid']}" for r in R if r.get("mapid") and norm(r["mapid"]) != norm(r["map"])] if ids else []
        part_id = [r["map"] for r in R if not r.get("mapid")] if ids else []
        if len(R) != 2 * len(maps): why.append(f"{len(R)} games, expected {2 * len(maps)}")
        if lost: why.append(f"{len(lost)} games not won: {lost[:6]}")
        if faults: why.append(f"faults: {faults[:6]}")
        if over: why.append(f"turn over {LIMIT} fuel: {over[:6]}")
        if nofuel: why.append(f"fuel data missing: {nofuel[:4]}")
        if exh: why.append(f"fuelExhausted after round 1: {exh[:6]}")
        if bad_id: why.append(f"mapid mismatch: {bad_id[:6]}")
        if part_id: why.append(f"mapid not printed on: {part_id[:6]}")
        if t.get("maps_hash_local") != t.get("pool_hash_ground_truth"): why.append("local maps/ differ from recorded pool (run preflight)")
        if t.get("engine_played") != [t.get("engine_ground_truth")]: why.append(f"engine played {t.get('engine_played')} != recorded {t.get('engine_ground_truth')}")
        summary = {"games": len(R), "wins": len(R) - len(lost), "max_fuel": max([r.get("fuel_max") or 0 for r in R], default=0),
                   "mapid_checked": bool(ids), "mapgate": os.path.basename(base) + "-mapgate.json", "tags": t}
    verdict = "PASS" if not why else "FAIL"
    out = {"bot": a.bot, "verdict": verdict, "reasons": why, "pool": pool, "date": time.strftime("%Y-%m-%d %H:%M"), **summary}
    json.dump(out, open(base + ".json", "w"), indent=1)
    s = summary
    print(f"GATE {verdict} {a.bot}: {s.get('wins', 0)}/{s.get('games', 0)} won on {len(pool)} pool maps, max fuel {s.get('max_fuel')}"
          + (" | " + " | ".join(why) if why else "") + f" -> {os.path.relpath(base + '.json', ROOT)}")
    sys.exit(0 if verdict == "PASS" else 1)


if __name__ == "__main__":
    main()
