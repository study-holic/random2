# A2: champion-vs-champion draft matrix from intel/matches CSV (rating-adjusted, cluster bootstrap by group).
import numpy as np, pandas as pd, json, itertools, sys
from scipy.optimize import minimize
R='C:/cambc-solo/'
df=pd.read_csv(R+'intel/matches-2026-10-18.csv')
CH=['Ryth','Vesper','Vendetta','CassiIssac','Stellar','Prism','Marcella','Willow','Ren','Varg','Tavi']
ci={c:i for i,c in enumerate(CH)}
def vec(s):
    v=np.zeros(11,bool)
    for c in s.split('+'): v[ci[c]]=True
    return v
A=np.stack([vec(s) for s in df.draft_a]); B=np.stack([vec(s) for s in df.draft_b])
ra=df.rating_a.values; rb=df.rating_b.values
E=1/(1+10**((rb-ra)/400)); y=(df.winner=='a').values.astype(float)
n=len(df); e=y-E  # side A excess
gid=pd.factorize(df.group_id)[0]; ng=gid.max()+1
maps=pd.factorize(df.map_id)[0]; nm=maps.max()+1
print('games',n,'groups',ng,'mean E_A',E.mean(),'win A',y.mean(),'mean excess A',e.mean())
# calibration of elo itself: fit y ~ sigmoid(k*offset+b)
off=np.log(10)/400*(ra-rb)
def fit_logit(X,off,y,l2=0.0,w0=None):
    p=X.shape[1]
    def f(w):
        z=off+X@w; pr=1/(1+np.exp(-z))
        ll=-(y*np.log(pr+1e-12)+(1-y)*np.log(1-pr+1e-12)).sum()+0.5*l2*(w**2).sum()
        g=X.T@(pr-y)+l2*w
        return ll,g
    r=minimize(f,np.zeros(p) if w0 is None else w0,jac=True,method='L-BFGS-B')
    return r.x,r.fun
# elo scale check
Xk=np.stack([off,np.ones(n)],1)
def f2(w):
    z=w[0]*off+w[1]; pr=1/(1+np.exp(-z))
    return -(y*np.log(pr)+(1-y)*np.log(1-pr)).sum()
r=minimize(f2,[1,0]); print('elo scale k (1=exact)',r.x)
# ---- exclusive pair cells
dA=A&~B; dB=B&~A   # champion exclusively on A side / B side
pairs=[(i,j) for i in range(11) for j in range(11) if i!=j]
# per-view contributions: view A: X in dA, Y in dB, value e ; view B: X in dB, Y in dA, value -e
S=np.zeros((ng,11,11)); N=np.zeros((ng,11,11))
for k in range(n):
    xa=np.where(dA[k])[0]; xb=np.where(dB[k])[0]
    for i in xa:
        for j in xb:
            S[gid[k],i,j]+=e[k]; N[gid[k],i,j]+=1
            S[gid[k],j,i]-=e[k]; N[gid[k],j,i]+=1
rng=np.random.default_rng(1)
tot=S.sum(0); cnt=N.sum(0)
B_=1000
bs=np.zeros((B_,11,11))
for b in range(B_):
    w=np.bincount(rng.integers(0,ng,ng),minlength=ng)
    s=np.tensordot(w,S,1); c=np.tensordot(w,N,1)
    bs[b]=np.where(c>0,s/np.maximum(c,1),np.nan)
mean=np.where(cnt>0,tot/np.maximum(cnt,1),np.nan)
lo=np.nanpercentile(bs,2.5,axis=0); hi=np.nanpercentile(bs,97.5,axis=0)
# number of distinct games per cell (each game counted once per ordered cell)
rows=[]
for i in range(11):
    for j in range(11):
        if i<j:
            rows.append(dict(a=CH[i],b=CH[j],games=int(cnt[i,j]),exc_pp=100*mean[i,j],lo=100*lo[i,j],hi=100*hi[i,j],
                             sig=bool(cnt[i,j]>0 and (lo[i,j]>0 or hi[i,j]<0))))
