"""maps_csv.py: data/MAPS.csv (one row per map), data/MAP-GROUPS.csv (one row per camp group), data/renders/<map>.png.

Inputs (engine 1.0.3): data/readers/dump-1.0.3.json (terrain through the engine's map parser, full map) and
data/census-groups.json (full-map groups, spawn regions, team size from the census bot).
Conventions (stated, not engine-verified unless noted):
- Walking: 8 directions, one tile per step; blocked = walls | holes | altars (BotAPICpp walking mask). Camp spawn tiles count
  as passable (a cleared camp's tiles are walkable). Diagonal steps past wall corners are allowed (no corner rule is documented).
- Tribute needs Chebyshev distance <= submit_gems_range (1, census) from an altar.
- Teleport crystal: lands on a passable tile within Chebyshev 3 of the user (B1b #6, landing rules, is OPEN).
- Supply ceiling: sum over groups of gems / (respawn + capture_time 5) (A3 S1 definition; clear and travel time excluded).
- Reward multiplier: group gems / sum of the documented per-camp gems (wolf 4, elk 15, gem cluster 7, Monolith 200, golem 36);
  gold-only groups use gold (gold cluster 20 ...). Inputs are GameRules values, so the column is DERIVED with a documented input.
"""
import json, csv, os, collections, math
import numpy as np
import networkx as nx
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

H = "/home/claude"
dump = json.load(open(f"{H}/data/readers/dump-1.0.3.json"))
cen = json.load(open(f"{H}/data/census-groups.json"))
facts = json.load(open(f"{H}/data/ENGINE-FACTS.json"))
CAPTURE = facts["settings"]["capture_time"]
SUBMIT_R = facts["settings"]["submit_range"]
GEMS = dict(wolf=4, elk=15, gem_cluster=7, gold_cluster=0, monolith=200, golem=36)
GOLD = dict(wolf=40, elk=90, gem_cluster=0, gold_cluster=20, monolith=300, golem=160)
RESP = dict(wolf=40, elk=60, gem_cluster=50, gold_cluster=50, monolith=150, golem=80)
SYM = {0: "horizontal", 1: "vertical", 2: "rotational"}
DIRS = [(dx, dy) for dx in (-1, 0, 1) for dy in (-1, 0, 1) if dx or dy]
os.makedirs(f"{H}/data/renders", exist_ok=True)


def bfs(passable, starts):
    W, Hh = passable.shape
    INF = 10 ** 9
    d = np.full((W, Hh), INF, dtype=np.int64)
    q = collections.deque()
    for (x, y) in starts:
        if 0 <= x < W and 0 <= y < Hh and passable[x, y] and d[x, y] == INF:
            d[x, y] = 0; q.append((x, y))
    while q:
        x, y = q.popleft()
        for dx, dy in DIRS:
            nx_, ny_ = x + dx, y + dy
            if 0 <= nx_ < W and 0 <= ny_ < Hh and passable[nx_, ny_] and d[nx_, ny_] == INF:
                d[nx_, ny_] = d[x, y] + 1; q.append((nx_, ny_))
    return d


def ring(passable, tiles, r):
    """passable tiles within Chebyshev r of any tile in `tiles`."""
    W, Hh = passable.shape
    out = set()
    for (x, y) in tiles:
        for dx in range(-r, r + 1):
            for dy in range(-r, r + 1):
                a, b = x + dx, y + dy
                if 0 <= a < W and 0 <= b < Hh and passable[a, b]:
                    out.add((a, b))
    return out


def min_cut(passable, A, B):
    """Minimum vertex cut between spawn regions A and B on the walking graph (spawn tiles merged into source and sink)."""
    W, Hh = passable.shape
    As, Bs = set(map(tuple, A)), set(map(tuple, B))
    G = nx.DiGraph()
    for x in range(W):
        for y in range(Hh):
            if not passable[x, y] or (x, y) in As or (x, y) in Bs:
                continue
            G.add_edge(("i", x, y), ("o", x, y), capacity=1)
            for dx, dy in DIRS:
                a, b = x + dx, y + dy
                if 0 <= a < W and 0 <= b < Hh and passable[a, b]:
                    if (a, b) in As:
                        G.add_edge("S", ("i", x, y))
                    elif (a, b) in Bs:
                        G.add_edge(("o", x, y), "T")
                    else:
                        G.add_edge(("o", x, y), ("i", a, b))
    if "S" not in G or "T" not in G:
        return 0, []
    if not nx.has_path(G, "S", "T"):
        return 0, []
    val, (S_side, _) = nx.minimum_cut(G, "S", "T")
    cut = [(n[1], n[2]) for n in S_side if isinstance(n, tuple) and n[0] == "i" and ("o", n[1], n[2]) not in S_side]
    return int(val), cut


