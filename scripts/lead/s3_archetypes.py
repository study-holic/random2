"""s3_archetypes.py: map archetypes from S1 features, with measured pace and records per archetype (S3).
Features (data/MAPS.csv, data/S3-PRODUCTION.csv): log area, team size, total gems, share of gems in Monoliths and golems,
share of gems in gem clusters, spawn-to-altar distance over map diagonal, narrowest cut width, and the model's rounds to
1,000 (draft "top", scope "half"). k-means (k = --k, 50 restarts, seed 0) on standardised features.
Joins: top-20 finish rounds and pick rates (data/s2/finish_by_map.csv, pick_rates.csv), Xanarchy ranked per build
(data/s2/xanarchy_by_map.csv), and v5r local games against the SDK starter (runs/f1-v5r-e103/metrics.csv from
s2_metrics.py local). Writes data/S3-ARCHETYPES.csv (one row per map) and prints a per-archetype table.
usage: python3 s3_archetypes.py [--k 6] [--patch 1.0.3]"""
import argparse, csv, math, os, statistics as st
from collections import defaultdict
import numpy as np

H = "/home/claude"
ap = argparse.ArgumentParser()
ap.add_argument("--k", type=int, default=6)
ap.add_argument("--patch", default="1.0.3")
ap.add_argument("--local", default=f"{H}/runs/f1-v5r-e103/stats/metrics.csv")
a = ap.parse_args()

M = {r["map"]: r for r in csv.DictReader(open(f"{H}/data/MAPS.csv"))}
P = {(r["map"], r["draft"], r["scope"]): r for r in csv.DictReader(open(f"{H}/data/S3-PRODUCTION.csv"))}
fin = {(r["set"], r["patch"], r["map"]): r for r in csv.DictReader(open(f"{H}/data/s2/finish_by_map.csv"))}
xan = defaultdict(dict)
for r in csv.DictReader(open(f"{H}/data/s2/xanarchy_by_map.csv")):
    xan[r["map"]][r["build"]] = r
loc = defaultdict(list)
if os.path.exists(a.local):
    for r in csv.DictReader(open(a.local)):   # baseline.py names stats <map>-<side>; bot A (v5r) plays team <side>
        m, side = r["match_id"].rsplit("-", 1)
        if r["team"] == side:
            r["map"] = m; loc[m].append(r)

maps = sorted(M)
feat_names = ["log_area", "team_size", "gems_total", "big_share", "cluster_share", "altar_dist_rel", "cut_width", "model_r1000"]
X = []
for m in maps:
    r = M[m]
    area = float(r["w"]) * float(r["h"]); diag = math.hypot(float(r["w"]), float(r["h"]))
    gems = float(r["gems_total"])
    big = 200 * float(r["monolith"]) + 36 * float(r["golem"])
    pr = P.get((m, "top", "half"), {}).get("rounds_to_1000") or 4000
    X.append([math.log(area), float(r["team_size"]), gems, big / gems if gems else 0, 7 * float(r["gem_cluster"]) / gems if gems else 0,
              float(r["dist_spawn_a_to_altar"]) / diag, float(r["cut_width"]), min(4000.0, float(pr))])
X = np.array(X); Z = (X - X.mean(0)) / X.std(0)
rng = np.random.default_rng(0); best = None
for _ in range(50):
    C = Z[rng.choice(len(Z), a.k, replace=False)]
    for _ in range(100):
        lab = np.argmin(((Z[:, None, :] - C[None]) ** 2).sum(-1), 1)
        C2 = np.array([Z[lab == j].mean(0) if (lab == j).any() else C[j] for j in range(a.k)])
        if np.allclose(C2, C):
            break
        C = C2
    sse = ((Z - C[lab]) ** 2).sum()
    if best is None or sse < best[0]:
        best = (sse, lab.copy(), C.copy())
_, lab, C = best
# order archetypes by mean model rounds to 1,000
order = np.argsort([X[lab == j, -1].mean() for j in range(a.k)])
rename = {int(j): f"A{i + 1}" for i, j in enumerate(order)}

rows = []
for i, m in enumerate(maps):
    f20 = fin.get(("TOP20", a.patch, m), {}); x = xan.get(m, {})
    lr = loc.get(m, [])
    rows.append(dict(map=m, archetype=rename[int(lab[i])], **{k: round(float(v), 3) for k, v in zip(feat_names, X[i])},
                     top20_games=f20.get("team_games", ""), top20_win=f20.get("wins", ""), top20_gem_wins=f20.get("gem_wins", ""),
                     top20_median_finish=f20.get("median_finish", ""), top20_mean_score=f20.get("mean_score", ""),
                     v4_ranked=f'{x["v4/1.0.3"]["wins"]}/{x["v4/1.0.3"]["games"]}' if "v4/1.0.3" in x else "", v5r_ranked=f'{x["v5r"]["wins"]}/{x["v5r"]["games"]}' if "v5r" in x else "",
                     v4_mean_score=x.get("v4/1.0.3", {}).get("mean_score", ""), v5r_local_r1000=";".join(r["round_1000"] or "-" for r in lr),
                     v5r_local_gems=";".join(r["gems_submitted"] for r in lr)))
with open(f"{H}/data/S3-ARCHETYPES.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)


def num(v):
    try:
        return float(v)
    except (TypeError, ValueError):
        return None


print("arch maps | model_r1000 top/half | top20 median finish | top20 win | v4 ranked | v5r ranked | v5r local r1000 (median)")
for j in sorted(set(r["archetype"] for r in rows)):
    rs = [r for r in rows if r["archetype"] == j]
    mf = [num(r["top20_median_finish"]) for r in rs if num(r["top20_median_finish"])]
    tw = sum(int(r["top20_win"] or 0) for r in rs); tg = sum(int(r["top20_games"] or 0) for r in rs)
    v4 = [tuple(map(int, r["v4_ranked"].split("/"))) for r in rs if r["v4_ranked"]]
    v5 = [tuple(map(int, r["v5r_ranked"].split("/"))) for r in rs if r["v5r_ranked"]]
    lr = [num(x) for r in rs for x in r["v5r_local_r1000"].split(";") if num(x)]
    print(f'{j} {",".join(r["map"] for r in rs)}')
    print(f'   model {st.median(r["model_r1000"] for r in rs):.0f} | top20 finish {st.median(mf) if mf else "-"} | top20 win {tw}/{tg} | '
          f'v4 {sum(w for w, _ in v4)}/{sum(g for _, g in v4)} | v5r {sum(w for w, _ in v5)}/{sum(g for _, g in v5)} | '
          f'v5r local {st.median(lr) if lr else "-"} ({len(lr)} of {2 * len(rs)} reach 1,000)')
