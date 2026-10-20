"""Copy of analyze_unranked.py that filters on label prefix v3b (env BOT=v3b|v2); writes data/unranked/games_<BOT>.tsv and note_<BOT>.md."""
"""Unranked baseline of our live v2 against 8 real teams (data/unranked/matches.jsonl, 160 games, 17 maps).
Round-level decodes in data/unranked/decoded/s13. Writes notes/unranked-v2-baseline.md and data/unranked/games.tsv.
Position classes and income measures are the same as tools/v3/analyze_econ.py (dead, spawn, altar, gem camps, gold camps, transit; altar tiles learned
from tributes in data/v3/decoded/games_s13.jsonl; camp tiles from data/v3/maps_camps_platform.jsonl).
Run: py -I tools/v3/analyze_unranked.py [label]   (label is added to the note title, default 'v2')
"""
import json, os, sys, collections, statistics, glob
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, "C:/cambc-solo/tools/v3")
from killers import deaths_nearby, TN

V3 = "C:/cambc-solo/data/v3"
U = "C:/cambc-solo/data/unranked"
cheb = lambda a, b, c, d: max(abs(a - c), abs(b - d))
CATS = ["dead", "spawn", "altar", "gem camps", "gold camps", "transit"]
names = json.load(open(f"{V3}/lists/camps_probe_names.json"))
CAMPS = {}
for l in open(f"{V3}/maps_camps_platform.jsonl", encoding="utf8"):
    d = json.loads(l); CAMPS[names[d["file"][:-4]]] = d["camps"]
ALT = collections.defaultdict(collections.Counter)
for l in open(f"{V3}/decoded/games_s13.jsonl", encoding="utf8"):
    g = json.loads(l)
    for e in g["ev"]["tribute"]:
        if e[5] >= 0: ALT[g["mapId"]][(e[5], e[6])] += 1
ALTT = {m: [t for t, c in cnt.items() if c >= 5] for m, cnt in ALT.items()}
_memo = {}


def classify(x, y, mp, mid):
    k = (mid, x, y)
    if k not in _memo:
        altar = ALTT.get(mid, []); camps = CAMPS.get(mp, [])
        if altar and min(cheb(x, y, a, b) for a, b in altar) <= 2: _memo[k] = "altar"
        else:
            dg = min((cheb(x, y, c[1], c[2]) for c in camps if c[0] != 3), default=99)
            dd = min((cheb(x, y, c[1], c[2]) for c in camps if c[0] == 3), default=99)
            _memo[k] = ("gem camps" if dg <= dd else "gold camps") if min(dg, dd) <= 4 else "transit"
    return _memo[k]


def analyse(d, mp, mid):
    snaps = d["rounds"]; c0 = snaps[0]["c"]; n = len(c0); team = [r[0] for r in c0]
    sp = {t: (sum(r[3] for r in c0 if r[0] == t) / sum(1 for r in c0 if r[0] == t), sum(r[4] for r in c0 if r[0] == t) / sum(1 for r in c0 if r[0] == t)) for t in (0, 1)}
    cnt = [collections.Counter(), collections.Counter()]; col = [0, 0]; deaths = [0, 0]; lost = [0, 0]
    for k, s in enumerate(snaps):
        for i in range(n):
            c = s["c"][i]; t = team[i]
            if not c[2]: cnt[t]["dead"] += 1
            elif cheb(c[3], c[4], sp[t][0], sp[t][1]) <= 3: cnt[t]["spawn"] += 1
            else: cnt[t][classify(c[3], c[4], mp, mid)] += 1
        if k:
            a = snaps[k - 1]
            for i in range(n):
                if s["c"][i][7] > a["c"][i][7]: col[team[i]] += s["c"][i][7] - a["c"][i][7]
                if a["c"][i][2] and not s["c"][i][2]: deaths[team[i]] += 1; lost[team[i]] += a["c"][i][7]
    tr = [[], []]
    for k in range(1, len(snaps)):
        for t in (0, 1):
            dg = snaps[k]["sg"][t] - snaps[k - 1]["sg"][t]
            if dg > 0: tr[t].append((snaps[k]["r"], dg))
    return {"rounds": snaps[-1]["r"], "sg": snaps[-1]["sg"], "cnt": [dict(c) for c in cnt], "col": col, "deaths": deaths, "lost": lost,
            "trib": [len(x) for x in tr], "first": [x[0][0] if x else None for x in tr], "bag": [round(sum(b for _, b in x) / len(x), 1) if x else None for x in tr], "n": n}


