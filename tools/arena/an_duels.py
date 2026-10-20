"""Tables for tests 5/6 from results/arena-1020-duels.json (perspective: Prism in test 5, Heart of Ruin in test 6)."""
import json, collections, statistics as st, sys
d = json.load(open(sys.argv[1] if len(sys.argv) > 1 else 'results/arena-1020-duels.json'))
def rate(c): n = c['W'] + c['L']; return f"{c['W']}-{c['L']} ({100*c['W']/n:.0f}%)" if n else '-'
out = []
# test 5
T = collections.defaultdict(lambda: collections.Counter()); O = collections.defaultdict(lambda: collections.Counter()); C = collections.defaultdict(list); GF = collections.defaultdict(set)
for g in d['games']:
    if g['test'] != 5: continue
    opp = g['name'].split('_')[1]; gems = int(g['name'].split('_')[2][1:]); side = g['name'].split('_')[3][0]
    for x in g['duels']:
        if x['me'] != 'Prism': continue
        T[(opp, gems)][x['res']] += 1; O[(opp, gems, side)][x['res']] += 1; GF[gems].add((x['gems'], x['focus']))
    for c in g['casts']:
        if c['who'] == 'Prism': C[(gems, c['kind'])].append(c['dmg'])
out.append('T5 ' + json.dumps({f"{o}/g{gm}": {'all': rate(T[(o, gm)]), 'A': rate(O[(o, gm, 'A')]), 'B': rate(O[(o, gm, 'B')]), 'other': sum(v for k, v in T[(o, gm)].items() if k not in 'WL')} for o, gm in sorted(T)}))
out.append('T5gems ' + json.dumps({g: sorted(v) for g, v in GF.items()}))
out.append('T5casts ' + json.dumps({f"g{g}/{k}": {'n': len(v), 'median': st.median(v), 'max': max(v)} for (g, k), v in sorted(C.items())}))
# test 6
H = collections.defaultdict(lambda: collections.Counter()); HO = collections.defaultdict(lambda: collections.Counter())
for g in d['games']:
    if g['test'] != 6: continue
    opp = g['name'].split('_')[1]; side = g['name'].split('_')[2][0]
    for x in g['duels']:
        if x['me'] != 'HoR': continue
        H[opp][x['res']] += 1; HO[(opp, side)][x['res']] += 1
out.append('T6 ' + json.dumps({o: {'all': rate(H[o]), 'A': rate(HO[(o, 'A')]), 'B': rate(HO[(o, 'B')]), 'other': sum(v for k, v in H[o].items() if k not in 'WL')} for o in sorted(H)}))
# 2v2: a run ends when one team's survivors log W; cluster W lines by round
R = collections.Counter(); RS = collections.Counter(); runs = 0
for g in d['games']:
    if g['test'] != '2v2': continue
    hs_side = 0 if g['name'].split('_')[1][0] == 'A' else 1
    ws = sorted((x['r1'], x['side']) for x in g['duels'] if x['res'] == 'W')
    last = -99
    for r1, side in ws:
        if r1 - last <= 3: continue
        last = r1; win = 'HoR+Stellar' if side == hs_side else 'Ren+Varg'; R[win] += 1; RS[(win, 'A' if hs_side == 0 else 'B')] += 1
    other = [x for x in g['duels'] if x['res'] not in 'WL']
    R['other_lines'] += len(other)
out.append('2v2 ' + json.dumps({**R, 'by_side_of_HoR': {f"{k[0]}|HoR on {k[1]}": v for k, v in RS.items()}}))
print('\n'.join(out))
