"""w11: enemy tributes seen by our champions (TRIB lines from P_DENY_LOG) per game, against the enemy's tributed gems.
python results/w11-trib.py results/w11-measure.json"""
import json, sys, zipfile, re, collections
d = json.load(open(sys.argv[1]))
pat = re.compile(r"TRIB r(\d+) e(\d+) t(-?\d+) g(-?\d+) rem(-?\d+) a(-?\d+),(-?\d+) de(\d+) da(\d+) me(\d+) L(\d) S(\d) sr(-?\d+)")
rows = []
for r in d["results"]:
    if "gems_opp" not in r: continue
    z = zipfile.ZipFile(r["replay"])
    out = z.read("team-a.stdout" if r["side"] == "A" else "team-b.stdout").decode(errors="replace")
    obs = collections.defaultdict(list)   # enemy handle -> list of (round, gems, de, da, L, S, sr)
    for m in pat.finditer(out):
        rd, e, t, g, rem, ax, ay, de, da, me, L, S, sr = map(int, m.groups())
        obs[e].append((rd, g, de, da, L, S, sr, rem))
    eps = []
    for e, lst in obs.items():
        lst.sort()
        cur = None
        for o in lst:
            if cur and o[0] - cur["last"] <= 1:
                cur["last"] = o[0]; cur["obs"].append(o)
            else:
                cur = {"e": e, "first": o[0], "last": o[0], "obs": [o]}; eps.append(cur)
    for ep in eps:
        o = ep["obs"]
        ep["g"] = max(x[1] for x in o)
        ep["de"] = min(x[2] for x in o)
        ep["lockable"] = any(x[3] <= 2 for x in o)               # an observer within 2 of the altar
        ep["lockable_have"] = any(x[3] <= 2 and x[4] for x in o)
        ep["stunnable"] = any(x[5] and x[6] >= 0 and x[2] <= x[6] + 2 for x in o)   # stun ready, within range + 2 steps
        ep["hit"] = any(x[2] <= 3 for x in o)
    rows.append((r, eps))

def agg(sel, label):
    n = len(sel); opp = sum(r["gems_opp"] for r, _ in sel)
    E = [e for _, eps in sel for e in eps]
    g = sum(e["g"] for e in E)
    lk = sum(e["g"] for e in E if e["lockable"]); st = sum(e["g"] for e in E if e["stunnable"]); ht = sum(e["g"] for e in E if e["hit"])
    print(f"{label:<22} games {n:>3}  enemy tributed {opp/n:7.1f}/game  seen eps {len(E)/n:5.2f}/game  seen gems {g/n:6.1f} ({100*g/max(1,opp):4.1f}%)"
          f"  lock-range {lk/n:6.1f}  stun-range {st/n:6.1f}  within3 {ht/n:6.1f}")
agg(rows, "all")
for opp in sorted(set(r["opp"] for r, _ in rows)):
    agg([x for x in rows if x[0]["opp"] == opp], "vs " + opp)
print()
for mp in sorted(set(r["map"] for r, _ in rows)):
    agg([x for x in rows if x[0]["map"] == mp], mp)
