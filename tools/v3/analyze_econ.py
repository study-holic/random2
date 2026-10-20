"""Gem income and champion-round allocation from round-level decodes (extract_pilot.mjs --stride 13).
For every decoded game (our games in decoded/ours_s13, everyone else in decoded/s13) and each team:
  tributed gems per round, gems collected (sum of positive bag increments), gems lost on death, tributes, and where each alive champion stands at every
  round start: dead / spawn (within 3 of the team's round-1 centroid) / altar zone (within 2 of a tile where tributes were made on that map id, >= 5 times)
  / gem camps (within 4 of a wolf, elk, gem or monolith camp: types 0,1,2,4) / gold camps (within 4 of a type-3 camp) / transit (everything else).
Camp tiles come from the first state of one platform replay per map (tools/v3/camps0.mjs -> data/v3/maps_camps_platform.jsonl).
Output: data/v3/decoded/econ.jsonl (one line per game). Run: py -I tools/v3/analyze_econ.py
"""
import json, os, sys, glob, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, "C:/cambc-solo/tools/v3")
from analyze_rounds import load_index

V3 = "C:/cambc-solo/data/v3"
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
RANK = {t["teamId"]: t["rank"] for t in top}
CATS = ["dead", "spawn", "altar", "gem camps", "gold camps", "transit"]
cheb = lambda a, b, c, d: max(abs(a - c), abs(b - d))

names = json.load(open(f"{V3}/lists/camps_probe_names.json"))
CAMPS = {}
for l in open(f"{V3}/maps_camps_platform.jsonl", encoding="utf8"):
    d = json.loads(l)
    CAMPS[names[d["file"][:-4]]] = d["camps"]

idx = load_index()
ALT = collections.defaultdict(collections.Counter)
for l in open(f"{V3}/decoded/games_s13.jsonl", encoding="utf8"):
    g = json.loads(l)
    for e in g["ev"]["tribute"]:
        if e[5] >= 0:
            ALT[g["mapId"]][(e[5], e[6])] += 1
ALTT = {m: [t for t, c in cnt.items() if c >= 5] for m, cnt in ALT.items()}


_memo = {}


def classify(x, y, camps, altar, key):
    k = (key, x, y)
    if k not in _memo:
        _memo[k] = _classify(x, y, camps, altar)
    return _memo[k]


def _classify(x, y, camps, altar):
    if altar and min(cheb(x, y, a, b) for a, b in altar) <= 2:
        return "altar"
    dg = min((cheb(x, y, c[1], c[2]) for c in camps if c[0] != 3), default=99)
    dd = min((cheb(x, y, c[1], c[2]) for c in camps if c[0] == 3), default=99)
    if min(dg, dd) <= 4:
        return "gem camps" if dg <= dd else "gold camps"
    return "transit"


def analyse(d, info):
    snaps = d["rounds"]
    c0 = snaps[0]["c"]
    n = len(c0)
    team = [r[0] for r in c0]
    sp = {}
    for t in (0, 1):
        xs = [r[3] for r in c0 if r[0] == t]; ys = [r[4] for r in c0 if r[0] == t]
        sp[t] = (sum(xs) / len(xs), sum(ys) / len(ys))
    camps = CAMPS.get(info["map"], [])
    altar = ALTT.get(info["mapId"], [])
    cnt = [collections.Counter(), collections.Counter()]
    col = [0, 0]; lost = [0, 0]; deaths = [0, 0]; recalls = [0, 0]
    for k, s in enumerate(snaps):
        for i in range(n):
            c = s["c"][i]; t = team[i]
            if not c[2]:
                cnt[t]["dead"] += 1
                continue
            if cheb(c[3], c[4], sp[t][0], sp[t][1]) <= 3:
                cnt[t]["spawn"] += 1
            else:
                cnt[t][classify(c[3], c[4], camps, altar, info["mapId"])] += 1
        if k:
            a = snaps[k - 1]
            for i in range(n):
                ga, gb = a["c"][i][7], s["c"][i][7]
                if gb > ga: col[team[i]] += gb - ga
                if a["c"][i][2] and not s["c"][i][2]:
                    lost[team[i]] += ga; deaths[team[i]] += 1
                ca, cb = a["c"][i], s["c"][i]
                if ca[2] and cb[2] and cheb(ca[3], ca[4], cb[3], cb[4]) >= 8 and cheb(cb[3], cb[4], sp[team[i]][0], sp[team[i]][1]) <= 3 and cb[5] >= ca[5]:
                    recalls[team[i]] += 1
    rounds = snaps[-1]["r"]
    tr = [0, 0]; trr = [[], []]
    for k in range(1, len(snaps)):
        for t in (0, 1):
            dg = snaps[k]["sg"][t] - snaps[k - 1]["sg"][t]
            if dg > 0: tr[t] += 1; trr[t].append((snaps[k]["r"], dg))
    return {"rounds": rounds, "n": n, "sg": snaps[-1]["sg"], "cnt": [dict(c) for c in cnt], "collected": col, "recalls": recalls, "lost": lost, "deaths": deaths, "tributes": tr,
            "first_trib": [trr[t][0][0] if trr[t] else None for t in (0, 1)], "mean_bag": [round(sum(b for _, b in trr[t]) / len(trr[t]), 1) if trr[t] else None for t in (0, 1)]}


out = 0
seen = set()
with open(f"{V3}/decoded/econ.jsonl", "w", encoding="utf8", newline="\n") as fh:
    for src in ("ours_s13", "s13"):
        for f in sorted(glob.glob(f"{V3}/decoded/{src}/*.json")):
            m = os.path.basename(f)[:-5]
            if m not in idx or m in seen:
                continue
            seen.add(m)
            info = idx[m]
            tid = info["teamIds"]
            coh = "ours" if XAN in tid else ("top10" if all(RANK.get(x, 999) <= 10 for x in tid) else ("micron" if "695ddbdf-afd9-43bf-9988-2a98fdb0f750" in tid else "other"))
            if coh == "other":
                continue
            try:
                r = analyse(json.load(open(f, encoding="utf8")), info)
            except Exception as e:
                print("ERR", m, type(e).__name__, e)
                continue
            r.update({"match": m, "map": info["map"], "mapId": info["mapId"], "teams": info["teams"], "teamIds": tid, "cohort": coh, "t": info["t"], "draft": info["draft"], "rating": info["rating"], "score": info["score"], "endReason": info["endReason"]})
            fh.write(json.dumps(r, ensure_ascii=False) + "\n")
            out += 1
print(out, "games written")
