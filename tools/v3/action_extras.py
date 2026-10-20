"""Action-level counts per team-side from full-action decodes (decoded/full and decoded/ours_full): stun (cc) casts, hits on carriers, altar-lock placements.
Carrier = enemy champion carrying >= 20 gems at the round start. Baseline = share of enemy champion-rounds in which the enemy carried >= 20 gems.
Cohorts: top10 = both teams current top-10 ; ours = xanarchy side of our games. Run: py -I tools/v3/action_extras.py
"""
import json, os, sys, glob, collections, statistics
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, "C:/cambc-solo/tools/v3")
from analyze_rounds import load_index

V3 = "C:/cambc-solo/data/v3"
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
RANK = {t["teamId"]: t["rank"] for t in top}
cheb = lambda a, b, c, d: max(abs(a - c), abs(b - d))
idx = load_index()
files = {}
for d in ("full", "ours_full"):
    for f in glob.glob(f"{V3}/decoded/{d}/*.json"):
        files.setdefault(os.path.basename(f)[:-5], f)
rows = collections.defaultdict(list)
for m, f in files.items():
    if m not in idx: continue
    info = idx[m]; tid = info["teamIds"]
    coh = "ours" if XAN in tid else ("top10" if all(RANK.get(x, 999) <= 10 for x in tid) else None)
    if not coh: continue
    d = json.load(open(f, encoding="utf8"))
    S = {s["r"]: s for s in d["rounds"]}
    n = len(d["rounds"][0]["c"]); team = [r[0] for r in d["rounds"][0]["c"]]
    rs = sorted(S)
    def snap(r):
        while r not in S and r > 0: r -= 1
        return S.get(r, d["rounds"][0])
    base = [[0, 0], [0, 0]]  # per attacking team: enemy alive champion-rounds, of which carrying >= 20
    for s in d["rounds"]:
        for i in range(n):
            if s["c"][i][2]:
                base[1 - team[i]][0] += 1; base[1 - team[i]][1] += s["c"][i][7] >= 20
    rec = [dict(cc=0, cc_carrier=0, hits=0, hits_carrier=0, locks=0, locks_enemy_bag=0, att=0) for _ in (0, 1)]
    for a in d["actions"]:
        t = team[a[1]]; s = snap(a[0])
        if a[2] == 5: rec[t]["att"] += 1
        for h in a[9]:
            if h[0] == "c" and h[2] > 0:
                rec[t]["hits"] += 1; rec[t]["hits_carrier"] += s["c"][h[1]][7] >= 20
        if a[2] == 6 and a[11] == 4:
            rec[t]["cc"] += 1
            if any(team[j] != t and s["c"][j][2] and s["c"][j][7] >= 20 and cheb(s["c"][j][3], s["c"][j][4], a[5], a[6]) <= 1 for j in range(n)): rec[t]["cc_carrier"] += 1
        if a[2] == 20 and a[12] & 4 and not a[13] & 4:
            rec[t]["locks"] += 1
            if any(team[j] != t and s["c"][j][2] and s["c"][j][7] > 0 and cheb(s["c"][j][3], s["c"][j][4], a[5], a[6]) <= 2 for j in range(n)): rec[t]["locks_enemy_bag"] += 1
    sides = [0, 1] if coh == "top10" else [tid.index(XAN)]
    for t in sides:
        r = rec[t]; r["base"] = base[t][1] / max(1, base[t][0]); r["rounds"] = d["rounds"][-1]["r"]; r["map"] = info["map"]; r["team"] = info["teams"][t]
        rows[coh].append(r)
for coh, rs in rows.items():
    k = len(rs); tot = lambda key: sum(r[key] for r in rs)
    print(f"\n### {coh}: {len(files)} full-action files in total; {k} team-sides{'' if k >= 30 else ' (below 30)'}")
    print(f"stun (cc) casts per team-game {tot('cc') / k:.1f}; share landing within 1 tile of an enemy carrying >= 20 gems {100 * tot('cc_carrier') / max(1, tot('cc')):.0f}%")
    print(f"damaging hits per team-game {tot('hits') / k:.0f}; share on carriers {100 * tot('hits_carrier') / max(1, tot('hits')):.0f}% against {100 * statistics.mean(r['base'] for r in rs):.0f}% of enemy champion-rounds being carrier-rounds")
    print(f"altar-lock placements per team-game {tot('locks') / k:.2f}; with an enemy holding a bag within 2 tiles of the lock tile: {100 * tot('locks_enemy_bag') / max(1, tot('locks')):.0f}% ({tot('locks_enemy_bag')} of {tot('locks')})")
    if coh == "top10":
        byteam = collections.defaultdict(list)
        for r in rs: byteam[r["team"]].append(r)
        for tm, v in sorted(byteam.items(), key=lambda kv: -len(kv[1])):
            print(f"  {tm}: n={len(v)}{'†' if len(v) < 30 else ''}, cc {sum(x['cc'] for x in v) / len(v):.1f}, hits on carriers {100 * sum(x['hits_carrier'] for x in v) / max(1, sum(x['hits'] for x in v)):.0f}%, locks {sum(x['locks'] for x in v) / len(v):.2f}")
