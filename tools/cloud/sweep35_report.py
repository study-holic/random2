# Markdown tables from tools/cloud/sweep35.py rows. usage: python3 -I tools/cloud/sweep35_report.py <rows.json> [bot ...]
import json, sys, collections

rows = [r for r in json.load(open(sys.argv[1])) if "error" not in r]
bots = sys.argv[2:] or sorted({r["bot"] for r in rows})
opps = ["idle", "starter-cpp", "v4"]
key = lambda r: (r["bot"], r["opp"], r["map"])
by = collections.defaultdict(list)
for r in rows: by[key(r)].append(r)
mean = lambda xs: sum(xs) / len(xs) if xs else 0
nz = lambda v: 0 if v is None else v

print("### Per bot and opponent (each cell: mean per game over 35 maps x 2 sides x seeds 0-2)\n")
print("| bot | opp | games | wins | gems | deaths monster / champion | fuel peak max (seed 0) | turns >= 90k (seed 0) | turns >= 100k (seed 0 / all) | stall ch-rounds | altar stall | zero-gem games |")
print("|---|---|---|---|---|---|---|---|---|---|---|---|")
for b in bots:
    for o in opps:
        g = [r for r in rows if r["bot"] == b and r["opp"] == o]
        if not g: continue
        print(f"| {b} | {o} | {len(g)} | {sum(r['win'] for r in g)} | {mean([r['gems'] for r in g]):.0f} | {mean([nz(r['deaths_monster']) for r in g]):.1f} / {mean([nz(r['deaths_champion']) for r in g]):.1f} | "
              f"{max(r['fuel_peak'] for r in g if r['salt'] == 0):,} | {sum(r['turns_90k'] for r in g if r['salt'] == 0)} | {sum(r['turns_100k'] for r in g if r['salt'] == 0)} / {sum(r['turns_100k'] for r in g)} | {mean([nz(r['stall_rounds']) for r in g]):.0f} | "
              f"{mean([nz(r['altar_stall_rounds']) for r in g]):.0f} | {sum(r['zero_gem'] for r in g)} |")

maps = sorted({r["map"] for r in rows})
for b in bots:
    print(f"\n### {b} per map (gems / stall champion-rounds / fuel peak, mean of 6 games; vs idle | starter-cpp | v4)\n")
    print("| map | " + " | ".join(opps) + " | deaths m/c vs v4 | zero-gem | turns >= 90k |")
    print("|---|" + "---|" * (len(opps) + 3))
    for m in maps:
        cells = []
        z = t90 = 0
        for o in opps:
            g = by.get((b, o, m), [])
            if not g: cells.append("-"); continue
            z += sum(r["zero_gem"] for r in g); t90 += sum(r["turns_90k"] for r in g)
            cells.append(f"{mean([r['gems'] for r in g]):.0f} / {mean([nz(r['stall_rounds']) for r in g]):.0f} / {max(r['fuel_peak'] for r in g) // 1000}k")
        gv = by.get((b, "v4", m), [])
        dmc = f"{mean([nz(r['deaths_monster']) for r in gv]):.1f}/{mean([nz(r['deaths_champion']) for r in gv]):.1f}" if gv else "-"
        print(f"| {m} | " + " | ".join(cells) + f" | {dmc} | {z} | {t90} |")

print("\n### Flagged map-bot pairs (zero-gem game, stall over 200 champion-rounds in a game, or a turn >= 90,000 after round 1 at seed 0)\n")
print("Fuel columns use seed 0 only: seeds 1 and 2 burn 20k / 40k extra on the first turn (mapgate salt), which tips round 1 over 200,000 on many maps.\n")
print("| bot | map | opp | zero-gem games | games with stall > 200 (worst run, from round) | seed-0 turns >= 90k (>= 100k) | seed-0 fuel peak |")
print("|---|---|---|---|---|---|---|")
for (b, o, m), g in sorted(by.items()):
    if b not in bots: continue
    z = sum(r["zero_gem"] for r in g); st = [r for r in g if nz(r["stall_rounds"]) > 200]
    g0 = [r for r in g if r["salt"] == 0]
    t90 = sum(r["turns_90k"] for r in g0); t100 = sum(r["turns_100k"] for r in g0)
    if z or st or t90:
        w = max(st, key=lambda r: nz(r["worst_stall"])) if st else None
        ws = f"{len(st)} ({w['worst_stall']}, r{w['worst_at']})" if w else "0"
        print(f"| {b} | {m} | {o} | {z} | {ws} | {t90} ({t100}) | {max((r['fuel_peak'] for r in g0), default=0):,} |")
