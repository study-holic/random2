"""Meta answers for 2026-10-18 from platform JSON only (ladder, match-group listings and details on disk).

Maps are identified by map id; only ranked games on the current 17 ids, finished after each id's first ranked
appearance, are used. Samples: (a) current top 50, (b) teams rated 1450-1600 (data/samples.json).
Writes intel/meta-2026-10-18.md and intel/matches-2026-10-18.csv.
Usage: python -I meta.py <label>
"""
import json, os, sys, glob, csv, math, statistics, collections, datetime

sys.stdout.reconfigure(encoding="utf-8", errors="replace")
R = "C:/cambc-solo"; label = sys.argv[1] if len(sys.argv) > 1 else ""
DRAFT = ['Ryth', 'Vesper', 'Vendetta', 'CassiIssac', 'Stellar', 'Prism', 'Marcella', 'Willow', 'Ren', 'Varg', 'Tavi']
L = json.load(open(f"{R}/data/ladder.json", encoding="utf8"))
T = L["teams"]; RANK = {t["teamId"]: t["rank"] for t in T}
S = json.load(open(f"{R}/data/samples.json", encoding="utf8")); A, B = set(S["A"]), set(S["B"])
HIST = json.load(open(f"{R}/data/mapids_hist.json", encoding="utf8"))
now = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
med = lambda v: statistics.median(v) if v else None
f1 = lambda x, d=1: "n/a" if x is None else f"{x:.{d}f}"

# ---- listings (ranked groups per team, rating changes)
groups = {}
span = {}
for f in glob.glob(f"{R}/data/listings/*.jsonl"):
    tid = os.path.basename(f)[:-6]; ts = []
    for line in open(f, encoding="utf8"):
        g = json.loads(line); groups[g["id"]] = g; ts.append(g["createdAt"])
    if ts: span[tid] = (min(ts), max(ts))

# ---- details
games = []
first_ranked = {k: v["firstRanked"] for k, v in HIST.items() if v.get("firstRanked")}
for f in glob.glob(f"{R}/data/groups/*.json"):
    G = json.load(open(f, encoding="utf8")); g = G["group"]
    if g["kind"] != "ranked": continue
    gs = {s["teamId"]: s for s in g["sides"]}
    for m in G["matches"]:
        if m.get("status") != "finished" or not m.get("mapId"): continue
        t = m.get("finishedAt") or m["createdAt"]
        if m["mapId"] not in first_ranked or t < first_ranked[m["mapId"]]: first_ranked[m["mapId"]] = t
        sd = sorted(m["sides"], key=lambda s: s["side"])
        games.append({"match": m["id"], "group": g["id"], "mapId": m["mapId"], "map": m["map"], "t": t, "rounds": m["rounds"],
                      "patch": (m.get("patchId") or "")[:5], "stats": m.get("statistics"),
                      "sides": [{"teamId": s["teamId"], "team": s["teamName"], "result": s["result"], "score": s.get("score"),
                                 "draft": sorted(DRAFT[int(x)] for x in (s.get("draft") or [])),
                                 "rating": (gs.get(s["teamId"]) or {}).get("ratingBefore")} for s in sd]})
# current ids: ranked ids seen in the newest 6 hours of detailed games
if games:
    tmax = max(g["t"] for g in games)
    cut6 = (datetime.datetime.fromisoformat(tmax.replace("Z", "+00:00")) - datetime.timedelta(hours=6)).strftime("%Y-%m-%dT%H:%M:%S")
    cur = collections.Counter(g["mapId"] for g in games if g["t"] >= cut6)
else:
    cur = collections.Counter()
CUR = sorted(cur, key=lambda k: games and next(g["map"] for g in games if g["mapId"] == k))
G = [g for g in games if g["mapId"] in cur and g["t"] >= first_ranked[g["mapId"]]]
NAME = {g["mapId"]: g["map"] for g in G}
insample = lambda s, k: s["teamId"] in (A if k == "a" else B)

