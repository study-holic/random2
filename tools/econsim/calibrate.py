"""Calibration hook: real gems tributed per round per map vs econsim.
Sources:
  1. data/v3/decoded/econ.jsonl (decoded ladder/local games: sg = tributed gems per team, rounds)   [default]
  2. `cambc matches summary <uuid> --json -o f.json` files (pass with --summary f.json ...); the schema is not
     documented here, so the reader looks for per-team tributed totals under keys containing 'tribut' / 'score'
     and a round count under 'rounds' / 'round'.
Usage: python calibrate.py [--since 2026-10-20] [--cohort ours|top10|micron] [--summary f.json ...]
Prints a table map | n | real gems/round (mean, median of winners' side) | sim best | sim greedy | ratio.
Check: after v4 base exists, run its local games, summarise them with (2) and compare the 'ours' column with
'sim greedy' (v4 base is roughly greedy) - the plan asks for +-10%.
"""
import sys, json, os, statistics as S, argparse
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))

def find(d, pats):
    if isinstance(d, dict):
        for k, v in d.items():
            if any(p in k.lower() for p in pats): return v
        for v in d.values():
            r = find(v, pats)
            if r is not None: return r
    return None

def from_summary(path):
    d = json.load(open(path, encoding='utf8'))
    tr = find(d, ['tribut', 'score']); rd = find(d, ['rounds', 'round'])
    mp = find(d, ['map'])
    if isinstance(tr, list) and isinstance(rd, (int, float)) and rd:
        return [dict(map=str(mp), rate=x / rd, cohort='summary') for x in tr if isinstance(x, (int, float))]
    return []

if __name__ == '__main__':
    ap = argparse.ArgumentParser(); ap.add_argument('--since', default='2026-10-20'); ap.add_argument('--cohort', default='top10')
    ap.add_argument('--summary', nargs='*', default=[]); a = ap.parse_args()
    rows = []
    for l in open(os.path.join(ROOT, 'data/v3/decoded/econ.jsonl'), encoding='utf8'):
        g = json.loads(l)
        if g['cohort'] != a.cohort or g['t'][:10] < a.since or g['rounds'] < 100: continue
        for t in (0, 1): rows.append(dict(map=g['map'], rate=g['sg'][t] / g['rounds'], won=g['sg'][t] >= g['sg'][1 - t], n=g['n']))
    for p in a.summary: rows += from_summary(p)
    sim = {}
    pth = os.path.join(ROOT, 'results', 'econsim.json')
    if os.path.exists(pth): sim = json.load(open(pth))
    print(f"{'map':20s} {'n':>4s} {'real mean':>9s} {'real med':>9s} {'sim best':>9s} {'sim greedy':>10s} {'real/best':>9s}")
    for m in sorted({r['map'] for r in rows}):
        v = [r['rate'] for r in rows if r['map'] == m]
        sb = sg = None
        if m in sim:
            res = sim[m]['walk']['all']; sb = max(x['gpr'] for x in res.values()); sg = max(x['gpr'] for k, x in res.items() if k.startswith('greedy'))
        f = lambda x: '-' if x is None else f'{x:.3f}'
        print(f"{m:20s} {len(v):4d} {S.mean(v):9.3f} {S.median(v):9.3f} {f(sb):>9s} {f(sg):>10s} {f(S.mean(v) / sb if sb else None):>9s}")
