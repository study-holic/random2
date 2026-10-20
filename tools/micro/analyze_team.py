"""w14 team-fight analysis. python tools/micro/analyze_team.py <base_tag> [<variant_tag> ...]
Team score per fight from the policy side: W (all foes dead with a survivor) = 1, L (all members dead) = 0, D (escape /
timeout / mixed) = 0.5. For a variant run (polA=V, polB=base pol) the gain per unit pair is score(V side) - score(base).
"""
import collections, json, os, sys

ROOT = "C:/cambc-solo"


def load(tag):
    return json.load(open(os.path.join(ROOT, "results", "w14-lab", f"{tag}.json")))


def cells(d, pol, foe_pol):
    C = collections.defaultdict(collections.Counter)
    for f in d["fights"]:
        if f["my_pol"] == pol and f["foe_pol"] == foe_pol:
            C[(f["me"], f["foe"])][f["res"]] += 1
    return C


def score(c):
    n = sum(c.values())
    return (c["W"] + 0.5 * c["D"]) / n if n else None


def main():
    base = load(sys.argv[1])
    bp = base["polA"]
    C0 = cells(base, bp, bp)
    units = sorted({k[0] for k in C0})
    out = [f"#### Team fights {sys.argv[1]} (size {base['size']}, policy {bp} both sides): row unit's score vs column unit", "",
           "| row \\ col | " + " | ".join(units) + " | mean |", "|---|" + "---|" * (len(units) + 1)]
    for u in units:
        vals = [score(C0[(u, v)]) for v in units if (u, v) in C0]
        out.append(f"| **{u}** | " + " | ".join(f"{score(C0[(u, v)]):.2f}" if (u, v) in C0 else "-" for v in units) +
                   f" | {sum(x for x in vals if x is not None) / max(1, len(vals)):.2f} |")
    out += ["", f"Fights: {sum(sum(c.values()) for c in C0.values())}.", ""]
    for tag in sys.argv[2:]:
        d = load(tag)
        vp = d["polA"] if d["polA"] != bp else d["polB"]
        C1 = cells(d, vp, bp)
        gains = {k: score(C1[k]) - score(C0[k]) for k in C1 if k in C0 and sum(C1[k].values()) >= 4}
        if not gains:
            continue
        per = collections.defaultdict(list)
        for (u, v), g in gains.items():
            per[u].append(g)
        best = sorted(gains.items(), key=lambda kv: -kv[1])[:4]
        worst = sorted(gains.items(), key=lambda kv: kv[1])[:4]
        out.append(f"- **{tag}** (policy {vp} vs {bp}): mean gain {sum(gains.values()) / len(gains):+.3f} over {len(gains)} unit pairs; "
                   f"per unit: " + ", ".join(f"{u} {sum(g) / len(g):+.2f}" for u, g in sorted(per.items())) +
                   "; best: " + ", ".join(f"{u} v {v} {g:+.2f}" for (u, v), g in best) +
                   "; worst: " + ", ".join(f"{u} v {v} {g:+.2f}" for (u, v), g in worst))
    print("\n".join(out))


if __name__ == "__main__":
    main()
