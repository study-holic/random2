"""Summarise mapgate h2h JSONs: python3 tools/cloud/abltable.py results/v4abl-*.json"""
import json, sys
KEPT = ['ambush', 'collision', 'contest', 'corners', 'eight', 'elkring', 'expanse', 'fragment', 'interlock', 'lanes', 'monolith']
print('| variant | h2h wins / 126 | win % | kept maps lost 5-6 of 6 | deaths/game | gem margin/game |')
print('|---|---|---|---|---|---|')
for f in sys.argv[1:]:
    d = json.load(open(f)); R = d['results']; n = len(R)
    w = sum(r['win'] for r in R)
    per = {}
    for r in R: per.setdefault(r['map'], []).append(r['win'])
    bad = [m for m in KEPT if m in per and len(per[m]) - sum(per[m]) >= 5]
    deaths = sum(r.get('deaths', 0) for r in R) / n; gm = sum(r.get('gems', 0) - r.get('gems_opp', 0) for r in R) / n
    name = f.split('v4abl-')[-1].replace('.json', '') if 'v4abl-' in f else f
    print(f"| {name} | {w}/{n} | {100*w/n:.1f} | {', '.join(bad) or 'none'} | {deaths:.1f} | {gm:+.0f} |")
