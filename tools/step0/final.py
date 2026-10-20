import sys,glob,json,os,collections,csv,pickle
sys.path.insert(0,"C:/cambc-solo/tools/step0")
from deaths import maps,TN,ch,mapkey,R
sides=json.load(open(R+"data/top10/sides.json"))
T8=["Univardisty of Shingtonwash","procless","Mite","Nyx","Pantheon","new team(4)","Tartarus","AlphaSlop"]
def exact_deaths(path,mp,us_team):
    d=json.load(open(path)); S=d["rounds"]; m=maps[mapkey(mp)]
    team=[r[0] for r in S[0]["c"]]; typ=[r[1] for r in S[0]["c"]]; n=len(team)
    start={t:[(r[3],r[4]) for r in S[0]["c"] if r[0]==t] for t in (0,1)}
    mons=[(x,y) for t,x,y in m["camps"] if t in (0,1,4)]; alt=[tuple(a) for a in m["altars"]]
    br=collections.defaultdict(list)
    for a in d["actions"]: br[a[0]].append(a)
    out=[]
    for a,b in zip(S,S[1:]):
        evs=[]
        for i in range(n):
            if not(a["c"][i][2] and not b["c"][i][2]): continue
            t=team[i]; x,y=a["c"][i][3],a["c"][i][4]; k=None
            for r in (a["r"],b["r"]):
                for act in br.get(r,[]):
                    if team[act[1]]!=t and any(h[0]=="c" and h[1]==i and h[2]>0 for h in act[9]): k=act[1]
            if min(ch(x,y,sx,sy) for sx,sy in start[t])<=8: loc="spawn"
            elif any(ch(x,y,ax,ay)<=8 for ax,ay in alt): loc="altar"
            elif any(ch(x,y,cx,cy)<=8 for cx,cy in mons): loc="camp"
            else: loc="elsewhere"
            nearmon=any(ch(x,y,cx,cy)<=8 for cx,cy in mons)
            r=b["r"]; band="0-200" if r<200 else "200-600" if r<600 else "600+"
            evs.append(dict(i=i,t=t,k=k,loc=loc,nearmon=nearmon,band=band,bag=a["c"][i][7],victim=TN[typ[i]],killer=TN[typ[k]] if k is not None else None,r=r))
        # partner handling
        for e in evs:
            if e["victim"]=="C&I":
                mates=[o for o in evs if o is not e and o["t"]==e["t"] and o["victim"]=="C&I"]
                if mates and e["k"] is None and (mates[0]["k"] is not None or mates[0]["i"]<e["i"]): e["partner"]=True; e["k"]=mates[0]["k"]; e["nearmon"]=mates[0]["nearmon"]
        for e in evs:
            e["cause"]="champion" if e["k"] is not None else ("monster(near camp)" if e["nearmon"] else "other/no champion")
            out.append(e)
    return out
rows=[];games={}
for r in csv.DictReader(open(R+"data/unranked/games_v3b.tsv"),delimiter="\t"):
    p=R+"data/unranked/v3b_full/"+r["match"]+".json"
    if not os.path.exists(p): continue
    for e in exact_deaths(p,r["map"],0): e.update(set="v3b",map=r["map"],match=r["match"],res=r["res"],side="us" if e["t"]==0 else "them"); rows.append(e)
    games[("v3b",r["match"])]=r["map"]
for s in sides:
    st="rank-us" if s["team"]=="xanarchy" else "top10" if s["team"] in T8 else None
    p=R+"data/top10/full/"+s["match"]+".json"
    if not st or not os.path.exists(p): continue
    for e in exact_deaths(p,s["map"],s["side"]): e.update(set=st,map=s["map"],match=s["match"],res=s["win"],side="us" if e["t"]==s["side"] else "them"); rows.append(e)
    games[(st,s["match"]+str(s["side"]))]=s["map"]
    # tag sides: need which team index is this side
for e in rows: pass
pickle.dump((rows,games),open("C:/Users/willi/AppData/Local/Temp/claude/C--Users-willi-OneDrive-Desktop-cambc-solo/a868a45e-2afb-4da7-93b5-574919305849/scratchpad/rows2.pkl","wb"))
print(len(rows),collections.Counter(k[0] for k in games))
