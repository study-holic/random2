"""Regenerates notes/map-economics.md, data/maps21-table.md and data/maps-21.png from data/maps21.json
(node tools/maps21/dump.mjs <engine v1_0_0 mapeditor wasm> data/maps21.json maps/*.map26a).
Group rewards are the engine's own per-group figures [gems, gold, respawn, camps]; HP/strength from camp designs (engine 1.0)."""
import json, collections as C
import matplotlib; matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from matplotlib.lines import Line2D
D = json.load(open('data/maps21.json')); OLD = json.load(open('data/maps-engine0.json'))
HP = {0: 1350, 1: 2400, 2: 120, 3: 120, 4: 20000, 5: 5000}; ST = {0: 20, 1: 60, 2: 0, 3: 0, 4: 200, 5: 120}
def mirror_all(DD):
    # the map file stores camps and groups for team A's half only; the engine mirrors them by the map symmetry
    for m in DD.values():
        w, h, sy = m['w'], m['h'], m['sym']
        mir = (lambda x, y: (w - 1 - x, h - 1 - y)) if sy == 2 else (lambda x, y: (w - 1 - x, y)) if sy == 0 else (lambda x, y: (x, h - 1 - y))
        G = []
        for g in m['grp']:
            G.append(g); mc = [[t, *mir(x, y)] for t, x, y in g['camps']]
            if sorted(map(tuple, mc)) != sorted(map(tuple, g['camps'])): G.append({'info': g['info'], 'camps': mc})
        m['grp'] = G; m['camps'] = [c for g in G for c in g['camps']]
mirror_all(D)
for m in OLD.values():
    w, h, sy = m['w'], m['h'], m['sym']
    mir = (lambda x, y: (w - 1 - x, h - 1 - y)) if sy == 2 else (lambda x, y: (w - 1 - x, y)) if sy == 0 else (lambda x, y: (x, h - 1 - y))
    cs = {tuple(c) for c in m['camps']}; cs |= {(t, *mir(x, y)) for t, x, y in m['camps']}; m['camps'] = [list(c) for c in cs]
NM = {0: 'wolf', 1: 'elk', 2: 'gem', 3: 'gold', 4: 'monolith', 5: 'golem'}
SYM = {0: 'horizontal', 1: 'vertical', 2: 'rot180'}
NEW = 'big_world boss_battle broken_curve concentrated_wealth corridors elk_arena fracture gold_rush hooks islands'.split()
def spawnB(m):
    w, h, s = m['w'], m['h'], m['sym']; x0, y0, x1, y1 = m['spawnA']
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    if s == 2: return (w - 1 - cx, h - 1 - cy)
    if s == 0: return (w - 1 - cx, cy)
    return (cx, h - 1 - cy)
cheb = lambda a, b: max(abs(a[0] - b[0]), abs(a[1] - b[1]))
def rows(m):
    sA = ((m['spawnA'][0] + m['spawnA'][2]) / 2, (m['spawnA'][1] + m['spawnA'][3]) / 2); sB = spawnB(m); out = []
    for g in m['grp']:
        gems, gold, resp, n = g['info']; ty = C.Counter(c[0] for c in g['camps'])
        cx = sum(c[1] for c in g['camps']) / max(1, n); cy = sum(c[2] for c in g['camps']) / max(1, n)
        out.append(dict(comp='+'.join(f"{k}{NM[t]}" for t, k in sorted(ty.items())), gems=gems, gold=gold, resp=resp,
            hp=sum(HP[c[0]] for c in g['camps']), st=sum(ST[c[0]] for c in g['camps']), pos=(round(cx), round(cy)),
            side=round(cheb((cx, cy), sA) - cheb((cx, cy), sB)), dA=round(cheb((cx, cy), sA))))
    return sorted(out, key=lambda r: -r['gems'])
summ = []
L = ['# Map economics, 21 current maps (engine 1.0.0)\n',
     "Regenerated 20 Oct by tools/maps21/econ21.py from the official map files (data/maps21.json). Group gems, gold and respawn are the engine's own per-group figures (they include any per-group reward multiplier the map sets); HP and strength use engine-1.0 camp designs (gem and gold clusters 120 HP). Spawn B is the mirror of spawn A (approximate); side = Chebyshev distance to A minus to B (negative = nearer A). The previous 17-map tables (engine 0.0.3, site-doc rewards) are in notes/map-economics-engine0.md.\n",
     'Gem rate = sum over gem-bearing groups of gems / (respawn + 5 capture rounds).\n']
for n, m in D.items():
    r = rows(m); rate = sum(x['gems'] / (x['resp'] + 5) for x in r if x['gems']); gh = sum(x['hp'] for x in r if x['gems'])
    perhp = sum(x['gems'] for x in r) / max(1, gh) * 1000
    summ.append((n, len(r), sum(1 for x in r if x['gems']), sum(x['gems'] for x in r), rate, m['champs'], sum(x['gold'] for x in r)))
    L.append(f"## {n}: {len(m['camps'])} camps in {len(r)} groups; bodies per team {m['champs']}; spawn A area {tuple(m['spawnA'])}; gem supply {rate:.2f} gems/round; {perhp:.1f} gems per 1,000 HP of gem-bearing camps\n")
    L.append('| group | gems | gold | HP | strength | respawn | centre | side | dist A |'); L.append('|---|---|---|---|---|---|---|---|---|')
    for x in r: L.append(f"| {x['comp']} | {x['gems']} | {x['gold']} | {x['hp']} | {x['st']} | {x['resp']} | {x['pos']} | {x['side']} | {x['dA']} |")
    L.append('')
