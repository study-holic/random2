import pickle,collections,sys,json
S="C:/Users/willi/AppData/Local/Temp/claude/C--Users-willi-OneDrive-Desktop-cambc-solo/a868a45e-2afb-4da7-93b5-574919305849/scratchpad/"
rows,games=pickle.load(open(S+"rows.pkl","rb"))
ng=collections.Counter(k[0] for k in games)
def tab(sel,keyf,keys=None):
    c=collections.Counter(keyf(r) for r in sel);return c
for st in ("v3b","rank-us","top10"):
    us=[r for r in rows if r["set"]==st and r["side"]=="us"]; n=ng[st]; N=len(us)
    print("==",st,"games",n,"deaths",N,"per game %.1f"%(N/n))
    for k in("cause","loc","band"):
        c=collections.Counter(r[k] for r in us);print(k,{a:f"{b} ({100*b/N:.0f}%)" for a,b in c.most_common()})
    c=collections.Counter((r["cause"],r["loc"]) for r in us);print("cause x loc",c.most_common(8))
    for cs in("champion","monster","other"):
        x=[r for r in us if r["cause"]==cs]
        if x: print(cs,"n",len(x),"carry>0",sum(r["bag"]>0 for r in x),"carry>=20",sum(r["bag"]>=20 for r in x),"gems",sum(r["bag"] for r in x),"per game %.1f"%(sum(r["bag"] for r in x)/n))
    # theft
    th=[r for r in rows if r["set"]==st and r["side"]=="them"] if st=="v3b" else []
    if th:
        print("v3b gems we lose to champ kills %.1f/game; destroyed by non-champion %.1f/game; gems we take from their deaths (champion-killed by us) %.1f/game; all their death bags %.1f"%(
         sum(r["bag"] for r in us if r["cause"]=="champion")/n,sum(r["bag"] for r in us if r["cause"]!="champion")/n,
         sum(r["bag"] for r in th if r["cause"]=="champion")/n,sum(r["bag"] for r in th)/n), "their deaths/game %.1f"%(len(th)/n))
    else:
        print("lost to champ %.1f/g, to non-champ %.1f/g"%(sum(r["bag"] for r in us if r["cause"]=="champion")/n,sum(r["bag"] for r in us if r["cause"]!="champion")/n))