rows = []
for l in open(f"{U}/matches.jsonl", encoding="utf8"):
    m = json.loads(l); f = f"{U}/decoded/s13/{m['match']}.json"
    if m["label"].startswith("v3b") != (os.environ.get("BOT", "v3b") == "v3b"): continue
    if not os.path.exists(f): continue
    d = json.load(open(f, encoding="utf8"))
    a = analyse(d, m["map"], m["mapId"]); us = 0 if m["our_side"] == "a" else 1; th = 1 - us
    tot = lambda t: sum(a["cnt"][t].values()) or 1
    kills = [e for e in deaths_nearby(d) if e["vt"] == us]
    kc = collections.Counter(TN[e["ktype"]] for e in kills if e["ktype"] is not None)
    R = max(1, a["rounds"])
    rows.append({**m, "us": us, "gems_us": a["sg"][us], "gems_th": a["sg"][th], "rounds": a["rounds"], "win": int(a["sg"][us] > a["sg"][th]), "platform_result": m["result"],
                 "deaths_us": a["deaths"][us], "deaths_th": a["deaths"][th], "rate_us": a["sg"][us] / R, "rate_th": a["sg"][th] / R, "col_us": a["col"][us] / R, "col_th": a["col"][th] / R,
                 "trib_us": a["trib"][us], "trib_th": a["trib"][th], "first_us": a["first"][us], "first_th": a["first"][th], "bag_us": a["bag"][us], "bag_th": a["bag"][th],
                 "share_us": {c: a["cnt"][us].get(c, 0) / tot(us) for c in CATS}, "share_th": {c: a["cnt"][th].get(c, 0) / tot(th) for c in CATS},
                 "killers_of_us": dict(kc), "unattributed_us": sum(1 for e in kills if e["ktype"] is None)})
seen_k = set(); uniq = []; dropped = 0
for r in rows:
    k = (r["opponent"], r["map"], r["our_side"])
    if k in seen_k:
        dropped += 1; continue
    seen_k.add(k); uniq.append(r)
queued = len(rows); rows = uniq
sides = collections.Counter(r["our_side"] for r in rows)
assert all((r["win"] == (r["platform_result"] == "win")) or r["gems_us"] == r["gems_th"] for r in rows), "result mismatch"
with open(f"{U}/games_{os.environ.get("BOT","v3b")}.tsv", "w", encoding="utf8", newline="\n") as fh:
    cols = ["label", "opponent", "map", "mapId", "match", "platform_result", "gems_us", "gems_th", "rounds", "deaths_us", "deaths_th", "rate_us", "rate_th", "trib_us", "trib_th", "first_us", "first_th"]
    fh.write("\t".join(cols) + "\n")
    for r in rows: fh.write("\t".join(str(round(r[c], 3)) if isinstance(r[c], float) else str(r[c]) for c in cols) + "\n")

mean = lambda v: statistics.mean(v) if v else None
med = lambda v: statistics.median(v) if v else None
f1 = lambda x, d=1: "-" if x is None else f"{x:.{d}f}"
fl = lambda n: "†" if n < 30 else ""
shares = lambda rs, k: " / ".join(f"{100 * mean([r[k][c] for r in rs]):.0f}" for c in CATS)


def line(name, rs):
    w = sum(r["win"] for r in rs); n = len(rs)
    return (f"| {name} | {n}{fl(n)} | {w}-{n - w} | {f1(mean([r['gems_us'] for r in rs]), 0)} / {f1(mean([r['gems_th'] for r in rs]), 0)} | {f1(med([r['rounds'] for r in rs]), 0)} | "
            f"{f1(mean([r['rate_us'] for r in rs]), 2)} / {f1(mean([r['rate_th'] for r in rs]), 2)} | {f1(mean([r['col_us'] for r in rs]), 2)} / {f1(mean([r['col_th'] for r in rs]), 2)} | "
            f"{f1(mean([r['trib_us'] for r in rs]))} / {f1(mean([r['trib_th'] for r in rs]))} | {f1(med([r['first_us'] for r in rs if r['first_us']]), 0)} / {f1(med([r['first_th'] for r in rs if r['first_th']]), 0)} | "
            f"{f1(mean([r['deaths_us'] for r in rs]))} / {f1(mean([r['deaths_th'] for r in rs]))} |")