L.append('## Summary\n\n| map | groups | gem groups | gems on map | gold on map | gem supply / round | bodies |\n|---|---|---|---|---|---|---|')
for s in sorted(summ, key=lambda s: s[4]): L.append(f"| {s[0]} | {s[1]} | {s[2]} | {s[3]} | {s[6]} | {s[4]:.2f} | {s[5]} |")
open('notes/map-economics.md', 'w', encoding='utf8').write('\n'.join(L) + '\n')
T = ['| map | size | sym | champs/side | camps wolf/elk/gem/gold/monolith/golem | groups | altars | change vs engine 0 |', '|---|---|---|---|---|---|---|---|']
for n, m in D.items():
    c = C.Counter(k[0] for k in m['camps']); o = OLD.get(n); ch = 'new map' if n in NEW else ''
    if o and not ch:
        oc = C.Counter(k[0] for k in o['camps'])
        ch = ('terrain identical; camps ' + '/'.join(str(oc[t]) for t in range(6)) + ' -> ' + '/'.join(str(c[t]) for t in range(6))) if oc != c else 'identical'
        if m['walls'] != o['walls'] or m['holes'] != o['holes']: ch = 'TERRAIN CHANGED; ' + ch
    if n in NEW and o and o['camps'] != m['camps']: ch = 'new map (differs from maps-engine0 copy)'
    T.append(f"| {n} | {m['w']}x{m['h']} | {SYM[m['sym']]} | {m['champs']} | {'/'.join(str(c[t]) for t in range(6))} | {len(m['grp'])} | {len(m['altars'])} | {ch} |")
open('data/maps21-table.md', 'w').write('\n'.join(T) + '\n')
col = {0: '#d95f02', 1: '#8c510a', 2: '#1b9e77', 3: '#e6ab02', 4: '#7570b3', 5: '#e7298a'}
fig, axs = plt.subplots(3, 7, figsize=(28, 13.5))
for ax, (n, m) in zip(axs.flat, D.items()):
    w, h = m['w'], m['h']; ax.set_xlim(-1, w); ax.set_ylim(h, -1); ax.set_aspect('equal'); ax.set_xticks([]); ax.set_yticks([])
    ax.add_patch(plt.Rectangle((-.5, -.5), w, h, fc='#f4f1e8', ec='#888', lw=.8))
    for k, c in (('holes', '#9fd0e8'), ('bushes', '#b9dba4'), ('walls', '#303030')):
        if m[k]: ax.scatter(*zip(*m[k]), s=max(1, 900 / max(w, h)) * (2.2 if k != 'bushes' else 1.2), marker='s', c=c, linewidths=0)
    for t in range(6):
        pts = [(x, y) for tt, x, y in m['camps'] if tt == t]
        if pts: ax.scatter(*zip(*pts), s=max(6, 2400 / max(w, h)), c=col[t], marker='o', edgecolors='k', linewidths=.3)
    if m['altars']: ax.scatter(*zip(*m['altars']), s=70, marker='*', c='#e41a1c', edgecolors='k', linewidths=.4, zorder=5)
    x0, y0, x1, y1 = m['spawnA']; ax.add_patch(plt.Rectangle((x0 - .5, y0 - .5), x1 - x0 + 1, y1 - y0 + 1, fc='#377eb8', ec='k', lw=.6))
    bx, by = spawnB(m); ax.add_patch(plt.Rectangle((bx - (x1 - x0) / 2 - .5, by - (y1 - y0) / 2 - .5), x1 - x0 + 1, y1 - y0 + 1, fc='#e41a1c', ec='k', lw=.6, alpha=.8))
    ax.set_title(f"{n}  {w}x{h}  {m['champs']} per side", fontsize=11)
axs.flat[-1].axis('off')
leg = [Patch(fc='#303030', label='wall'), Patch(fc='#9fd0e8', label='hole'), Patch(fc='#b9dba4', label='bush')] + \
      [Line2D([], [], marker='o', ls='', mfc=col[t], mec='k', label=NM[t] + (' (none on any map)' if t == 5 else '')) for t in range(6)] + \
      [Line2D([], [], marker='*', ls='', mfc='#e41a1c', mec='k', ms=12, label='altar'), Patch(fc='#377eb8', label='spawn A'), Patch(fc='#e41a1c', label='spawn B (mirrored)')]
fig.legend(handles=leg, loc='lower right', bbox_to_anchor=(0.99, 0.03), ncol=2, fontsize=13, frameon=True)
fig.suptitle('CAMBC 2026 map pool, 21 maps (engine 1.0.0, 20 Oct)', fontsize=18); fig.tight_layout(rect=(0, 0, 1, .97))
fig.savefig('data/maps-21.png', dpi=60)
