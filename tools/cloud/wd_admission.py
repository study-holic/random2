# Watchdog admission (tools/cloud/sweep35.md task 3), fixed before testing. Baseline = s1g1 (bots/wd is a copy of it).
# usage: python3 -I tools/cloud/wd_admission.py <rows.json with s1g1 and wd> <maps21.json>
import json, sys, collections

rows = [r for r in json.load(open(sys.argv[1])) if "error" not in r]
old21 = set(json.load(open(sys.argv[2])).keys())
B, W = "s1g1", "wd"
nz = lambda v: 0 if v is None else v
sel = lambda b, **k: [r for r in rows if r["bot"] == b and all(r[x] == y for x, y in k.items())]

# 1. stall champion-rounds over the sweep (all opponents, maps, sides, seeds)
sb, sw = sum(nz(r["stall_rounds"]) for r in sel(B)), sum(nz(r["stall_rounds"]) for r in sel(W))
l1 = sw <= 0.5 * sb
print(f"1. stall champion-rounds: {B} {sb:,} -> {W} {sw:,} ({(sw - sb) / sb * 100:+.0f} %); need <= -50 %: {'PASS' if l1 else 'FAIL'}")

# 2. unopposed speed per map (vs idle): tributed gems per round played, mean of 6 games; fail if any map < 0.9 x baseline
spd = lambda b, m: [r["gems"] / max(1, r["round"]) for r in sel(b, opp="idle", map=m)]
slow = []
for m in sorted({r["map"] for r in rows}):
    a, c = spd(B, m), spd(W, m)
    if not a or not c: continue
    ma, mc = sum(a) / len(a), sum(c) / len(c)
    if ma > 0 and mc < 0.9 * ma: slow.append((m, round(ma, 3), round(mc, 3)))
l2 = not slow
print(f"2. unopposed speed (gems per round vs idle), maps more than 10 % slower: {slow or 'none'}: {'PASS' if l2 else 'FAIL'}")

# 3. shortcut vs starter-cpp, both sides, seeds 0-2
sc = sel(W, opp="starter-cpp", map="shortcut")
l3 = len(sc) == 6 and all(r["win"] for r in sc)
print(f"3. shortcut vs starter-cpp: {sum(r['win'] for r in sc)}/{len(sc)} " + str([(r['side'], r['salt'], r['gems'], r['gems_opp']) for r in sc]) + f": {'PASS' if l3 else 'FAIL'}")

# 4. peak fuel after round 1 <= 90,000 on all 35 maps (seed 0: seeds 1-2 add a 20k/40k first-turn burn)
pk = collections.defaultdict(int)
for r in sel(W, salt=0): pk[r["map"]] = max(pk[r["map"]], r["fuel_peak"])
over = sorted(((m, v) for m, v in pk.items() if v > 90000), key=lambda x: -x[1])
pk_all = collections.defaultdict(int)
for r in sel(W): pk_all[r["map"]] = max(pk_all[r["map"]], r["fuel_peak"])
l4 = not over
print(f"4. peak fuel after round 1 (seed 0), maps over 90,000: {over or 'none'}: {'PASS' if l4 else 'FAIL'}; all seeds: "
      f"{sorted(((m, v) for m, v in pk_all.items() if v > 90000), key=lambda x: -x[1])[:8]}")

# 5. self-play vs v4 on the 21 old maps >= 62/126 (rejection gate)
h = [r for r in sel(W, opp="v4") if r["map"] in old21]
w5 = sum(r["win"] for r in h)
print(f"5. vs v4 on the 21 old maps: {w5}/{len(h)} (baseline {B}: {sum(r['win'] for r in sel(B, opp='v4') if r['map'] in old21)}); need >= 62: {'PASS' if w5 >= 62 else 'FAIL'}")
