"""Gem and gold pickups per game by camp type, per team, from decoded v4top games (results/imitate.md section 2.1).
A pickup is a rise in a champion's bag or gold between round snapshots. It is a kill pickup when the champion's kill
count rose in the same interval; else it is a capture when the champion stands on a camp spawn tile in either snapshot.
The capture is split over the group's member types in proportion to their engine rewards (wolf 4 gems/40 gold, elk 15/90,
gem cluster 7/0, gold cluster 0/20, Monolith 200/300; a group pays the sum, data/maps21.json). Groups come from
maps21.json mirrored by the map's symmetry (it lists one side). Other rises are 'other' (e.g. kill gold, tribute gold).
Run from the repo root: python3 -I tools/imitate/pickups.py <dir of decoded v4top-*.json> [out.json]"""
import json, sys, glob, os, collections
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools", "cloud")); import metrics as M
REW = {0: (4, 40), 1: (15, 90), 2: (7, 0), 3: (0, 20), 4: (200, 300)}
TYPE = {0: "wolf", 1: "elk", 2: "gem cluster", 3: "gold cluster", 4: "Monolith"}

def tile_groups(m):
    w, h, s = m["w"], m["h"], m["sym"]
    f = (lambda x, y: (w - 1 - x, y)) if s == 0 else (lambda x, y: (x, h - 1 - y)) if s == 1 else (lambda x, y: (w - 1 - x, h - 1 - y))
    out = {}
    for g in m["grp"]:
        mem = [(t, x, y) for t, x, y in g["camps"]]
        mir = [(t,) + f(x, y) for t, x, y in mem]
        if {(x, y) for _, x, y in mem} & {(x, y) for _, x, y in mir}:   # self-mirrored group: one group with both halves
            allm = list({(t, x, y) for t, x, y in mem + mir})
            for _, x, y in allm: out[(x, y)] = allm
        else:
            for _, x, y in mem: out[(x, y)] = mem
            for _, x, y in mir: out[(x, y)] = mir
    return out

def shares(members, j):
    tot = sum(REW[t][j] for t, _, _ in members)
    c = collections.Counter()
    for t, _, _ in members: c[TYPE[t]] += REW[t][j] / tot if tot else 0
    return c

def game(d, mp):
    m = M.MAPS[M.mapkey(mp)]; tg = tile_groups(m)
    S = M.snaps_of(d); team = [c[0] for c in S[0]["c"]]; n = len(team)
    out = {0: collections.Counter(), 1: collections.Counter()}
    for a, b in zip(S, S[1:]):
        for i in range(n):
            ca, cb = a["c"][i], b["c"][i]
            if not (ca[2] and cb[2]): continue
            dg, dgo = cb[7] - ca[7], cb[8] - ca[8]
            if dg <= 0 and dgo <= 0: continue
            t = team[i]
            if cb[9] > ca[9]:
                if dg > 0: out[t]["gems: kills"] += dg
                if dgo > 0: out[t]["gold: kills"] += dgo
                continue
            mem = tg.get((ca[3], ca[4])) or tg.get((cb[3], cb[4]))
            if mem is None:
                if dg > 0: out[t]["gems: other"] += dg
                if dgo > 0: out[t]["gold: other"] += dgo
                continue
            if dg > 0:
                for k, v in shares(mem, 0).items(): out[t]["gems: " + k] += dg * v
            if dgo > 0:
                for k, v in shares(mem, 1).items(): out[t]["gold: " + k] += dgo * v
    return out

if __name__ == "__main__":
    idx = {}
    for l in open(os.path.join(ROOT, "data", "unranked", "matches.jsonl"), encoding="utf8"):
        mm = json.loads(l); idx[(mm["label"], mm["position"])] = mm
    idx.update(M.panel_index(idx))
    rows = []
    for f in sorted(glob.glob(os.path.join(sys.argv[1], "*.json"))):
        bn = os.path.basename(f)[:-5]; lab, pos = bn.rsplit("-", 1); mm = idx[(lab, int(pos))]; us = 0 if mm["our_side"] == "a" else 1
        g = game(json.load(open(f)), mm["map"])
        rows.append(dict(label=bn, map=mm["map"][2:], opp=mm["opponent"], v4=dict(g[us]), top=dict(g[1 - us])))
    if len(sys.argv) > 2: json.dump(rows, open(sys.argv[2], "w"))
    keys = sorted({k for r in rows for s in ("v4", "top") for k in r[s]})
    n = len(rows)
    print("| pickup | v4 a game | top 10 a game |\n|---|---|---|")
    for k in keys:
        print(f"| {k} | {sum(r['v4'].get(k, 0) for r in rows) / n:.1f} | {sum(r['top'].get(k, 0) for r in rows) / n:.1f} |")
