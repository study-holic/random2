"""State-conditioned decision rates for v4 and the top 10 (results/imitate.md section 3).
One row per alive champion-round, outside a recall or tribute channel, from decoded v4top games. State (round snapshot):
HP fraction, bag, Chebyshev distance to own spawn, to the nearest altar and to the nearest enemy, enemies within 8,
allies within 4, own HP / nearest enemy HP, own gear points minus the nearest enemy's (tools/v3/a2_fights.py units).
Enemy counts use positions, not line of sight. Outcomes:
  recall5  a recall that completes is started in rounds r..r+4 (start-and-cancel steps excluded: Pantheon);
  trib3    a tribute start (step kind 12) in rounds r..r+2;
  hit2     an attack or cast by this champion damages an enemy champion in rounds r..r+1.
Writes binned counts to a JSON file: python3 -I tools/imitate/policies.py <dir of decoded v4top-*.json | mapgate result.json> <out.json>\n(for a mapgate result the tested bot is labelled "v4" and its opponent "top10")"""
import json, sys, glob, os, collections, math
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools", "cloud")); sys.path.insert(0, os.path.join(ROOT, "tools", "imitate"))
import metrics as M
from recalls import recall_events, idx
from queries import item_timeline
from a2_fights import stats_from_items
ch = lambda a, b: max(abs(a[0] - b[0]), abs(a[1] - b[1]))

def B(v, edges):   # index of the first edge v is below, else len(edges)
    for k, e in enumerate(edges):
        if v < e: return k
    return len(edges)

HPE, BAGE, DSE, EN8E, DAE, RATE, GDE, AL4E = [0.3, 0.5, 0.7, 0.9], [1, 10, 20, 40, 80], [6, 11, 21, 31], [1, 2, 3], [2, 4, 8, 16], [0.5, 0.8, 1.25, 2.0], [-3, -1, 1, 3], [1, 2]

if __name__ == "__main__":
    T = collections.defaultdict(lambda: [0, 0])   # key -> [rows, events]
    jobs = []
    if sys.argv[1].endswith(".json"):   # a scripts/mapgate.py result: our bot is "v4" in the output, the panel bot "top10"
        G = json.load(open(sys.argv[1])); cache = os.path.join(G["work"], "decoded")
        M.decode([r["replay"] for r in G["results"]], cache)
        for r in G["results"]:
            f = os.path.join(cache, os.path.basename(r["replay"])[:-8] + ".json")
            if os.path.exists(f): jobs.append((f, 0 if r["side"] == "A" else 1, r["map"], r["opp"]))
    else:
        for f in sorted(glob.glob(os.path.join(sys.argv[1], "*.json"))):
            bn = os.path.basename(f)[:-5]; lab, pos = bn.rsplit("-", 1); mm = idx[(lab, int(pos))]
            jobs.append((f, 0 if mm["our_side"] == "a" else 1, mm["map"], mm["opponent"]))
    for f, us, mp_, opp in jobs:
        mm = dict(map=mp_, opponent=opp)
        d = json.load(open(f)); ev, S = recall_events(d); m = M.MAPS.get(M.mapkey(mm["map"]))
        if m is None: continue
        alt = M.mirror(m, [tuple(a) for a in m["altars"]])
        team = [c[0] for c in S[0]["c"]]; typ = [c[1] for c in S[0]["c"]]; n = len(team)
        spawn = {t: [(c[3], c[4]) for c in S[0]["c"] if c[0] == t] for t in (0, 1)}
        tl = item_timeline(d)
        rec_start = collections.defaultdict(set); chan = collections.defaultdict(set)
        for e in ev:
            if e["fate"] == "done":
                rec_start[e["i"]].add(e["r"])
                for r in range(e["r"] + 1, e["r"] + e["dur"] + 1): chan[e["i"]].add(r)
        trib = collections.defaultdict(set); hit = collections.defaultdict(set)
        for a in d["actions"]:
            r, i, k = a[0], a[1], a[2]
            if k == 12:
                trib[i].add(r)
                for rr in range(r + 1, r + 1 + max(1, math.ceil(a[7] / 10))): chan[i].add(rr)
            if k in (5, 6) and any(h[0] == "c" and team[h[1]] != team[i] for h in a[9]): hit[i].add(r)
        gear_at = []
        for i in range(n):
            g = {}; li = tl[i]; k = 0; cur = {}
            for s in S:
                while k + 1 < len(li) and li[k + 1][0] <= s["r"]: k += 1
                g[s["r"]] = sum(stats_from_items(typ[i], li[k][1])[3])
            gear_at.append(g)
        for s in S[1:]:
            r = s["r"]; C = s["c"]
            if s.get("final"): continue
            alive = [i for i in range(n) if C[i][2]]
            for i in alive:
                if r in chan[i]: continue
                t = team[i]; p = (C[i][3], C[i][4]); side = "v4" if t == us else ("pantheon" if mm["opponent"] == "Pantheon" else "top10")
                hpf = C[i][5] / max(C[i][6], 1); bag = C[i][7]
                ds = min(ch(p, q) for q in spawn[t])
                foes = [j for j in alive if team[j] != t]; mates = [j for j in alive if team[j] == t and j != i]
                en8 = sum(1 for j in foes if ch(p, (C[j][3], C[j][4])) <= 8)
                al4 = sum(1 for j in mates if ch(p, (C[j][3], C[j][4])) <= 4)
                da = min(ch(p, q) for q in alt)
                rc = any(rr in rec_start[i] for rr in range(r, r + 5))
                tb = any(rr in trib[i] for rr in range(r, r + 3))
                if ds >= 3:
                    k = ("recall", side, B(hpf, HPE), B(bag, BAGE), B(ds, DSE), B(en8, EN8E)); T[k][0] += 1; T[k][1] += rc
                if bag > 0:
                    k = ("bank", side, B(bag, BAGE), B(da, DAE), B(en8, EN8E)); T[k][0] += 1; T[k][1] += tb
                if foes:
                    jn = min(foes, key=lambda j: ch(p, (C[j][3], C[j][4]))); dn = ch(p, (C[jn][3], C[jn][4]))
                    if dn <= 4:
                        hb = any(rr in hit[i] for rr in (r, r + 1))
                        ratio = C[i][5] / max(C[jn][5], 1); gd = gear_at[i][r] - gear_at[jn][r]
                        k = ("engage", side, B(ratio, RATE), B(gd, GDE), B(al4, AL4E), min(dn, 4)); T[k][0] += 1; T[k][1] += hb
    json.dump({"|".join(map(str, k)): v for k, v in T.items()}, open(sys.argv[2], "w"))
    print(len(T), "cells")
