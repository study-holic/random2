"""w12: why lone fights are lost. For each of our deaths with no ally within 4 at fight start (w12-deaths.py rows in
results/w12-engage.json), sum over the fight window [fight start - 2, death]: damage the victim dealt to enemy champions,
damage it took from enemy champions, its ability casts, the enemy's casts, and the max-health / item-count comparison
with the enemies that hit it. usage: py -I results/w12-duels.py"""
import json, collections
T = ['Ryth', 'Vesper', 'Vendetta', 'Cassi', 'Issac', 'Stellar', 'Prism', 'Marcella', 'Willow', 'Ren', 'Varg', 'Tavi']
E = json.load(open('results/w12-engage.json'))
tot = collections.Counter(); rows = []
for gid, g in E.items():
    if gid == 'all' or not g['deaths']: continue
    d = json.load(open(f'data/ladder/decoded/{gid}.json', encoding='utf8'))
    us = g['our_team']
    snaps = {}
    for s in d['rounds']: snaps.setdefault(s['r'], s)
    c0 = snaps[min(snaps)]['c']; team = [c[0] for c in c0]
    for w in g['deaths']:
        if w['al4'] != 0 or w['fs'] is None: continue
        rd, fs = w['r'], w['fs']
        s0 = snaps.get(fs, snaps[rd - 1])['c']
        vic = [i for i in range(len(c0)) if team[i] == us and c0[i][1] != 99 and s0[i][3] == w['x'] and s0[i][4] == w['y'] and s0[i][2]]
        if not vic: continue
        v = vic[0]
        dealt = taken = vcast = ecast = vatk = eatk = 0; hitters = set()
        for a in d['actions']:
            if a[0] < fs - 2 or a[0] > rd: continue
            actor = a[1]
            if actor == v:
                dd = sum(h[2] for h in a[9] if h[0] == 'c' and team[h[1]] != us)
                dealt += dd
                if a[10] >= 0: vcast += 1
                elif dd > 0: vatk += 1
            elif team[actor] != us:
                dt = sum(h[2] for h in a[9] if h[0] == 'c' and h[1] == v)
                if dt > 0:
                    taken += dt; hitters.add(actor)
                    if a[10] >= 0: ecast += 1
                    else: eatk += 1
        hs = list(hitters)
        e_mhp = sum(s0[h][6] for h in hs) / max(1, len(hs)); e_items = sum(s0[h][11] for h in hs) / max(1, len(hs))
        rows.append(dict(game=gid[:8], r=rd, champ=w['champ'], hp0=w['hp0'], mhp=s0[v][6], items=s0[v][11], dealt=dealt, taken=taken,
                         vatk=vatk, vcast=vcast, eatk=eatk, ecast=ecast, n_hitters=len(hs), hitters=[T[c0[h][1]] for h in hs],
                         e_mhp=round(e_mhp), e_items=round(e_items, 1), rounds=rd - fs))
print('game     round champ    hp0  mhp it | dealt taken | vAtk vCast | eAtk eCast nHit | e_mhp e_it rnds hitters')
for r in rows:
    print(f"{r['game']} {r['r']:5} {r['champ']:8} {r['hp0']:5} {r['mhp']:5} {r['items']:2} | {r['dealt']:5} {r['taken']:5} | {r['vatk']:4} {r['vcast']:5} | {r['eatk']:4} {r['ecast']:5} {r['n_hitters']:4} | {r['e_mhp']:5} {r['e_items']:4} {r['rounds']:4} {','.join(r['hitters'])}")
n = len(rows)
S = lambda k: sum(r[k] for r in rows)
print(f"n={n} dealt={S('dealt')} taken={S('taken')} ratio={S('taken')/max(1,S('dealt')):.2f} victim basic hits={S('vatk')} victim casts={S('vcast')} "
      f"enemy basic hits={S('eatk')} enemy casts={S('ecast')} multi-hitter fights={sum(r['n_hitters']>1 for r in rows)} "
      f"zero-cast victims={sum(r['vcast']==0 for r in rows)} victim items mean={S('items')/max(1,n):.1f} enemy items mean={sum(r['e_items'] for r in rows)/max(1,n):.1f} "
      f"victim maxhp mean={S('mhp')/max(1,n):.0f} enemy maxhp mean={sum(r['e_mhp'] for r in rows)/max(1,n):.0f}")
one = [r for r in rows if r['n_hitters'] == 1]
print(f"single-hitter fights n={len(one)} dealt={sum(r['dealt'] for r in one)} taken={sum(r['taken'] for r in one)} vcast={sum(r['vcast'] for r in one)} ecast={sum(r['ecast'] for r in one)} vatk={sum(r['vatk'] for r in one)} eatk={sum(r['eatk'] for r in one)}")
json.dump(rows, open('results/w12-duels.json', 'w'), indent=0)
