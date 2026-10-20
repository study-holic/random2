"""v4 vs v3b paired panel (unranked, 8 teams x 20 maps). Reads data/unranked/matches.jsonl + sumj/. Run: py -I tools/v3/panel_v4_vs_v3b.py"""
import json, os, sys, collections, statistics as st, math
sys.stdout.reconfigure(encoding="utf-8")
U = "C:/cambc-solo/data/unranked"
def load(prefix):
    out = {}
    for l in open(f"{U}/matches.jsonl", encoding="utf8"):
        m = json.loads(l)
        if not m["label"].startswith(prefix): continue
        f = f"{U}/sumj/{m['match']}.json"
        if not os.path.exists(f): continue
        d = json.load(open(f, encoding="utf8"))
        us = "teamA" if m["our_side"] == "a" else "teamB"; th = "teamB" if us == "teamA" else "teamA"
        sg = {t["team"]: int(t["submittedGems"]) for t in d["teams"]}
        tr = {"teamA": 0, "teamB": 0}; de = {"teamA": 0, "teamB": 0}; loss = {"teamA": 0, "teamB": 0}
        for e in d["events"]:
            o = e.get("owner"); t = o.get("team") if isinstance(o, dict) else None
            if t not in tr: continue
            if e["kind"] == "gemSubmission": tr[t] += sum(1 for x in e["entries"] if int(float(x["value"])) > 0)
            if e["kind"] == "death": de[t] += len(e["entries"])
            if e["kind"] == "gemLoss": loss[t] += sum(int(float(x["value"])) for x in e["entries"] if x.get("context") == "champion")
        out[(m["opponent"], m["map"])] = dict(opp=m["opponent"], map=m["map"], res=m["result"], rounds=d["match"]["roundsCompleted"], gu=sg[us], gt=sg[th],
            tru=tr[us], trt=tr[th], du=de[us], dt=de[th], lossu=loss[us], losst=loss[th], draft=".".join(map(str, m["our_draft"])), match=m["match"], fin=m["finished"])
    return out
A = load("v3b-"); B = load("v4-")
B = {k: v for k, v in B.items()}
def agg(rs):
    n = len(rs); w = sum(r["res"] == "win" for r in rs); R = sum(r["rounds"] for r in rs); m = lambda k: sum(r[k] for r in rs) / n
    return n, w, m("gu"), m("gt"), sum(r["tru"] for r in rs) / R, m("du"), m("lossu"), m("losst"), m("tru"), m("rounds")
def line(name, rs):
    if not rs: return f"| {name} | 0 |"
    n, w, gu, gt, trr, du, lu, lt, tru, rd = agg(rs)
    return f"| {name} | {n}{'†' if n < 30 else ''} | {w}-{n-w} | {gu:.0f} / {gt:.0f} | {rd:.0f} | {tru:.1f} ({trr:.2f}/rd) | {du:.1f} | {lu:.0f} / {lt:.0f} |"
H = "| group | games | W-L | gems us / them | mean rounds | tributes (per round) | deaths us | gems lost to champion kills us / them |\n|---|---|---|---|---|---|---|---|"
if __name__ == "__main__":
    keys = sorted(set(A) & set(B)); print("pairs", len(keys), "v3b", len(A), "v4", len(B))
    print("max v4 finish", max(v["fin"] for v in B.values()), "min", min(v["fin"] for v in B.values()))
    print("OVERALL"); print(H)
    print(line("v3b all", [A[k] for k in keys])); print(line("v4 all", [B[k] for k in keys]))
    for key, nm in (("opp", "TEAM"), ("map", "MAP")):
        print(nm); print("| group | v3b W-L | v4 W-L | flips L->W | flips W->L | v3b gems us/them | v4 gems us/them | v3b deaths | v4 deaths | v3b trib | v4 trib | v3b lost/ v4 lost (us) |\n|---|---|---|---|---|---|---|---|---|---|---|---|")
        for g in sorted({k[0 if key == "opp" else 1] for k in keys}):
            ks = [k for k in keys if k[0 if key == "opp" else 1] == g]; a = [A[k] for k in ks]; b = [B[k] for k in ks]
            wa = sum(r["res"] == "win" for r in a); wb = sum(r["res"] == "win" for r in b)
            lw = sum(A[k]["res"] != "win" and B[k]["res"] == "win" for k in ks); wl = sum(A[k]["res"] == "win" and B[k]["res"] != "win" for k in ks)
            n = len(ks); mm = lambda rs, f: sum(r[f] for r in rs) / n
            print(f"| {g} | {wa}-{n-wa} | {wb}-{n-wb} | {lw} | {wl} | {mm(a,'gu'):.0f}/{mm(a,'gt'):.0f} | {mm(b,'gu'):.0f}/{mm(b,'gt'):.0f} | {mm(a,'du'):.1f} | {mm(b,'du'):.1f} | {mm(a,'tru'):.1f} | {mm(b,'tru'):.1f} | {mm(a,'lossu'):.0f} / {mm(b,'lossu'):.0f} |")
    lw = sum(A[k]["res"] != "win" and B[k]["res"] == "win" for k in keys); wl = sum(A[k]["res"] == "win" and B[k]["res"] != "win" for k in keys)
    print("flips L->W", lw, "W->L", wl)
    # exact McNemar
    nn = lw + wl; p = min(1, 2 * sum(math.comb(nn, i) for i in range(0, min(lw, wl) + 1)) / 2 ** nn) if nn else 1
    print("McNemar exact two-sided p", round(p, 3))
    print("draws v3b", sum(A[k]["gu"] == A[k]["gt"] for k in keys), "v4", sum(B[k]["gu"] == B[k]["gt"] for k in keys))
    print("gem-margin paired mean diff", st.mean(B[k]["gu"] - B[k]["gt"] - (A[k]["gu"] - A[k]["gt"]) for k in keys))
    d = [B[k]["du"] - A[k]["du"] for k in keys]; print("deaths diff mean", st.mean(d), "sd", st.stdev(d))
    dd = collections.Counter((A[k]["draft"] == B[k]["draft"]) for k in keys); print("same draft per pair", dd)
    print("differing-draft maps", sorted({k[1] for k in keys if A[k]["draft"] != B[k]["draft"]}))
    print("flip list L->W:", [k for k in keys if A[k]["res"] != "win" and B[k]["res"] == "win"])
    print("flip list W->L:", [k for k in keys if A[k]["res"] == "win" and B[k]["res"] != "win"])
    print("v4 deaths by map desc", sorted(((round(st.mean(B[k]["du"] for k in keys if k[1]==m),1), m) for m in {k[1] for k in keys}), reverse=True)[:6])
    print("draw sets / tiebreak wins v4:", [(k[1]) for k in keys if B[k]["gu"]==B[k]["gt"] and B[k]["res"]=="win"])
