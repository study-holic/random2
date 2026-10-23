"""s3_model.py: clear-time production model per map (S3, DERIVED).

One squad holding every body moves together, walks 8-directionally one tile per round, clears a camp group by focus fire,
captures it, and tributes the cycle's gems at one altar per cycle. The squad repeats a fixed cycle of groups; each group in the
cycle comes back only after its respawn, so the cycle period is the larger of the walking-and-working time and the slowest
respawn plus capture in the cycle. Groups enter the cycle greedily (cheapest insertion) while the gem rate rises.

Inputs: data/readers/dump-1.0.3.json (terrain), data/census-groups.json (groups, spawns, team size), data/ENGINE-FACTS.json.
Mechanics used (KNOWN, CONTEXT section 3): capture pays 5 rounds after the first capture phase on the tile (6 rounds from arrival);
tribute channels max(1, ceil(g / 10)) turn ends within Chebyshev 1 of an altar, each champion channelling its own bag in
parallel; spawn heals to full; recall 5 turn ends; camps hit the nearest champion with their strength each round while awake.
Assumptions (stated): base stats with no shards; damage per round per champion = strength (basic attack every round) plus
strength / 8 for an attack reset plus (multiplier x Focus) / 8 for each damaging ability, one target each (area and splash
bonuses, Heart of Ruin's missing-health bonus, and Vendetta's reset on kill excluded); camps die one at a time, highest
strength first; the squad recalls to heal when a cycle's damage taken exceeds 60 per cent of its health pool, and the
recall plus the walk back is charged to the cycle in proportion; no enemy.
Scopes: "half" = groups nearer spawn A than spawn B (equal distance counts as half a group's gems); "full" = every group.
Outputs: data/S3-PRODUCTION.csv; prints a summary of 60 lines or fewer.
usage: python3 s3_model.py [--draft name1,name2,...] [--label X]"""
import argparse, collections, csv, json, math
import numpy as np

H = "/home/claude"
ap = argparse.ArgumentParser()
ap.add_argument("--drafts", default="top:heart_of_ruin,vesper_vale,marcella,ryth,vendetta;v5r:vendetta,cassi_and_issac,varg,ryth,tavi")
ap.add_argument("--maps", default="")
ap.add_argument("--heal-share", type=float, default=0.6)
ap.add_argument("--dps-mult", type=float, default=1.0, help="sensitivity: multiply every body's damage per round")
ap.add_argument("--max-squads", type=int, default=9)
ap.add_argument("--capture", type=int, default=6)
ap.add_argument("--quiet", action="store_true")
ap.add_argument("--out", default=f"{H}/data/S3-PRODUCTION.csv")
a = ap.parse_args()

dump = json.load(open(f"{H}/data/readers/dump-1.0.3.json"))
cen = json.load(open(f"{H}/data/census-groups.json"))
F = json.load(open(f"{H}/data/ENGINE-FACTS.json"))
CAMP = {k: v[0] for k, v in F["camp_base_stats"].items()}
RECALL = F["settings"]["recall_time"]
CAPTURE_ROUNDS = a.capture


def champ_dps(name):
    c = F["champion"][name]; b = c["base"]; d = float(b["str"])
    for ab in (c["a1"], c["a2"]):
        k = ab["kind"]
        if k == "attack_reset":
            d += b["str"] / ab["cd"]
        elif k in ("damage", "aoe_damage", "cc", "dash") and ab.get("mod"):
            d += ab["mod"] * b["foc"] / ab["cd"]
    return d, b["hp"]


def bodies_of(names, size):
    out = []
    for p in names[:size]:
        out += ["cassi", "issac"] if p == "cassi_and_issac" else [p]
    return out


def squads_of(bodies, k):
    """split bodies into k squads in round-robin order of draft; returns [(dps, hp, n_bodies)]."""
    sq = [[] for _ in range(k)]
    for j, b in enumerate(bodies):
        sq[j % k].append(b)
    return [(a.dps_mult * sum(champ_dps(b)[0] for b in s), sum(champ_dps(b)[1] for b in s), len(s)) for s in sq]