out = [f"# Meta 2026-10-18 ({label}, written {now})", "",
       "Source: platform JSON only (`/api/v1/ladder`, `/api/v1/match-groups?team=` to find group ids, `/api/v1/match-groups/{id}`). "
       "No replays downloaded. Maps are keyed by map id; only ranked games on the current ids, finished after each id's first ranked "
       f"appearance. Samples: (a) current top 50 ({len(A)} teams), (b) teams rated 1,450-1,600 ({len(B)} teams) on the ladder snapshot of {L['retrievedAt']}.",
       f"Coverage: {len(groups)} listed groups, {len({g['group'] for g in G})} ranked groups with details, {len(G)} ranked games on {len(cur)} current map ids"
       + (f", finished {min(g['t'] for g in G)[:16]}Z to {max(g['t'] for g in G)[:16]}Z." if G else ".")
       + f" Detailed ranked groups by sample: (a) {len({g['group'] for g in G if any(insample(s, 'a') for s in g['sides'])})}, (b) {len({g['group'] for g in G if any(insample(s, 'b') for s in g['sides'])})}.", ""]

# ---- 1
out += ["## 1. Ladder ratings, rating change per group score, ranked cadence", ""]
out += ["| Rank | Team | Rating |", "|---|---|---|"] + [f"| {r} | {T[r-1]['name']} | {T[r-1]['rating']:.1f} |" for r in (50, 100, 200) if r <= len(T)] + [""]
rows = collections.defaultdict(list)
for g in groups.values():
    if g["kind"] != "ranked" or g["status"] != "finished": continue
    for s in g["sides"]:
        o = next(x for x in g["sides"] if x is not s)
        if s.get("ratingBefore") is None or s.get("ratingAfter") is None or o.get("ratingBefore") is None: continue
        if 1450 <= s["ratingBefore"] <= 1550:
            rows[(s["wins"], o["wins"])].append((s["ratingAfter"] - s["ratingBefore"], o["ratingBefore"] - s["ratingBefore"]))
out += ["Ranked groups where the team was rated 1,450-1,550 before the group. Gap = opponent rating minus own rating.", "",
        "| Score | Groups | Median change | Median change, gap within ±25 (n) | Median gap |", "|---|---|---|---|---|"]
for sc in sorted(rows, key=lambda k: (-(k[0] - k[1]), -k[0])):
    v = rows[sc]; close = [d for d, gp in v if abs(gp) <= 25]
    out.append(f"| {sc[0]}-{sc[1]} | {len(v)} | {med([d for d, _ in v]):+.1f} | {f1(med(close)) if close else 'n/a'} ({len(close)}) | {med([gp for _, gp in v]):+.0f} |")
res = []
for g in groups.values():
    if g["kind"] != "ranked" or g["status"] != "finished": continue
    for s in g["sides"]:
        o = next(x for x in g["sides"] if x is not s); n = s["wins"] + o["wins"]
        if None in (s.get("ratingBefore"), s.get("ratingAfter"), o.get("ratingBefore")) or not n: continue
        e = 1 / (1 + 10 ** ((o["ratingBefore"] - s["ratingBefore"]) / 400))
        res.append(abs((s["ratingAfter"] - s["ratingBefore"]) - 32 * (s["wins"] / n - e)))
if res:
    out += ["", f"Model check over all {len(res)} ranked side-groups in the listings: change = 32 × (wins/games − Elo expectation, 400-point scale) "
            f"fits with median absolute error {med(res):.3f} points ({100 * sum(r < 0.05 for r in res) / len(res):.1f}% within 0.05)."]
cad = {"a": [], "b": []}
for tid, (t0, t1) in span.items():
    h = (datetime.datetime.fromisoformat(t1.replace("Z", "+00:00")) - datetime.datetime.fromisoformat(t0.replace("Z", "+00:00"))).total_seconds() / 3600
    if h < 2: continue
    n = sum(1 for g in groups.values() if g["kind"] == "ranked" and t0 <= g["createdAt"] <= t1 and any(s["teamId"] == tid for s in g["sides"]))
    for k, SS in (("a", A), ("b", B)):
        if tid in SS: cad[k].append((n / h, h))
