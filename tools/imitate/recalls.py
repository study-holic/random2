"""Recall starts and their fate per team, from decoded v4top games (results/imitate.md section 1).
Run from the repo root: python3 -I tools/imitate/recalls.py <dir of decoded v4top-*.json>"""
import json,sys,glob,os,collections
ROOT=os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0,os.path.join(ROOT,'tools','cloud')); import metrics as M
ch=lambda a,b:max(abs(a[0]-b[0]),abs(a[1]-b[1]))
idx={}
for l in open(os.path.join(ROOT,'data','unranked','matches.jsonl')):
    m=json.loads(l); idx[(m['label'],m['position'])]=m
idx.update(M.panel_index(idx))
def recall_events(d):
    """Each recall start (kind 11) with its fate: 'done' (the champion appears at its spawn with no other action
    between), 'cancel' (kind 16 or any other action first), 'died', 'interrupted' (hp fell, no action, no arrival)."""
    S=M.snaps_of(d); byr={s['r']:s for s in S}; team=[c[0] for c in S[0]['c']]; n=len(team)
    spawn={t:[(c[3],c[4]) for c in S[0]['c'] if c[0]==t] for t in (0,1)}
    near=lambda t,p: min(ch(p,q) for q in spawn[t])<=2
    by=collections.defaultdict(list)
    for a in d['actions']: by[(a[0],a[1])].append(a[2])
    out=[]
    for a in d['actions']:
        if a[2]!=11: continue
        r,i=a[0],a[1]; s=byr.get(r)
        if not s: continue
        c=s['c'][i]; p0=(c[3],c[4]); t=team[i]; fate='open'; dur=None
        # actions after the start in the same round
        same=by[(r,i)]; after=same[same.index(11)+1:] if 11 in same else []
        if any(k in (4,5,6,12,16) for k in after): fate='cancel'
        else:
            for rr in range(r+1,r+9):
                x=byr.get(rr)
                if not x: break
                cc=x['c'][i]
                if not cc[2]: fate='died'; break
                if near(t,(cc[3],cc[4])) and not near(t,p0): fate='done'; dur=rr-r; break
                ks=by.get((rr,i),[])
                nx=byr.get(rr+1)
                if nx and nx['c'][i][2] and near(t,(nx['c'][i][3],nx['c'][i][4])) and not near(t,p0): fate='done'; dur=rr+1-r; break   # arrived at this turn's start, then acted
                if any(k in (4,5,6,12,16,11) for k in ks): fate='cancel'; break
        out.append(dict(i=i,t=t,r=r,p0=p0,hp=c[5]/max(c[6],1),bag=c[7],fate=fate,dur=dur,dist=min(ch(p0,q) for q in spawn[t]),atspawn=near(t,p0)))
    return out,S
if __name__=='__main__':
    agg=collections.defaultdict(collections.Counter); games=collections.Counter(); crounds=collections.Counter()
    for f in sorted(glob.glob(os.path.join(sys.argv[1],'*.json'))):
        b=os.path.basename(f)[:-5]; lab,pos=b.rsplit('-',1); m=idx[(lab,int(pos))]; us=0 if m['our_side']=='a' else 1
        d=json.load(open(f)); ev,S=recall_events(d)
        team=[c[0] for c in S[0]['c']]
        for side,nm in ((us,'v4'),(1-us,'top10' if m['opponent']!='Pantheon' else 'Pantheon')):
            games[nm]+=1
            crounds[nm]+=sum(1 for s in S[1:] for i,c in enumerate(s['c']) if team[i]==side)
            for e in ev:
                if e['t']==side and not e['atspawn']: agg[nm][e['fate']]+=1; agg[nm]['starts']+=1; agg[nm]['done_rounds']+= (e['dur'] or 0) if e['fate']=='done' else 0
    for nm in ('v4','top10','Pantheon'):
        A=agg[nm]; g=games[nm]
        print(nm,'games',g,'starts/game %.1f'%(A['starts']/g),'done/game %.1f'%(A['done']/g),'completed share %.0f%%'%(100*A['done']/max(A['starts'],1)),
              'cancel %.1f died %.1f open %.1f'%(A['cancel']/g,A['died']/g,A['open']/g),'channel rounds of completed recalls share %.1f%%'%(100*A['done_rounds']/crounds[nm]))
