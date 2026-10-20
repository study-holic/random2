"""Re-run the sweep ('all' scenario, walk) under changed assumptions -> results/econsim-sens.json"""
import sys, json, os, time
sys.path.insert(0, os.path.dirname(__file__))
from mapmodel import *
from sim import *
from run_all import combos, splits, BANKS
VAR = {'base': {}, 'dpr30': dict(dpr=30), 'dpr80': dict(dpr=80), 'budget300': dict(hp_budget=300), 'budget1500': dict(hp_budget=1500),
       'mono_ok': dict(allow_mono=True, hp_budget=4000), 'gold0.1': dict(gw=0.1), 'recall': dict(recall=True)}
if __name__ == '__main__':
    D = load_all(); names = sys.argv[1:] or list(D); R = {}
    for nm in names:
        M = MapModel(nm, D[nm]); n = M.champs; R[nm] = {}
        for vn, kw in VAR.items():
            best = {}
            for pol, sp in combos(n):
                for b in BANKS:
                    r = Sim(M, P(policy=pol, split=splits(n)[sp], bank=b, scenario='all', **kw)).run()
                    if pol not in best or r['gpr'] > best[pol][0]: best[pol] = (r['gpr'], f'{pol}/{sp}', b)
            R[nm][vn] = best
        print(nm, {v: max(R[nm][v].values())[1:] + (round(max(R[nm][v].values())[0], 3),) for v in VAR}, flush=True)
    json.dump(R, open(os.path.join(ROOT, 'results', 'econsim-sens.json'), 'w'), indent=1)
