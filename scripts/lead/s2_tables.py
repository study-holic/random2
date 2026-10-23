"""s2_tables.py: draft, pick-rate, finish-round, and Xanarchy tables from data/s2/index.csv (s2_index.py output).
Sets: TOP20 (ladder ranks 1-20 in data/s2/ladder.csv), UK8 (UK category ranks 1-8), XAN (Xanarchy).
Engines are kept apart (patch column); nothing is merged across patches.
Writes data/s2/{pick_rates,team_drafts,finish_by_map,xanarchy_by_map}.csv and prints a summary of 60 lines or fewer.
usage: python3 s2_tables.py [--v5r-live 2026-10-23T08:03:28Z]"""
import argparse, csv, statistics as st
from collections import Counter, defaultdict

D = "/home/claude/data/s2"
ap = argparse.ArgumentParser()
ap.add_argument("--v5r-live", default="2026-10-23T08:03:28Z")
ap.add_argument("--v4-from", default="2026-10-21T05:30:00Z")
a = ap.parse_args()

lad = list(csv.DictReader(open(f"{D}/ladder.csv")))
top20 = {r["team_id"] for r in lad if int(r["rank"]) <= 20}
uk8 = {r["team_id"] for r in sorted((r for r in lad if r["category"] == "uk"), key=lambda r: int(r["rank"]))[:8]}
xan = {r["team_id"] for r in lad if r["name"].lower() == "xanarchy"}
name = {r["team_id"]: r["name"] for r in lad}
rank = {r["team_id"]: int(r["rank"]) for r in lad}
SETS = {"TOP20": top20, "UK8": uk8, "XAN": xan}

rows = [r for r in csv.DictReader(open(f"{D}/index.csv")) if r["kind"] == "ranked"]
seen, R = set(), []
for r in rows:  # a match appears once per indexed team; keep one copy
    if r["match_id"] not in seen:
        seen.add(r["match_id"]); R.append(r)

# team-games: one row per side
TG = []
for r in R:
    for s, o in (("a", "b"), ("b", "a")):
        res = r["result_a"] if s == "a" else {"win": "loss", "loss": "win"}.get(r["result_a"], r["result_a"])
        TG.append(dict(team=r[f"id_{s}"], opp=r[f"id_{o}"], patch=r["patch"], map=r["map"], win=int(res == "win"),
                       draft=[c for c in r[f"draft_{s}"].split(";") if c], score=float(r[f"score_{s}"] or 0),
                       opp_score=float(r[f"score_{o}"] or 0), rounds=int(r["rounds"] or 0), end=r["end"], created=r["created"],
                       rating=float(r[f"rating_{s}"] or 0), opp_rating=float(r[f"rating_{o}"] or 0)))

CH = sorted({c for t in TG for c in t["draft"]})

# A. pick rates per set, patch, map (map "ALL" = pooled over maps within one patch)
out = []
for sn, S in SETS.items():
    g = defaultdict(list)
    for t in TG:
        if t["team"] in S:
            g[(t["patch"], t["map"])].append(t); g[(t["patch"], "ALL")].append(t)
    for (p, m), ts in sorted(g.items()):
        n = len(ts); w = sum(t["win"] for t in ts)
        for c in CH:
            pk = [t for t in ts if c in t["draft"]]
            out.append(dict(set=sn, patch=p, map=m, champion=c, team_games=n, team_wins=w, picks=len(pk),
                            pick_rate=round(len(pk) / n, 3), wins_with=sum(t["win"] for t in pk),
                            win_rate_with=round(sum(t["win"] for t in pk) / len(pk), 3) if pk else ""))