def dist_field(passable, sources):
    """multi-source 8-directional BFS on a boolean grid by repeated dilation (numpy)."""
    W, Hh = passable.shape
    INF = 10 ** 6
    d = np.full((W, Hh), INF, dtype=np.int32)
    cur = np.zeros((W, Hh), bool)
    for (x, y) in sources:
        if 0 <= x < W and 0 <= y < Hh and passable[x, y]:
            cur[x, y] = True
    d[cur] = 0; seen = cur.copy(); step = 0
    while cur.any():
        step += 1
        nxt = np.zeros_like(cur)
        nxt[1:, :] |= cur[:-1, :]; nxt[:-1, :] |= cur[1:, :]; nxt[:, 1:] |= cur[:, :-1]; nxt[:, :-1] |= cur[:, 1:]
        nxt[1:, 1:] |= cur[:-1, :-1]; nxt[:-1, :-1] |= cur[1:, 1:]; nxt[1:, :-1] |= cur[:-1, 1:]; nxt[:-1, 1:] |= cur[1:, :-1]
        nxt &= passable & ~seen
        d[nxt] = step; seen |= nxt; cur = nxt
    return d


def clear(group, dps):
    """rounds to kill every camp and damage taken, focus fire on the highest-strength camp first."""
    camps = sorted(group["camps"], key=lambda c: -CAMP[c["kind"]]["str"])
    t = dmg = 0.0
    alive_str = sum(CAMP[c["kind"]]["str"] for c in camps)
    for c in camps:
        k = CAMP[c["kind"]]; dt = k["hp"] / dps
        dmg += alive_str * dt; t += dt; alive_str -= k["str"]
    return math.ceil(t), dmg


