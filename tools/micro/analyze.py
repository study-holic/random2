"""w14 duel lab analysis: matrices and per-variant gains.

python tools/micro/analyze.py [--out results/w14-duels.json]
Reads results/w14-lab/{A,B}_base.json and {A,B}_v<bits>.json. For variant V in world A (both sides keep v2's 45% retreat)
the cell (me, foe) uses records of champions running V against foes running 0; the gain is score(V) - score(base) on the
same cell. World B: both sides fight to the death (bit 1), V|1 vs 1.
Score per duel: W = 1, L = 0, E/X/T (escape, foe escaped, timeout) = 0.5. Also death rate (L share) and kill rate (W share).
"""
import argparse, collections, json, math, os

ROOT = "C:/cambc-solo"
NAMES = ["Ryth", "Vesper", "Vendetta", "C&I", "Stellar", "Prism", "Marcella", "Willow", "Ren", "Varg", "Tavi"]
VARIANTS = {1: "NORETREAT", 2: "EARLYCAST", 4: "KITE(v2)", 8: "DASHIN", 16: "DASHOUT", 32: "SHIELDENG", 64: "WEAVE",
            128: "STUNSTEP", 256: "HOLDRESET(ctrl)", 512: "NOSPELL(ctrl)", 1024: "RETREAT30", 4096: "CDWAIT"}
RANGED = {"Vesper", "C&I", "Stellar", "Prism", "Willow", "Tavi"}   # C&I counted by Cassi's line


def load(tag):
    p = os.path.join(ROOT, "results", "w14-lab", f"{tag}.json")
    return json.load(open(p)) if os.path.exists(p) else None


def cells(recs, pol, foe_pol):
    C = collections.defaultdict(collections.Counter)
    HP = collections.defaultdict(list)
    DUR = collections.defaultdict(list)
    for r in recs:
        if r["my_pol"] != pol or r["foe_pol"] != foe_pol:
            continue
        k = (r["me"], r["foe"])
        C[k][r["res"]] += 1
        if r["res"] == "W":
            HP[k].append(r["hp"] / max(1, r["mhp"]))
            DUR[k].append(r["dur"])
    return C, HP, DUR


def paired_gains(base_recs, base_pol, var_recs, pol, min_n=5):
    """Gain per (me, foe) cell, paired by (game, corridor, side): the same corridor and side in the same schedule game
    run once with the baseline and once with the variant on the `me` side. Corridor and side change the duel sequence
    (initiative, arrival order), so unpaired cells mix configurations; pairing removes that."""
    G0 = collections.defaultdict(collections.Counter)
    for r in base_recs:
        if r["my_pol"] == base_pol and r["foe_pol"] == base_pol:
            G0[(r["g"], r["c"], r["side"])][r["res"]] += 1
    G1 = collections.defaultdict(collections.Counter)
    cellof = {}
    for r in var_recs:
        if r["my_pol"] == pol and r["foe_pol"] == base_pol:
            k = (r["g"], r["c"], r["side"])
            G1[k][r["res"]] += 1
            cellof[k] = (r["me"], r["foe"])
    acc = collections.defaultdict(lambda: [0.0, 0])
    for k, c1 in G1.items():
        c0 = G0.get(k)
        if not c0:
            continue
        w = min(sum(c1.values()), sum(c0.values()))
        acc[cellof[k]][0] += w * (score(c1) - score(c0))
        acc[cellof[k]][1] += w
    return {cell: v[0] / v[1] for cell, v in acc.items() if v[1] >= min_n}


def score(c):
    n = sum(c.values())
    return (c["W"] + 0.5 * (c["E"] + c["X"] + c["T"])) / n if n else None


