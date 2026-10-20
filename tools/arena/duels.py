"""Arena duel driver (tests 5 and 6 of tools/cloud/arena-tests.md): writes duel maps, runs bots/arena-duel mirror games,
parses D (duel) and S (cast) lines into results/arena-1020-duels.json.
python3 tools/arena/duels.py [--jobs 3] [--games5 3] [--games6 2] [--games2v2 3]"""
import argparse, glob, json, os, re, subprocess, time, zipfile, collections
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PICK = {'ryth': 0, 'vesper': 1, 'vendetta': 2, 'ci': 3, 'stellar': 4, 'prism': 5, 'marcella': 6, 'willow': 7, 'ren': 8, 'varg': 9, 'tavi': 10, 'hor': 11}
POL = 1 << 15   # P_ADAPT: the w14 lab's per-champion policy table (+ HoR: shield on engage, dash-in)
D = re.compile(r"D (\d) (\d) (\d+) (\d+) (\S+) (\S+) ([WLEXT]) (-?\d+) (\d+) (-?\d+) (\d+) g(\d+) f(\d+)")
S = re.compile(r"S (\d+) (\S+) (\d) (\w+) (-?\d+) (\d+) (\d+)")

def specs(a):
    POL = a.pol
    out = []
    for opp in ('stellar', 'ren', 'varg'):
        for gems in (0, 20, 40):
            for side in 'AB':
                for g in range(a.games5):
                    pa, pb = (['prism'], [opp]) if side == 'A' else ([opp], ['prism'])
                    out.append({'test': 5, 'name': f't5_{opp}_g{gems}_{side}{g}', 'champs': 1, 'picksA': [PICK[x] for x in pa], 'picksB': [PICK[x] for x in pb],
                                'polA': POL, 'polB': POL, 'gemsA': gems if side == 'A' else 0, 'gemsB': gems if side == 'B' else 0, 'jit': g})
    for opp in [k for k in PICK if k != 'hor']:
        for side in 'AB':
            for g in range(a.games6):
                pa, pb = (['hor'], [opp]) if side == 'A' else ([opp], ['hor'])
                out.append({'test': 6, 'name': f't6_{opp}_{side}{g}', 'champs': 1, 'picksA': [PICK[x] for x in pa], 'picksB': [PICK[x] for x in pb], 'polA': POL, 'polB': POL, 'jit': g})
    for side in 'AB':
        for g in range(a.games2v2):
            hs, rv = [PICK['hor'], PICK['stellar']], [PICK['ren'], PICK['varg']]
            pa, pb = (hs, rv) if side == 'A' else (rv, hs)
            out.append({'test': '2v2', 'name': f't2v2_{side}{g}', 'champs': 2, 'picksA': pa, 'picksB': pb, 'polA': POL, 'polB': POL,
                        'groupsA': [0, 0], 'groupsB': [0, 0], 'jit': g})
    return out

def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--jobs', type=int, default=3); ap.add_argument('--games5', type=int, default=3)
    ap.add_argument('--games6', type=int, default=2); ap.add_argument('--games2v2', type=int, default=3); ap.add_argument('--only'); ap.add_argument('--pol', type=int, default=POL | 1, help='policy bits for both teams (default P_ADAPT | P_NORETREAT)'); ap.add_argument('--out', default='results/arena-1020-duels.json')
    a = ap.parse_args()
    sp = specs(a)
    if a.only: sp = [s for s in sp if s['name'].startswith(a.only)]
    work = os.path.join(ROOT, 'matches', 'arena-duels'); os.makedirs(work, exist_ok=True)
    mapdir = os.path.join(ROOT, 'maps-arena', 'duel'); js = os.path.join(work, 'specs.json'); json.dump(sp, open(js, 'w'))
    subprocess.check_call(['node', os.path.join(ROOT, 'tools', 'arena', 'mkduel.mjs'), js, mapdir], stderr=subprocess.DEVNULL)
    B = os.path.join(ROOT, 'bots', 'arena-duel')
    for s_ in sp:
        f = f"{work}/{s_['name']}.repl26a"
        if os.path.exists(f): os.remove(f)
    man = os.path.join(work, 'manifest.txt')
    open(man, 'w').write(''.join(f"{B} {B} {mapdir}/{s['name']}.map26a {work}/{s['name']}.repl26a\n" for s in sp))
    t0 = time.time()
    with open(os.path.join(work, 'batch.log'), 'w') as log:
        rc = subprocess.call(['cambc', 'run-batch', '--release', '--jobs', str(a.jobs), '--timeout', '600', man], cwd=ROOT, stdout=log, stderr=subprocess.STDOUT)
    secs = time.time() - t0
    games = []
    for s in sp:
        rec = {**s, 'duels': [], 'casts': [], 'errors': 0}
        try:
            z = zipfile.ZipFile(f"{work}/{s['name']}.repl26a")
            for side, fn in ((0, 'team-a.stdout'), (1, 'team-b.stdout')):
                txt = z.read(fn).decode('utf8', 'replace').replace('\x00', '')
                rec['errors'] += txt.count('bot error')
                for m in D.finditer(txt):
                    c, sd, r0, r1, me, foe, res, hp, mhp, fhp, pol, g, f = m.groups()
                    rec['duels'].append({'side': side, 'r0': int(r0), 'r1': int(r1), 'me': me, 'foe': foe, 'res': res, 'hp': int(hp), 'mhp': int(mhp), 'fhp': int(fhp), 'gems': int(g), 'focus': int(f)})
                for m in S.finditer(txt):
                    r, who, slot, kind, dmg, g, f = m.groups()
                    rec['casts'].append({'side': side, 'r': int(r), 'who': who, 'slot': int(slot), 'kind': kind, 'dmg': int(dmg), 'gems': int(g), 'focus': int(f)})
        except Exception as e:
            rec['error'] = str(e)
        games.append(rec)
    out = os.path.join(ROOT, a.out)
    json.dump({'date': time.strftime('%Y-%m-%d %H:%M'), 'seconds': round(secs, 1), 'jobs': a.jobs, 'rc': rc, 'policy': a.pol, 'games': games}, open(out, 'w'))
    print(f"{len(sp)} games in {secs:.0f} s (jobs {a.jobs}), rc {rc}, duel lines {sum(len(g['duels']) for g in games)}, errors {sum(g['errors'] for g in games)} -> {out}")

if __name__ == '__main__':
    main()