def rate_of(walk, work, gems, dmg, max_resp, min_spawn, sq_hp, bodies):
    """gem rate of a closed cycle (altar ring -> groups -> altar ring) from its totals."""
    trib = max(1, math.ceil(gems / bodies / 10))
    cycles_per_heal = max(1, int(a.heal_share * sq_hp // dmg)) if dmg > 0 else 0
    heal = (RECALL + min_spawn) / cycles_per_heal if dmg > 0 else 0.0
    period = max(walk + work + trib + heal, max_resp)
    return gems / period, dict(walk=walk, work=work, trib=trib, heal=round(heal, 1), period=round(period, 1), gems=gems)


def best_cycle(G, D, alt, dspawn, cand, sq):
    """greedy cheapest insertion; each step adds the (group, position) pair that gives the highest cycle rate."""
    sq_dps, sq_hp, bodies = sq
    order, rate, info = [], 0.0, {}
    best_seen = ([], 0.0, {})
    walk = work = gems = dmg = 0.0; max_resp = 0; min_spawn = 10 ** 6
    while True:
        best = None
        for i in cand:
            if i in order:
                continue
            gi = G[i]
            wk, gm, dg = work + gi["clear"] + CAPTURE_ROUNDS, gems + gi["gems_w"], dmg + gi["dmg"]
            mr, ms = max(max_resp, gi["respawn"] + CAPTURE_ROUNDS), min(min_spawn, dspawn[i])
            if not order:
                deltas = [(0, 2 * alt[i])]
            else:
                deltas = [(0, alt[i] + D[i][order[0]] - alt[order[0]]), (len(order), D[order[-1]][i] + alt[i] - alt[order[-1]])]
                deltas += [(p, D[order[p - 1]][i] + D[i][order[p]] - D[order[p - 1]][order[p]]) for p in range(1, len(order))]
            p, dl = min(deltas, key=lambda x: x[1])
            r, inf = rate_of(walk + dl, wk, gm, dg, mr, ms, sq_hp, bodies)
            if best is None or r > best[0]:
                best = (r, i, p, dl, inf)
        if best is None:
            return best_seen
        r, i, p, dl, inf = best
        order = order[:p] + [i] + order[p:]
        rate, info = r, inf
        if r > best_seen[1]:
            best_seen = (list(order), r, inf)
        gi = G[i]
        walk += dl; work += gi["clear"] + CAPTURE_ROUNDS; gems += gi["gems_w"]; dmg += gi["dmg"]
        max_resp = max(max_resp, gi["respawn"] + CAPTURE_ROUNDS); min_spawn = min(min_spawn, dspawn[i])


rows = []
drafts = [(d.split(":")[0], d.split(":")[1].split(",")) for d in a.drafts.split(";")]
for key in sorted(dump):
    name = key[2:].replace("-", "_")
    if a.maps and name not in a.maps.split(","):
        continue
    dm, cm = dump[key], cen[name]
    W, Hh = dm["w"], dm["h"]
    blocked = np.zeros((W, Hh), bool)
    for t in dm["walls"] + dm["holes"] + dm["altars"]:
        blocked[t[0], t[1]] = True
    passable = ~blocked
    A = [tuple(t) for t in cm["spawn_a"]]; B = [tuple(t) for t in cm["spawn_b"]]
    dA, dB = dist_field(passable, A), dist_field(passable, B)
    ring = set()
    for (x, y) in (tuple(t) for t in dm["altars"]):
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                if 0 <= x + dx < W and 0 <= y + dy < Hh and passable[x + dx, y + dy]:
                    ring.add((x + dx, y + dy))
    dAlt = dist_field(passable, ring)
    groups = cm["groups"]
    fields = []
    for g in groups:
        tiles = [tuple(t) for t in g["spawn_tiles"]]
        fields.append(dist_field(passable, tiles))
    n = len(groups)
    tiles_of = [[tuple(t) for t in g["spawn_tiles"]] for g in groups]
    D = [[min(int(fields[i][t]) for t in tiles_of[j]) for j in range(n)] for i in range(n)]
    alt = [min(int(dAlt[t]) for t in tiles_of[i]) for i in range(n)]
    da = [min(int(dA[t]) for t in tiles_of[i]) for i in range(n)]
    db = [min(int(dB[t]) for t in tiles_of[i]) for i in range(n)]
    for label, names in drafts:
        bodies = bodies_of(names, cm["team_size"])
        for scope in ("half", "full"):
            wgt = [1.0 if scope == "full" or da[i] < db[i] else (0.5 if da[i] == db[i] else 0.0) for i in range(n)]
            best = None
            for k in range(1, min(len(bodies), a.max_squads) + 1):
                total, parts, used = 0.0, [], set()
                for (dps, hp, nb) in squads_of(bodies, k):
                    G = []
                    for i, g in enumerate(groups):
                        c, dmg = clear(g, dps)
                        cap = a.heal_share * hp
                        trips = max(1, math.ceil(dmg / cap)) if da[i] < 10 ** 6 else 1
                        if trips > 1:   # the squad recalls to heal mid-clear; camps do not regenerate (CONTEXT section 3)
                            c += (trips - 1) * (RECALL + 2 * da[i]); dmg -= (trips - 1) * cap
                        G.append(dict(clear=c, dmg=dmg, gems_w=g["gems"] * wgt[i], respawn=g["respawn"], trips=trips))
                    cand = [i for i in range(n) if i not in used and G[i]["gems_w"] > 0 and da[i] < 10 ** 6 and alt[i] < 10 ** 6]
                    order, rate, info = best_cycle(G, D, alt, da, cand, (dps, hp, nb))
                    used |= set(order); total += rate
                    parts.append(dict(n=nb, dps=round(dps), groups=len(order), rate=round(100 * rate, 1), period=info.get("period", ""),
                                      walk=info.get("walk", ""), start=min((da[i] for i in order), default=0),
                                      cycle=":".join(str(groups[i]["i"]) for i in order)))
                if best is None or total > best[0]:
                    best = (total, k, parts)
            total, k, parts = best
            start = max(p["start"] for p in parts if p["groups"]) if any(p["groups"] for p in parts) else 0
            rows.append(dict(map=name, draft=label, scope=scope, team_size=cm["team_size"], bodies=len(bodies), squads=k,
                             squad_sizes=";".join(str(p["n"]) for p in parts), groups_in_cycles=sum(p["groups"] for p in parts),
                             gems_per_100r=round(100 * total, 1), rounds_to_1000=round(start + 1000 / total) if total > 0 else "",
                             squad_rates=";".join(str(p["rate"]) for p in parts), squad_periods=";".join(str(p["period"]) for p in parts),
                             cycles="|".join(p["cycle"] for p in parts)))
with open(a.out, "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
for r in rows if not a.quiet else []:
    if r["scope"] == "half":
        print(f'{r["map"][:14]:14s} {r["draft"]:4s} squads {r["squads"]} ({r["squad_sizes"]}) groups {r["groups_in_cycles"]:2d} '
              f'rate {r["gems_per_100r"]:6.1f} r1000 {r["rounds_to_1000"]}  per-squad {r["squad_rates"]}')