out += ["", "Ranked groups per hour per team (teams whose listing covers at least 2 hours; median and range):", ""]
for k, nm in (("a", "top 50"), ("b", "1,450-1,600")):
    v = cad[k]
    if v: out.append(f"- {nm}: median {med([x for x, _ in v]):.2f} per hour over {len(v)} teams (range {min(x for x, _ in v):.2f}-{max(x for x, _ in v):.2f}; median listing span {med([h for _, h in v]):.1f} h)")
out.append("")

# ---- 2
out += ["## 2. Maps by id: matches, kills per game, class", "",
        "Kills, deaths and collected gems are no longer in platform match JSON (statistics removed from about 05:30Z on 16 October). "
        "Kills per game below come from the last platform statistics for the same map id (ranked games before the removal, engine v0_0_1), "
        "so they are pre-patch and exist only for ids that were ranked before 05:30Z on 16 October. Class: race under 2 kills per game, contact over 8, else mixed.", "",
        "| Map id | Name | First ranked | Games (a) | Games (b) | Games total | Kills per game (pre-patch n) | Class |", "|---|---|---|---|---|---|---|---|"]
KILLS = json.load(open(f"{R}/data/kills_hist.json", encoding="utf8")) if os.path.exists(f"{R}/data/kills_hist.json") else {}
for mid in sorted(cur, key=lambda k: NAME.get(k, "")):
    gm = [g for g in G if g["mapId"] == mid]
    na = sum(any(insample(s, "a") for s in g["sides"]) for g in gm); nb = sum(any(insample(s, "b") for s in g["sides"]) for g in gm)
    k = KILLS.get(mid); kpg = k["kills"] / k["n"] if k and k["n"] else None
    cls = "n/a" if kpg is None else ("race" if kpg < 2 else "contact" if kpg > 8 else "mixed")
    out.append(f"| `{mid}` | {NAME.get(mid)} | {first_ranked[mid][:16]}Z | {na} | {nb} | {len(gm)} | {f1(kpg)} ({k['n'] if k else 0}) | {cls} |")
out.append("")

# ---- 3
def wilson_lo(k, n, z=1.96):
    if not n: return 0
    p = k / n; d = 1 + z * z / n; c = p + z * z / (2 * n); h = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)); return (c - h) / d

out += ["## 3. Per map id and sample: tempo and drafts", "",
        "Rounds to 1,000 = rounds of games the winner ended with 1,000+ submitted gems (score = submitted gems). "
        "Drafts are the sample team's side; top three by Wilson lower bound of win rate among drafts with n ≥ 8 (fewer than three qualify: by n). "
        "Gems per tribute: tribute counts are not in platform JSON, so this cannot be computed.", ""]
for k, nm in (("a", "(a) top 50"), ("b", "(b) 1,450-1,600")):
    out += [f"### {nm}", "", "| Map id | Name | Champions per team | Games | Winners' median rounds to 1,000 (n) | Winners' median submitted gems | Top drafts: n, win rate, distinct teams |", "|---|---|---|---|---|---|---|"]
    for mid in sorted(cur, key=lambda x: NAME.get(x, "")):
        gm = [g for g in G if g["mapId"] == mid and any(insample(s, k) for s in g["sides"])]
        if not gm: continue
        r1k, wsc = [], []
        dr = collections.defaultdict(lambda: [0, 0, set()])
        for g in gm:
            w = next((s for s in g["sides"] if s["result"] == "win"), None)
            if w and w["score"] is not None:
                wsc.append(w["score"])
                if w["score"] >= 1000: r1k.append(g["rounds"])
            for s in g["sides"]:
                if insample(s, k) and s["draft"]:
                    d = dr[" + ".join(s["draft"])]; d[0] += 1; d[1] += s["result"] == "win"; d[2].add(s["teamId"])
        q = [(x, v) for x, v in dr.items() if v[0] >= 8]
        q = sorted(q, key=lambda kv: -wilson_lo(kv[1][1], kv[1][0]))[:3] if len(q) >= 3 else sorted(dr.items(), key=lambda kv: -kv[1][0])[:3]
        tops = "<br>".join(f"{x.replace('CassiIssac', 'C&I')}: {v[0]}, {100 * v[1] / v[0]:.0f}%, {len(v[2])}" for x, v in q)
        dsz = med([len(s["draft"]) for g in gm for s in g["sides"] if s["draft"]])
        out.append(f"| `{mid}` | {NAME[mid]} | {f1(dsz, 0)} | {len(gm)} | {f1(med(r1k), 0)} ({len(r1k)}) | {f1(med(wsc), 0)} | {tops} |")
    out.append("")

