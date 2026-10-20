"""Action-level analysis of full-action decodes (extract_pilot.mjs --stride 1, data/v3/decoded/full/*.json).
Per game: killer attribution, channel (kind 11) outcomes with causes, altar-lock placements and whether they cancelled an enemy tribute,
stun casts, time dead / tribute-channel / recall-channel. Writes data/v3/decoded/actions_full.jsonl and prints cohort tables.
Run: py -I tools/v3/analyze_actions.py
Cohort per game: xan if xanarchy played; micron if Micron played; else top10 if both teams are current top-10.
Heuristics (labelled in the note): a channel start is tribute-like if the bag is positive and the start tile is within 2 tiles of a tile where some
tribute of the same map id completed; success = own submitted gems rose and the bag fell (tribute) or the champion teleported to its spawn (recall).
Failure cause, in order: died; stunned (enemy cc cast, ability type 4, within 1 tile of the champion); locked (enemy altar-lock use within 3 tiles);
hit (hp fell); other (cancelled or unknown).
"""
import json, os, sys, glob, collections, math, statistics
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, "C:/cambc-solo/tools/v3")
from analyze_rounds import load_index

V3 = "C:/cambc-solo/data/v3"
TN = {0: "Ryth", 1: "Vesper", 2: "Vendetta", 3: "C&I", 4: "C&I", 5: "Stellar", 6: "Prism", 7: "Marcella", 8: "Willow", 9: "Ren", 10: "Varg", 11: "Tavi"}
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]
RANK = {t["teamId"]: t["rank"] for t in top}
XAN = "6c59147e-576f-465f-8d00-bdde9ed12f22"
MIC = "695ddbdf-afd9-43bf-9988-2a98fdb0f750"
cheb = lambda a, b, c, d: max(abs(a - c), abs(b - d))


def load(f):
    d = json.load(open(f, encoding="utf8"))
    S = {s["r"]: s for s in d["rounds"]}
    return d, S


def altar_tiles(files):
    """Tiles where a channel started and a tribute completed, per map id (pass 1)."""
    A = collections.defaultdict(set)
    for f, mid, info in files:
        d, S = load(f)
        rs = sorted(S)
        team = [r[0] for r in d["rounds"][0]["c"]]
        for a in d["actions"]:
            if a[2] != 11 or a[7] <= 0:
                continue
            r0, i = a[0], a[1]
            for r in range(r0 + 1, r0 + 3 + math.ceil(a[7] / 10)):
                if r in S and r - 1 in S:
                    t = team[i]
                    if S[r]["sg"][t] > S[r - 1]["sg"][t] and S[r]["c"][i][7] < a[7] and S[r]["c"][i][2]:
                        c = S[r0]["c"][i] if r0 in S else None
                        if c: A[info["mapId"]].add((c[3], c[4]))
                        break
    return A


