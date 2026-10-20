"""Recall contexts (2.2) and purchases (2.3) for v4 and the top 10 in the v4top games (results/imitate.md).
Items: index = family * 6 + slot, slot 0/2/4 = shard of strength/focus/health, 1/3/5 = emblem (notes/v3-data-scope.md);
families monster, power, durability, treasure, jungle, home, bruiser. Gold spent = 30 x the rise in shards + 5 x emblems
(an emblem is five shards' worth, 150 gold) + 10 per consumable bought.
Run from the repo root: python3 -I tools/imitate/queries.py <dir of decoded v4top-*.json>"""
import json, sys, glob, os, collections, statistics as st
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools", "cloud")); sys.path.insert(0, os.path.join(ROOT, "tools", "imitate"))
import metrics as M
from recalls import recall_events, idx
FAM = ["monster", "power", "durability", "treasure", "jungle", "home", "bruiser"]
STAT = ["strength", "focus", "health"]

def parse(k):
    out = {}
    if k:
        for t in k.split(","):
            a, b = t.split(":"); out[int(a)] = int(b)
    return out

def shard_eq(items): return sum(c * (5 if (ix % 6) % 2 else 1) for ix, c in items.items())

def item_timeline(d):
    """Per champion: list of (round, items dict) at every change."""
    n = len(d["rounds"][0]["c"]); cur = [{} for _ in range(n)]; tl = [[(0, {})] for _ in range(n)]
    for s in d["rounds"]:
        for i, k in s.get("it", []):
            cur[i] = parse(k); tl[i].append((s["r"], dict(cur[i])))
    return tl

def bin_(v, edges, labels):
    for e, l in zip(edges, labels):
        if v < e: return l
    return labels[-1]

if __name__ == "__main__":
    R = collections.defaultdict(list); spend = collections.defaultdict(list); first = collections.defaultdict(collections.Counter)
    emb_round = collections.defaultdict(list); emb_fam = collections.defaultdict(collections.Counter); no_emb = collections.Counter(); champs = collections.Counter()
    order = collections.defaultdict(collections.Counter)
    for f in sorted(glob.glob(os.path.join(sys.argv[1], "*.json"))):
        bn = os.path.basename(f)[:-5]; lab, pos = bn.rsplit("-", 1); mm = idx[(lab, int(pos))]; us = 0 if mm["our_side"] == "a" else 1
        d = json.load(open(f)); ev, S = recall_events(d); byr = {s["r"]: s for s in S}; team = [c[0] for c in S[0]["c"]]
        tl = item_timeline(d)
        side = lambda t: "v4" if t == us else "top10"
        # 2.2 recalls: completed, started away from spawn
        for e in ev:
            if e["fate"] != "done" or e["atspawn"]: continue
            i = e["i"]; ra = e["r"] + e["dur"]
            se0 = shard_eq(max((x for x in tl[i] if x[0] <= ra), key=lambda x: x[0])[1])
            se1 = shard_eq(max((x for x in tl[i] if x[0] <= ra + 10), key=lambda x: x[0])[1])
            c0 = byr[e["r"]]["c"][i]
            m0 = c0[10]; bought_cons = any((byr[r]["c"][i][10] & ~m0) for r in range(ra, ra + 11) if r in byr)
            hp_after = max((byr[r]["c"][i][5] for r in range(ra, ra + 3) if r in byr and byr[r]["c"][i][2]), default=0)
            died = any(not byr[r]["c"][i][2] for r in range(ra, ra + 11) if r in byr)
            R[side(team[i])].append(dict(dist=e["dist"], hp=e["hp"], bag=e["bag"], buy=se1 > se0 or bought_cons, heal=hp_after > c0[5] * 1.1 + 1, died=died))
        # 2.3 purchases
        for i in range(len(team)):
            sd = side(team[i]); champs[sd] += 1
            se200 = shard_eq(max((x for x in tl[i] if x[0] <= 200), key=lambda x: x[0])[1])
            cons = sum(bin(S[k]["c"][i][10] & ~S[k - 1]["c"][i][10]).count("1") for k in range(1, len(S)) if S[k]["r"] <= 200)
            spend[sd].append(30 * se200 + 10 * cons)
            # first purchase and the first three distinct item kinds bought
            seen = []
            for r, it in tl[i][1:]:
                prev = max((x for x in tl[i] if x[0] < r), key=lambda x: x[0])[1]
                for ix in sorted(it):
                    if it[ix] > prev.get(ix, 0):
                        kind = f"{FAM[ix // 6]} {'emblem' if (ix % 6) % 2 else 'shard'} of {STAT[(ix % 6) // 2]}"
                        if kind not in seen: seen.append(kind)
            if seen: first[sd][seen[0]] += 1
            order[sd][" > ".join(seen[:3])] += 1
            er = next(((r, ix) for r, it in tl[i] for ix, c in it.items() if (ix % 6) % 2 and c > 0), None)
            if er: emb_round[sd].append(er[0]); emb_fam[sd][f"{FAM[er[1] // 6]} of {STAT[(er[1] % 6) // 2]}"] += 1
            else: no_emb[sd] += 1
    print("## 2.2 completed recalls (started away from spawn)")
    for sd in ("v4", "top10"):
        X = R[sd]; n = len(X); print(f"### {sd}: {n} recalls")
        for nm, key, edges, labels in (("distance from own spawn", "dist", [11, 21, 31], ["0-10", "11-20", "21-30", "31+"]),
                                       ("HP fraction at start", "hp", [0.3, 0.6, 0.9], ["<0.3", "0.3-0.6", "0.6-0.9", ">=0.9"]),
                                       ("bag at start", "bag", [1, 20, 50], ["0", "1-19", "20-49", "50+"])):
            c = collections.Counter(bin_(x[key], edges, labels) for x in X)
            print(nm, {l: f"{100 * c[l] / n:.0f}%" for l in labels})
        print("within 10 rounds of arrival: purchase %.0f%%, heal %.0f%%, death %.0f%%" % tuple(100 * sum(x[k] for x in X) / n for k in ("buy", "heal", "died")))
        print("median distance %.0f, median HP fraction %.2f, share with a bag %.0f%%" % (st.median(x["dist"] for x in X), st.median(x["hp"] for x in X), 100 * sum(x["bag"] > 0 for x in X) / n))
    print("## 2.3 purchases")
    for sd in ("v4", "top10"):
        print(sd, "gold spent by round 200 per champion: mean %.0f, median %.0f" % (st.mean(spend[sd]), st.median(spend[sd])),
              "| first emblem: median round %s, champions without one %d of %d" % (st.median(emb_round[sd]) if emb_round[sd] else None, no_emb[sd], champs[sd]))
        print("  first purchase:", first[sd].most_common(4))
        print("  first emblem kind:", emb_fam[sd].most_common(4))
        print("  first three kinds:", order[sd].most_common(4))
