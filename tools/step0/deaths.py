import json,glob,os,collections,csv,sys
R="C:/cambc-solo/"
maps=json.load(open(R+"data/maps21.json"))
TN={0:"Ryth",1:"Vesper",2:"Vendetta",3:"C&I",4:"C&I",5:"Stellar",6:"Prism",7:"Marcella",8:"Willow",9:"Ren",10:"Varg",11:"Tavi"}
ch=lambda a,b,c,d:max(abs(a-c),abs(b-d))
def mapkey(n): return n[2:] if n[:2].isdigit() else n
def analyse(path,mp,us):
    d=json.load(open(path)); S=d["rounds"]; m=maps[mapkey(mp)]
    team=[r[0] for r in S[0]["c"]]; typ=[r[1] for r in S[0]["c"]]; n=len(team)
    start={t:[(r[3],r[4]) for r in S[0]["c"] if r[0]==t] for t in (0,1)}
    mons=[(x,y) for t,x,y in m["camps"] if t in (0,1,4)]
    alt=[tuple(a) for a in m["altars"]]
    out=[]
    for a,b in zip(S,S[1:]):
        for i in range(n):
            if not(a["c"][i][2] and not b["c"][i][2]): continue
            t=team[i]; x,y=a["c"][i][3],a["c"][i][4]; bag=a["c"][i][7]
            cand=[j for j in range(n) if team[j]!=t and b["c"][j][9]>a["c"][j][9]]
            best=None;bd=99
            for j in cand:
                dd=ch(x,y,a["c"][j][3],a["c"][j][4])
                if dd<bd:best,bd=j,dd
            champ=best is not None and bd<=8
            cause="champion" if champ else ("monster" if any(ch(x,y,cx,cy)<=8 for cx,cy in mons) else "other")
            if min(ch(x,y,sx,sy) for sx,sy in start[t])<=8: loc="spawn"
            elif any(ch(x,y,ax,ay)<=8 for ax,ay in alt): loc="altar"
            elif any(ch(x,y,cx,cy)<=8 for cx,cy in mons): loc="camp"
            else: loc="elsewhere"
            r=b["r"]; band="0-200" if r<200 else "200-600" if r<600 else "600+"
            out.append(dict(side="us" if t==us else "them",cause=cause,loc=loc,band=band,bag=bag,victim=TN[typ[i]],killer=TN[typ[best]] if champ else None,r=r))
    return out,d
