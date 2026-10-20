import pickle,collections
S="C:/Users/willi/AppData/Local/Temp/claude/C--Users-willi-OneDrive-Desktop-cambc-solo/a868a45e-2afb-4da7-93b5-574919305849/scratchpad/"
rows,games=pickle.load(open(S+"rows2.pkl","rb"))
ng=collections.Counter(k[0] for k in games)
def pct(a,b): return f"{a} ({100*a/max(b,1):.0f}%)"
for st in("v3b","rank-us","top10"):
    us=[r for r in rows if r["set"]==st and r["side"]=="us"];n=ng[st];N=len(us)
    print(f"\n== {st}: games {n}, deaths {N}, {N/n:.1f}/game, partner(C&I) deaths {sum(1 for r in us if r.get('partner'))}")
    for k in("cause","loc","band"): print(k,{a:pct(b,N) for a,b in collections.Counter(r[k] for r in us).most_common()})
    print("cause x loc",[(a,b) for a,b in collections.Counter((r["cause"],r["loc"]) for r in us).most_common(7)])
    print("cause x band",sorted(collections.Counter((r["cause"],r["band"]) for r in us).items()))
    for cs in("champion","monster(near camp)","other/no champion"):
        x=[r for r in us if r["cause"]==cs]
        print(cs,"n",len(x),"carry>0",sum(r["bag"]>0 for r in x),"carry>=20",sum(r["bag"]>=20 for r in x),"gems",sum(r["bag"] for r in x),"/game %.1f"%(sum(r["bag"] for r in x)/n))
    th=[r for r in rows if r["set"]==st and r["side"]=="them"]
    if st in("v3b","rank-us"):
        print("their deaths/game %.1f; gems we take (their carried, champion-killed) %.1f/game; all their carried at death %.1f/game"%(len(th)/n,sum(r["bag"] for r in th if r["cause"]=="champion")/n,sum(r["bag"] for r in th)/n))
        print("their deaths: champion-killed",pct(sum(r["cause"]=="champion" for r in th),len(th)),"killers(ours)",collections.Counter(r["killer"] for r in th if r["killer"]).most_common(4))
    print("our victims by killer type",collections.Counter(r["killer"] for r in us if r["killer"]).most_common(6),"; victim type",collections.Counter(r["victim"] for r in us).most_common(6))
    if st=="v3b":
        pm=collections.defaultdict(list)
        for r in us: pm[r["map"]].append(r)
        gm=collections.Counter(v for k,v in games.items() if k[0]==st)
        print("map | games | deaths/g | champ% | monster% | other% | gems lost champion/g | carry>=20 deaths/g")
        for m in sorted(gm):
            x=pm[m];g=gm[m];N2=len(x)
            print(f"| {m} | {g} | {N2/g:.1f} | {100*sum(r['cause']=='champion' for r in x)/N2:.0f} | {100*sum(r['cause'].startswith('monster') for r in x)/N2:.0f} | {100*sum(r['cause'].startswith('other') for r in x)/N2:.0f} | {sum(r['bag'] for r in x if r['cause']=='champion')/g:.0f} | {sum(r['bag']>=20 for r in x)/g:.1f} |")
