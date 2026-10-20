"""w14 duel lab driver.

python tools/micro/lab.py run --tag T --polA N --polB N [--picks core|all|a,b,..] [--jobs 2] [--reps 1]
    Writes arena maps (maps-lab/T/*.map26a) for a cyclic schedule covering every ordered pair of the pick set (both side
    assignments of the two policies), runs bots/w14d vs bots/w14d with cambc run-batch, and parses the D lines of both
    teams' stdout into results/w14-lab/T.json.
python tools/micro/lab.py table T [T2] [--pol P] [--pol2 P2]
    Prints the score matrix of T (rows = champions running policy P) and the per-cell difference to T2.

Records are from each champion's own perspective: (me, foe, my_pol, foe_pol, res, hp, mhp, fhp, dur), res in
W (foe died), L (I died), E (I escaped past my line), X (foe escaped / left sight), T (timeout).
"""
import argparse, json, os, re, subprocess, time, zipfile, collections

ROOT = "C:/cambc-solo"
CAMBC = os.path.expandvars(r"%APPDATA%\Python\Python314\Scripts\cambc.exe")
NAMES = ["Ryth", "Vesper", "Vendetta", "C&I", "Stellar", "Prism", "Marcella", "Willow", "Ren", "Varg", "Tavi"]
PICK = {n: i for i, n in enumerate(NAMES)}
CORE = ["C&I", "Stellar", "Ryth", "Varg", "Ren", "Tavi", "Vendetta", "Prism"]
LINE = re.compile(r"D (\d) (\d) (\d+) (\d+) (\w+) ([\w+]+) ([WLEXT]) (\d+) (\d+) (-?\d+) (\d+)")


def schedule(pickset):
    """Cyclic schedule: for shift k and block start j in 0,5,10,..: A = p[j..j+4], B = p[j+k..j+k+4] (mod n)."""
    p = [PICK[n] for n in pickset]
    n = len(p)
    games = []
    for k in range(n):
        for j in range(0, n, 5):
            A = [p[(j + i) % n] for i in range(min(5, n))]
            B = [p[(j + k + i) % n] for i in range(min(5, n))]
            games.append((A, B))
    return games


def parse_game(m):
    recs, errors = [], 0
    try:
        z = zipfile.ZipFile(m["replay"])
    except Exception:
        return recs, 1
    for side, fn in ((0, "team-a.stdout"), (1, "team-b.stdout")):
        try:
            txt = z.read(fn).decode("utf8", "replace")
        except KeyError:
            continue
        errors += txt.count("bot error")
        my_picks = m["picksA"] if side == 0 else m["picksB"]
        foe_picks = m["picksB"] if side == 0 else m["picksA"]
        my_pol = m["polA"] if side == 0 else m["polB"]
        foe_pol = m["polB"] if side == 0 else m["polA"]
        for mt in LINE.finditer(txt):
            c, s, r0, r1, me, foe, res, hp, mhp, fhp, pol = mt.groups()
            c = int(c)
            if me == "Issac":   # C&I: Cassi's line speaks for the pair (they die together)
                continue
            recs.append({"g": m["name"], "c": c, "side": side, "me": NAMES[my_picks[c]], "foe": NAMES[foe_picks[c]],
                         "my_pol": my_pol, "foe_pol": foe_pol, "res": res, "hp": int(hp), "mhp": int(mhp),
                         "fhp": int(fhp), "dur": int(r1) - int(r0), "r0": int(r0)})
    return recs, errors


