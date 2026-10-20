"""Per-game, per-team metrics from decoded replays (tools/top10/extract_pilot.mjs output, stride 1).
Shared by the v5 cloud sessions (field names fixed in tools/cloud/v5-common.md). Run with python3 -I.

Inputs (pick one):
  --panel DIR            decoded real games named <label>-<pos>.json (data/real/v4-panel groups); our team from data/unranked/matches.jsonl
  --mapgate RES.json     a scripts/mapgate.py result; replays are decoded into --cache (default <work>/decoded) with node
  FILE.json:TEAM:MAP     explicit triples (TEAM 0 = side A)
Options: --jobs N (parallel decodes, default 3), --out f.json (one record per game), --by map|opp|none (table grouping, default map), --quiet, --flip (measure the other team).

Fields per record (us = the named team, them = the other):
  banked_by_round {250,500,...,2000}, t1000 (round banked gems reach 1,000), gems, gems_opp, win
  funnel: acquired_camps, gained_kills, tributed, destroyed_monster, lost_champion, carried_end, residual
          (acquired_camps + gained_kills - tributed - destroyed_monster - lost_champion - carried_end; 0 when every gem is traced)
  deaths, deaths_opp, deaths_by_cause {champion, monster, other}, deaths_by_loc {spawn, altar, camp, elsewhere}, deaths_by_band {0-200, 200-600, 600+}
  locks_placed, locks_placed_opp, gear200 (mean gear points per champion at round 200; same unit as tools/top10/analyze.py pts200)
  fights_opened, fights_opened_ability, ability_open_share (fights we open whose first hit is a cast; tools/v3/a2_fights.py definition)
  time_use: share of champion-rounds dead, tributing, recalling, fighting, clearing, capturing, walking, idle
Death cause follows tools/step0/agg2.py: champion if an enemy action hit the victim in the death round or the one before
(Cassi and Issac share fate: a partner's death takes the partner's killer), else monster if within 8 of a monster camp, else other.
"""
import json, os, sys, glob, math, collections, subprocess
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.join(ROOT, "tools", "v3"))
import a2_fights  # noqa: E402
from a2_fights import process as fights_of, stats_from_items, parse_key  # noqa: E402
a2_fights.BASE.setdefault(12, (1000, 100, 40, 5, 100, 15))  # Heart of Ruin (body type 12): 1000/40/100, growth 100/5/15 (FACTS.md)

MAPS = json.load(open(os.path.join(ROOT, "data", "maps21.json")))
TN = {0: "Ryth", 1: "Vesper", 2: "Vendetta", 3: "C&I", 4: "C&I", 5: "Stellar", 6: "Prism", 7: "Marcella", 8: "Willow", 9: "Ren", 10: "Varg", 11: "Tavi", 12: "HoR"}
ch = lambda a, b: max(abs(a[0] - b[0]), abs(a[1] - b[1]))
mapkey = lambda n: n[2:] if n[:2].isdigit() else n
# engine 1.0 step kinds (checked 20 Oct on decoded games: 12 targets an altar tile and precedes each tribute; 11 precedes recalls)
K_MOVE, K_ATK, K_CAST, K_RECALL, K_TRIB, K_CAPT = 4, 5, 6, 11, 12, 23
LOCK_BIT = 2


def mirror(m, pts):
    """data/maps21.json lists half the camps (one side); sym 0 mirrors x, 1 mirrors y, 2 rotates (checked against both
    teams' spawn tiles on the 10 panel maps). Returns the points plus their mirror images, deduplicated."""
    w, h, s = m["w"], m["h"], m["sym"]
    f = (lambda x, y: (w - 1 - x, y)) if s == 0 else (lambda x, y: (x, h - 1 - y)) if s == 1 else (lambda x, y: (w - 1 - x, h - 1 - y))
    return sorted(set(pts) | {f(x, y) for x, y in pts})


def snaps_of(d):
    out, last = [], -1
    for s in d["rounds"]:
        if s["r"] != last or s.get("final"):
            out.append(s); last = s["r"]
    return out