def analyse(d, S, info, ALT):
    snaps = d["rounds"]
    c0 = snaps[0]["c"]
    n = len(c0)
    team = [r[0] for r in c0]
    typ = [r[1] for r in c0]
    sp = {}
    for t in (0, 1):
        xs = [r[3] for r in c0 if r[0] == t]; ys = [r[4] for r in c0 if r[0] == t]
        sp[t] = (sum(xs) / len(xs), sum(ys) / len(ys))
    acts = d["actions"]
    by_round = collections.defaultdict(list)
    for a in acts:
        by_round[a[0]].append(a)
    out = {"kills": [], "channels": [], "locks": [], "stuns": [0, 0], "dead": [0, 0]}
    # time dead
    for s in snaps:
        for i in range(n):
            if not s["c"][i][2]:
                out["dead"][team[i]] += 1
    # deaths and killers
    for a, b in zip(snaps, snaps[1:]):
        for i in range(n):
            if a["c"][i][2] and not b["c"][i][2]:
                cand = None
                for r in (a["r"], b["r"]):
                    for act in by_round.get(r, []):
                        if team[act[1]] != team[i] and any(h[0] == "c" and h[1] == i and h[2] > 0 for h in act[9]):
                            cand = act[1]
                out["kills"].append([b["r"], i, team[i], typ[i], a["c"][i][7], typ[cand] if cand is not None else -1, a["c"][i][3], a["c"][i][4]])
    # stun casts
    for a in acts:
        if a[2] == 6 and a[11] == 4:
            out["stuns"][team[a[1]]] += 1
    cc = [a for a in acts if a[2] == 6 and a[11] == 4]
    locks = [a for a in acts if a[2] == 20]
    chans = []
    for a in acts:
        if a[2] != 11:
            continue
        r0, i = a[0], a[1]
        t = team[i]
        B = a[7]
        c = S.get(r0, {}).get("c", [None] * n)[i] if r0 in S else None
        if not c:
            continue
        px, py = c[3], c[4]
        tl = B > 0 and any(cheb(px, py, x, y) <= 2 for x, y in ALT.get(info["mapId"], ()))
        W = max(8, (math.ceil(B / 10) if B > 0 else 0) + 3)
        res, rend = "other", r0 + W
        for r in range(r0 + 1, r0 + W + 1):
            if r not in S or r - 1 not in S:
                continue
            cur, prv = S[r]["c"][i], S[r - 1]["c"][i]
            if not cur[2]:
                res, rend = "killed", r; break
            if S[r]["sg"][t] > S[r - 1]["sg"][t] and cur[7] < prv[7]:
                res, rend = "tribute", r; break
            if cheb(cur[3], cur[4], prv[3], prv[4]) >= 8 and cheb(cur[3], cur[4], sp[t][0], sp[t][1]) <= 3:
                res, rend = "recall", r; break
        cause = res
        if res in ("other", "killed"):
            if any(team[x[1]] != t and r0 <= x[0] <= rend and cheb(px, py, x[5], x[6]) <= 1 for x in cc):
                cause = "stunned"
            elif any(team[x[1]] != t and r0 <= x[0] <= rend and cheb(px, py, x[5], x[6]) <= 3 for x in locks):
                cause = "locked"
            elif res == "killed":
                cause = "killed"
            else:
                e = S.get(min(rend, max(S)), {}).get("c", [None] * n)[i]
                cause = "hit" if e and e[5] < c[5] else "other"
        chans.append({"r0": r0, "i": i, "t": t, "type": typ[i], "bag": B, "tl": int(tl), "res": res, "cause": cause, "rend": rend, "x": px, "y": py})
    out["channels"] = chans
    for a in locks:
        if not (a[12] & 4 and not a[13] & 4):
            continue
        r, i = a[0], a[1]
        t = team[i]
        hit = [ch for ch in chans if ch["t"] != t and ch["tl"] and ch["r0"] <= r <= ch["rend"] and cheb(ch["x"], ch["y"], a[5], a[6]) <= 3]
        out["locks"].append({"r": r, "i": i, "t": t, "type": typ[i], "x": a[5], "y": a[6], "cancelled": int(any(ch["cause"] == "locked" for ch in hit)), "enemy_tributing": int(bool(hit))})
    return out


if __name__ == "__main__":
    idx = load_index()
    files = []
    for f in sorted(glob.glob(f"{V3}/decoded/full/*.json")):
        mid = os.path.basename(f)[:-5]
        if mid in idx:
            files.append((f, mid, idx[mid]))
    ALT = altar_tiles(files)
    print(f"full-action games: {len(files)}; altar standing tiles learned on {len(ALT)} maps", file=sys.stderr)
    rows = []
    with open(f"{V3}/decoded/actions_full.jsonl", "w", encoding="utf8", newline="\n") as fh:
        for f, mid, info in files:
            d, S = load(f)
            r = analyse(d, S, info, ALT)
            tid = info["teamIds"]
            coh = "xan" if XAN in tid else ("micron" if MIC in tid else ("top10" if all(RANK.get(x, 999) <= 10 for x in tid) else "other"))
            r.update({"match": mid, "map": info["map"], "teams": info["teams"], "teamIds": tid, "cohort": coh, "rounds": info["rounds"], "winner": int(info["score"][1] > info["score"][0])})
            fh.write(json.dumps(r, ensure_ascii=False) + "\n")
            rows.append(r)
    print(f"games {len(rows)}; by cohort {dict(collections.Counter(r['cohort'] for r in rows))}")
