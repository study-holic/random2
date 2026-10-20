"""Paired gain of a variant run over a baseline run (1v1 lab records).
python tools/micro/pair.py <base_tag> <base_pol> <var_tag> <var_pol> [name]
Prints mean gain, per-champion mean gain and the best/worst cells; appends a JSON row to results/w14-lab/pairs.jsonl.
"""
import collections, json, os, sys
sys.path.insert(0, os.path.dirname(__file__))
from analyze import paired_gains, NAMES, load  # noqa: E402


def main():
    bt, bp, vt, vp = sys.argv[1], int(sys.argv[2]), sys.argv[3], int(sys.argv[4])
    name = sys.argv[5] if len(sys.argv) > 5 else vt
    b, v = load(bt), load(vt)
    g = paired_gains(b["records"], bp, v["records"], vp)
    per = collections.defaultdict(list)
    for (m, f), x in g.items():
        per[m].append(x)
    mean = sum(g.values()) / max(1, len(g))
    print(f"{name}: mean gain {mean:+.3f} over {len(g)} cells; changed (|g|>=0.1): {sum(1 for x in g.values() if abs(x) >= 0.1)}")
    print("per champion: " + ", ".join(f"{n} {sum(per[n]) / len(per[n]):+.2f}" for n in NAMES if per[n]))
    best = sorted(g.items(), key=lambda kv: -kv[1])[:6]
    worst = sorted(g.items(), key=lambda kv: kv[1])[:6]
    print("best: " + ", ".join(f"{m} v {f} {x:+.2f}" for (m, f), x in best))
    print("worst: " + ", ".join(f"{m} v {f} {x:+.2f}" for (m, f), x in worst))
    with open(os.path.join("C:/cambc-solo/results/w14-lab", "pairs.jsonl"), "a") as fh:
        fh.write(json.dumps({"name": name, "base": bt, "var": vt, "mean": mean, "cells": len(g),
                             "per_champion": {n: sum(per[n]) / len(per[n]) for n in NAMES if per[n]},
                             "gain": {f"{m}|{f}": x for (m, f), x in g.items()}}) + "\n")


if __name__ == "__main__":
    main()
