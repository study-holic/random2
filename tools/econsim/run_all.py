"""Sweep policies x squad splits x bank per map -> results/econsim.json.  Usage: python run_all.py [map ...]"""
import sys, json, os, time
sys.path.insert(0, os.path.dirname(__file__))
from mapmodel import *
from sim import *

BANKS = [10, 20, 30, 40, 60, 80]

def splits(n):
    out = {'all': [n]}
    if n >= 2:
        out['half'] = [-(-n // 2), n // 2]
        out['pairs'] = [2] * (n // 2) + ([1] if n % 2 else [])
        out['solo'] = [1] * n
    seen = {}
    for k, v in out.items():
        seen.setdefault(tuple(v), k)
    return {k: v for k, v in out.items() if seen[tuple(v)] == k}

def combos(n):
    sp = splits(n); c = []
    c.append(('greedy', 'all')); c.append(('team', 'all'))
    for k in sp:
        c.append(('value', k)); c.append(('orient', k))
    for k in sp:
        if k in ('all', 'half', 'pairs'): c.append(('conveyor', k))
    return c

def best_over_bank(M, pol, sp, n, **kw):
    best = None
    for b in BANKS:
        r = Sim(M, P(policy=pol, split=splits(n)[sp], bank=b, **kw)).run()
        r['bank'] = b
        if best is None or r['gpr'] > best['gpr']: best = r
    return best

def run_map(name, D, mode='walk'):
    M = MapModel(name, D[name], mode); n = M.champs; out = {}
    for scen in ('all', 'half'):
        res = {}
        for pol, sp in combos(n):
            r = best_over_bank(M, pol, sp, n, scenario=scen)
            res[f'{pol}/{sp}'] = dict(gpr=r['gpr'], bank=r['bank'], share=r['share'], gold_pr=r['gold_pr'], tributes=r['tributes'])
        out[scen] = res
    return M, out

def summarize(res):
    pol_best = {}
    for k, v in res.items():
        pol = k.split('/')[0]
        if pol not in pol_best or v['gpr'] > pol_best[pol][1]['gpr']: pol_best[pol] = (k, v)
    return pol_best

if __name__ == '__main__':
    D = load_all(); names = sys.argv[1:] or list(D)
    R = {}
    if os.path.exists(os.path.join(ROOT, 'results', 'econsim.json')): R = json.load(open(os.path.join(ROOT, 'results', 'econsim.json')))
    for nm in names:
        t0 = time.time()
        M, out = run_map(nm, D)
        info = dict(champs=M.champs, groups=len(M.groups), unreach=[g['comp'] + str(g['anchor']) for g in M.groups if not g['walk']],
                    mono=[g['comp'] for g in M.groups if g['mono']], altars=len(M.my_altars))
        R[nm] = dict(info=info, walk=out)
        if info['unreach']:
            M2 = MapModel(nm, D[nm], 'crystal'); n = M2.champs; res = {}
            for pol, sp in combos(n):
                if pol in ('greedy', 'team', 'value', 'orient', 'conveyor'):
                    r = best_over_bank(M2, pol, sp, n, scenario='all'); res[f'{pol}/{sp}'] = dict(gpr=r['gpr'], bank=r['bank'], share=r['share'], gold_pr=r['gold_pr'], tributes=r['tributes'])
            R[nm]['crystal'] = {'all': res}
        print(nm, round(time.time() - t0, 1), 's', {p: round(v[1]['gpr'], 3) for p, v in summarize(out['all']).items()}, flush=True)
    os.makedirs(os.path.join(ROOT, 'results'), exist_ok=True)
    json.dump(R, open(os.path.join(ROOT, 'results', 'econsim.json'), 'w'), indent=1)
