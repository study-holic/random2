"""S1 unopposed metrics from a mapgate JSON (bot vs idle): per map the round to 1,000 tributed gems (projected
2000*1000/gems when 1,000 is not reached; inf with 0 gems), gems, deaths, plus the S1D counters printed by a
P_S1_DIAG build (last line per champion; summed over champions, mean over sides).
usage: python3 tools/s1/diag.py <mapgate.json> [--md]"""
import json, re, sys, zipfile, statistics
d = json.load(open(sys.argv[1]))
md = "--md" in sys.argv
rows = {}
for r in d["results"]:
    m = rows.setdefault(r["map"], {"r1000": [], "gems": [], "deaths": [], "on": [], "sb": [], "dup": [], "cp": [], "dupc": []})
    g = r.get("gems", 0)
    m["gems"].append(g); m["deaths"].append(r.get("deaths", 0))
    m["r1000"].append(r["round"] if g >= 1000 else (2000 * 1000 / g if g else float("inf")))
    try:
        with zipfile.ZipFile(r["replay"]) as z:
            txt = z.read("team-%s.stdout" % r["side"].lower()).decode("utf8", "replace")
        last = {}
        for mm in re.finditer(r"S1D ([^\n]*)", txt):
                k = dict(re.findall(r"([a-z]+)(-?\d+)", mm.group(1)))
                last[k["a"]] = k
        if last:
            for f in ("on", "sb", "dup", "cp", "dupc"):
                m[f].append(sum(int(v.get(f, 0)) for v in last.values()))
    except Exception:
        pass
def mean(x): return sum(x) / len(x) if x else float("nan")
out = []
for mp in sorted(rows):
    m = rows[mp]
    r = mean(m["r1000"])
    out.append((mp, r, mean(m["gems"]), mean(m["deaths"]), mean(m["on"]), mean(m["sb"]), mean(m["dup"]), mean(m["cp"]), mean(m["dupc"])))
fmt = "| {} | {} | {:.0f} | {:.1f} | {:.0f} | {:.0f} | {:.0f} | {:.0f} | {:.0f} |" if md else "{:<20} {:>6} {:>6.0f} {:>5.1f} {:>6.0f} {:>5.0f} {:>5.0f} {:>5.0f} {:>5.0f}"
print(("| map | round to 1,000 (proj.) | gems | deaths | on-tile | self-block | shared | capt | shared-capt |\n|---|---|---|---|---|---|---|---|---|") if md else "map                    r1000   gems death     on    sb   dup    cp  dupc")
for o in out:
    print(fmt.format(o[0], ("%.0f" % o[1]) if o[1] != float("inf") else "inf", *o[2:]))
rs = [o[1] for o in out]
print("median round to 1,000 over %d maps: %.0f; reached on %d maps" % (len(rs), statistics.median(rs), sum(1 for o in out if all(x <= 2000 for x in rows[o[0]]["r1000"]))))