with open(f"{D}/pick_rates.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(out[0])); w.writeheader(); w.writerows(out)

# B. drafts per team and patch (modal set of champions, order ignored)
tdr = []
for tid in sorted(top20 | uk8 | xan, key=lambda x: rank.get(x, 999)):
    for p in sorted({t["patch"] for t in TG}):
        ts = [t for t in TG if t["team"] == tid and t["patch"] == p]
        if not ts:
            continue
        five = Counter(";".join(sorted(t["draft"])) for t in ts if len(t["draft"]) == 5)
        pc = Counter(c for t in ts for c in set(t["draft"]))
        md, mn = five.most_common(1)[0] if five else ("", 0)
        tdr.append(dict(rank=rank.get(tid), team=name.get(tid), patch=p, team_games=len(ts), wins=sum(t["win"] for t in ts),
                        modal_5_draft=md, modal_share=round(mn / max(1, sum(five.values())), 3),
                        picks=";".join(f"{c}:{round(k / len(ts), 2)}" for c, k in pc.most_common())))
with open(f"{D}/team_drafts.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(tdr[0])); w.writeheader(); w.writerows(tdr)

# C. finish rounds per map: games a set member won by reaching 1,000 gems
fin = []
for sn in ("TOP20", "UK8"):
    S = SETS[sn]
    g = defaultdict(list)
    for t in TG:
        if t["team"] in S:
            g[(t["patch"], t["map"])].append(t)
    for (p, m), ts in sorted(g.items()):
        won = [t for t in ts if t["win"]]
        gem = sorted(t["rounds"] for t in won if t["end"] == "gems")
        q = st.quantiles(gem, n=4) if len(gem) >= 4 else [None, None, None]
        fin.append(dict(set=sn, patch=p, map=m, team_games=len(ts), wins=len(won), gem_wins=len(gem),
                        median_finish=st.median(gem) if gem else "", p25=round(q[0]) if q[0] else "", p75=round(q[2]) if q[2] else "",
                        mean_score=round(st.mean(t["score"] for t in ts)), mean_opp_score=round(st.mean(t["opp_score"] for t in ts))))
with open(f"{D}/finish_by_map.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(fin[0])); w.writeheader(); w.writerows(fin)

# D. Xanarchy per build and map
xr = []
for t in TG:
    if t["team"] in xan and t["created"] >= a.v4_from:
        t["build"] = "v5r" if t["created"] >= a.v5r_live else f"v4/{t['patch']}"   # engines kept apart (A2.13)
        xr.append(t)
xg = defaultdict(list)
for t in xr:
    xg[(t["build"], t["map"])].append(t)
xm = []
for (b, m), ts in sorted(xg.items()):
    xm.append(dict(build=b, map=m, games=len(ts), wins=sum(t["win"] for t in ts), mean_score=round(st.mean(t["score"] for t in ts)),
                   mean_opp_score=round(st.mean(t["opp_score"] for t in ts)), mean_opp_rating=round(st.mean(t["opp_rating"] for t in ts))))
with open(f"{D}/xanarchy_by_map.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(xm[0])); w.writeheader(); w.writerows(xm)

# summary
print(f"matches {len(R)}; patches {dict(Counter(r['patch'] for r in R))}; end reasons {dict(Counter(r['end'] for r in R))}")
for sn in ("TOP20", "UK8"):
    for p in sorted({t["patch"] for t in TG}):
        allr = [o for o in out if o["set"] == sn and o["patch"] == p and o["map"] == "ALL"]
        if not allr:
            continue
        top = sorted(allr, key=lambda o: -o["picks"])[:8]
        print(f"{sn} {p} team-games {allr[0]['team_games']} win {allr[0]['team_wins'] / allr[0]['team_games']:.2f}: "
              + ", ".join(f"{o['champion']} {o['pick_rate']:.2f}/{o['win_rate_with']}" for o in top))
for b in ("v4/1.0.2", "v4/1.0.3", "v5r"):
    ts = [t for t in xr if t["build"] == b]
    if ts:
        print(f"Xanarchy {b}: {sum(t['win'] for t in ts)}/{len(ts)} ranked, mean score {st.mean(t['score'] for t in ts):.0f} vs "
              f"{st.mean(t['opp_score'] for t in ts):.0f}, opp rating {st.mean(t['opp_rating'] for t in ts):.0f}, patches {dict(Counter(t['patch'] for t in ts))}")
