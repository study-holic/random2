import json,glob,sys,os
rows=[]
for f in sorted(glob.glob(sys.argv[1]+'/*.json')):
    try: d=json.load(open(f))
    except Exception: continue
    m=os.path.basename(f).replace('.repl26a.json','')
    mapn,side=m.rsplit('-',1)
    a,b=('v11','v12') if side=='A11' else ('v12','v11')
    g={t['team']:int(t['submittedGems']) for t in d['teams']}
    out=d['match']['outcome']; rounds=d['match']['roundsStarted']
    winner=a if out=='teamA' else b if out=='teamB' else out
    ch={}
    for c in d['champions']:
        t=a if c['team']=='teamA' else b
        ch.setdefault(t,[]).append(f"{c['name'][:4]}:d{c['metrics']['deaths']}")
    rows.append((mapn,side,a,b,g['teamA'],g['teamB'],rounds,winner,' '.join(ch.get('v11',[])),' '.join(ch.get('v12',[]))))
w={'v11':0,'v12':0}
for r in rows:
    print(f"{r[0]:12} {r[2]}={r[4]:4} {r[3]}={r[5]:4} r{r[6]:4} -> {r[7]:4} | v11 {r[8]} | v12 {r[9]}")
    if r[7] in w: w[r[7]]+=1
print(w)
