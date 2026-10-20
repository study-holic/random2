"""Head-to-head summary per map: python results/w11-h2h.py run.json [run2.json ...]
Wins out of (2 sides x salts), mean gem margin (ours - theirs), median win round, and our denial event counts."""
import json, sys, zipfile, collections, statistics
EV = ("LOCK", "STUN", "HIT", "BUYLOCK")
runs = []
for f in sys.argv[1:]:
    d = json.load(open(f)); per = collections.defaultdict(list)
    for r in d["results"]:
        ev = collections.Counter()
        try:
            out = zipfile.ZipFile(r["replay"]).read("team-a.stdout" if r["side"] == "A" else "team-b.stdout").decode(errors="replace").replace("\x00", "")
            for l in out.splitlines():
                w = l.split()
                if w and w[0] in EV: ev[w[0]] += 1
                if "bot error" in l: ev["err"] += 1
        except Exception: ev["unreadable"] += 1
        r["ev"] = ev; per[r["map"]].append(r)
    runs.append((f, per))
maps = sorted(runs[0][1])
hdr = f"{'map':<12}" + "".join(f" | {'W':>5} {'gemD':>7} {'medR':>5} {'lock':>4} {'stun':>4} {'hit':>4} {'err':>3}" for _ in runs)
print(hdr)
tot = [[0, 0, 0.0, 0] for _ in runs]
for mp in maps:
    line = f"{mp:<12}"
    for k, (f, per) in enumerate(runs):
        g = per.get(mp, [])
        w = sum(r["win"] for r in g); n = len(g)
        gm = sum(r.get("gems", 0) - r.get("gems_opp", 0) for r in g) / max(1, n)
        wr = [r["round"] for r in g if r["win"]]
        med = int(statistics.median(wr)) if wr else "-"
        ev = sum((r["ev"] for r in g), collections.Counter())
        line += f" | {w:>2}/{n:<2} {gm:>7.1f} {med:>5} {ev['LOCK'] / max(1, n):>4.1f} {ev['STUN'] / max(1, n):>4.1f} {ev['HIT'] / max(1, n):>4.1f} {ev['err']:>3}"
        tot[k][0] += w; tot[k][1] += n; tot[k][2] += gm; tot[k][3] += (w * 2 > n)
    print(line)
for k, (f, _) in enumerate(runs):
    print(f"{f}: total {tot[k][0]}/{tot[k][1]}, mean gem margin per map {tot[k][2] / len(maps):.1f}, maps with W > half: {tot[k][3]}/{len(maps)}")
