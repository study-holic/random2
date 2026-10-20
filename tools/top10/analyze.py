"""Per team-side metrics for the top-10 sample (and our games). Run: py -I tools/top10/analyze.py
Inputs: data/top10/s13/<id>.json (round-level), data/top10/full/<id>.json (action log), data/top10/raw/g_*.json (platform results).
Output: data/top10/sides.json (one record per team-side)."""
import json, os, sys, glob, statistics as st
sys.path.insert(0, "C:/cambc-solo/tools/v3")
from derive_events import derive
from a2_fights import process, stats_from_items, parse_key
D = "C:/cambc-solo/data/top10"
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
NM = ['Ryth','Vesper','Vendetta','Cassi','Issac','Stellar','Prism','Marcella','Willow','Ren','Varg','Tavi']
CH = ['Ryth','Vesper','Vendetta','CassiIssac','Stellar','Prism','Marcella','Willow','Ren','Varg','Tavi']
meta = {}
for f in glob.glob(D + "/raw/g_*.json"):
    G = json.load(open(f, encoding="utf8"))
    for m in G["matches"]:
        if m["status"] == "finished":
            meta[m["id"]] = dict(map=m["map"], mapId=m["mapId"], t=m["createdAt"], rounds=m["rounds"], sides=m["sides"], patch=m["patchId"][:5], end=m["endReason"])
ids = [i for i in os.listdir(D + "/s13") if i.endswith(".json")]
out = []
cheb = lambda a, b: max(abs(a[0] - b[0]), abs(a[1] - b[1]))
for fn in sorted(ids):
    mid = fn[:-5]
    if mid not in meta: continue
    d = json.load(open(f"{D}/s13/{fn}")); ev, summ = derive(d)
    mt = meta[mid]
    snaps = []; last = -1
    for s in d["rounds"]:
        if s["r"] != last or s.get("final"): snaps.append(s); last = s["r"]
    byr = {s["r"]: s for s in snaps}
    # items per round -> gear points per champion
    n = len(snaps[0]["c"]); cur = [{} for _ in range(n)]; pts_at = {}
    typ = [c[1] for c in snaps[0]["c"]]; team = [c[0] for c in snaps[0]["c"]]
    for s in snaps:
        for i, k in s.get("it", []): cur[i] = parse_key(k)
        pts_at[s["r"]] = [sum(stats_from_items(typ[i], cur[i])[3]) for i in range(n)]
    def pts(t, r):
        rr = max(k for k in pts_at if k <= r)
        v = [pts_at[rr][i] for i in range(n) if team[i] == t]
        return sum(v) / len(v)
    # fights from the action log
    fights = []
    ff = f"{D}/full/{fn}"
    if os.path.exists(ff):
        try: fights = process(ff, mid, {}, "t")
        except Exception as e: print("fight err", mid, e)
    for t in (0, 1):
        sd = mt["sides"][t]
        bodies = [c for c in snaps[0]["c"] if c[0] == t]
        fin = snaps[-1]; R = fin["r"]
        sp = (sum(c[3] for c in bodies) / len(bodies), sum(c[4] for c in bodies) / len(bodies))
        osp = tuple(sum(c[k] for c in snaps[0]["c"] if c[0] == 1 - t) / len([c for c in snaps[0]["c"] if c[0] == 1 - t]) for k in (3, 4))
        tr = {e[0]: e[4] for e in ev["tribute"] if e[1] == t}
        t1000 = next((s["r"] for s in snaps if s["sg"][t] >= 1000), None)
        deaths = [e for e in ev["death"] if e[2] == t]
        # spread and spawn pressure over rounds 100..400
        spread = []; press = 0; alive_r = 0
        for s in snaps:
            if 100 <= s["r"] <= 400:
                a = [(c[3], c[4]) for c in s["c"] if c[0] == t and c[2]]
                if len(a) >= 2: spread.append(st.mean(cheb(p, q) for i, p in enumerate(a) for q in a[i + 1:]))
                press += sum(1 for p in [(c[3], c[4]) for c in s["c"] if c[0] == 1 - t and c[2]] if cheb(p, sp) <= 5)  # enemy champs near OUR spawn (opponent camping us)
        kills_by = lambda r: sum(c[9] for c in byr[max(k for k in byr if k <= r)]["c"] if c[0] == t)
        ft = [f for f in fights if f["init_team"] == t]
        rec = dict(match=mid, map=mt["map"], mapId=mt["mapId"], patch=mt["patch"], t=mt["t"], team=sd["teamName"], teamId=sd["teamId"], side=t, win=int(sd["result"] == "win"),
                   score=sd["score"], rounds=R, draft=sd["draft"], types=sorted(c[1] for c in bodies), bodies=len(bodies), sg=fin["sg"][t], t1000=t1000,
                   n_trib=len(tr), mean_bag=(sum(tr.values()) / len(tr) if tr else None), first_trib=min(tr) if tr else None, deaths=len(deaths),
                   deaths_carry=sum(1 for e in deaths if e[7] > 0), gems_lost=sum(e[7] for e in deaths),
                   recalls=sum(e[2] == t for e in ev["recall"]), recalls_carry=sum(1 for e in ev["recall"] if e[2] == t and e[7] > 0),
                   lock_buy=sum(1 for e in ev["buy"] if e[2] == t and e[4] == "cons2"), lock_use=sum(1 for e in ev["use"] if e[2] == t and e[4] == "cons2"),
                   cons_buy=sum(1 for e in ev["buy"] if e[2] == t and str(e[4]).startswith("cons")),
                   captures=sum(1 for e in ev["capture"] if e[2] == t), captures300=sum(1 for e in ev["capture"] if e[2] == t and e[0] <= 300),
                   pts200=pts(t, 200) if R >= 200 else None, pts400=pts(t, 400) if R >= 400 else None,
                   kills200=kills_by(200), kills_end=kills_by(R), spread=(st.mean(spread) if spread else None), camped_on=press,
                   nfights=len(ft), nspell=sum(1 for f in ft if f["init_kind"] == "spell"), has_full=bool(fights),
                   fights_all=len(fights), fights_near_enemy_spawn=sum(1 for f in ft if cheb(f["c0"], osp) <= 6), fights_near_own_spawn=sum(1 for f in ft if cheb(f["c0"], sp) <= 6))
        out.append(rec)
json.dump(out, open(D + "/sides.json", "w"))
print(len(out), "sides from", len(ids), "games;", sum(r["has_full"] for r in out), "with action log")
