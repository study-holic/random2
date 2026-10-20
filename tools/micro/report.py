"""Concise w14 duel-lab tables: python tools/micro/report.py > results/w14-duels-tables.md ; writes results/w14-duels.json."""
import collections, json, os, sys
sys.path.insert(0, os.path.dirname(__file__))
from analyze import load, cells, score, paired_gains, NAMES  # noqa: E402

SHORT = {"Ryth": "Ryth", "Vesper": "Vesp", "Vendetta": "Vend", "C&I": "C&I", "Stellar": "Stel", "Prism": "Prsm",
         "Marcella": "Marc", "Willow": "Will", "Ren": "Ren", "Varg": "Varg", "Tavi": "Tavi"}
RUNS = [  # (label, world, base tag, base pol, variant tag, variant pol)
    ("NORETREAT: fight to the death (no 45% walk home)", "A", "A_base", 0, "A_v1", 1),
    ("RETREAT30: walk home at 30% instead of 45%", "A", "A_base", 0, "A_v1024", 1024),
    ("EARLYCAST: spells at spell range 4, before moving", "A", "A_base", 0, "A_v2", 2),
    ("KITE (v2 kite_step, P_RANGED_KITE)", "A", "A_base", 0, "A_v4", 4),
    ("DASHIN: melee dash next to a foe 2-3 away", "A", "A_base", 0, "A_v8", 8),
    ("DASHOUT: Vesper dashes away from an adjacent melee, shoots", "A", "A_base", 0, "A_v16", 16),
    ("SHIELDENG: Varg shields when the foe is within 4", "A", "A_base", 0, "A_v32", 32),
    ("WEAVE: ranged step + reset + attack", "A", "A_base", 0, "A_v64", 64),
    ("STUNSTEP: step away from a stunned melee", "A", "A_base", 0, "A_v128", 128),
    ("CDWAIT: hold off while our burst is down and theirs is up", "A", "A_base", 0, "A_v4096", 4096),
    ("control HOLDRESET: never use the attack reset", "A", "A_base", 0, "A_v256", 256),
    ("control NOSPELL: never cast on champions", "A", "A_base", 0, "A_v512", 512),
    ("EARLYCAST", "B", "B_base", 1, "B_v2", 3),
    ("KITE", "B", "B_base", 1, "B_v4", 5),
    ("DASHIN", "B", "B_base", 1, "B_v8", 9),
    ("DASHOUT", "B", "B_base", 1, "B_v16", 17),
    ("SHIELDENG", "B", "B_base", 1, "B_v32", 33),
    ("CDWAIT", "B", "B_base", 1, "B_v4096", 4097),
    ("control HOLDRESET", "B", "B_base", 1, "B_v256", 257),
    ("control NOSPELL", "B", "B_base", 1, "B_v512", 513),
    ("NORETREAT (also no lone flee)", "T", "T1_base", 0, "T1_nr", 1),
    ("RACE: walk home only if losing the strength race", "T", "T1_base", 0, "T1_race", 16384),
    ("RACEK2: ... losing it by more than 2x", "T", "T1_base", 0, "T1_k2", 64),
    ("CIUNIT: do not lone-flee from Cassi+Issac", "T", "T1_base", 0, "T1_ci", 128),
    ("ADAPT2: ADAPT without CIUNIT (keeps v2's lone flee from Cassi+Issac)", "T", "T1_base", 0, "T1_adapt2", 32768),
    ("ADAPT: per-champion table (RACEK2+CIUNIT+EARLYCAST not Marcella+dashes+shield+kite Vesper/Prism)", "T", "T1_base", 0, "T1_adapt", 32768),
]


def matrix(tag, pol, title):
    d = load(tag)
    C, HP, DUR = cells(d["records"], pol, pol)
    out = [f"**{title}** (row's score vs column; W 1, escape/timeout 0.5, L 0; {sum(sum(c.values()) for c in C.values())} duels)", "",
           "| | " + " | ".join(SHORT[n] for n in NAMES) + " | mean |", "|---|" + "---|" * (len(NAMES) + 1)]
    js = {}
    for m in NAMES:
        row, vals = [], []
        for f in NAMES:
            c = C.get((m, f))
            s = score(c) if c else None
            row.append(f"{s:.2f}" if s is not None else "-")
            if s is not None and f != m:
                vals.append(s)
            if c:
                js[f"{m}|{f}"] = {"n": sum(c.values()), "score": s, "res": dict(c),
                                  "hp_left_med": sorted(HP[(m, f)])[len(HP[(m, f)]) // 2] if HP[(m, f)] else None,
                                  "ttk_med": sorted(DUR[(m, f)])[len(DUR[(m, f)]) // 2] if DUR[(m, f)] else None}
        out.append(f"| {SHORT[m]} | " + " | ".join(row) + f" | {sum(vals) / max(1, len(vals)):.2f} |")
    return out + [""], js


def main():
    md, J = [], {"matrices": {}, "variants": []}
    for tag, pol, title in (("B_base", 1, "World B, base stats, both fight to the death (raw 1v1 strength)"),
                            ("A_base", 0, "World A, base stats, v2a micro on both sides (incl. the 45% walk home)")):
        lines, js = matrix(tag, pol, title)
        md += lines
        J["matrices"][tag] = js
    md += ["**Paired gains** (variant side vs baseline foe minus baseline vs baseline, same game/corridor/side; mean over the 11 foes per champion; blank = no effect possible)", "",
           "| world | variant | mean | " + " | ".join(SHORT[n] for n in NAMES) + " | biggest cell changes |",
           "|---|---|---|" + "---|" * len(NAMES) + "---|"]
    for label, world, bt, bp, vt, vp in RUNS:
        b, v = load(bt), load(vt)
        if not b or not v:
            continue
        g = paired_gains(b["records"], bp, v["records"], vp)
        per = collections.defaultdict(list)
        for (m, f), x in g.items():
            per[m].append(x)
        pc = {n: sum(per[n]) / len(per[n]) for n in NAMES if per[n]}
        mean = sum(g.values()) / max(1, len(g))
        top = sorted(g.items(), key=lambda kv: -abs(kv[1]))[:4]
        top_s = "; ".join(f"{SHORT[m]} v {SHORT[f]} {x:+.2f}" for (m, f), x in top if abs(x) >= 0.1) or "-"
        md.append(f"| {world} | {label} | {mean:+.3f} | " + " | ".join(f"{pc[n]:+.2f}" if n in pc and abs(pc[n]) >= 0.005 else "" for n in NAMES) + f" | {top_s} |")
        J["variants"].append({"world": world, "label": label, "base": bt, "var": vt, "var_pol": vp, "mean_gain": mean,
                              "per_champion": pc, "gain": {f"{m}|{f}": x for (m, f), x in g.items()}})
    md.append("")
    json.dump(J, open("C:/cambc-solo/results/w14-duels.json", "w"), indent=0)
    print("\n".join(md))


if __name__ == "__main__":
    main()
