"""v5 shared metrics (S3). One record per (game, side) from full-action decodes (tools/top10/extract_pilot.mjs --stride 1).

Usage (run with python3 -I; inputs are replay-derived data):
  python3 -I tools/cloud/metrics.py panel <decoded_dir> [--labels v4top-] [--out f.jsonl]     real panel: <label>--<pos>.json, side and map from data/unranked/matches.jsonl
  python3 -I tools/cloud/metrics.py mapgate <mapgate.json> [--out f.jsonl] [--cache dir]      local games: decodes each replay (cached), side = the bot's side
  python3 -I tools/cloud/metrics.py files <decoded.json ...> --map <name> --side A|B           any decode
  python3 -I tools/cloud/metrics.py table <f.jsonl ...> [--by map|opp]                       aggregate table of the fields below

Fields (the v5-common.md names first):
  banked        submitted gems at rounds 200, 400, ..., 2000 (carried forward after the game ends)
  r1000         first round with submitted gems >= 1000 (None if never)
  deaths        total; death_cause {champion, monster, other}; death_loc {spawn, altar, camp, elsewhere}; death_band {0-200, 200-600, 600+}
  locks_placed  altar locks used (consumable bit 2 cleared while alive); locks_bought
  gear200       mean gear points per champion at round 200 (shards 1, emblems 5, Power 6, Bruiser 7)
  fights, fights_init, fights_init_spell, ability_open  fights this side started; share whose first damaging action was an ability cast
  first_hit_spell share of all fights this side hit in whose first own damaging action was an ability cast
  funnel        gems: acq_camp, acq_kill, tributed, lost_champion, destroyed_monster (monster/other deaths), carried_end, residual (balance check)
  time          champion-round shares: dead, tribute, recall, fight, clear, capture, walk, spawn_idle, idle
  tributes      tribute completions and mean bag
Death cause (notes/step0-deaths.md): champion = an enemy champion action damaged the victim in the death round or the one before;
monster = otherwise, with a wolf, elk or monolith camp within 8 tiles; other = neither. Cassi and Issac partner deaths take the first's cause.
Action kinds (engine 1.0.0, inferred 20 Oct from state changes): 4 move, 5 attack, 6 cast, 11 recall start, 12 tribute start, 13 shop.
"""
import json, os, sys, glob, collections, subprocess, math

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
MAPS = json.load(open(os.path.join(ROOT, "data/maps21.json")))
TN = {0: "Ryth", 1: "Vesper", 2: "Vendetta", 3: "C&I", 4: "C&I", 5: "Stellar", 6: "Prism", 7: "Marcella", 8: "Willow", 9: "Ren", 10: "Varg", 11: "Tavi", 12: "HoR"}
cheb = lambda a, b: max(abs(a[0] - b[0]), abs(a[1] - b[1]))
BANK_R = list(range(200, 2001, 200))


def mapkey(n):
    n = os.path.basename(n).replace(".map26a", "")
    return n[2:] if n[:2].isdigit() else n


def gear_points(items):
    pts = 0
    for idx, cnt in items.items():
        fam, slot = idx // 6, idx % 6
        if slot % 2 == 0: pts += cnt
        elif fam == 1: pts += 6 * cnt
        elif fam == 6: pts += 7 * cnt
        else: pts += 5 * cnt
    return pts


def parse_key(k):
    return {int(a): int(b) for a, b in (t.split(":") for t in k.split(","))} if k else {}


