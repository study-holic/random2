import numpy as np, pandas as pd, collections
R='C:/cambc-solo/'
df=pd.read_csv(R+'intel/matches-2026-10-18.csv')
ra=df.rating_a.values; rb=df.rating_b.values
E=1/(1+10**((rb-ra)/400)); y=(df.winner=='a').values.astype(float)
def short(s): return s.replace('CassiIssac','C&I')
da=[short(s) for s in df.draft_a]; db=[short(s) for s in df.draft_b]
gid=pd.factorize(df.group_id)[0]; ng=gid.max()+1
rows=collections.defaultdict(list)
mir=0
for k in range(len(df)):
    a,b=da[k],db[k]
    if a==b: mir+=1; continue
    if a<b: rows[(a,b)].append((k,y[k]-E[k],y[k]))
    else: rows[(b,a)].append((k,-(y[k]-E[k]),1-y[k]))
print('mirror games',mir)
rng=np.random.default_rng(2)
out=[]
for (a,b),L in rows.items():
    if len(L)<30: continue
    ks=[l[0] for l in L]; ex=np.array([l[1] for l in L]); w=np.array([l[2] for l in L])
    g=gid[ks]
    # cluster bootstrap
    ug=np.unique(g); idx={u:np.where(g==u)[0] for u in ug}
    bs=[]
    for _ in range(500):
        pick=rng.choice(ug,len(ug)); ii=np.concatenate([idx[u] for u in pick]); bs.append(ex[ii].mean())
    maps=collections.Counter(df.map_name.values[ks]).most_common(3)
    teams=len(set(df.team_a.values[ks])|set(df.team_b.values[ks]))
    out.append(dict(draft1=a,draft2=b,games=len(L),wins1=int(w.sum()),wr1=w.mean(),exc_pp=100*ex.mean(),lo=100*np.percentile(bs,2.5),hi=100*np.percentile(bs,97.5),
        meanE1=float((w-ex).mean()),teams=teams,maps='; '.join(f'{m}:{c}' for m,c in maps)))
o=pd.DataFrame(out).sort_values('games',ascending=False)
o.to_csv(R+'data/v3/matchups/draft_fullpairs.csv',index=False)
pd.set_option('display.width',250); pd.set_option('display.max_colwidth',80)
print(len(o)); print(o.head(20).to_string())
# distribution of distinct pairings
allp=len(rows); print('distinct pairings',allp,'with >=30 games',len(o),'games covered',sum(len(L) for L in rows.values()))
big=sum(len(L) for L in rows.values() if len(L)>=30); print('games in pairings>=30',big)
