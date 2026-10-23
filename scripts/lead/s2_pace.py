"""s2_pace.py: tables from data/s2/index.csv (public API index; engine 1.0.3 unless --patch all).
Writes data/s2/tables/{teams,picks,picks_by_map,maps_top,xanarchy_maps}.csv and prints a summary of 60 lines or fewer.
Definitions: score = gems tributed by that side; pace = score per 100 rounds; a "team game" is one side of one match."""
import csv, collections, sys, os, statistics as st

H = "/home/claude/data/s2"
patch = "1.0.3" if "--patch" not in sys.argv else sys.argv[sys.argv.index("--patch") + 1]
os.makedirs(f"{H}/tables", exist_ok=True)
L = {r["name"]: r for r in csv.DictReader(open(f"{H}/ladder.csv"))}
top20 = [r["name"] for r in sorted(L.values(), key=lambda r: int(r["rank"]))[:20]]
rows = [r for r in csv.DictReader(open(f"{H}/index.csv")) if patch == "all" or r["patch"] == patch]

side_rows = []   # one per team game
for r in rows:
    for s, o in (("a", "b"), ("b", "a")):
        try:
            sc, oc, rd = int(r[f"score_{s}"] or 0), int(r[f"score_{o}"] or 0), int(r["rounds"] or 0)
        except ValueError:
            continue
        won = (r["result_a"] == "win") if s == "a" else (r["result_a"] == "loss")
        side_rows.append(dict(team=r[f"team_{s}"], opp=r[f"team_{o}"], map=r["map"], score=sc, opp_score=oc, rounds=rd, won=int(won),
                              end=r["end"], draft=r[f"draft_{s}"], created=r["created"], patch=r["patch"]))

def summarise(rs):
    n = len(rs)
    if not n:
        return {}
    wins = sum(x["won"] for x in rs)
    wg = [x for x in rs if x["won"] and x["end"] == "gems"]
    return dict(games=n, win=round(wins / n, 3), score=round(st.mean(x["score"] for x in rs)), opp_score=round(st.mean(x["opp_score"] for x in rs)),
                pace=round(st.mean(100 * x["score"] / max(1, x["rounds"]) for x in rs), 1),
                win_by_gems=round(len(wg) / n, 3), rounds_to_1000=round(st.median(x["rounds"] for x in wg)) if wg else "")

by_team = collections.defaultdict(list)
for x in side_rows:
    by_team[x["team"]].append(x)
teams = top20 + [t for t in ("xanarchy", "MicroStella") if t in by_team]
with open(f"{H}/tables/teams.csv", "w", newline="") as f:
    w = csv.writer(f); w.writerow(["rank", "team", "rating", "games", "win", "score", "opp_score", "pace", "win_by_gems", "median_rounds_to_1000", "top_picks"])
    print(f"engine {patch}: {len(rows)} matches, {len(side_rows)} team games")
    print(f"{'rk':>3} {'team':22s} {'rating':>6} {'games':>5} {'win':>5} {'score':>5} {'opp':>5} {'pace':>5} {'r1000':>5}")
    for t in teams:
        s = summarise(by_team[t])
        if not s:
            continue
        lr = L.get(t, {})
        w.writerow([lr.get("rank"), t, lr.get("rating"), s["games"], s["win"], s["score"], s["opp_score"], s["pace"], s["win_by_gems"], s["rounds_to_1000"], lr.get("picks_last50", "")])
        print(f"{lr.get('rank',''):>3} {t[:22]:22s} {lr.get('rating',''):>6} {s['games']:>5} {s['win']:>5} {s['score']:>5} {s['opp_score']:>5} {s['pace']:>5} {s['rounds_to_1000']:>5}")

# pick rates among top-20 team games, and win rate when picked
top_games = [x for x in side_rows if x["team"] in top20]
pc, pw = collections.Counter(), collections.Counter()
for x in top_games:
    for c in set(x["draft"].split(";")):
        if c:
            pc[c] += 1; pw[c] += x["won"]
with open(f"{H}/tables/picks.csv", "w", newline="") as f:
    w = csv.writer(f); w.writerow(["champion", "pick_rate_top20", "win_when_picked_top20"])
    for c, n in pc.most_common():
        w.writerow([c, round(n / len(top_games), 3), round(pw[c] / n, 3)])
print("top-20 pick rate (win rate when picked):", ", ".join(f"{c} {n/len(top_games):.2f} ({pw[c]/n:.2f})" for c, n in pc.most_common(12)))
cores = collections.Counter(";".join(sorted(x["draft"].split(";"))) for x in top_games)
print("top-20 most common drafts:", " | ".join(f"{k} x{v}" for k, v in cores.most_common(4)))

# per map: top-20 pace and win rate; Xanarchy per map
mt = collections.defaultdict(list)
for x in top_games:
    mt[x["map"]].append(x)
xm = collections.defaultdict(list)
for x in by_team.get("xanarchy", []):
    xm[x["map"]].append(x)
with open(f"{H}/tables/maps.csv", "w", newline="") as f:
    w = csv.writer(f); w.writerow(["map", "top20_games", "top20_win", "top20_pace", "top20_rounds_to_1000", "xan_games", "xan_win", "xan_pace", "xan_score", "xan_opp_score"])
    for m in sorted(set(mt) | set(xm)):
        a, b = summarise(mt[m]), summarise(xm[m])
        w.writerow([m, a.get("games", 0), a.get("win", ""), a.get("pace", ""), a.get("rounds_to_1000", ""), b.get("games", 0), b.get("win", ""), b.get("pace", ""), b.get("score", ""), b.get("opp_score", "")])
xs = sorted(((summarise(v)["win"], k, summarise(v)["games"], summarise(v)["pace"]) for k, v in xm.items() if len(v) >= 6))
print("xanarchy worst maps (win, map, games, pace):", "; ".join(f"{m} {wr:.2f}/{g} p{p}" for wr, m, g, p in xs[:8]))
print("xanarchy best maps:", "; ".join(f"{m} {wr:.2f}/{g}" for wr, m, g, p in xs[-5:]))
# Xanarchy by build era (v5r live from 08:03 UTC 23 Oct)
pre = [x for x in by_team.get("xanarchy", []) if x["created"] < "2026-10-23T08:03:28"]
post = [x for x in by_team.get("xanarchy", []) if x["created"] >= "2026-10-23T08:03:28"]
print("xanarchy v4 era:", summarise(pre)); print("xanarchy v5r era:", summarise(post))
xd = collections.Counter(x["draft"] for x in by_team.get("xanarchy", []))
print("xanarchy drafts:", " | ".join(f"{k} x{v}" for k, v in xd.most_common(3)))