def med(v):
    v = sorted(v)
    return v[len(v) // 2] if v else None


def matrix_md(C, HP, DUR, title):
    lines = [f"#### {title}", "", "Row champion's score against the column champion (W = 1, escape/timeout = 0.5); n = duels in the cell.", "",
             "| row \\ col | " + " | ".join(NAMES) + " | row mean |", "|---|" + "---|" * (len(NAMES) + 1)]
    for me in NAMES:
        row, vals = [], []
        for foe in NAMES:
            c = C.get((me, foe))
            s = score(c) if c else None
            row.append(f"{s:.2f}" if s is not None else "-")
            if s is not None and foe != me:
                vals.append(s)
        lines.append(f"| **{me}** | " + " | ".join(row) + f" | {sum(vals) / len(vals):.2f} |" if vals else f"| **{me}** | " + " | ".join(row) + " | - |")
    n = sum(sum(c.values()) for c in C.values())
    lines += ["", f"Duels: {n}.", ""]
    return lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=os.path.join(ROOT, "results", "w14-duels.json"))
    ap.add_argument("--md", default=os.path.join(ROOT, "results", "w14-duels-tables.md"))
    a = ap.parse_args()
    out = {"worlds": {}, "variants": {}}
    md = []
    for world, base_pol in (("A", 0), ("B", 1)):
        b = load(f"{world}_base")
        if not b:
            continue
        C0, HP0, DUR0 = cells(b["records"], base_pol, base_pol)
        out["worlds"][world] = {f"{m}|{f}": {"n": sum(C0[(m, f)].values()), "score": score(C0[(m, f)]), "res": dict(C0[(m, f)]),
                                             "hp_left_med": med(HP0[(m, f)]), "ttk_med": med(DUR0[(m, f)])}
                                for (m, f) in C0}
        md += matrix_md(C0, HP0, DUR0, f"World {world} baseline ({'v2 micro incl. its 45% retreat walk' if world == 'A' else 'v2 micro, both fight to the death'})")
        md += [f"#### World {world}: gain of each variant (variant side vs baseline foe, minus baseline vs baseline, same cell)", "",
               "| variant | cells changed (abs gain >= 0.1) | mean gain over all cells | mean gain over changed cells | best cells (gain) | worst cells (gain) |",
               "|---|---|---|---|---|---|"]
        for v, name in VARIANTS.items():
            if world == "B" and v in (1, 1024):
                continue
            pol = v if world == "A" else (v | 1)
            d = load(f"{world}_v{v}")
            if not d:
                continue
            gains = paired_gains(b["records"], base_pol, d["records"], pol)
            if not gains:
                continue
            changed = {k: g for k, g in gains.items() if abs(g) >= 0.1}
            allmean = sum(gains.values()) / len(gains)
            chmean = sum(changed.values()) / len(changed) if changed else 0.0
            best = sorted(gains.items(), key=lambda kv: -kv[1])[:5]
            worst = sorted(gains.items(), key=lambda kv: kv[1])[:5]
            fmt = lambda lst: ", ".join(f"{m} v {f} {g:+.2f}" for (m, f), g in lst if abs(g) >= 0.05) or "-"
            md.append(f"| {name} ({v}) | {len(changed)}/{len(gains)} | {allmean:+.3f} | {chmean:+.2f} | {fmt(best)} | {fmt(worst)} |")
            per_me = collections.defaultdict(list)
            for (m, f), g in gains.items():
                per_me[m].append(g)
            out["variants"][f"{world}:{v}"] = {"name": name, "world": world, "mean_gain": allmean, "changed": len(changed), "cells": len(gains),
                                               "gain": {f"{m}|{f}": g for (m, f), g in gains.items()},
                                               "per_champion": {m: sum(v_) / len(v_) for m, v_ in per_me.items()}}
        md.append("")
        # per-champion gains table for the world
        md += [f"#### World {world}: mean gain per champion running the variant (averaged over its 11 foes)", "",
               "| variant | " + " | ".join(NAMES) + " |", "|---|" + "---|" * len(NAMES)]
        for key, vv in out["variants"].items():
            if not key.startswith(world + ":"):
                continue
            pc = vv["per_champion"]
            md.append(f"| {vv['name']} | " + " | ".join(f"{pc[n]:+.2f}" if n in pc else "-" for n in NAMES) + " |")
        md.append("")
    json.dump(out, open(a.out, "w"), indent=1)
    open(a.md, "w").write("\n".join(md) + "\n")
    print("\n".join(md))


if __name__ == "__main__":
    main()
