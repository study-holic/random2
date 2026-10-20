"""Deaths by cause for our side of a mapgate head-to-head (the step0 method, notes/step0-deaths.md, cloud paths).
python3 tools/step0/h2h_deaths.py <mapgate.json> [--jobs 3] [--cache matches/decoded]
Champion kill = an enemy champion action damaged the victim in the death round or the one before; else monster = a
wolf/elk/monolith camp within 8 tiles; else other. Cassi/Issac partner deaths (same round) take the first body's cause.
Location: spawn (8 tiles of the team's round-1 positions), camp (8 tiles of any camp), elsewhere. Decodes with
tools/top10/extract_pilot.mjs (CAMBC_ENGINE_CACHE = the cambc visualiser assets)."""
import argparse, collections, glob, json, os, subprocess, sys
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ch = lambda a, b, c, d: max(abs(a - c), abs(b - d))

MAPS = json.load(open(os.path.join(ROOT, 'data', 'maps21.json')))
def map_camps(name):
    """All camps of a pool map as (type, x, y): the canonical half from data/maps21.json plus its mirror image
    (sym 0 = x mirror, 1 = y mirror, 2 = rotation). The decoder's final camp list puts dead camps at (-1, -1)."""
    m = MAPS[name]; W, H = m['w'], m['h']; out = set()
    for t, x, y in m['camps']:
        out.add((t, x, y))
        out.add((t, W - 1 - x, y) if m['sym'] == 0 else (t, x, H - 1 - y) if m['sym'] == 1 else (t, W - 1 - x, H - 1 - y))
    return sorted(out)

def classify(path, us, mapname=None):
    d = json.load(open(path)); S = d['rounds']
    if mapname in MAPS: d['camps'] = [list(c) for c in map_camps(mapname)]
    team = [c[0] for c in S[0]['c']]; typ = [c[1] for c in S[0]['c']]; n = len(team)
    start = {t: [(c[3], c[4]) for c in S[0]['c'] if c[0] == t] for t in (0, 1)}
    camps = [(x, y) for t, x, y in d['camps']]; mons = [(x, y) for t, x, y in d['camps'] if t in (0, 1, 4)]
    hits = collections.defaultdict(set)   # round -> (attacker team, victim index)
    for a in d['actions']:
        for h in a[9]:
            if h[0] == 'c': hits[a[0]].add((team[a[1]] if a[1] < n else -1, h[1]))
    out = []
    for a, b in zip(S, S[1:]):
        died = [i for i in range(n) if a['c'][i][2] and not b['c'][i][2] and team[i] == us]
        first = {}
        for i in died:
            x, y = a['c'][i][3], a['c'][i][4]
            champ = any((1 - us, i) in hits[r] for r in (b['r'], b['r'] - 1, a['r']))
            cause = 'champion' if champ else ('monster' if any(ch(x, y, cx, cy) <= 8 for cx, cy in mons) else 'other')
            if typ[i] in (3, 4):
                if 'ci' in first: cause = first['ci']
                else: first['ci'] = cause
            loc = 'spawn' if min(ch(x, y, sx, sy) for sx, sy in start[us]) <= 8 else ('camp' if any(ch(x, y, cx, cy) <= 8 for cx, cy in camps) else 'elsewhere')
            out.append((b['r'], typ[i], x, y, cause, loc, a['c'][i][7]))
    return out

def main():
    ap = argparse.ArgumentParser(); ap.add_argument('gate'); ap.add_argument('--jobs', type=int, default=3); ap.add_argument('--cache', default=os.path.join(ROOT, 'matches', 'decoded'))
    ap.add_argument('--out'); ap.add_argument('--them', action='store_true', help='classify the opponent side instead of ours'); a = ap.parse_args()
    G = json.load(open(a.gate)); os.makedirs(a.cache, exist_ok=True)
    env = dict(os.environ); env.setdefault('CAMBC_ENGINE_CACHE', glob.glob(os.path.expanduser('~/.cache/cambc/assets/*/visualiser/assets'))[0])
    todo = []
    for r in G['results']:
        key = os.path.basename(os.path.dirname(r['replay'])) + '__' + os.path.basename(r['replay']).replace('.repl26a', '')
        r['_dec'] = os.path.join(a.cache, key + '.json')
        if not os.path.exists(r['_dec']): todo.append(r)
    procs = []
    for r in todo:   # extract_pilot names its output after the input file, so decode into a per-replay temp dir
        tmp = r['_dec'] + '.d'
        procs.append((subprocess.Popen(['node', os.path.join(ROOT, 'tools', 'top10', 'extract_pilot.mjs'), r['replay'], '--out', tmp, '--stride', '1'], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL), r, tmp))
        while sum(p.poll() is None for p, _, _ in procs) >= a.jobs:
            import time; time.sleep(0.2)
    for p, r, tmp in procs:
        p.wait(); fs = glob.glob(os.path.join(tmp, '*.json'))
        if fs: os.replace(fs[0], r['_dec'])
        try: os.rmdir(tmp)
        except OSError: pass
    tot = collections.Counter(); bymap = collections.defaultdict(collections.Counter); games = 0; rows = []
    for r in G['results']:
        if not os.path.exists(r['_dec']): continue
        us = 0 if r['side'] == 'A' else 1
        if a.them: us = 1 - us
        ds = classify(r['_dec'], us, r['map']); games += 1
        for dd in ds: tot[dd[4]] += 1; tot['loc_' + dd[5]] += 1; bymap[r['map']][dd[4]] += 1; tot['gems_lost_monster'] += dd[6] if dd[4] == 'monster' else 0
        bymap[r['map']]['games'] += 1; rows.append({'map': r['map'], 'side': r['side'], 'salt': r.get('salt'), 'deaths': ds})
    summ = {'games': games, 'per_game': {k: round(v / max(1, games), 2) for k, v in tot.items()},
            'by_map_monster_per_game': {m: round(c['monster'] / c['games'], 1) for m, c in sorted(bymap.items())}}
    print(json.dumps(summ))
    if a.out: json.dump({'summary': summ, 'games': rows}, open(a.out, 'w'))

if __name__ == '__main__':
    main()
