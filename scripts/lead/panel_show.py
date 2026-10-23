"""panel_show.py BOT [BOT2]: one line per panel opponent from runs/panel-<BOT>-<opp>/summary.json; with BOT2, both side by side."""
import json, glob, os, sys
a = sys.argv[1]; b = sys.argv[2] if len(sys.argv) > 2 else None
def load(bot):
    out = {}
    for p in glob.glob(f"/home/claude/runs/panel-{bot}-*/summary.json"):
        d = json.load(open(p)); out[d["b"]] = d
    return out
A = load(a); Bd = load(b) if b else {}
tw = tg = 0
print(f"{'opponent':34s} {a:>14s}" + (f" {b:>14s}" if b else ""))
for opp in sorted(set(A) | set(Bd)):
    x = A.get(opp); y = Bd.get(opp)
    fx = f"{x['a_wins']:>2}/{x['games']} {x['a_gems_mean']:>5.0f}-{x['b_gems_mean']:<4.0f}" if x else "-"
    fy = f"{y['a_wins']:>2}/{y['games']} {y['a_gems_mean']:>5.0f}-{y['b_gems_mean']:<4.0f}" if y else "-"
    print(f"{opp[:34]:34s} {fx:>18s}" + (f" {fy:>18s}" if b else ""))
    if x: tw += x["a_wins"]; tg += x["games"]
print(f"{a}: {tw}/{tg} over {len(A)} opponents")