def analyse(d, mp, side):
    """d: decoded game; mp: map name; side: 0 (team A) or 1 (team B). Returns the record for that side."""
    m = MAPS[mapkey(mp)]
    snaps, last = [], -1
    for s in d["rounds"]:
        if s["r"] != last or s.get("final"): snaps.append(s); last = s["r"]
    c0 = snaps[0]["c"]; n = len(c0)
    team = [c[0] for c in c0]; typ = [c[1] for c in c0]
    start = {t: [(c[3], c[4]) for c in c0 if c[0] == t] for t in (0, 1)}
    mons = [(x, y) for t, x, y in m["camps"] if t in (0, 1, 4)]
    camp_tiles = {(x, y) for t, x, y in m["camps"]}
    altars = [tuple(a) for a in m["altars"]]
    acts = d["actions"]
    by_r = collections.defaultdict(list)
    for a in acts: by_r[a[0]].append(a)
    me = [i for i in range(n) if team[i] == side]
    fin = snaps[-1]; R = fin["r"]
    rec = dict(map=mapkey(mp), side="AB"[side], rounds=R, sg=fin["sg"][side], sg_opp=fin["sg"][1 - side],
               win=int(fin["sg"][side] > fin["sg"][1 - side]) if fin["sg"][side] != fin["sg"][1 - side] else None,
               draft=d.get("drafts", [[], []])[side], n_champ=len(me))
    # banked by round, r1000
    sgr = {s["r"]: s["sg"][side] for s in snaps}
    keys = sorted(sgr)
    def sg_at(r):
        k = [x for x in keys if x <= r]
        return sgr[k[-1]] if k else 0
    rec["banked"] = {r: sg_at(r) for r in BANK_R}
    rec["r1000"] = next((s["r"] for s in snaps if s["sg"][side] >= 1000), None)
    # deaths by cause
    deaths = []
    for a, b in zip(snaps, snaps[1:]):
        evs = []
        for i in me:
            if not (a["c"][i][2] and not b["c"][i][2]): continue
            x, y = a["c"][i][3], a["c"][i][4]; k = None
            for r in (a["r"], b["r"]):
                for act in by_r.get(r, []):
                    if team[act[1]] != side and any(h[0] == "c" and h[1] == i and h[2] > 0 for h in act[9]): k = act[1]
            if min(cheb((x, y), p) for p in start[side]) <= 8: loc = "spawn"
            elif any(cheb((x, y), p) <= 8 for p in altars): loc = "altar"
            elif any(cheb((x, y), p) <= 8 for p in mons): loc = "camp"
            else: loc = "elsewhere"
            r = b["r"]
            evs.append(dict(i=i, r=r, k=k, loc=loc, nearmon=any(cheb((x, y), p) <= 8 for p in mons), xy=(x, y),
                            band="0-200" if r < 200 else "200-600" if r < 600 else "600+", bag=a["c"][i][7], ty=typ[i]))
        for e in evs:
            if e["ty"] in (3, 4):
                mates = [o for o in evs if o is not e and o["ty"] in (3, 4)]
                if mates and e["k"] is None and (mates[0]["k"] is not None or mates[0]["i"] < e["i"]):
                    e["partner"] = 1; e["k"] = mates[0]["k"]; e["nearmon"] = mates[0]["nearmon"]
        for e in evs:
            e["cause"] = "champion" if e["k"] is not None else ("monster" if e["nearmon"] else "other")
            deaths.append(e)
    rec["deaths"] = len(deaths)
    rec["death_cause"] = dict(collections.Counter(e["cause"] for e in deaths))
    rec["death_loc"] = dict(collections.Counter(e["loc"] for e in deaths))
    rec["death_band"] = dict(collections.Counter(e["band"] for e in deaths))
    rec["death_list"] = [[e["r"], TN[e["ty"]], e["cause"], e["loc"], e["xy"][0], e["xy"][1], e["bag"]] for e in deaths]
    # locks, gear
    rec["locks_placed"] = rec["locks_bought"] = 0
    cur = [{} for _ in range(n)]; gear200 = None
    for a, b in zip(snaps, snaps[1:]):
        for i in me:
            on, off = b["c"][i][10] & ~a["c"][i][10], a["c"][i][10] & ~b["c"][i][10]
            if on >> 2 & 1: rec["locks_bought"] += 1
            if off >> 2 & 1 and b["c"][i][2]: rec["locks_placed"] += 1
    for s in snaps:
        for i, k in s.get("it", []): cur[i] = parse_key(k)
        if s["r"] <= 200: gear200 = sum(gear_points(cur[i]) for i in me) / len(me)
    rec["gear200"] = round(gear200, 1) if gear200 is not None and R >= 200 else None
    # funnel
    f = dict(acq_camp=0, acq_kill=0, tributed=fin["sg"][side], lost_champion=0, destroyed_monster=0, carried_end=0)
    death_at = {(e["i"], e["r"]) for e in deaths}
    for a, b in zip(snaps, snaps[1:]):
        trib = b["sg"][side] - a["sg"][side]
        for i in me:
            ga, gb = a["c"][i][7], b["c"][i][7]
            if not (a["c"][i][2] and b["c"][i][2]): continue
            if gb > ga:
                if b["c"][i][9] > a["c"][i][9]: f["acq_kill"] += gb - ga
                else: f["acq_camp"] += gb - ga
            elif gb < ga and trib > 0 and gb > 0:   # tributed and gained in the same interval
                pass
    for e in deaths:
        if e["cause"] == "champion": f["lost_champion"] += e["bag"]
        else: f["destroyed_monster"] += e["bag"]
    f["carried_end"] = sum(fin["c"][i][7] for i in me if fin["c"][i][2])
    f["residual"] = f["acq_camp"] + f["acq_kill"] - f["tributed"] - f["lost_champion"] - f["destroyed_monster"] - f["carried_end"]
    rec["funnel"] = f
    # tributes
    ut = {}
    for a, b in zip(snaps, snaps[1:]):
        dg = b["sg"][side] - a["sg"][side]
        if dg > 0: ut[b["r"]] = dg
    rec["tributes"] = len(ut); rec["mean_bag"] = round(sum(ut.values()) / len(ut), 1) if ut else None
    # time use and fights
    chan = {}   # (i, r) -> 'tribute' | 'recall'
    for a in acts:
        r, i, kind = a[0], a[1], a[2]
        if team[i] != side: continue
        if kind == 12:
            for rr in range(r, r + max(1, math.ceil(a[7] / 10)) + 1): chan.setdefault((i, rr), "tribute")
        elif kind == 11:
            for rr in range(r, r + 6): chan.setdefault((i, rr), "recall")
    fight_r, clear_r, move_r = set(), set(), set()
    for a in acts:
        r, i, kind, hits = a[0], a[1], a[2], a[9]
        for h in hits:
            if h[0] == "c" and team[h[1]] != team[i] and h[2] > 0:
                if team[i] == side: fight_r.add((i, r))
                else: fight_r.add((h[1], r))
            if h[0] == "m" and team[i] == side: clear_r.add((i, r))
        if kind == 4 and team[i] == side: move_r.add((i, r))
    tu = collections.Counter()
    for s in snaps[:-1]:
        r = s["r"]
        for i in me:
            c = s["c"][i]; p = (c[3], c[4])
            if not c[2]: k = "dead"
            elif (i, r) in chan:
                k = chan[(i, r)]
                if k == "tribute" and not any(cheb(p, al) <= 1 for al in altars): k = "walk" if (i, r) in move_r else "idle"
            elif (i, r) in fight_r: k = "fight"
            elif (i, r) in clear_r: k = "clear"
            elif p in camp_tiles: k = "capture"
            elif (i, r) in move_r: k = "walk"
            elif min(cheb(p, q) for q in start[side]) <= 2: k = "spawn_idle"
            else: k = "idle"
            tu[k] += 1
    tot = sum(tu.values()) or 1
    rec["time"] = {k: round(v / tot, 3) for k, v in sorted(tu.items())}
    # fights: champion-vs-champion damage events clustered (gap <= 5 rounds, within 8 tiles of the fight centre)
    pos = {}
    for s in snaps: pos[s["r"]] = [(c[3], c[4]) for c in s["c"]]
    evs = []
    for a in acts:
        r, i, kind, hits = a[0], a[1], a[2], a[9]
        tgt = [h[1] for h in hits if h[0] == "c" and team[h[1]] != team[i] and h[2] > 0]
        if tgt and kind in (5, 6): evs.append((r, i, kind, tgt[0]))
    fights = []
    for r, i, kind, v in evs:
        pr = pos.get(r) or pos[max(k for k in pos if k <= r)]
        c = pr[v]
        fi = next((F for F in reversed(fights) if r - F["last"] <= 5 and cheb(F["c"], c) <= 8), None)
        if fi is None:
            fi = dict(first=r, last=r, c=c, init=team[i], init_kind=kind, opener={}); fights.append(fi)
        fi["last"] = r
        fi["opener"].setdefault(team[i], kind)
    ft = [F for F in fights if F["init"] == side]
    rec["fights"] = len(fights); rec["fights_init"] = len(ft); rec["fights_init_spell"] = sum(F["init_kind"] == 6 for F in ft)
    rec["ability_open"] = round(rec["fights_init_spell"] / len(ft), 3) if ft else None
    hit = [F for F in fights if side in F["opener"]]
    rec["first_hit_spell"] = round(sum(F["opener"][side] == 6 for F in hit) / len(hit), 3) if hit else None
    return rec


