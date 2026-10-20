"""Aggregate data/top10/sides.json into team rows and cross-team gaps. Run: py -I tools/top10/agg.py"""
import json, statistics as st, collections, sys
sys.stdout.reconfigure(encoding="utf-8")
S = json.load(open("C:/cambc-solo/data/top10/sides.json"))
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
TOP = {"Intercontinental Fish":"Fish","Nyx":"Nyx","Univardisty of Shingtonwash":"UoSW","Mite":"Mite","procless":"procless","Pantheon":"Pantheon","Tartarus":"Tartarus","Sneaky Bush Campers":"SBC","new team(4)":"new4","AlphaSlop":"AlphaSlop"}
for s in S:
    if s["teamId"] == XAN: s["lab"] = "us-v3b?" if s["t"] >= "2026-10-20T05:10" else "us-pre"
    elif s["team"] in TOP: s["lab"] = TOP[s["team"]]
    else: s["lab"] = None
import os
S = [s for s in S if s["lab"]]
if os.environ.get("PATCH"): S = [s for s in S if s["patch"] == os.environ["PATCH"]]
def med(v): v = [x for x in v if x is not None]; return round(st.median(v)) if v else None
def mean(v): v = [x for x in v if x is not None]; return sum(v) / len(v) if v else float("nan")
def row(g):
    n = len(g); f = [s for s in g if s["has_full"]]
    nf = sum(s["nfights"] for s in f)
    return dict(n=n, W=sum(s["win"] for s in g), r1000=med([s["rounds"] for s in g if s["win"] and s["sg"] >= 1000]), nwin1000=sum(1 for s in g if s["win"] and s["sg"] >= 1000),
        gpr=sum(s["sg"] for s in g) / sum(s["rounds"] for s in g), trib=mean([s["n_trib"] for s in g]), bag=mean([s["mean_bag"] for s in g]), deaths=mean([s["deaths"] for s in g]),
        carry=sum(s["deaths_carry"] for s in g) / max(1, sum(s["deaths"] for s in g)), p200=mean([s["pts200"] for s in g]), p400=mean([s["pts400"] for s in g]),
        spell=(sum(s["nspell"] for s in f) / nf if nf else float("nan")), nfull=len(f), nfi=nf, locks=mean([s["lock_buy"] for s in g]), rec=mean([s["recalls"] for s in g]),
        cap300=mean([s["captures300"] for s in g]), k200=mean([s["kills200"] for s in g]), spread=mean([s["spread"] for s in g]), camped=mean([s["camped_on"] for s in g]),
        nes=(sum(s["fights_near_enemy_spawn"] for s in f) / nf if nf else float("nan")), first=med([s["first_trib"] for s in g]), size=mean([s["bodies"] for s in g]))
labs = list(TOP.values()) + ["us-pre", "us-v3b?"]
R = {l: row([s for s in S if s["lab"] == l]) for l in labs if any(s["lab"] == l for s in S)}
T = [s for s in S if s["lab"] not in ("us-pre", "us-v3b?")]
R["TOP10 all"] = row(T); R["TOP10 1.0.0"] = row([s for s in T if s["patch"] == "1.0.0"]); pass
hdr = "team|n|W|med rnds to 1000 (wins,n)|gems/rnd|trib/g|gems/trib|first trib|deaths/g|%deaths carrying|pts@200|pts@400|spell-open (fights)|lock buys/g|recalls/g|captures<=300|kills<=200|spread100-400|enemy-near-my-spawn rnds|fights near enemy spawn"
print(hdr.replace("|", " | "))
for l, r in R.items():
    print(f"{l} | {r['n']}{'' if r['n']>=30 else '†'} | {r['W']} | {r['r1000']} ({r['nwin1000']}) | {r['gpr']:.2f} | {r['trib']:.1f} | {r['bag']:.1f} | {r['first']} | {r['deaths']:.1f} | {100*r['carry']:.0f}% | {r['p200']:.1f} | {r['p400']:.1f} | {100*r['spell']:.0f}% ({r['nfi']}) | {r['locks']:.2f} | {r['rec']:.1f} | {r['cap300']:.1f} | {r['k200']:.1f} | {r['spread']:.1f} | {r['camped']:.1f} | {100*r['nes']:.0f}%")
# per team x map-group-by-size
print("\nteam size/modal draft per team")
for l in [x for x in labs[:10] if x in R]:
    g = [s for s in S if s["lab"] == l]
    print(l, dict(collections.Counter(s["bodies"] for s in g)))
json.dump(R, open("C:/cambc-solo/data/top10/team_rows%s.json" % os.environ.get("PATCH", ""), "w"))
