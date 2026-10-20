"""w13: champion-rounds by activity from extract_pilot.mjs JSON (tools/v3/extract_pilot.mjs --stride N).

usage: python results/w13-activity.py <side A|B> <camps0.json> <extract.json ...>
(camps0.json: one line of node tools/v3/camps0.mjs on any replay of the map; extract's final camps lose dead positions)
Classes per champion snapshot of our team: dead; gem (within 4 tiles of a camp of a gem-bearing group, nearest
group wins); gold (same, gold-only group); other (walking, banking, recalling, healing). Groups = camps joined by
cardinal adjacency (GameRules: "any monsters that are cardinally connected"); gems per camp from the rules table.
"""
import json, sys

GEMS = {0: 22, 1: 36, 2: 24, 3: 0, 4: 200}


def groups_of(camps):
    pos = {(x, y): i for i, (t, x, y) in enumerate(camps) if x >= 0}
    gid = [-1] * len(camps)
    n = 0
    for i, (t, x, y) in enumerate(camps):
        if gid[i] >= 0 or x < 0:
            continue
        stack = [i]; gid[i] = n
        while stack:
            j = stack.pop(); _, a, b = camps[j]
            for d in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                k = pos.get((a + d[0], b + d[1]))
                if k is not None and gid[k] < 0:
                    gid[k] = n; stack.append(k)
        n += 1
    gems = [0] * n
    for i, (t, x, y) in enumerate(camps):
        if gid[i] >= 0:
            gems[gid[i]] += GEMS.get(t, 0)
    return gid, gems


def classify(path, side, camps):
    d = json.load(open(path))
    team = 0 if side == "A" else 1
    gid, gems = groups_of(camps)
    tiles = [(gid[i], x, y) for i, (t, x, y) in enumerate(camps) if gid[i] >= 0]
    cnt = {"gem": 0, "gold": 0, "other": 0, "dead": 0}
    last = -1
    for snap in d["rounds"]:
        if snap["r"] == last:
            continue
        last = snap["r"]
        for c in snap["c"]:
            if c[0] != team:
                continue
            if not c[2]:
                cnt["dead"] += 1; continue
            best, bd = None, 5
            for g, x, y in tiles:
                dd = max(abs(x - c[3]), abs(y - c[4]))
                if dd < bd:
                    bd, best = dd, g
            if best is None:
                cnt["other"] += 1
            else:
                cnt["gem" if gems[best] > 0 else "gold"] += 1
    tot = sum(cnt.values()) or 1
    return {k: round(100 * v / tot, 1) for k, v in cnt.items()}, d.get("finalRound")


if __name__ == "__main__":
    side = sys.argv[1]
    camps = json.loads(open(sys.argv[2]).readline())["camps"]
    agg = {"gem": 0, "gold": 0, "other": 0, "dead": 0}
    files = sys.argv[3:]
    for f in files:
        r, fr = classify(f, side, camps)
        print(f, fr, r)
        for k in agg:
            agg[k] += r[k] / len(files)
    print("mean", {k: round(v, 1) for k, v in agg.items()})