# ---- 4
out += ["## 4. Per map id: champion pick share and rating-adjusted win excess", "",
        "Side-games of sample teams (a or b), each team-game counted once. Pick share = share of side-games drafting the champion. "
        "Excess = mean(actual game win − Elo expectation from the group's pre-group ratings, 400-point scale), shown where n ≥ 40; percentage points.", ""]
sg = []
for g in G:
    for i, s in enumerate(g["sides"]):
        if not (insample(s, "a") or insample(s, "b")) or not s["draft"]: continue
        o = g["sides"][1 - i]
        e = 1 / (1 + 10 ** ((o["rating"] - s["rating"]) / 400)) if s["rating"] is not None and o["rating"] is not None else None
        sg.append((g["mapId"], s["draft"], s["result"] == "win", e))
CH = [c for c in DRAFT]
out += ["| Map id | Name | Side-games | " + " | ".join(c.replace("CassiIssac", "C&I") for c in CH) + " |", "|---|---|---|" + "---|" * len(CH)]
for mid in sorted(cur, key=lambda x: NAME.get(x, "")):
    v = [x for x in sg if x[0] == mid]
    if not v: continue
    cells = []
    for c in CH:
        w = [x for x in v if c in x[1]]; share = 100 * len(w) / len(v)
        we = [x[2] - x[3] for x in w if x[3] is not None]
        cells.append(f"{share:.0f}%" + (f" ({100 * sum(we) / len(we):+.1f})" if len(we) >= 40 else ""))
    out.append(f"| `{mid}` | {NAME[mid]} | {len(v)} | " + " | ".join(cells) + " |")
out += ["", "Cells: pick share, then (excess in points) where n ≥ 40.", ""]
if any(g["stats"] for g in G): out.append("Note: some current games carry platform statistics again.")
open(f"{R}/intel/meta-2026-10-18.md.tmp", "w", encoding="utf8").write("\n".join(out) + "\n")
os.replace(f"{R}/intel/meta-2026-10-18.md.tmp", f"{R}/intel/meta-2026-10-18.md")

# ---- csv
with open(f"{R}/intel/matches-2026-10-18.csv.tmp", "w", encoding="utf8", newline="") as f:
    w = csv.writer(f)
    w.writerow(["match_id", "group_id", "map_id", "map_name", "finished_at", "sample_a", "sample_b", "team_a", "team_a_id", "team_b", "team_b_id",
                "rating_a", "rating_b", "draft_a", "draft_b", "rounds", "score_a", "score_b", "winner", "deaths_a", "deaths_b",
                "collected_a", "collected_b", "submitted_a", "submitted_b"])
    for g in sorted(G, key=lambda x: x["t"]):
        a, b = g["sides"]
        st = g["stats"]; dth = col = ["", ""]
        if st and st.get("champions"):
            dth = [sum(c["deaths"] for c in st["champions"] if c["team"] == i) for i in (0, 1)]
            col = [sum(c["collectedGems"] for c in st["champions"] if c["team"] == i) for i in (0, 1)]
        w.writerow([g["match"], g["group"], g["mapId"], g["map"], g["t"], int(any(insample(s, "a") for s in g["sides"])), int(any(insample(s, "b") for s in g["sides"])),
                    a["team"], a["teamId"], b["team"], b["teamId"], f1(a["rating"]), f1(b["rating"]), "+".join(a["draft"]), "+".join(b["draft"]), g["rounds"],
                    a["score"], b["score"], "a" if a["result"] == "win" else "b" if b["result"] == "win" else "", dth[0], dth[1], col[0], col[1], a["score"], b["score"]])
os.replace(f"{R}/intel/matches-2026-10-18.csv.tmp", f"{R}/intel/matches-2026-10-18.csv")
print(f"games={len(G)} ids={len(cur)} groups={len(groups)}")
