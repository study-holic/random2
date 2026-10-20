"""Per-map camp economics from decoded replays (extract_pilot.mjs output): camp groups (cardinally connected camps),
gems/gold/HP/strength per group, respawn (longest member), distance to each team's spawn, and the map's gem supply.
usage: python tools/v3/map_econ.py data/maps/camps0.jsonl > notes/map-economics.md (lines from tools/v3/camps0.mjs)"""
import json, sys, os, collections as C
T = {0: ('wolf', 1350, 20, 90, 22, 40), 1: ('elk', 2400, 60, 160, 36, 60), 2: ('gem', 500, 0, 0, 24, 50),
     3: ('gold', 500, 0, 35, 0, 50), 4: ('monolith', 20000, 200, 300, 200, 150)}  # GameRules monster table
def groups(camps):
    pos = {(x, y): i for i, (t, x, y) in enumerate(camps)}
    seen, out = set(), []
    for i in range(len(camps)):
        if i in seen: continue
        stack, g = [i], []
        seen.add(i)
        while stack:
            j = stack.pop(); g.append(j); _, x, y = camps[j]
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                k = pos.get((x + dx, y + dy))
                if k is not None and k not in seen: seen.add(k); stack.append(k)
        out.append(g)
    return out
print('# Map economics (camp groups from decoded replays; GameRules monster table)\n')
print('Gem rate = sum over gem-bearing groups of gems / (respawn + 5 capture rounds): the most a map can yield per round if every group is taken on cooldown by one team. Side = Chebyshev distance to spawn A minus to spawn B (negative = nearer A).\n')
summary = []
for line in open(sys.argv[1]):
    d = json.loads(line); name = d['file'].split('__')[0]
    r0 = [[c[0], c[1], 1, c[2], c[3]] for c in d['champs']]
    sp = {t: [(c[3], c[4]) for c in r0 if c[0] == t] for t in (0, 1)}
    sA, sB = sp[0][0], sp[1][0]
    cheb = lambda a, b: max(abs(a[0] - b[0]), abs(a[1] - b[1]))
    camps = d['camps']; gs = groups(camps)
    rows = []
    for g in gs:
        ty = C.Counter(camps[j][0] for j in g)
        gems = sum(T[camps[j][0]][4] for j in g); gold = sum(T[camps[j][0]][3] for j in g)
        hp = sum(T[camps[j][0]][1] for j in g); st = sum(T[camps[j][0]][2] for j in g)
        resp = max(T[camps[j][0]][5] for j in g)
        cx = sum(camps[j][1] for j in g) / len(g); cy = sum(camps[j][2] for j in g) / len(g)
        side = cheb((cx, cy), sA) - cheb((cx, cy), sB)
        rows.append(dict(comp='+'.join(f"{n}{T[t][0]}" for t, n in sorted(ty.items())), gems=gems, gold=gold, hp=hp, st=st, resp=resp,
                         pos=(round(cx), round(cy)), side=round(side), dA=round(cheb((cx, cy), sA))))
    rows.sort(key=lambda r: -r['gems'])
    rate = sum(r['gems'] / (r['resp'] + 5) for r in rows if r['gems'])
    per_hp = sum(r['gems'] for r in rows) / max(1, sum(r['hp'] for r in rows if r['gems'])) * 1000
    team = len([c for c in r0 if c[0] == 0])
    summary.append((name, len(gs), sum(1 for r in rows if r['gems']), sum(r['gems'] for r in rows), rate, team))
    print(f"## {name}: {len(camps)} camps in {len(gs)} groups; bodies per team {team}; spawns A {sA} B {sB}; gem supply {rate:.2f} gems/round; {per_hp:.1f} gems per 1,000 HP of gem-bearing camps\n")
    print('| group | gems | gold | HP | strength | respawn | centre | side | dist A |'); print('|---|---|---|---|---|---|---|---|---|')
    for r in rows: print(f"| {r['comp']} | {r['gems']} | {r['gold']} | {r['hp']} | {r['st']} | {r['resp']} | {r['pos']} | {r['side']} | {r['dA']} |")
    print()
print('## Summary\n\n| map | groups | gem groups | gems on map | gem supply / round | bodies |\n|---|---|---|---|---|---|')
for s in sorted(summary, key=lambda s: s[4]): print(f"| {s[0]} | {s[1]} | {s[2]} | {s[3]} | {s[4]:.2f} | {s[5]} |")
