"""Extra behaviour tables for notes/v3-top10-behaviour.md: top-10 killer matrix (round-level nearby rule), shard purchases (family, stat, field buys by Tavi),
drafts in pick order with rating-adjusted excess, win rate by hour for our games.
Inputs: data/v3/decoded/games_s13.jsonl (everyone but our later games), games_ours.jsonl (our 515 games). Run: py -I tools/v3/behaviour_extras.py
"""
import json, sys, collections, statistics, glob, os
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, "C:/cambc-solo/tools/v3")
from killers import deaths_nearby, TN
V3 = "C:/cambc-solo/data/v3"
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
RANK = {t["teamId"]: t["rank"] for t in top}
FAM = ["monster", "power", "durability", "treasure", "jungle", "home", "bruiser"]
STAT = {0: "str", 2: "foc", 4: "hp"}
T1 = [g for g in map(json.loads, open(f"{V3}/decoded/games_s13.jsonl", encoding="utf8")) if all(RANK.get(x, 999) <= 10 for x in g["teamIds"])]
OURS = [json.loads(l) for l in open(f"{V3}/decoded/games_ours.jsonl", encoding="utf8")]
print(f"top-10 mutual games {len(T1)}; our games {len(OURS)}")

# --- 1 killers among the top 10 (round-level nearby rule, 88% of deaths attributed with no wrong attributions on 1,276 checked deaths)
mat = collections.Counter(); ktot = collections.Counter(); vtot = collections.Counter(); unk = 0; n = 0
for g in T1:
    f = f"{V3}/decoded/s13/{g['match']}.json"
    if not os.path.exists(f): continue
    d = json.load(open(f, encoding="utf8"))
    for e in deaths_nearby(d):
        n += 1; vtot[TN[e["vtype"]]] += 1
        if e["ktype"] is None: unk += 1
        else: mat[(TN[e["ktype"]], TN[e["vtype"]])] += 1; ktot[TN[e["ktype"]]] += 1
print(f"\n### K1 top-10 mutual games: {len(T1)} games, {n} champion deaths, {unk} with no enemy kill nearby (camp or other)")
print("killers:", ", ".join(f"{k} {v}" for k, v in ktot.most_common()))
print("victims:", ", ".join(f"{k} {v}" for k, v in vtot.most_common()))
print("top pairs:", ", ".join(f"{k}->{v} {c}" for (k, v), c in mat.most_common(10)))

# --- 2 purchases
def purchases(games, who):
    first = collections.Counter(); per_type = collections.defaultdict(lambda: [0, 0]); tavi = [0, 0, 0]; emb = 0; ng = 0
    for g in games:
        ts = [0, 1] if who == "both" else [g["teamIds"].index(XAN)]
        for t in ts:
            ng += 1; seen = set()
            for e in sorted(g["ev"]["item"], key=lambda e: e[0]):
                if e[2] != t: continue
                idx = e[4]
                if idx % 6 in (0, 2, 4):
                    per_type[TN[e[3]]][0] += e[5]
                    if e[1] not in seen: seen.add(e[1]); first[(TN[e[3]], FAM[idx // 6], STAT[idx % 6])] += 1
                    if e[3] == 11: tavi[0] += e[5]; tavi[1] += e[5] if e[6] else 0
                else: emb += e[5]
    return first, per_type, tavi, emb, ng

for name, games, who in (("top-10 mutual (both sides)", T1, "both"), ("ours", OURS, "ours")):
    first, per_type, tavi, emb, ng = purchases(games, who)
    print(f"\n### P1 shards, {name}: {ng} team-games; shards bought per champion body per team-game and share of Tavi shards bought away from spawn (> 6 tiles)")
    fam = collections.Counter(); stat = collections.Counter()
    for (tn, fa, st), c in first.items(): fam[fa] += c; stat[st] += c
    print("first purchase of each champion: family", dict(fam.most_common()), "| stat", dict(stat.most_common()))
    byt = collections.defaultdict(collections.Counter)
    for (tn, fa, st), c in first.items(): byt[tn][(fa, st)] += c
    for tn in ("Tavi", "Stellar", "Ryth", "Varg", "Ren", "C&I", "Vendetta"):
        c = byt[tn]; tot = sum(c.values())
        if tot: print(f"  {tn}: first shard n={tot}: " + ", ".join(f"{fa}/{st} {100 * v / tot:.0f}%" for (fa, st), v in c.most_common(3)) + f"; shards per team-game {per_type[tn][0] / ng:.1f}")
    print(f"  Tavi shards bought {tavi[0]}, away from spawn {tavi[1]} ({100 * tavi[1] / max(1, tavi[0]):.0f}%); emblem completions per team-game {emb / ng:.2f}")

# --- 3 drafts
def expd(g, t):
    a, b = g["rating"][t], g["rating"][1 - t]
    return 1 / (1 + 10 ** ((b - a) / 400)) if a and b else None

def draft_table(games, who, title, perMap=False, minn=1):
    c = collections.defaultdict(lambda: [0, 0, 0.0, 0])
    for g in games:
        ts = [0, 1] if who == "both" else [g["teamIds"].index(XAN)]
        for t in ts:
            key = ((g["map"],) if perMap else ()) + (tuple(sorted(g["draft"][t])),)
            e = expd(g, t)
            r = c[key]; r[0] += 1; r[1] += g["winner"] == t
            if e is not None: r[2] += e; r[3] += 1
    print(f"\n### {title}")
    for key, r in sorted(c.items(), key=lambda kv: -kv[1][0]):
        if r[0] < minn: continue
        ex = 100 * (r[1] - r[2]) / r[3] if r[3] else 0
        print(f"| {' / '.join(key[:-1])} {'+'.join(key[-1])} | n={r[0]}{'†' if r[0] < 30 else ''} | win {100 * r[1] / r[0]:.0f}% | excess {ex:+.0f} pp |")
        if not perMap and sum(1 for _ in range(1)) and list(sorted(c.items(), key=lambda kv: -kv[1][0])).index((key, r)) >= 7: break

draft_table(T1, "both", "D1 top-10 mutual: most common five-sets (champion sets, all maps)")
draft_table(OURS, "ours", "D2 our drafts (all maps)")
fp = collections.Counter(g["draft"][t][0] for g in T1 for t in (0, 1) if g["draft"][t])
fo = collections.Counter(g["draft"][g["teamIds"].index(XAN)][0] for g in OURS if g["draft"][g["teamIds"].index(XAN)])
print("\nD3 first pick, top-10:", dict(fp.most_common(6)), "| ours:", dict(fo.most_common(6)))
op = collections.Counter(g["draft"][1 - g["teamIds"].index(XAN)][0] for g in OURS if g["draft"][1 - g["teamIds"].index(XAN)])
print("D3 first pick of our opponents:", dict(op.most_common(6)))

# --- 4 win rate by hour (our games)
b = collections.defaultdict(lambda: [0, 0])
for g in OURS:
    t = g["teamIds"].index(XAN); k = g["t"][:13]
    b[k][0] += g["winner"] == t; b[k][1] += 1
print("\n### W1 our win rate by hour (UTC; BST = UTC+1), wins/games")
print(" ".join(f"{k[8:10]}h{k[11:13]}Z {v[0]}/{v[1]}" for k, v in sorted(b.items())))