pr=pd.DataFrame(rows)
# Benjamini-Hochberg on bootstrap-normal p
sd=np.nanstd(bs,axis=0)
from scipy.stats import norm
pv=[]
for i in range(11):
    for j in range(11):
        if i<j:
            z=mean[i,j]/sd[i,j] if sd[i,j]>0 else 0
            pv.append(2*(1-norm.cdf(abs(z))))
pr['p']=pv
order=np.argsort(pr.p.values); m=len(pr); bh=np.zeros(m,bool); 
thr=0
for rank,idx in enumerate(order,1):
    if pr.p.values[idx]<=0.05*rank/m: thr=rank
for rank,idx in enumerate(order,1):
    if rank<=thr: bh[idx]=True
pr['bh05']=bh
pr.to_csv(R+'data/v3/matchups/draft_pairs.csv',index=False)
print(pr.sort_values('p').head(25).to_string())
print('cells',m,'raw sig',pr.sig.sum(),'BH sig',pr.bh05.sum(),'expected false pos at 5%',0.05*m)
# --- champion main effect exclusive (X only on my side vs only theirs)
mrows=[]
for i in range(11):
    sA=np.zeros(ng); nA=np.zeros(ng)
    for side,mask,sign in (('A',dA[:,i],1),('B',dB[:,i],-1)):
        np.add.at(sA,gid[mask],sign*e[mask]); np.add.at(nA,gid[mask],1)
    # average over games where i exclusively on one side (each game gives one view)
    tt=sA.sum()/nA.sum(); bsx=[]
    for b in range(B_):
        w=np.bincount(rng.integers(0,ng,ng),minlength=ng); bsx.append((w@sA)/(w@nA))
    mrows.append(dict(champ=CH[i],games_excl=int(nA.sum()),exc_pp=100*tt,lo=100*np.percentile(bsx,2.5),hi=100*np.percentile(bsx,97.5),
                      share_any=float((A[:,i].sum()+B[:,i].sum())/(2*n))))
mr=pd.DataFrame(mrows); mr.to_csv(R+'data/v3/matchups/champ_main.csv',index=False); print(mr.to_string())
# --- full matrix print
Mx=pd.DataFrame(100*mean,index=CH,columns=CH).round(1); print(Mx.to_string())
Cn=pd.DataFrame(cnt.astype(int),index=CH,columns=CH); print(Cn.to_string())
np.save(R+'data/v3/matchups/draft_matrix_mean.npy',mean); 
# ---- regression check: does a pair-interaction model beat additive? 5-fold CV by group
dX=(dA.astype(float)-dB.astype(float))
Pm=np.zeros((n,55)); pidx=[(i,j) for i in range(11) for j in range(i+1,11)]
for q,(i,j) in enumerate(pidx):
    Pm[:,q]=(dA[:,i]&dB[:,j]).astype(float)-(dB[:,i]&dA[:,j]).astype(float)
side=np.zeros((n,nm)); side[np.arange(n),maps]=1
mc=np.zeros((n,11*nm)); 
for i in range(11):
    mc[:,i*nm:(i+1)*nm]=dX[:,[i]]*side
folds=gid%5
def cv(X,l2):
    ll=0
    for f in range(5):
        tr=folds!=f; te=~tr
        w,_=fit_logit(X[tr],off[tr],y[tr],l2)
        z=off[te]+X[te]@w; p=1/(1+np.exp(-z)); ll+=-(y[te]*np.log(p)+(1-y[te])*np.log(1-p)).sum()
    return ll/n
base=-(y*np.log(1/(1+np.exp(-off)))+(1-y)*np.log(1-1/(1+np.exp(-off)))).mean()
print('logloss elo only',base)
print('side*map',cv(side,1.0))
print('+additive champs',cv(np.hstack([side,dX]),1.0))
for l2 in (5,20,50):
    print('+pairs l2',l2,cv(np.hstack([side,dX,Pm]),l2))
for l2 in (5,20):
    print('+champ*map l2',l2,cv(np.hstack([side,dX,mc]),l2))