rows, grows, notes = [], [], []
for key in sorted(dump):
    name = key[2:].replace("-", "_")
    dm, cm = dump[key], cen[name]
    W, Hh = dm["w"], dm["h"]
    wall = np.zeros((W, Hh), bool); hole = wall.copy(); bush = wall.copy(); alt = wall.copy()
    for t in dm["walls"]: wall[t[0], t[1]] = True
    for t in dm["holes"]: hole[t[0], t[1]] = True
    for t in dm["bushes"]: bush[t[0], t[1]] = True
    for t in dm["altars"]: alt[t[0], t[1]] = True
    passable = ~(wall | hole | alt)
    A = [tuple(t) for t in cm["spawn_a"]]; B = [tuple(t) for t in cm["spawn_b"]]
    dA, dB = bfs(passable, A), bfs(passable, B)
    INF = 10 ** 9
    altars = [tuple(t) for t in dm["altars"]]
    alt_ring = ring(passable, altars, SUBMIT_R)
    dAlt = bfs(passable, alt_ring)
    # reachability from A: walking, then one crystal hop (3 tiles) and walking again
    R0 = {(x, y) for x in range(W) for y in range(Hh) if dA[x, y] < INF}
    land = ring(passable, R0, 3) - R0
    dHop = bfs(passable, land)
    R1 = {(x, y) for x in range(W) for y in range(Hh) if dHop[x, y] < INF} | R0
    gem_near = collections.Counter(); gem_total = 0; ceiling = 0.0; ceil_walk = 0.0; walk_gems = hop_gems = far_gems = 0
    mults = []
    for g in cm["groups"]:
        tiles = [tuple(t) for t in g["spawn_tiles"]]
        kinds = collections.Counter(c["kind"] for c in g["camps"])
        ga = min((dA[t] for t in tiles), default=INF); gb = min((dB[t] for t in tiles), default=INF)
        galt = min((dAlt[t] for t in tiles), default=INF)
        base_g = sum(GEMS[k] * n for k, n in kinds.items()); base_o = sum(GOLD[k] * n for k, n in kinds.items())
        mult = round(g["gems"] / base_g, 3) if base_g else (round(g["gold"] / base_o, 3) if base_o else None)
        mults.append(mult)
        resp_doc = max(RESP[k] for k in kinds)
        reach = "walk" if any(t in R0 for t in tiles) else ("hop1" if any(t in R1 for t in tiles) else "hop2+")
        side = "A" if ga < gb else ("B" if gb < ga else "equal")
        gem_near[side] += g["gems"]; gem_total += g["gems"]
        rate = g["gems"] / (g["respawn"] + CAPTURE); ceiling += rate
        if reach == "walk":
            ceil_walk += rate; walk_gems += g["gems"]
        elif reach == "hop1":
            hop_gems += g["gems"]
        else:
            far_gems += g["gems"]
        cx = sum(t[0] for t in tiles) / len(tiles); cy = sum(t[1] for t in tiles) / len(tiles)
        grows.append(dict(map=name, group=g["i"], kinds=";".join(f"{k}:{n}" for k, n in sorted(kinds.items())), gems=g["gems"], gold=g["gold"],
                          respawn=g["respawn"], respawn_doc_max=resp_doc, mult=mult, x=round(cx, 1), y=round(cy, 1),
                          dist_spawn_a=ga if ga < INF else "", dist_spawn_b=gb if gb < INF else "", dist_altar=galt if galt < INF else "",
                          reach_from_a=reach, nearer=side, gems_per_100r=round(100 * rate, 2)))
    cut_w, cut = min_cut(passable, A, B)
    alt_walk = sum(1 for a in altars if any(t in R0 for t in ring(passable, [a], SUBMIT_R)))
    alt_hop = sum(1 for a in altars if any(t in R1 for t in ring(passable, [a], SUBMIT_R))) - alt_walk
    alt_from_a = min((dA[t] for t in alt_ring), default=INF)
    camp_t = np.zeros((W, Hh), bool)
    for g in cm["groups"]:
        for c in g["camps"]: camp_t[c["pos"][0], c["pos"][1]] = True
    dAc = bfs(passable & ~camp_t, A)
    alt_walk_live = sum(1 for a in altars if any(dAc[t] < INF for t in ring(passable & ~camp_t, [a], SUBMIT_R)))
    kinds_all = collections.Counter(c["kind"] for g in cm["groups"] for c in g["camps"])
    rows.append(dict(map=name, w=W, h=Hh, symmetry=SYM.get(dm["sym"], dm["sym"]), team_size=cm["team_size"], groups=len(cm["groups"]),
                     wolf=kinds_all["wolf"], elk=kinds_all["elk"], gem_cluster=kinds_all["gem_cluster"], gold_cluster=kinds_all["gold_cluster"],
                     golem=kinds_all["golem"], monolith=kinds_all["monolith"], gems_total=gem_total,
                     gold_total=sum(g["gold"] for g in cm["groups"]), mult_max=max(m for m in mults if m is not None),
                     groups_mult_gt1=sum(1 for m in mults if m and m > 1.001), altars=len(altars), altars_walk_a=alt_walk, altars_walk_a_live_camps=alt_walk_live, altars_hop1_a=alt_hop,
                     dist_spawn_a_to_altar=alt_from_a if alt_from_a < INF else "",
                     gems_walk_a=walk_gems, gems_hop1_a=hop_gems, gems_hop2_a=far_gems,
                     gem_share_nearer_a=round(gem_near["A"] / gem_total, 3) if gem_total else "", gem_share_equal=round(gem_near["equal"] / gem_total, 3) if gem_total else "",
                     cut_width=cut_w, cut_x=round(np.mean([c[0] for c in cut]), 1) if cut else "", cut_y=round(np.mean([c[1] for c in cut]), 1) if cut else "",
                     bush_share=round(bush.sum() / passable.sum(), 4), walls=int(wall.sum()), holes=int(hole.sum()),
                     ceiling_gems_per_100r=round(100 * ceiling, 2), ceiling_walk_a_per_100r=round(100 * ceil_walk, 2),
                     rounds_to_1000_at_ceiling=round(1000 / ceiling) if ceiling else ""))
    # render
    img = np.ones((Hh, W, 3))
    img[wall.T] = (0.25, 0.25, 0.25); img[hole.T] = (0.0, 0.0, 0.0); img[bush.T] = (0.55, 0.8, 0.5); img[alt.T] = (0.55, 0.2, 0.75)
    for (x, y) in A: img[y, x] = (0.3, 0.5, 1.0)
    for (x, y) in B: img[y, x] = (1.0, 0.35, 0.35)
    col = dict(wolf=(0.55, 0.45, 0.35), elk=(0.95, 0.55, 0.1), gem_cluster=(0.1, 0.8, 0.85), gold_cluster=(0.95, 0.8, 0.1), golem=(0.45, 0.25, 0.1), monolith=(0.9, 0.1, 0.6))
    for g in cm["groups"]:
        for c in g["camps"]:
            x, y = c["pos"]; img[y, x] = col[c["kind"]]
    fig, ax = plt.subplots(figsize=(max(4, W / 9), max(4, Hh / 9)), dpi=110)
    ax.imshow(img, origin="upper", interpolation="nearest")
    for (x, y) in cut: ax.plot(x, y, "kx", ms=4)
    for g in grows[-len(cm["groups"]):]:
        if g["gems"]:
            ax.text(g["x"], g["y"], str(g["gems"]), fontsize=5, ha="center", va="center", color="black")
    ax.set_title(f"{name}  {W}x{Hh}  team {cm['team_size']}  ceiling {round(100*ceiling,1)}/100r  cut {cut_w}", fontsize=7)
    ax.set_xticks([]); ax.set_yticks([])
    fig.tight_layout(); fig.savefig(f"{H}/data/renders/{name}.png"); plt.close(fig)

