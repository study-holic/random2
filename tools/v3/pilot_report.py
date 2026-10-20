"""Pilot validation + timing report. Reads data/v3/pilot/{full,s13}/*.json (extract_pilot output), logs/v3-pilot-*.log (timings),
and the legacy index (final gems per team) for a cross-check. Run: py -I tools/v3/pilot_report.py
"""
import json, os, sys, glob, statistics, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from derive_events import derive
R = "C:/cambc-solo"; B = f"{R}/intel/copied/cambc-top30-intel/data"
idx = {}
for l in open(f"{B}/replay_index.jsonl", encoding="utf8"):
    d = json.loads(l); idx[d["match"]] = d
PL = {x["match"]: x for x in json.load(open(f"{R}/data/v3/pilot/pilot_list.json", encoding="utf8"))}
def timings(pat):
    out = {}
    for f in glob.glob(pat):
        for l in open(f, encoding="utf8"):
            try: d = json.loads(l)
            except Exception: continue
            if "ms" in d: out[d["file"][:-4]] = d
    return out
T1 = timings(f"{R}/logs/v3-pilot-full-*.log"); T13 = timings(f"{R}/logs/v3-pilot-s13.log")
rows, agree_sg, agree_win, kill_ok, n = [], 0, 0, 0, 0
for m, x in PL.items():
    f = f"{R}/data/v3/pilot/full/{m}.json"
    if not os.path.exists(f): continue
    d = json.load(open(f, encoding="utf8")); ev, s = derive(d)
    last = d["rounds"][-1]
    kills = [sum(c[9] for c in last["c"] if c[0] == t) for t in (0, 1)]
    ri = idx[m]; n += 1
    agree_sg += list(s["sg"]) == list(ri["gems"]); agree_win += s["winner"] == ri["winner"]
    rows.append((m[:8], x["map"], x["ranks"], s["rounds"], d["steps"], len(d["actions"]), round(T1[m]["ms"] / 1000, 1), s["sg"],
                 s["t0"]["deaths"], s["t1"]["deaths"], kills, len({(e[0], e[1]) for e in ev["tribute"]}), len(ev["recall"]), len(ev["use"]), len(ev["capture"])))
print("| match | map | ranks | rounds | steps | actions | decode s | final gems | deaths t0/t1 | kills t0/t1 (final) | tribute events | recalls | cons uses | captures |")
print("|---|---|---|---|---|---|---|---|---|---|---|---|---|---|")
for r in rows: print(f"| {r[0]} | {r[1]} | {r[2]} | {r[3]} | {r[4]} | {r[5]} | {r[6]} | {r[7]} | {r[8]}/{r[9]} | {r[10]} | {r[11]} | {r[12]} | {r[13]} | {r[14]} |")
print(f"\nvalidation (n={n}): final submitted gems equal platform score {agree_sg}/{n}; winner equal {agree_win}/{n}")
ms = [T1[m]["ms"] / 1000 for m in PL if m in T1]; st = [T1[m]["steps"] for m in PL if m in T1]
print(f"full-action decode: n={len(ms)}, mean {statistics.mean(ms):.1f}s, median {statistics.median(ms):.1f}s, min {min(ms):.1f}, max {max(ms):.1f}; ms/step mean {1000*sum(ms)/sum(st):.2f}")
m13 = [v for v in T13.values()]
if m13:
    print(f"round-level (stride 13): n={len(m13)}, mean {statistics.mean(v['ms'] for v in m13)/1000:.1f}s, median {statistics.median(v['ms'] for v in m13)/1000:.1f}s, max {max(v['ms'] for v in m13)/1000:.1f}s, mean steps {statistics.mean(v['steps'] for v in m13):.0f}")
# derive on all stride-13 games: sanity of event rates
tot = collections.Counter(); ng = 0; ok = 0
for f in glob.glob(f"{R}/data/v3/pilot/s13/*.json"):
    d = json.load(open(f, encoding="utf8")); ev, s = derive(d); ng += 1
    m = os.path.basename(f)[:-5]; ri = idx.get(m)
    if ri: ok += list(s["sg"]) == list(ri["gems"])
    for k, v in ev.items(): tot[k] += len(v)
print(f"stride-13 derive on {ng} games: final-gems match platform {ok}/{ng}; events per game", {k: round(v / ng, 1) for k, v in tot.items()})
