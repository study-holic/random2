import json, collections, sys
rows = [json.loads(l) for l in open('C:/cambc-solo/data/v3/fights.jsonl', encoding='utf8')]
print('fights', len(rows), 'games', len(set(r['game'] for r in rows)))
src = collections.Counter()
for r in rows:
    ours = 'xanarchy' in (r['teams'] or [])
    src[(r['src'], ours)] += 1
print(src)
nex = 0
cat = collections.Counter()
for r in rows:
    d0, d1 = r['dmg']['0'], r['dmg']['1']
    ex = d0 > 0 and d1 > 0
    nd = len(r['deaths']['0']) + len(r['deaths']['1'])
    n0, n1 = len(r['parts']['0']), len(r['parts']['1'])
    cat[(ex, nd > 0)] += 1
print('(exchange, any death):', cat)
sz = collections.Counter()
for r in rows:
    n0, n1 = len(r['parts']['0']), len(r['parts']['1'])
    sz[(min(n0, n1), max(n0, n1))] += 1
print(sorted(sz.items(), key=lambda x: -x[1])[:15])
print('events per fight', collections.Counter(min(r['nev'], 10) for r in rows))
dur = collections.Counter(min(r['dur'], 12) for r in rows)
print('dur', sorted(dur.items()))
# deaths totals
td = sum(len(r['deaths']['0']) + len(r['deaths']['1']) for r in rows)
print('total deaths in fights', td)
# fights by map
print(collections.Counter(r['map'] for r in rows).most_common(20))
# start round 1-3 spawn rushes
print('r0<=3', sum(1 for r in rows if r['r0'] <= 3))
print('monster hits >0', sum(1 for r in rows if r['mon_hits'] > 0), 'camp_d<=4', sum(1 for r in rows if r['camp_d'] <= 4))