def game_metrics(d, us, mp, label=""):
    S = snaps_of(d); m = MAPS[mapkey(mp)]
    c0 = S[0]["c"]; n = len(c0); team = [c[0] for c in c0]; typ = [c[1] for c in c0]
    th = 1 - us
    start = {t: [(c[3], c[4]) for c in c0 if c[0] == t] for t in (0, 1)}
    mons = mirror(m, [(x, y) for t, x, y in m["camps"] if t in (0, 1, 4)])
    alt = mirror(m, [tuple(a) for a in m["altars"]])
    by_r = collections.defaultdict(list)
    for a in d["actions"]:
        by_r[a[0]].append(a)
    fin = S[-1]; R = fin["r"]
    rec = dict(label=label, map=mapkey(mp), team=us, rounds=R, gems=fin["sg"][us], gems_opp=fin["sg"][th])
    oc = d.get("outcome")  # decoder outcome: 1 = side A won, 2 = side B won (checked against platform results)
    rec["win"] = int(oc == us + 1) if oc in (1, 2) else int(fin["sg"][us] > fin["sg"][th])
    rec["banked_by_round"] = {}
    for rr in range(250, 2001, 250):
        prev = [s for s in S if s["r"] <= rr]
        rec["banked_by_round"][rr] = prev[-1]["sg"][us] if prev else 0
    rec["t1000"] = next((s["r"] for s in S if s["sg"][us] >= 1000), None)

    # deaths with cause (step0 exact method)
    deaths = []
    for a, b in zip(S, S[1:]):
        evs = []
        for i in range(n):
            if not (a["c"][i][2] and not b["c"][i][2]):
                continue
            t = team[i]; p = (a["c"][i][3], a["c"][i][4]); k = None
            for r in (a["r"], b["r"]):
                for act in by_r.get(r, []):
                    if team[act[1]] != t and any(h[0] == "c" and h[1] == i and h[2] > 0 for h in act[9]):
                        k = act[1]
            if min(ch(p, s) for s in start[t]) <= 8: loc = "spawn"
            elif any(ch(p, q) <= 8 for q in alt): loc = "altar"
            elif any(ch(p, q) <= 8 for q in mons): loc = "camp"
            else: loc = "elsewhere"
            r = b["r"]; band = "0-200" if r < 200 else "200-600" if r < 600 else "600+"
            evs.append(dict(i=i, t=t, k=k, loc=loc, nearmon=any(ch(p, q) <= 8 for q in mons), band=band, bag=a["c"][i][7], r=r, x=p[0], y=p[1],
                            victim=TN[typ[i]]))
        for e in evs:
            if e["victim"] == "C&I" and e["k"] is None:
                mates = [o for o in evs if o is not e and o["t"] == e["t"] and o["victim"] == "C&I" and o["k"] is not None]
                if mates: e["k"] = mates[0]["k"]; e["partner"] = True
        for e in evs:
            e["cause"] = "champion" if e["k"] is not None else ("monster" if e["nearmon"] else "other")
            e["killer"] = TN[typ[e["k"]]] if e["k"] is not None else None
            deaths.append(e)
    du = [e for e in deaths if e["t"] == us]
    rec["deaths"] = len(du); rec["deaths_opp"] = sum(e["t"] == th for e in deaths)
    rec["deaths_by_cause"] = dict(collections.Counter(e["cause"] for e in du))
    rec["deaths_by_loc"] = dict(collections.Counter(e["loc"] for e in du))
    rec["deaths_by_band"] = dict(collections.Counter(e["band"] for e in du))
    rec["death_list"] = [[e["r"], e["victim"], e["cause"], e["killer"], e["loc"], e["bag"], e["x"], e["y"]] for e in du]

    # gem funnel: positive bag changes, split into kill pickups (killer's kill count rose) and camp gems
    acq = 0; killg = 0
    for a, b in zip(S, S[1:]):
        for i in range(n):
            if team[i] != us: continue
            dg = b["c"][i][7] - a["c"][i][7]
            if dg <= 0 or not b["c"][i][2]: continue
            if b["c"][i][9] > a["c"][i][9]: killg += dg
            else: acq += dg
    trib = fin["sg"][us]
    dmon = sum(e["bag"] for e in du if e["cause"] != "champion")
    dch = sum(e["bag"] for e in du if e["cause"] == "champion")
    carry = sum(c[7] for c in fin["c"] if c[0] == us and c[2])
    rec["funnel"] = dict(acquired_camps=acq, gained_kills=killg, tributed=trib, destroyed_monster=dmon, lost_champion=dch, carried_end=carry,
                         residual=acq + killg - trib - dmon - dch - carry)

    # locks and gear
    lk = [0, 0]
    for a, b in zip(S, S[1:]):
        for i in range(n):
            if (a["c"][i][10] >> LOCK_BIT & 1) and not (b["c"][i][10] >> LOCK_BIT & 1) and b["c"][i][2]:
                lk[team[i]] += 1
    rec["locks_placed"], rec["locks_placed_opp"] = lk[us], lk[th]
    cur = [{} for _ in range(n)]; g200 = None
    for s in S:
        if s["r"] > 200: break
        for i, k in s.get("it", []): cur[i] = parse_key(k)
    if R >= 200:
        v = [sum(stats_from_items(typ[i], cur[i])[3]) for i in range(n) if team[i] == us]
        g200 = sum(v) / len(v)
    rec["gear200"] = g200

    # fights (a2_fights definition)
    return rec, S, deaths


