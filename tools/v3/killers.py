"""Who kills whom. Two attributions are compared on the full-action decodes (data/v3/decoded/ours_full):
  exact  = enemy actor of the last action whose damage list hit the victim in the death round or the one before (from the action log)
  nearby = round-level only: candidates are enemy champions whose kill counter rose between the two snapshots; each victim takes the nearest
           candidate (Chebyshev distance on the earlier snapshot, at most 8); no candidate = camp or other.
The nearby rule is then applied to every round-level decode of our games (ours_s13). Run: py -I tools/v3/killers.py
"""
import json, os, sys, glob, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
TN = {0: "Ryth", 1: "Vesper", 2: "Vendetta", 3: "C&I", 4: "C&I", 5: "Stellar", 6: "Prism", 7: "Marcella", 8: "Willow", 9: "Ren", 10: "Varg", 11: "Tavi"}
cheb = lambda a, b, c, d: max(abs(a - c), abs(b - d))


def deaths_nearby(d):
    snaps = d["rounds"]
    team = [r[0] for r in snaps[0]["c"]]
    typ = [r[1] for r in snaps[0]["c"]]
    n = len(team)
    out = []
    for a, b in zip(snaps, snaps[1:]):
        vict = [i for i in range(n) if a["c"][i][2] and not b["c"][i][2]]
        for i in vict:
            t = team[i]
            cand = [j for j in range(n) if team[j] != t and b["c"][j][9] > a["c"][j][9]]
            # a candidate may take several victims if its counter rose by several
            best, bd = None, 99
            for j in cand:
                dd = cheb(a["c"][i][3], a["c"][i][4], a["c"][j][3], a["c"][j][4])
                if dd < bd: best, bd = j, dd
            out.append({"r": b["r"], "v": i, "vt": t, "vtype": typ[i], "bag": a["c"][i][7], "k": best if (best is not None and bd <= 8) else None, "ktype": typ[best] if (best is not None and bd <= 8) else None, "ncand": len(cand)})
    return out


def deaths_exact(d):
    snaps = d["rounds"]
    team = [r[0] for r in snaps[0]["c"]]
    typ = [r[1] for r in snaps[0]["c"]]
    n = len(team)
    by_round = collections.defaultdict(list)
    for a in d["actions"]:
        by_round[a[0]].append(a)
    out = []
    for a, b in zip(snaps, snaps[1:]):
        for i in range(n):
            if a["c"][i][2] and not b["c"][i][2]:
                cand = None
                for r in (a["r"], b["r"]):
                    for act in by_round.get(r, []):
                        if team[act[1]] != team[i] and any(h[0] == "c" and h[1] == i and h[2] > 0 for h in act[9]):
                            cand = act[1]
                out.append({"r": b["r"], "v": i, "vt": team[i], "vtype": typ[i], "bag": a["c"][i][7], "k": cand, "ktype": typ[cand] if cand is not None else None})
    return out


if __name__ == "__main__":
    ok = tot = 0; noex = 0; conf = collections.Counter()
    for f in sorted(glob.glob(f"{V3}/decoded/ours_full/*.json")):
        d = json.load(open(f, encoding="utf8"))
        ex = deaths_exact(d); nb = deaths_nearby(d)
        em = {(e["r"], e["v"]): e for e in ex}
        for e in nb:
            x = em.get((e["r"], e["v"]))
            if not x: continue
            tot += 1
            if x["k"] is None: noex += 1
            same = (x["k"] == e["k"])
            ok += same
            conf[(x["k"] is None, e["k"] is None, same)] += 1
    print(f"validation on full-action games: {tot} deaths; nearby rule equals exact killer in {ok} ({100 * ok / max(1, tot):.0f}%); exact says no enemy hit in {noex}")
    print("(exact none?, nearby none?, same) ->", dict(conf))
    # apply to all round-level decodes of our games
    idx = {}
    for l in open(f"{V3}/decoded/games_ours.jsonl", encoding="utf8"):
        g = json.loads(l); idx[g["match"]] = g
    mat = collections.Counter(); ours_v = collections.Counter(); opp_v = collections.Counter(); bymap = collections.defaultdict(collections.Counter); unk = collections.Counter(); carriers = collections.Counter()
    ng = 0
    for f in sorted(glob.glob(f"{V3}/decoded/ours_s13/*.json")):
        m = os.path.basename(f)[:-5]
        if m not in idx: continue
        g = idx[m]; t = g["teamIds"].index(XAN); ng += 1
        d = json.load(open(f, encoding="utf8"))
        for e in deaths_nearby(d):
            side = "ours" if e["vt"] == t else "opp"
            if side == "ours":
                ours_v[TN[e["vtype"]]] += 1
                bymap[g["map"]]["deaths"] += 1
                if e["ktype"] is None: unk["no enemy kill nearby (camp or other)"] += 1
                else:
                    mat[(TN[e["ktype"]], TN[e["vtype"]])] += 1; bymap[g["map"]][TN[e["ktype"]]] += 1
                if e["bag"] >= 20: carriers[("ours", "carrier>=20")] += 1
            else:
                opp_v[TN[e["vtype"]]] += 1
                if e["bag"] >= 20: carriers[("opp", "carrier>=20")] += 1
    print(f"\nround-level nearby attribution over {ng} of our games")
    print("our champions that died:", ours_v.most_common(), "total", sum(ours_v.values()), "unattributed", dict(unk), "bag>=20 at death", dict(carriers))
    ktot = collections.Counter()
    for (k, v), c in mat.items(): ktot[k] += c
    print("killer champion type (enemy) -> count:", ktot.most_common())
    print("top killer->victim pairs:", [(k, v, c) for (k, v), c in mat.most_common(12)])
    print("opponent champions we killed (by victim type):", opp_v.most_common())
    print("\nper map: our deaths and share by killer type")
    for mp, c in sorted(bymap.items()):
        tot = c["deaths"]
        top3 = [(k, v) for k, v in c.most_common() if k != "deaths"][:3]
        print(f"| {mp} | {tot} | " + ", ".join(f"{k} {100 * v / tot:.0f}%" for k, v in top3) + " |")
