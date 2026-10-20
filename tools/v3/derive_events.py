"""Turn extract_pilot.mjs output (round snapshots, optional action log) into per-game events and a summary.
Reads untrusted-derived JSON, so run with: py -I tools/v3/derive_events.py <dir-of-json> <out.json>
Event definitions (round resolution; at stride 1 snapshots are the first state seen in each round):
  death    alive 1->0 between snapshots: round, champion, team, type, last-seen xy, hp, carried gems lost, enemy kills gained that round
  tribute  team's submittedGems rose by d: round, team, champion(s) whose bag fell, bag size d, last-seen xy
  recall   alive both snapshots, moved >=8 tiles to within 3 of the team's round-1 spawn centroid with hp not lower
  buy      item count +1 (item) or consumable-mask bit set (consumable index = bit)
  use      consumable-mask bit cleared (altar lock or crystal; index = bit), last-seen xy
  capture  camp-group captureTeam changed: round, group index, team
"""
import json, os, sys, glob, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")

def derive(d):
    snaps, last = [], -1
    for s in d["rounds"]:
        if s["r"] != last or s.get("final"):
            snaps.append(s); last = s["r"]
    c0 = snaps[0]["c"]; n = len(c0)
    sp = {}
    for t in (0, 1):
        xs = [r[3] for r in c0 if r[0] == t]; ys = [r[4] for r in c0 if r[0] == t]
        sp[t] = (sum(xs) / len(xs), sum(ys) / len(ys))
    ev = {"death": [], "tribute": [], "recall": [], "buy": [], "use": [], "capture": []}
    for a, b in zip(snaps, snaps[1:]):
        r = b["r"]; kd = [0, 0]
        for i in range(n):
            ka, kb = a["c"][i], b["c"][i]
            if kb[9] > ka[9]: kd[kb[0]] += kb[9] - ka[9]
        for i in range(n):
            ka, kb = a["c"][i], b["c"][i]; t = ka[0]
            if ka[2] and not kb[2]:
                ev["death"].append([r, i, t, ka[1], ka[3], ka[4], ka[5], ka[7], kd[1 - t]])
            if ka[2] and kb[2] and kb[3] >= 0 and ka[3] >= 0:
                dist = max(abs(kb[3] - ka[3]), abs(kb[4] - ka[4]))
                if dist >= 8 and max(abs(kb[3] - sp[t][0]), abs(kb[4] - sp[t][1])) <= 3 and kb[5] >= ka[5]:
                    ev["recall"].append([r, i, t, ka[1], ka[3], ka[4], ka[5], ka[7]])
            if kb[11] > ka[11]: ev["buy"].append([r, i, t, ka[1], "item", ka[8] - kb[8]])
            on, off = kb[10] & ~ka[10], ka[10] & ~kb[10]
            for bit in range(8):
                if on >> bit & 1: ev["buy"].append([r, i, t, ka[1], f"cons{bit}", ka[8] - kb[8]])
                if off >> bit & 1: ev["use"].append([r, i, t, ka[1], f"cons{bit}", ka[3], ka[4]])
        for t in (0, 1):
            dg = b["sg"][t] - a["sg"][t]
            if dg > 0:
                who = [i for i in range(n) if a["c"][i][0] == t and a["c"][i][7] > b["c"][i][7] - 0 and a["c"][i][7] > 0 and b["c"][i][7] < a["c"][i][7]]
                for i in who or [-1]:
                    ev["tribute"].append([r, t, i, a["c"][i][1] if i >= 0 else -1, dg, a["c"][i][3] if i >= 0 else -1, a["c"][i][4] if i >= 0 else -1])
        for gi, (ga, gb) in enumerate(zip(a["g"], b["g"])):
            if gb[1] != ga[1] and gb[1] in (0, 1): ev["capture"].append([r, gi, gb[1]])
    fin = snaps[-1]
    summ = {"rounds": fin["r"], "sg": fin["sg"], "winner": int(fin["sg"][1] > fin["sg"][0]), "n_champs": n,
            "types": [sorted(r[1] for r in c0 if r[0] == t) for t in (0, 1)], "drafts": d.get("drafts")}
    for t in (0, 1):
        tr = [e for e in ev["tribute"] if e[1] == t]
        ut = {e[0]: e[4] for e in tr}  # one tribute completion per (round, team); a bag shared by several champions counts once
        summ[f"t{t}"] = {"first_tribute": min(ut, default=None), "n_tribute_events": len(ut),
                         "mean_bag": round(sum(ut.values()) / len(ut), 1) if ut else None,
                         "deaths": sum(e[2] == t for e in ev["death"]), "gems_lost_on_death": sum(e[7] for e in ev["death"] if e[2] == t),
                         "recalls": sum(e[2] == t for e in ev["recall"]), "buys": sum(e[2] == t for e in ev["buy"]),
                         "cons_uses": sum(e[2] == t for e in ev["use"]), "captures": sum(e[2] == t for e in ev["capture"])}
    return ev, summ

if __name__ == "__main__":
    src, out = sys.argv[1], sys.argv[2]
    res = {}
    for f in sorted(glob.glob(os.path.join(src, "*.json"))):
        d = json.load(open(f, encoding="utf8"))
        ev, summ = derive(d)
        res[os.path.basename(f)[:-5]] = {"summary": summ, "events": ev}
    json.dump(res, open(out, "w", encoding="utf8"))
    print(len(res), "games ->", out)