def fights_and_time(path, d, S, us, mp, rec):
    try:
        F = fights_of(path, "", {}, "m")
    except Exception as e:  # noqa: BLE001
        F = []; rec["fight_error"] = repr(e)
    ft = [f for f in F if f["init_team"] == us]
    rec["fights_all"] = len(F); rec["fights_opened"] = len(ft); rec["fights_opened_ability"] = sum(f["init_kind"] == "spell" for f in ft)
    fo = [f for f in F if f["init_team"] == 1 - us]
    rec["fights_opened_opp"] = len(fo); rec["fights_opened_ability_opp"] = sum(f["init_kind"] == "spell" for f in fo)
    rec["ability_open_share"] = rec["fights_opened_ability"] / len(ft) if ft else None
    rec["ability_open_share_opp"] = rec["fights_opened_ability_opp"] / len(fo) if fo else None

    # time use per champion-round
    m = MAPS[mapkey(mp)]; alt = mirror(m, [tuple(a) for a in m["altars"]])
    grp_tiles = set(mirror(m, [(x, y) for g in m["grp"] for _, x, y in g["camps"]]))
    c0 = S[0]["c"]; n = len(c0); team = [c[0] for c in c0]
    acts = collections.defaultdict(list)
    for a in d["actions"]:
        acts[(a[0], a[1])].append(a)
    chan_until = {}; chan_kind = {}
    cnt = collections.Counter()
    for s in S[1:]:
        if s.get("final"): continue
        r = s["r"]
        for i in range(n):
            if team[i] != us: continue
            c = s["c"][i]
            if not c[2]: cnt["dead"] += 1; chan_until.pop(i, None); continue
            A = acts.get((r, i), [])
            kinds = {a[2] for a in A}
            if K_TRIB in kinds:
                chan_kind[i] = "tributing"; chan_until[i] = r + max(1, math.ceil(c[7] / 10))
            elif K_RECALL in kinds:
                chan_kind[i] = "recalling"; chan_until[i] = r + 5
            hit_c = any(h[0] == "c" for a in A for h in a[9] if a[2] in (K_ATK, K_CAST))
            hit_m = any(h[0] == "m" for a in A for h in a[9] if a[2] in (K_ATK, K_CAST))
            moved = K_MOVE in kinds or any(a[2] == K_CAST and a[11] == 6 for a in A)
            if (K_TRIB in kinds or K_RECALL in kinds) or (i in chan_until and r <= chan_until[i] and not (moved or hit_c or hit_m)):
                cnt[chan_kind[i]] += 1; continue
            chan_until.pop(i, None)
            if hit_c: cnt["fighting"] += 1
            elif hit_m: cnt["clearing"] += 1
            elif moved: cnt["walking"] += 1
            elif K_CAPT in kinds or (c[3], c[4]) in grp_tiles: cnt["capturing"] += 1
            else: cnt["idle"] += 1
    tot = sum(cnt.values()) or 1
    rec["time_use"] = {k: round(cnt[k] / tot, 3) for k in ("dead", "tributing", "recalling", "fighting", "clearing", "capturing", "walking", "idle")}


def run_one(path, us, mp, label):
    d = json.load(open(path))
    rec, S, deaths = game_metrics(d, us, mp, label)
    fights_and_time(path, d, S, us, mp, rec)
    return rec


def decode(replays, cache, jobs=3):
    os.makedirs(cache, exist_ok=True)
    todo = [r for r in replays if not os.path.exists(os.path.join(cache, os.path.basename(r)[:-8] + ".json"))]
    env = dict(os.environ); env.setdefault("CAMBC_ENGINE_CACHE", os.path.join(ROOT, "data", "top10", "engine"))
    chunks = [todo[k::jobs] for k in range(jobs) if todo[k::jobs]]
    procs = [subprocess.Popen(["node", os.path.join(ROOT, "tools", "top10", "extract_pilot.mjs"), *c, "--out", cache, "--stride", "1"],
                              env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) for c in chunks]
    for p in procs: p.wait()


def mean(xs):
    xs = [x for x in xs if x is not None]
    return sum(xs) / len(xs) if xs else None


