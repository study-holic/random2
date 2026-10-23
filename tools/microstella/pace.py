#!/usr/bin/env python3
"""pace.py <bot> [opp=v20] [maps...]: run bot vs opp both sides on the 01 maps, print gems@300/@600, first tribute, deaths, winner."""
import os
WS = os.environ.get('CAMBC_WS', os.getcwd())  # workspace root (folder with bots/, maps/, cambc.toml)
def MP(m):  # platform map files use hyphens (01big-world); random2/maps and older lists use underscores
    for c in (m, m.replace('_', '-')):
        if os.path.exists(f'{WS}/maps/{c}.map26a'): return f'{WS}/maps/{c}.map26a'
    return f'{WS}/maps/{m}.map26a'
import sys, os, json, subprocess, glob
bot=sys.argv[1]; opp=sys.argv[2] if len(sys.argv)>2 else 'v20'
maps=sys.argv[3:] or ['01big_world','01broken_curve','01concentrated_wealth','01corridors','01elk_arena','01fracture','01gold_rush','01hooks','01islands','00interlock']
out=f'replays/pace_{bot}_vs_{opp}'; os.makedirs(out, exist_ok=True)
man=f'{out}/manifest.txt'
with open(man,'w') as f:
    for m in maps:
        for side,(a,b) in (('A',(bot,opp)),('B',(opp,bot))):
            r=f'{out}/{m}-{side}.repl26a'
            if os.path.exists(r+'.json'): continue
            f.write(f'{WS}/bots/{a} {WS}/bots/{b} {MP(m)} {WS}/{r} --statistics {WS}/{r}.json --json\n')
if os.path.getsize(man)>0:
    subprocess.run(['cambc','run-batch',man,'--jobs','2','--release','--json'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
def stats(path, team):
    d=json.load(open(path)); res={}
    for t in d['teamTimelines']:
        if t['owner']!=team: continue
        acc=0; first=None; v300=0; v600=0; last=0
        for r in t['runs']:
            if first is None and r['value']!='0': first=acc
            if acc<=300: v300=int(r['value'])
            if acc<=600: v600=int(r['value'])
            last=int(r['value']); acc+=r['duration']
        res.update(first=first, g300=v300, g600=v600, final=last, rounds=acc)
    deaths=0
    for t in d['championTimelines']:
        if t['owner']['team']==team and t['metric']=='deaths': deaths+=int(t['runs'][-1]['value'])
    res['deaths']=deaths
    return res
wins=0; tot=0; sum600=0; sum600o=0
print(f'{"map":24s} side  first  g300  g600  final  deaths | opp g600 final deaths | result')
for m in maps:
    for side in 'AB':
        p=f'{out}/{m}-{side}.repl26a.json'
        if not os.path.exists(p): print(m,side,'MISSING'); continue
        me=stats(p,'teamA' if side=='A' else 'teamB'); op=stats(p,'teamB' if side=='A' else 'teamA')
        win = me['final']>op['final'] or (me['final']==op['final'] and side=='B')
        wins+=win; tot+=1; sum600+=me['g600']; sum600o+=op['g600']
        print(f'{m:24s} {side}    {str(me["first"]):5s} {me["g300"]:5d} {me["g600"]:5d} {me["final"]:6d} {me["deaths"]:6d} |    {op["g600"]:5d} {op["final"]:5d} {op["deaths"]:5d} | {"WIN" if win else "loss"}')
print(f'\n{bot} vs {opp}: {wins}/{tot} wins, mean gems@600 {sum600/tot:.0f} vs {sum600o/tot:.0f}')
