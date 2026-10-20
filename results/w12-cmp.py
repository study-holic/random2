"""w12: per-map comparison of mapgate JSONs (means per game over all salts).
usage: py -I results/w12-cmp.py <base.json> <cand.json> [maps,comma]   (panel runs)
       py -I results/w12-cmp.py --h2h <cand-vs-v2.json> [maps]        (head-to-head: wins of 6, margins)"""
import json, sys, statistics as st

def per_map(path, maps=None):
    d = json.load(open(path)); out = {}
    for r in d['results']:
        if maps and r['map'] not in maps: continue
        out.setdefault(r['map'], []).append(r)
    return out

def agg(rs):
    n = len(rs); g = lambda k: sum(r.get(k, 0) for r in rs) / n
    wr = sorted(r['round'] for r in rs if r['win'] and 'round' in r)
    return dict(n=n, wins=sum(r['win'] for r in rs), deaths=g('deaths'), kills=g('kills'),
                gem=sum(r.get('gems', 0) - r.get('gems_opp', 0) for r in rs) / n,
                gold=sum(r.get('gold', 0) - r.get('gold_opp', 0) for r in rs) / n,
                medR=wr[len(wr) // 2] if wr else None, faults=sum(r.get('faults', 0) for r in rs))

if sys.argv[1] == '--h2h':
    maps = sys.argv[3].split(',') if len(sys.argv) > 3 else None
    P = per_map(sys.argv[2], maps)
    print(f"{'map':<12} {'wins':>5} {'gemM':>7} {'goldM':>7} {'ourD':>5} {'ourK':>5} {'medR':>5} flt")
    T = []
    for m, rs in sorted(P.items()):
        a = agg(rs); T += rs
        print(f"{m:<12} {a['wins']:>2}/{a['n']:<2} {a['gem']:>7.0f} {a['gold']:>7.0f} {a['deaths']:>5.1f} {a['kills']:>5.1f} {str(a['medR']):>5} {a['faults']}")
    a = agg(T); print(f"{'TOTAL':<12} {a['wins']:>2}/{a['n']:<3} {a['gem']:>6.0f} {a['gold']:>7.0f} {a['deaths']:>5.1f} {a['kills']:>5.1f}")
else:
    maps = sys.argv[3].split(',') if len(sys.argv) > 3 else None
    B, C = per_map(sys.argv[1], maps), per_map(sys.argv[2], maps)
    print(f"{'map':<12} {'W base':>7} {'W cand':>7} | {'D base':>6} {'D cand':>6} {'dD%':>5} | {'gem b':>6} {'gem c':>6} | {'gold b':>6} {'gold c':>6} | {'medR b':>6} {'medR c':>6} flt")
    for m in sorted(C):
        if m not in B: continue
        b, c = agg(B[m]), agg(C[m])
        ns_b, ns_c = b['n'] / 12, c['n'] / 12
        dd = 100 * (c['deaths'] - b['deaths']) / b['deaths'] if b['deaths'] else 0
        print(f"{m:<12} {b['wins']/ns_b:>7.1f} {c['wins']/ns_c:>7.1f} | {b['deaths']:>6.1f} {c['deaths']:>6.1f} {dd:>5.0f} | {b['gem']:>6.0f} {c['gem']:>6.0f} | {b['gold']:>6.0f} {c['gold']:>6.0f} | {str(b['medR']):>6} {str(c['medR']):>6} {c['faults']}")