def run(a):
    tag = a.tag
    mapdir = os.path.join(ROOT, "maps-lab", tag)
    work = os.path.join(ROOT, "matches", "w14-lab", tag)
    os.makedirs(mapdir, exist_ok=True)
    os.makedirs(work, exist_ok=True)
    pickset = CORE if a.picks == "core" else (NAMES if a.picks == "all" else a.picks.split(","))
    specs, rows, meta = [], [], []
    for rep in range(a.reps):
        for g, (A, B) in enumerate(schedule(pickset)):
            for swap in (0, 1):   # both side assignments of the two policies
                pa, pb = (a.polA, a.polB) if not swap else (a.polB, a.polA)
                PA, PB = (A, B) if not swap else (B, A)
                name = f"g{rep}_{g:03d}_{swap}"
                f = os.path.join(mapdir, name + ".map26a").replace("\\", "/")
                specs.append({"file": f, "picksA": PA, "picksB": PB, "polA": pa, "polB": pb, "levelA": 0, "levelB": 0})
                base = os.path.join(work, name).replace("\\", "/")
                rows.append(f"{ROOT}/bots/{a.botA} {ROOT}/bots/{a.botB} {f} {base}.repl26a")
                meta.append({"name": name, "picksA": PA, "picksB": PB, "polA": pa, "polB": pb, "replay": base + ".repl26a"})
    sj = os.path.join(work, "specs.json")
    json.dump(specs, open(sj, "w"))
    subprocess.check_call(["node", os.path.join(ROOT, "tools", "micro", "mkmaps.mjs"), sj],
                          cwd=os.path.join(ROOT, "tools", "micro"), stderr=subprocess.DEVNULL)
    man = os.path.join(work, "manifest.txt")
    open(man, "w").write("\n".join(rows) + "\n")
    t0 = time.time()
    with open(os.path.join(work, "batch.log"), "w") as log:
        rc = subprocess.call([CAMBC, "run-batch", "--release", "--jobs", str(a.jobs), "--timeout", "300", man], cwd=ROOT,
                             stdout=log, stderr=subprocess.STDOUT, creationflags=0x40)  # IDLE_PRIORITY_CLASS
    secs = time.time() - t0
    recs, errors = [], 0
    for m in meta:
        r, e = parse_game(m)
        recs += r
        errors += e
    out = {"tag": tag, "polA": a.polA, "polB": a.polB, "picks": pickset, "games": len(meta), "seconds": round(secs, 1),
           "rc": rc, "errors": errors, "records": recs}
    os.makedirs(os.path.join(ROOT, "results", "w14-lab"), exist_ok=True)
    json.dump(out, open(os.path.join(ROOT, "results", "w14-lab", f"{tag}.json"), "w"))
    print(f"{tag}: {len(meta)} games in {secs:.0f}s rc {rc}, {len(recs)} duel records, bot errors {errors}")


def load(tag):
    return json.load(open(os.path.join(ROOT, "results", "w14-lab", f"{tag}.json")))


def cells(recs, pol=None, foe_pol=None):
    C = collections.defaultdict(collections.Counter)
    H = collections.defaultdict(list)
    for r in recs:
        if pol is not None and r["my_pol"] != pol:
            continue
        if foe_pol is not None and r["foe_pol"] != foe_pol:
            continue
        C[(r["me"], r["foe"])][r["res"]] += 1
        if r["res"] == "W":
            H[(r["me"], r["foe"])].append((r["hp"] / max(1, r["mhp"]), r["dur"]))
    return C, H


def score(c):
    n = sum(c.values())
    return (c["W"] + 0.5 * (c["E"] + c["X"] + c["T"])) / n if n else float("nan")


def table(a):
    d = load(a.tags[0])
    C, H = cells(d["records"], a.pol, a.foe_pol)
    picks = d["picks"]
    print(f"{a.tags[0]}: score of row vs column (W=1, escape/timeout=0.5); pol={a.pol} foe_pol={a.foe_pol}")
    print("row\\col  " + " ".join(f"{p[:7]:>7}" for p in picks))
    for me in picks:
        row = []
        for foe in picks:
            c = C.get((me, foe))
            row.append(f"{score(c):7.2f}" if c else "      -")
        print(f"{me[:8]:8} " + " ".join(row))
    if len(a.tags) > 1:
        d2 = load(a.tags[1])
        C2, _ = cells(d2["records"], a.pol2, a.foe_pol2)
        print(f"\ndelta score {a.tags[0]}(pol {a.pol}) - {a.tags[1]}(pol {a.pol2})")
        for me in picks:
            row = []
            for foe in picks:
                c, c2 = C.get((me, foe)), C2.get((me, foe))
                row.append(f"{score(c) - score(c2):+7.2f}" if c and c2 else "      -")
            print(f"{me[:8]:8} " + " ".join(row))


UNITS2 = [["Stellar", "Varg"], ["Ryth", "Stellar"], ["Ren", "Stellar"], ["Tavi", "Varg"], ["Ryth", "Varg"], ["Willow", "Varg"],
          ["C&I"]]
UNITS3 = [["C&I", "Stellar"], ["Ryth", "Stellar", "Varg"], ["Ren", "Stellar", "Varg"], ["Ryth", "Stellar", "Tavi"],
          ["C&I", "Varg"], ["Vendetta", "Varg", "Stellar"], ["Willow", "Varg", "Stellar"]]


