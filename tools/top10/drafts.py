"""Draft tables from data/top10/all_games.tsv (listing-level; top-10 team-sides since the 22:01Z pool change). Run: py -I tools/top10/drafts.py"""
import csv, re, collections, sys
sys.stdout.reconfigure(encoding="utf-8")
CH = ['Ryth','Vesper','Vendetta','Cassi+Issac','Stellar','Prism','Marcella','Willow','Ren','Varg','Tavi','HeartOfRuin']
RT = {"Fish":2231,"Nyx":2152,"UoSW":2098,"Mite":2047,"procless":2023,"Pantheon":2015,"Tartarus":2012,"SBC":1977,"new4":1976,"AlphaSlop":1975}
AL={"Intercontinental Fish":"Fish","Univardisty of Shingtonwash":"UoSW","Sneaky Bush Campers":"SBC","new team(4)":"new4"}
R = [x for x in csv.DictReader(open("C:/cambc-solo/data/top10/all_games.tsv"), delimiter="\t") if x["t"] >= __import__("os").environ.get("SINCE", "2026-10-19T22:01")]
def dr(x):
    return tuple(sorted(int(t.strip("()")) for t in re.findall(r"\(\d+\)|\d", x["draft"])))
name = lambda d: "+".join(CH[i][:4] if CH[i]!="Cassi+Issac" else "CI" for i in d)
for x in R: x["opp"] = AL.get(x["opp"], x["opp"]); x["d"] = dr(x); x["w"] = x["result"] == "win"
print("## usage per champion by map group (share of top-10 team-sides containing champion)")
for grp, f in (("00 kept", lambda m: m.startswith("00")), ("01 new", lambda m: m.startswith("01"))):
    S = [x for x in R if f(x["map"])]
    print(grp, len(S), " ".join(f"{CH[i]}:{100*sum(i in x['d'] for x in S)/len(S):.0f}%" for i in range(12)))
print("\n## per map: team size, top draft sets (n, win%, rating-adjusted excess in points = win - expected, mutual games only)")
exp = lambda a, b: 1 / (1 + 10 ** ((b - a) / 400))
M = collections.defaultdict(list)
for x in R: M[x["map"]].append(x)
for m in sorted(M):
    S = M[m]; sz = collections.Counter(len(x["d"]) for x in S)
    c = collections.Counter(x["d"] for x in S)
    cells = []
    for d, n in c.most_common(3):
        xs = [x for x in S if x["d"] == d]
        e = [x for x in xs if x["oppTop10"] == "True"]
        ex = sum(x["w"] - exp(RT[x["team"]], RT.get(x["opp"], RT[x["team"]])) for x in e)
        cells.append(f"{name(d)} n={n} w={100*sum(x['w'] for x in xs)/n:.0f}%" + (f" adj{100*ex/len(e):+.0f}pt(n={len(e)})" if e else ""))
    pr = sum(5 in x["d"] for x in S); hr = sum(11 in x["d"] for x in S)
    print(f"{m} | sides {len(S)} | size {dict(sz)} | Prism {pr} HoR {hr} | " + " ; ".join(cells))
print("\n## champion presence: win excess in top10-vs-top10 games (rating adjusted), n sides")
E = [x for x in R if x["oppTop10"] == "True"]
for i in range(12):
    a = [x for x in E if i in x["d"]]
    if a: print(CH[i], len(a), f"{100*sum(x['w']-exp(RT[x['team']],RT[x['opp']]) for x in a)/len(a):+.1f}pt")
# head to head: draft-set A vs draft-set B among mutual games
print("\n## mutual-game draft matchups (set vs set; side A wins/n), only pairs with n>=6")
H = collections.defaultdict(lambda: [0, 0, 0.0])
G = collections.defaultdict(list)
for x in E: G[x["match"]].append(x)
for k, v in G.items():
    if len(v) != 2: continue
    a, b = v
    if a["d"] <= b["d"]: A, B = a, b
    else: A, B = b, a
    if a["d"] == b["d"]: continue
    h = H[(A["d"], B["d"])]; h[0] += A["w"]; h[1] += 1; h[2] += A["w"] - exp(RT[A["team"]], RT[B["team"]])
for (a, b), h in sorted(H.items(), key=lambda kv: -kv[1][1])[:12]:
    print(f"{name(a)} vs {name(b)}: {h[0]}/{h[1]} adj {100*h[2]/h[1]:+.0f}pt")
print("pairs with n>=6:", sum(1 for h in H.values() if h[1] >= 6), "of", len(H))
