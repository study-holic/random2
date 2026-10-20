"""w7: adjusted excess of named drafts per map, and a team's drafts. usage: py -I tools/v3/w7_query.py"""
import sys, math, collections
sys.path.insert(0, "C:/cambc-solo/tools/v3")
from w7_drafts import S, fmt
Q = {
 "v2a5": {"CassiIssac","Ryth","Tavi","Varg","Vendetta"},
 "rep5": {"Ren","Ryth","Stellar","Tavi","Varg"},
 "modal5": {"CassiIssac","Ren","Ryth","Stellar","Varg"},
 "CI+TSRV": {"CassiIssac","Ryth","Stellar","Tavi","Varg"},
 "v2a3": {"CassiIssac","Stellar","Vendetta"},
 "rep3": {"CassiIssac","Stellar","Tavi"},
 "CRS3": {"CassiIssac","Ren","Stellar"},
 "v2a4": {"CassiIssac","Ryth","Varg","Vendetta"},
 "rep4": {"CassiIssac","Ryth","Stellar","Tavi"},
 "CRSV4": {"CassiIssac","Ren","Stellar","Varg"},
 "CRRS4": {"CassiIssac","Ren","Ryth","Stellar"},
 "CRSV4b": {"CassiIssac","Ryth","Stellar","Varg"},
}
maps = sorted({s["map"] for s in S})
for m in maps:
    g = [s for s in S if s["map"] == m]
    size = collections.Counter(len(s["draft"]) for s in g).most_common(1)[0][0]
    out = []
    for k, d in Q.items():
        if len(d) != size: continue
        L = [x for x in g if x["draft"] == frozenset(d)]
        if not L: out.append(f"{k}:0"); continue
        ex = sum(x["win"] - x["exp"] for x in L) / len(L)
        out.append(f"{k}:n{len(L)}:{ex*100:+.1f}")
    print(f"{m:<12} {size} " + "  ".join(out))
print()
for s in S:
    pass
fish = collections.defaultdict(list)
for s in S:
    if "fish" in s["team"].lower(): fish[(s["team"], s["map"])].append(s)
teams = sorted({t for t, _ in fish})
print("fish-like teams:", teams)
for (t, m), L in sorted(fish.items()):
    if m != "swirl": continue
    c = collections.defaultdict(list)
    for x in L: c[x["draft"]].append(x)
    for d, LL in c.items():
        print(f"  {t} {m} {fmt(d)} n={len(LL)} w={sum(x['win'] for x in LL):.0f} ex={sum(x['win']-x['exp'] for x in LL)/len(LL)*100:+.1f} rating={LL[0]['rating']:.0f}")