def decode(replay, cache):
    os.makedirs(cache, exist_ok=True)
    out = os.path.join(cache, os.path.basename(replay).replace(".repl26a", "").replace(".zip", "") + ".json")
    if not os.path.exists(out):
        env = dict(os.environ, CAMBC_ENGINE_CACHE=os.environ.get("CAMBC_ENGINE_CACHE", os.path.join(ROOT, "data/top10/engine")))
        subprocess.run(["node", os.path.join(ROOT, "tools/top10/extract_pilot.mjs"), replay, "--out", cache, "--stride", "1"],
                       env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return out


def table(rows, by=None):
    groups = collections.defaultdict(list)
    for r in rows: groups[r.get(by, "all") if by else "all"].append(r)
    hdr = "| group | n | W | gems us/them | r1000 | deaths | champ/mon/other | locks | gear200 | ability_open | acq_camp | acq_kill | tributed | lost_champ | destroyed_mon | carried_end |"
    print(hdr); print("|" + "---|" * hdr.count("|")[:-1] if False else "|" + "---|" * (hdr.count("|") - 1))
    mean = lambda xs: sum(xs) / len(xs) if xs else float("nan")
    for g in sorted(groups):
        x = groups[g]; N = len(x)
        dc = lambda k: mean([r["death_cause"].get(k, 0) for r in x])
        ao = [r["ability_open"] for r in x if r["ability_open"] is not None]
        r1 = [r["r1000"] for r in x if r["r1000"]]
        print(f"| {g} | {N} | {sum(r['win'] or 0 for r in x)} | {mean([r['sg'] for r in x]):.0f}/{mean([r['sg_opp'] for r in x]):.0f} | "
              f"{len(r1)}@{mean(r1):.0f} | {mean([r['deaths'] for r in x]):.1f} | {dc('champion'):.1f}/{dc('monster'):.1f}/{dc('other'):.1f} | "
              f"{mean([r['locks_placed'] for r in x]):.2f} | {mean([r['gear200'] for r in x if r['gear200'] is not None]):.1f} | {mean(ao):.2f} | "
              + " | ".join(f"{mean([r['funnel'][k] for r in x]):.0f}" for k in ("acq_camp", "acq_kill", "tributed", "lost_champion", "destroyed_monster", "carried_end")) + " |")
    allt = collections.Counter()
    for r in rows:
        for k, v in r["time"].items(): allt[k] += v / len(rows)
    print("time use (share of champion-rounds):", {k: round(v, 3) for k, v in allt.most_common()})


def main():
    a = sys.argv[1:]
    if not a: print(__doc__); return
    opt = lambda k, dv=None: a[a.index(k) + 1] if k in a else dv
    out = opt("--out"); rows = []
    if a[0] == "panel":
        idx = {}
        for l in open(os.path.join(ROOT, "data/unranked/matches.jsonl"), encoding="utf8"):
            g = json.loads(l); idx[(g["label"], g["position"])] = g
        lab = opt("--labels", "")
        for fn in sorted(glob.glob(os.path.join(a[1], "*.json"))):
            b = os.path.basename(fn)[:-5]
            if "--" not in b or not b.startswith(lab): continue
            label, pos = b.split("--"); g = idx.get((label, int(pos)))
            if not g: continue
            rec = analyse(json.load(open(fn)), g["map"], 0 if g["our_side"] == "a" else 1)
            rec.update(game=b, opp=g["opponent"], match=g["match"]); rows.append(rec)
    elif a[0] == "mapgate":
        mg = json.load(open(a[1])); cache = opt("--cache", os.path.join(ROOT, "matches", "decoded"))
        for r in mg["results"]:
            if "error" in r or not os.path.exists(r["replay"]): continue
            fn = decode(r["replay"], cache)
            if not os.path.exists(fn): continue
            rec = analyse(json.load(open(fn)), r["map"], 0 if r["side"] == "A" else 1)
            rec.update(game=os.path.basename(r["replay"]), opp=r["opp"], salt=r.get("salt")); rows.append(rec)
    elif a[0] == "files":
        for fn in [x for x in a[1:] if x.endswith(".json")]:
            rec = analyse(json.load(open(fn)), opt("--map"), 0 if opt("--side", "A") == "A" else 1); rec["game"] = os.path.basename(fn); rows.append(rec)
    elif a[0] == "table":
        for fn in [x for x in a[1:] if x.endswith(".jsonl")]:
            rows += [json.loads(l) for l in open(fn)]
        table(rows, opt("--by")); return
    if out:
        with open(out, "w") as fo:
            for r in rows: fo.write(json.dumps(r) + "\n")
    table(rows, opt("--by"))


if __name__ == "__main__":
    main()
