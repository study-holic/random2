"""mapgate: a bot against the panel on each map, both sides, with the per-map gate.

python scripts/mapgate.py <bot> <map ...|all> --jobs N [--panel a,b,...] [--out f.json] [--strong a,b]

Gate per map (PLAN.md): at least 7 of 12 panel games and at least 2 of the 4 games
against the strong pair (x-v4-14, x-foundry-v16). With a non-default panel the gate
is reported as wins/games only.
"""
import argparse, hashlib, json, os, struct, subprocess, sys, time, glob, zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CAMBC = os.environ.get("CAMBC") or (os.path.expandvars(r"%APPDATA%\Python\Python314\Scripts\cambc.exe") if os.name == "nt" else (__import__("shutil").which("cambc") or "cambc"))
PANEL = ["x-v4-14", "x-foundry-v16", "x-hello", "x-ifish", "x-mite", "x-shawn"]
STRONG = ["x-v4-14", "x-foundry-v16"]


def maps_all():
    return sorted(os.path.splitext(os.path.basename(p))[0] for p in glob.glob(os.path.join(ROOT, "maps", "*.map26a")))


def make_salted(bot, salt):
    """Copy bots/<bot>/src to bots/<bot>-s<salt>/src with src/salt.h and build it (the fuel burn sits in plan.cpp)."""
    import shutil
    dst = os.path.join(ROOT, "bots", f"{bot}-s{salt}")
    if os.path.exists(os.path.join(dst, "src")):
        shutil.rmtree(os.path.join(dst, "src"))
    shutil.copytree(os.path.join(ROOT, "bots", bot, "src"), os.path.join(dst, "src"))
    open(os.path.join(dst, "src", "salt.h"), "w").write(f"#pragma once\n#define P_SALT {salt}\n")
    rc = subprocess.call([CAMBC, "build", "--release", f"{bot}-s{salt}"], cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    if rc:
        sys.exit(f"salted build {bot}-s{salt} failed")


def fuel_stats(bundle, side, nch):
    """Per-turn fuel from <bundle>/team-?.fuel: magic(8)+5 header bytes, then u64 values: 8 limits, 5 draft turns, then
    one value per champion turn (round 1 first, one per living champion). Returns (max round 1, max after round 1)."""
    try:
        with zipfile.ZipFile(bundle) as z:
            b = z.read("team-%s.fuel" % side.lower())
        v = struct.unpack_from("<%dQ" % ((len(b) - 13) // 8), b, 13)
        r1, late = v[13:13 + nch], v[13 + nch:]
        return {"fuel_max_r1": max(r1, default=0), "fuel_max": max(late, default=0), "fuel_draft_max": max(v[8:13], default=0)}
    except Exception as e:
        return {"fuel_max": None, "fuel_err": str(e)[:80]}


def stdout_mapid(bundle, side):
    """First 'mapid=<name>' printed by our bot (None if the bot does not print it)."""
    import re
    try:
        with zipfile.ZipFile(bundle) as z:
            m = re.search(r"mapid=(\S+)", z.read("team-%s.stdout" % side.lower()).decode("utf8", "replace"))
        return m.group(1) if m else None
    except Exception:
        return None


def sha16(b):
    return hashlib.sha256(b.replace(bytes([13, 10]), bytes([10]))).hexdigest()[:16]


def tags():
    """CLI version, engine from state/ground-truth.json, and a hash of the local map set (same recipe as preflight)."""
    t = {}
    try:
        t["cli"] = subprocess.run([CAMBC, "--version"], capture_output=True, text=True).stdout.split()[-1]
    except Exception:
        t["cli"] = None
    try:
        g = json.load(open(os.path.join(ROOT, "state", "ground-truth.json")))
        t["engine_ground_truth"], t["pool_hash_ground_truth"] = g.get("engine"), g.get("map_set_hash")
    except Exception:
        t["engine_ground_truth"] = t["pool_hash_ground_truth"] = None
    h = {}
    for p in glob.glob(os.path.join(ROOT, "maps", "*.map26a")):
        h[os.path.basename(p)[:-7].replace("-", "_")] = sha16(open(p, "rb").read())
    t["maps_hash_local"] = hashlib.sha256(json.dumps(h, sort_keys=True).encode()).hexdigest()[:12]
    t["maps_n_local"] = len(h)
    return t


def summarise(stat_path, our_side):
    d = json.load(open(stat_path))
    m = d["match"]
    gems = {t["team"]: int(t["submittedGems"]) for t in d["teams"]}
    deaths = {"teamA": 0, "teamB": 0}
    kills = {"teamA": 0, "teamB": 0}
    for ev in d["events"]:
        team = ev["owner"].get("team")
        if team in deaths:
            if ev["kind"] == "death":
                deaths[team] += len(ev["entries"])
            elif ev["kind"] == "kill":
                kills[team] += len(ev["entries"])
    gold = {"teamA": 0, "teamB": 0}
    for tl in d["championTimelines"]:
        if tl["metric"] == "earnedGold" and tl["runs"]:
            gold[tl["owner"]["team"]] += int(float(tl["runs"][-1]["value"]))
    faults = d["diagnostics"]["botFaults"]
    us = "teamA" if our_side == "A" else "teamB"
    fuel_init = fuel_late = fuel_r2 = 0   # fuelExhausted: execution suspends and resumes next round (GameRules, Fuel)
    for ev in d["events"]:
        if ev["kind"] == "fuelExhausted" and ev["owner"].get("team") == us:
            for x in ev["entries"]:
                if x["timestamp"]["round"] > 1: fuel_r2 += 1
                if x["timestamp"]["round"] <= 2: fuel_init += 1
                else: fuel_late += 1
    them = "teamB" if us == "teamA" else "teamA"
    nch = sum(1 for c in d["roster"]["champions"] if c["team"] == us)
    return {
        "engine": m.get("engineVersion"), "nch": nch,
        "win": m["outcome"] == us,
        "round": m["roundsStarted"],
        "gems": gems[us], "gems_opp": gems[them],
        "kills": kills[us], "deaths": deaths[us],
        "gold": gold[us], "gold_opp": gold[them],
        "faults": faults[us], "faults_opp": faults[them], "fuel_init": fuel_init, "fuel_late": fuel_late,
        "fuel_r2": fuel_r2,
        "codes": d["diagnostics"].get("codes", []),
    }


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bot")
    ap.add_argument("maps", nargs="+")
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--panel", default=",".join(PANEL))
    ap.add_argument("--strong", default=",".join(STRONG))
    ap.add_argument("--out", default=None)
    ap.add_argument("--keep", action="store_true", help="keep statistics JSON (replays are always kept)")
    ap.add_argument("--salts", default="0", help="comma list; salt s>0 runs a copy bots/<bot>-s<s> that burns s*20k fuel on turn 1 (src/salt.h)")
    a = ap.parse_args()
    salts = [int(x) for x in a.salts.split(",")]
    for s_ in salts:
        if s_ > 0:
            make_salted(a.bot, s_)

    maps = maps_all() if a.maps == ["all"] else a.maps
    panel = a.panel.split(",")
    strong = [s for s in a.strong.split(",") if s in panel]
    tag = f"{a.bot}-{time.strftime('%H%M%S')}"
    work = os.path.join(ROOT, "matches", tag)
    os.makedirs(work, exist_ok=True)

    rows, games = [], []
    for salt in salts:
        me = a.bot if salt == 0 else f"{a.bot}-s{salt}"
        for mp in maps:
            for opp in panel:
                for side in "AB":
                    a_bot, b_bot = (me, opp) if side == "A" else (opp, me)
                    base = os.path.join(work, f"{mp}__{opp}__{side}__s{salt}")
                    B = os.path.join(ROOT, "bots")
                    rows.append(f"{B}/{a_bot} {B}/{b_bot} {ROOT}/maps/{mp}.map26a {base}.repl26a --statistics {base}.json")
                    games.append((mp, opp, side, base, salt))
    man = os.path.join(work, "manifest.txt")
    open(man, "w").write("\n".join(rows) + "\n")

    t0 = time.time()
    with open(os.path.join(work, "batch.log"), "w") as log:
        rc = subprocess.call([CAMBC, "run-batch", "--release", "--json", "--jobs", str(a.jobs), "--timeout", "300", man],
                             cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    secs = time.time() - t0

    res = []
    for mp, opp, side, base, salt in games:
        r = {"map": mp, "opp": opp, "side": side, "salt": salt, "replay": base + ".repl26a"}
        try:
            r.update(summarise(base + ".json", side))
            r.update(fuel_stats(base + ".repl26a", side, r["nch"]))
            r["mapid"] = stdout_mapid(base + ".repl26a", side)
        except Exception as e:  # missing or unreadable statistics count as a loss
            r.update({"win": False, "error": str(e)})
        res.append(r)
        if not a.keep and os.path.exists(base + ".json"):
            os.remove(base + ".json")

    per_map = {}
    for mp in maps:
        g = [r for r in res if r["map"] == mp]
        s = [r for r in g if r["opp"] in strong]
        ns = len(salts)
        w, ws = round(sum(r["win"] for r in g) / ns, 2), round(sum(r["win"] for r in s) / ns, 2)
        wr = [r["round"] for r in g if r["win"] and "round" in r]
        default_panel = len(panel) == 6 and len(strong) == 2
        per_map[mp] = {
            "wins": w, "games": len(g) // ns, "strong_wins": ws, "strong_games": len(s) // ns, "salts": salts,
            "median_win_round": sorted(wr)[len(wr) // 2] if wr else None,
            "deaths": round(sum(r.get("deaths", 0) for r in g) / max(1, len(g)), 1),
            "gem_margin": round(sum(r.get("gems", 0) - r.get("gems_opp", 0) for r in g) / max(1, len(g)), 1),
            "faults": sum(r.get("faults", 0) for r in g) + sum(1 for r in g if "error" in r),
            "fuel_late": sum(r.get("fuel_late", 0) for r in g), "fuel_max": max([r.get("fuel_max") or 0 for r in g], default=0), "fuel_init": sum(r.get("fuel_init", 0) for r in g),
            "pass": (w >= 7 and ws >= 2) if default_panel else None,
        }

    tg = tags()
    engines = sorted({r["engine"] for r in res if r.get("engine")})
    out = {"tags": {**tg, "engine_played": engines, "date": time.strftime("%Y-%m-%d %H:%M")}, "bot": a.bot, "salts": salts, "panel": panel, "strong": strong, "maps": maps, "games": len(res),
           "seconds": round(secs, 1), "jobs": a.jobs, "rc": rc, "work": work, "per_map": per_map, "results": res}
    path = a.out or os.path.join(ROOT, "results", f"{tag}.json")
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    json.dump(out, open(path, "w"), indent=1)

    print(f"{a.bot}: {len(res)} games in {secs:.0f} s ({len(res) / secs * 60:.1f}/min, jobs {a.jobs}), rc {rc}")
    print(f"{'map':<12} {'wins':>6} {'strong':>7} {'medR':>5} {'death':>5} {'gemD':>6} {'flt':>3} {'fuelL':>5} gate")
    for mp, p in per_map.items():
        print(f"{mp:<12} {p['wins']:>2}/{p['games']:<3} {p['strong_wins']:>3}/{p['strong_games']:<3} "
              f"{str(p['median_win_round']):>5} {p['deaths']:>5} {p['gem_margin']:>6} {p['faults']:>3} {p['fuel_late']:>5} "
              f"{'' if p['pass'] is None else ('PASS' if p['pass'] else 'fail')}")
    if engines and tg["engine_ground_truth"] and engines != [tg["engine_ground_truth"]]:
        print(f"WARNING: engine played {engines} != ground truth {tg['engine_ground_truth']}")
    if tg["pool_hash_ground_truth"] and tg["maps_hash_local"] != tg["pool_hash_ground_truth"]:
        print("WARNING: local maps/ hash differs from ground-truth pool; run scripts/preflight.py")
    tw = sum(p["wins"] for p in per_map.values())
    print(f"total {tw}/{len(res) // len(salts)} (mean over salts {salts}); maps passing {sum(1 for p in per_map.values() if p['pass'])}/{len(maps)}; out {path}")


if __name__ == "__main__":
    main()
