import sys; sys.path.insert(0,"C:/cambc-solo/tools/step0")
from deaths import *
import pickle
rows=[]  # (set,map,match,deathdict)
games=collections.defaultdict(dict)
# v3b panel
for r in csv.DictReader(open(R+"data/unranked/games_v3b.tsv"),delimiter="\t"):
    p=R+"data/unranked/v3b_s13/"+r["match"]+".json"
    if not os.path.exists(p): continue
    ev,d=analyse(p,r["map"],0)
    # sanity: final sg vs g_us
    sg=d["rounds"][-1]["sg"]
    for e in ev: e.update(set="v3b",map=r["map"],match=r["match"])
    rows+=ev; games[("v3b",r["match"])]=dict(map=r["map"],sg=sg,gus=int(r["g_us"]),gth=int(r["g_th"]),res=r["res"])
sides=json.load(open(R+"data/top10/sides.json"))
for s in sides:
    p=R+"data/top10/s13/"+s["match"]+".json"
    if not os.path.exists(p): continue
    st="rank-us" if s["team"]=="xanarchy" else "top10"
    ev,d=analyse(p,s["map"],s["side"])
    for e in ev:
        if e["side"]=="us": e.update(set=st,map=s["map"],match=s["match"]); rows.append(e)
    games[(st,s["match"]+str(s["side"]))]=dict(map=s["map"])
pickle.dump((rows,dict(games)),open("C:/Users/willi/AppData/Local/Temp/claude/C--Users-willi-OneDrive-Desktop-cambc-solo/a868a45e-2afb-4da7-93b5-574919305849/scratchpad/rows.pkl","wb"))
print(len(rows),collections.Counter((r["set"],r["side"]) for r in rows))
bad=[(k,v) for k,v in games.items() if k[0]=="v3b" and v["sg"]!=[v["gus"],v["gth"]]];print("sg mismatch",len(bad),bad[:2])