def team_run(a):
    """Team fights in corridor 0 (unit vs unit), filler 1v1 duels elsewhere. Both policy sides."""
    units = UNITS2 if a.size == 2 else UNITS3
    tag = a.tag
    mapdir = os.path.join(ROOT, "maps-lab", tag)
    work = os.path.join(ROOT, "matches", "w14-lab", tag)
    os.makedirs(mapdir, exist_ok=True)
    os.makedirs(work, exist_ok=True)
    specs, rows, meta = [], [], []
    for i, uA in enumerate(units):
        for j, uB in enumerate(units):
            for swap in (0, 1):
                pa, pb = (a.polA, a.polB) if not swap else (a.polB, a.polA)
                def fill(u):
                    rest = [n for n in NAMES if n not in u and n not in ("C&I",)]
                    picks = [PICK[n] for n in u] + [PICK[n] for n in rest[: 5 - len(u)]]
                    groups = [0] * len(u) + [1, 2, 3, 4][: 5 - len(u)]
                    return picks, groups
                PA, GA = fill(uA)
                PB, GB = fill(uB)
                name = f"t{i:02d}_{j:02d}_{swap}"
                f = os.path.join(mapdir, name + ".map26a").replace("\\", "/")
                specs.append({"file": f, "picksA": PA, "picksB": PB, "polA": pa, "polB": pb, "groupsA": GA, "groupsB": GB})
                base = os.path.join(work, name).replace("\\", "/")
                rows.append(f"{ROOT}/bots/{a.bot} {ROOT}/bots/{a.bot} {f} {base}.repl26a")
                meta.append({"name": name, "unitA": "+".join(uA), "unitB": "+".join(uB), "polA": pa, "polB": pb, "replay": base + ".repl26a"})
    sj = os.path.join(work, "specs.json")
    json.dump(specs, open(sj, "w"))
    subprocess.check_call(["node", os.path.join(ROOT, "tools", "micro", "mkmaps.mjs"), sj],
                          cwd=os.path.join(ROOT, "tools", "micro"), stderr=subprocess.DEVNULL)
    man = os.path.join(work, "manifest.txt")
    open(man, "w").write("\n".join(rows) + "\n")
    t0 = time.time()
    with open(os.path.join(work, "batch.log"), "w") as log:
        rc = subprocess.call([CAMBC, "run-batch", "--release", "--jobs", str(a.jobs), "--timeout", "300", man], cwd=ROOT,
                             stdout=log, stderr=subprocess.STDOUT, creationflags=0x40)  # IDLE_PRIORITY_CLASS
    secs = time.time() - t0
    fights, errors = [], 0
    for m in meta:
        try:
            z = zipfile.ZipFile(m["replay"])
        except Exception:
            errors += 1
            continue
        for side, fn in ((0, "team-a.stdout"), (1, "team-b.stdout")):
            txt = z.read(fn).decode("utf8", "replace")
            errors += txt.count("bot error")
            lines = [mt.groups() for mt in LINE.finditer(txt) if mt.group(1) == "0"]
            lines.sort(key=lambda g: int(g[2]))
            clusters = []
            for g in lines:
                if clusters and int(g[2]) - int(clusters[-1][0][2]) <= 6:
                    clusters[-1].append(g)
                else:
                    clusters.append([g])
            for cl in clusters:
                res = [g[6] for g in cl]
                out = "W" if "W" in res else ("L" if all(r_ == "L" for r_ in res) else "D")
                alive = sum(1 for g in cl if g[6] == "W")
                fights.append({"g": m["name"], "side": side, "me": m["unitA"] if side == 0 else m["unitB"],
                               "foe": m["unitB"] if side == 0 else m["unitA"], "my_pol": m["polA"] if side == 0 else m["polB"],
                               "foe_pol": m["polB"] if side == 0 else m["polA"], "res": out, "survivors": alive,
                               "dur": max(int(g[3]) for g in cl) - int(cl[0][2]), "members": len(cl)})
    out = {"tag": tag, "size": a.size, "polA": a.polA, "polB": a.polB, "games": len(meta), "seconds": round(secs, 1),
           "rc": rc, "errors": errors, "fights": fights}
    json.dump(out, open(os.path.join(ROOT, "results", "w14-lab", f"{tag}.json"), "w"))
    print(f"{tag}: {len(meta)} games in {secs:.0f}s rc {rc}, {len(fights)} team-fight records, bot errors {errors}")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    sp = ap.add_subparsers(dest="cmd")
    r = sp.add_parser("run")
    r.add_argument("--tag", required=True)
    r.add_argument("--polA", type=int, default=0)
    r.add_argument("--polB", type=int, default=0)
    r.add_argument("--picks", default="core")
    r.add_argument("--jobs", type=int, default=2)
    r.add_argument("--reps", type=int, default=1)
    r.add_argument("--botA", default="w14d")
    r.add_argument("--botB", default="w14d")
    t = sp.add_parser("table")
    t.add_argument("tags", nargs="+")
    t.add_argument("--pol", type=int, default=None)
    t.add_argument("--foe_pol", type=int, default=None)
    t.add_argument("--pol2", type=int, default=None)
    t.add_argument("--foe_pol2", type=int, default=None)
    tr = sp.add_parser("team")
    tr.add_argument("--tag", required=True)
    tr.add_argument("--polA", type=int, default=0)
    tr.add_argument("--polB", type=int, default=0)
    tr.add_argument("--size", type=int, default=2)
    tr.add_argument("--jobs", type=int, default=2)
    tr.add_argument("--bot", default="w14t")
    a = ap.parse_args()
    {"run": run, "table": table, "team": team_run}[a.cmd](a)