HEAD = "| group | games | our wins-losses | gems us / them | median rounds | tributed per round us / them | collected per round us / them | tributes us / them | first tribute us / them | deaths us / them |\n|---|---|---|---|---|---|---|---|---|---|"
label = sys.argv[1] if len(sys.argv) > 1 else "v3b"
maps = sorted({r["map"] for r in rows}); opps = [o for o in dict.fromkeys(r["opponent"] for r in rows)]
mid8 = {r["map"]: r["mapId"][:8] for r in rows}
L = []
add = L.append
add(f"# Unranked baseline of our live {label} against 8 real teams ({len(rows)} distinct games of {queued} queued, 17 maps; written {__import__('time').strftime('%d %b %H:%M')} BST)")
add("")
sides_txt = "/".join(f"{k} ({v} games)" for k, v in sides.items())
add(f"Source: `data/unranked/groups.csv` (32 groups of 5, queued by the lead 19 Oct 18:12-18:16 BST; games finished 17:12Z), `cambc match-groups download`, round-level decode (`tools/v3/analyze_unranked.py`; final gems equal the platform result in every game). Maps by map id (first 8 hex). Opponent ratings are the 18:00 ladder values in `data/unranked/panel.csv`. Each group is 5 games on 5 different maps. stampede, contest and monolith were queued twice per opponent with us on the same side, and the engine is deterministic: the repeated games gave identical results, so the {dropped} repeats are removed (the matrix in section 3 shows one game per cell). That leaves 8 games per map (one per opponent) and 17 per opponent, all with us on side {sides_txt}; every cell is below 30 games (†). This is a baseline, not a rating-adjusted comparison: the opponents differ in strength, so compare v3a only on the same opponent and map.")
add("")
add("## 1. Overall and per opponent")
add("Cells are us / them. Deaths are champions per game.")
add(HEAD)
add(line("all", rows))
pr = {}
for l in open(f"{U}/panel.csv", encoding="utf8"):
    p = l.strip().split(",")
    if p[0] != "key": pr[p[0]] = (p[2], p[4])
for o in opps:
    nm, rt = pr.get(o, (o, "?"))
    add(line(f"{nm} ({rt})", [r for r in rows if r["opponent"] == o]))
add("")
add("## 2. Per map")
add(HEAD)
for m in maps:
    add(line(f"{m} ({mid8[m]})", [r for r in rows if r["map"] == m]))
add("")
add("## 3. Result matrix: our result, gems us-them, rounds (one cell per game; repeated games removed)")
add("| map | " + " | ".join(pr.get(o, (o,))[0] for o in opps) + " |")
add("|---|" + "---|" * len(opps))
for m in maps:
    cells = []
    for o in opps:
        gs = [r for r in rows if r["map"] == m and r["opponent"] == o]
        cells.append("; ".join(f"{'W' if r['win'] else 'L'} {r['gems_us']}-{r['gems_th']} r{r['rounds']}" for r in gs) or "-")
    add(f"| {m} | " + " | ".join(cells) + " |")
add("")
add("## 4. Where champion-rounds go: dead / spawn / altar / gem camps / gold camps / transit, % of champion-rounds (us | them)")
add("| group | games | us | them |")
add("|---|---|---|---|")
add(f"| all | {len(rows)} | {shares(rows, 'share_us')} | {shares(rows, 'share_th')} |")
for o in opps:
    rs = [r for r in rows if r["opponent"] == o]
    add(f"| {pr.get(o, (o,))[0]} | {len(rs)}† | {shares(rs, 'share_us')} | {shares(rs, 'share_th')} |")
for m in maps:
    rs = [r for r in rows if r["map"] == m]
    add(f"| {m} | {len(rs)}† | {shares(rs, 'share_us')} | {shares(rs, 'share_th')} |")
add("")
kc = collections.Counter(); un = 0
for r in rows:
    kc.update(r["killers_of_us"]); un += r["unattributed_us"]
tot = sum(r["deaths_us"] for r in rows)
add("## 5. Who kills us (nearest enemy champion whose kill counter rose; checked at 86% coverage and 4 wrong in 3,658 deaths on ranked games)")
add(f"{tot} of our champion deaths in {len(rows)} games; {sum(kc.values())} attributed, {un} with no enemy kill nearby. Killer champion: " + ", ".join(f"{k} {v}" for k, v in kc.most_common()) + ".")
bo = collections.defaultdict(collections.Counter)
for r in rows: bo[r["opponent"]].update(r["killers_of_us"])
for o in opps:
    top = bo[o].most_common(3)
    add(f"- {pr.get(o, (o,))[0]}: " + ", ".join(f"{k} {v}" for k, v in top))
add("")
add("## 6. Drafts (pick order)")
dr = collections.Counter(tuple(r["our_draft"]) for r in rows)
add("Ours (champion ids 0 Ryth, 1 Vesper, 2 Vendetta, 3 Cassi and Issac, 4 Stellar, 5 Prism, 6 Marcella, 7 Willow, 8 Ren, 9 Varg, 10 Tavi): " + "; ".join(f"{'.'.join(map(str, k))} x{v}" for k, v in dr.most_common(5)))
add("Per-game rows: `data/unranked/games.tsv`.")
open("C:/cambc-solo/data/unranked/note_" + os.environ.get("BOT", "v3b") + ".md", "w", encoding="utf8").write("\n".join(L) + "\n")
print(len(L), "lines;", len(rows), "games")