with open(f"{H}/data/MAPS.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(rows[0].keys())); w.writeheader(); w.writerows(rows)
with open(f"{H}/data/MAP-GROUPS.csv", "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(grows[0].keys())); w.writeheader(); w.writerows(grows)
resp_mismatch = [g for g in grows if g["respawn"] != g["respawn_doc_max"]]
print("maps", len(rows), "groups", len(grows), "renders", len(os.listdir(f"{H}/data/renders")))
print("respawn != documented longest member:", len(resp_mismatch), [(g["map"], g["group"], g["kinds"], g["respawn"], g["respawn_doc_max"]) for g in resp_mismatch[:4]])
print("multipliers seen:", sorted(collections.Counter(g["mult"] for g in grows).items(), key=lambda kv: (kv[0] is None, kv[0] or 0)))
print("cut width 0 (no walking route between spawns):", [r["map"] for r in rows if r["cut_width"] == 0])
print("altars not walkable from A (cleared camps):", [(r["map"], r["altars"], r["altars_walk_a"]) for r in rows if r["altars_walk_a"] < r["altars"]])
print("altars not walkable from A while camps stand:", [(r["map"], r["altars"], r["altars_walk_a_live_camps"]) for r in rows if r["altars_walk_a_live_camps"] < r["altars"]])
