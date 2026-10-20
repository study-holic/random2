# Summarise s2metrics.mjs output: usage python3 -I tools/cloud/s2summ.py <metrics.jsonl>
import json, sys
R = [json.loads(l) for l in open(sys.argv[1]) if 'error' not in l]
n = len(R); f = lambda k, s: sum(r[s][k] for r in R) / max(n, 1)
w = sum(1 for r in R if r['win'])
print(f"{n} games, wins {w}; deaths {f('deaths','us'):.1f}/{f('deaths','them'):.1f}; locks {f('locks_placed','us'):.1f}/{f('locks_placed','them'):.1f}; "
      f"gear200 {f('gear_200','us'):.2f}/{f('gear_200','them'):.2f}; unspent gold100 {f('gold_100','us'):.0f}/{f('gold_100','them'):.0f}, "
      f"gold200 {f('gold_200','us'):.0f}/{f('gold_200','them'):.0f}; gem margin {sum(r['gems'][0]-r['gems'][1] for r in R)/max(n,1):+.0f}")
