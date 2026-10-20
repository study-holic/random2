import json,glob,csv,collections
TN={"0":"Ryth","1":"Vesper","2":"Vendetta","3":"C&I","4":"C&I","5":"Stellar","6":"Prism","7":"Marcella","8":"Willow","9":"Ren","10":"Varg","11":"Tavi"}
mp={r['match']:r for r in csv.DictReader(open('C:/cambc-solo/data/unranked/games_v3b.tsv'),delimiter='\t')}
per=collections.defaultdict(list); allc=collections.Counter(); orders=collections.defaultdict(collections.Counter)
for f in glob.glob('C:/cambc-solo/data/unranked/sumj/*.json'):
    d=json.load(open(f)); m=__import__('os').path.basename(f)[:-5]
    seq=[x['champion'] for x in d['roster']['draft'] if x['team']=='teamA']
    # a map may have fewer picks
    per[mp[m]['map']].append(tuple(seq)); allc.update(seq)
n=sum(len(v) for v in per.values()); print("games",n)
print("pick frequency over 160 games:",{k:f"{v} ({100*v/n:.0f}%)" for k,v in allc.most_common()})
for k,v in sorted(per.items()):
    c=collections.Counter(v); print(k,len(v),dict((' > '.join(a),b) for a,b in c.most_common(3)))
print("pick-position distribution (pos1..):")
pos=collections.defaultdict(collections.Counter)
for v in per.values():
    for s in v:
        for i,x in enumerate(s): pos[i][x]+=1
for i in sorted(pos): print(i+1,dict(pos[i].most_common()))
# top10 core
sd=json.load(open('C:/cambc-solo/data/top10/sides.json'))
t=[s for s in sd if s['team']!='xanarchy']; c=collections.Counter()
for s in t:
    for x in set(TN[str(a)] for a in s["types"]): c[x]+=1
print("top-side(all 266 non-us) champion presence:",{k:f"{100*v/len(t):.0f}%" for k,v in c.most_common()}, len(t))