def table(recs, by):
    keys = sorted({r.get(by, "all") if by != "none" else "all" for r in recs})
    print(f"| {by} | n | W | gems us/them | t1000 | deaths us/them | champ/mon/other | acq camps | kills+ | tributed | destroyed mon | lost champ | carried end | locks us/them | gear200 | ability-open us/them |")
    print("|" + "---|" * 16)
    for k in keys + (["ALL"] if len(keys) > 1 else []):
        g = [r for r in recs if k == "ALL" or (r.get(by, "all") if by != "none" else "all") == k]
        f = lambda key: mean([r["funnel"][key] for r in g])
        dc = lambda c: mean([r["deaths_by_cause"].get(c, 0) for r in g])
        t1 = [r["t1000"] for r in g if r["t1000"]]
        fo = sum(r["fights_opened"] for r in g); fa = sum(r["fights_opened_ability"] for r in g)
        fo2 = sum(r["fights_opened_opp"] for r in g); fa2 = sum(r["fights_opened_ability_opp"] for r in g)
        g2 = mean([r["gear200"] for r in g])
        print(f"| {k} | {len(g)} | {sum(r['win'] for r in g)} | {mean([r['gems'] for r in g]):.0f} / {mean([r['gems_opp'] for r in g]):.0f} | {len(t1)} reach, med {sorted(t1)[len(t1)//2] if t1 else '-'} | "
              f"{mean([r['deaths'] for r in g]):.1f} / {mean([r['deaths_opp'] for r in g]):.1f} | {dc('champion'):.1f}/{dc('monster'):.1f}/{dc('other'):.1f} | "
              f"{f('acquired_camps'):.0f} | {f('gained_kills'):.0f} | {f('tributed'):.0f} | {f('destroyed_monster'):.0f} | {f('lost_champion'):.0f} | {f('carried_end'):.0f} | "
              f"{mean([r['locks_placed'] for r in g]):.1f} / {mean([r['locks_placed_opp'] for r in g]):.1f} | {g2 if g2 is None else round(g2, 1)} | "
              f"{100*fa/max(fo,1):.0f}% ({fo}) / {100*fa2/max(fo2,1):.0f}% ({fo2}) |")


def main(argv):
    out = None; by = "map"; panel = None; mg = None; cache = None; triples = []; quiet = False; flip = False; jobs_n = 3
    it = iter(argv)
    for a in it:
        if a == "--out": out = next(it)
        elif a == "--by": by = next(it)
        elif a == "--panel": panel = next(it)
        elif a == "--mapgate": mg = next(it)
        elif a == "--cache": cache = next(it)
        elif a == "--jobs": jobs_n = int(next(it))
        elif a == "--quiet": quiet = True
        elif a == "--flip": flip = True
        else: triples.append(a)
    jobs = []  # (path, team, map, label, extra)
    if panel:
        idx = {}
        for l in open(os.path.join(ROOT, "data", "unranked", "matches.jsonl"), encoding="utf8"):
            mm = json.loads(l); idx[(mm["label"], mm["position"])] = mm
        for f in sorted(glob.glob(os.path.join(panel, "*.json"))):
            b = os.path.basename(f)[:-5]; lab, pos = b.rsplit("-", 1)
            mm = idx.get((lab, int(pos)))
            if not mm: continue
            jobs.append((f, 0 if mm["our_side"] == "a" else 1, mm["map"], b, dict(opp=mm["opponent"], set=lab.split("-")[0], real_result=mm["result"])))
    if mg:
        M = json.load(open(mg)); cache = cache or os.path.join(M["work"], "decoded")
        reps = [r["replay"] for r in M["results"] if os.path.exists(r["replay"])]
        decode(reps, cache, jobs_n)
        for r in M["results"]:
            p = os.path.join(cache, os.path.basename(r["replay"])[:-8] + ".json")
            if os.path.exists(p):
                jobs.append((p, 0 if r["side"] == "A" else 1, r["map"], os.path.basename(p)[:-5], dict(opp=r["opp"], salt=r["salt"], side=r["side"])))
    for t in triples:
        p, tm, mp = t.rsplit(":", 2); jobs.append((p, int(tm), mp, os.path.basename(p)[:-5], {}))
    if flip: jobs = [(p, 1 - tm, mp, lab, extra) for p, tm, mp, lab, extra in jobs]  # measure the opponent instead
    recs = []
    for p, tm, mp, lab, extra in jobs:
        try:
            r = run_one(p, tm, mp, lab); r.update(extra); recs.append(r)
        except Exception as e:  # noqa: BLE001
            print("ERR", lab, repr(e), file=sys.stderr)
    if out: json.dump(recs, open(out, "w"))
    if not quiet and recs: table(recs, by)
    return recs


if __name__ == "__main__":
    main(sys.argv[1:])
