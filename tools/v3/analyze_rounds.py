"""Round-level game records from decoded replays (extract_pilot.mjs output) joined with the platform group details.
One JSON line per game in data/v3/decoded/games_<tag>.jsonl. Run with isolation: py -I tools/v3/analyze_rounds.py <decoded dir> <tag>

Team index 0 = platform side a, 1 = side b (verified: final submitted gems equal [score a, score b] in 5/5 v0_0_3 and 152/152 v0_0_2 games).
Events are tagged with the round of the snapshot that first shows them (round start state), i.e. they happened in the previous round.
Champion body types: 0 Ryth, 1 Vesper, 2 Vendetta, 3+4 Cassi and Issac (two bodies), 5 Stellar, 6 Prism, 7 Marcella, 8 Willow, 9 Ren, 10 Varg, 11 Tavi.
Item index = family*6 + {0 shard str, 1 emblem str, 2 shard focus, 3 emblem focus, 4 shard health, 5 emblem health}; families monster, power, durability, treasure, jungle, home, bruiser.
"""
import json, os, sys, glob, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
DRAFT = ['Ryth', 'Vesper', 'Vendetta', 'CassiIssac', 'Stellar', 'Prism', 'Marcella', 'Willow', 'Ren', 'Varg', 'Tavi']
TYPE_NAME = {0: 'Ryth', 1: 'Vesper', 2: 'Vendetta', 3: 'CassiIssac', 4: 'CassiIssac', 5: 'Stellar', 6: 'Prism', 7: 'Marcella', 8: 'Willow', 9: 'Ren', 10: 'Varg', 11: 'Tavi'}
MILES = (250, 500, 750, 1000)


def load_index():
    idx = {}
    for f in glob.glob(f"{V3}/raw/groups/*.json"):
        G = json.load(open(f, encoding="utf8")); g = G["group"]
        gs = {s["side"]: s for s in g["sides"]}
        for m in G["matches"]:
            sd = {s["side"]: s for s in m["sides"]}
            if "a" not in sd or "b" not in sd: continue
            idx[m["id"]] = {"group": g["id"], "mapId": m["mapId"], "map": m["map"], "rounds": m["rounds"], "t": m["createdAt"], "endReason": m.get("endReason"),
                            "teams": [sd["a"]["teamName"], sd["b"]["teamName"]], "teamIds": [sd["a"]["teamId"], sd["b"]["teamId"]],
                            "score": [sd["a"].get("score"), sd["b"].get("score")], "result": [sd["a"]["result"], sd["b"]["result"]],
                            "draft": [[DRAFT[int(x)] for x in (sd[k].get("draft") or [])] for k in ("a", "b")],
                            "rating": [(gs.get(k) or {}).get("ratingBefore") for k in ("a", "b")]}
    return idx


def analyse(d):
    snaps = d["rounds"]
    c0 = snaps[0]["c"]; n = len(c0)
    team = [r[0] for r in c0]; typ = [r[1] for r in c0]
    sp = {}
    for t in (0, 1):
        xs = [r[3] for r in c0 if r[0] == t]; ys = [r[4] for r in c0 if r[0] == t]
        sp[t] = (sum(xs) / len(xs), sum(ys) / len(ys))
    ev = {"tribute": [], "death": [], "recall": [], "item": [], "cons_buy": [], "cons_use": [], "capture": []}
    curve = {t: {} for t in (0, 1)}
    dead_rounds = [0] * n
    for s in snaps:
        for t in (0, 1):
            for m in MILES:
                if m not in curve[t] and s["sg"][t] >= m: curve[t][m] = s["r"]
        for i in range(n):
            if not s["c"][i][2]: dead_rounds[i] += 1
    for a, b in zip(snaps, snaps[1:]):
        r = b["r"]
        kinc = [0, 0]; kby = [[], []]
        for i in range(n):
            dk = b["c"][i][9] - a["c"][i][9]
            if dk > 0: kinc[team[i]] += dk; kby[team[i]].append(i)
        victims = [[], []]
        for i in range(n):
            ka, kb = a["c"][i], b["c"][i]
            if ka[2] and not kb[2]: victims[team[i]].append(i)
        for t in (0, 1):
            for i in victims[t]:
                ka = a["c"][i]
                # unique attribution only: one enemy kill increment and one victim of this team
                killer = kby[1 - t][0] if (len(victims[t]) == 1 and len(kby[1 - t]) == 1 and kinc[1 - t] == 1) else (-1 if kinc[1 - t] == 0 else -2)
                ev["death"].append([r, i, t, typ[i], ka[3], ka[4], ka[5], ka[7], typ[killer] if killer >= 0 else killer])
        for i in range(n):
            ka, kb = a["c"][i], b["c"][i]; t = team[i]
            if ka[2] and kb[2] and ka[3] >= 0 and kb[3] >= 0:
                if max(abs(kb[3] - ka[3]), abs(kb[4] - ka[4])) >= 8 and max(abs(kb[3] - sp[t][0]), abs(kb[4] - sp[t][1])) <= 3 and kb[5] >= ka[5]:
                    ev["recall"].append([r, i, t, typ[i], ka[3], ka[4], ka[5], ka[7]])
            on, off = kb[10] & ~ka[10], ka[10] & ~kb[10]
            for bit in range(8):
                if on >> bit & 1: ev["cons_buy"].append([r, i, t, typ[i], bit, ka[8] - kb[8]])
                if off >> bit & 1: ev["cons_use"].append([r, i, t, typ[i], bit, ka[3], ka[4]])
        for t in (0, 1):
            dg = b["sg"][t] - a["sg"][t]
            if dg > 0:
                who = [i for i in range(n) if team[i] == t and b["c"][i][2] and a["c"][i][7] > b["c"][i][7]]
                k = max(1, len(who))
                for i in (who or [-1]):
                    ev["tribute"].append([r, t, i, typ[i] if i >= 0 else -1, round(dg / k), a["c"][i][3] if i >= 0 else -1, a["c"][i][4] if i >= 0 else -1, dg])
        for gi, (ga, gb) in enumerate(zip(a["g"], b["g"])):
            if gb[1] != ga[1] and gb[1] in (0, 1): ev["capture"].append([r, gi, gb[1]])
    prev = {}
    for s in snaps:
        for i, key in s.get("it", []):
            old = prev.get(i, {})
            new = {}
            for kv in key.split(","):
                if kv: j, v = kv.split(":"); new[int(j)] = int(v)
            c = s["c"][i]
            for j, v in new.items():
                if v > old.get(j, 0):
                    far = max(abs(c[3] - sp[team[i]][0]), abs(c[4] - sp[team[i]][1])) > 6 and c[2] == 1
                    ev["item"].append([s["r"], i, team[i], typ[i], j, v - old.get(j, 0), 1 if far else 0])
            prev[i] = new
    fin = snaps[-1]
    sg_at = {}
    for R_ in (100, 200, 300, 400, 500, 600):
        row = next((x for x in snaps if x["r"] >= R_), fin)
        sg_at[str(R_)] = list(row["sg"])
    return {"sg_at": sg_at, "n": n, "types": [sorted(typ[i] for i in range(n) if team[i] == t) for t in (0, 1)], "rounds": fin["r"], "sg": fin["sg"], "winner": int(fin["sg"][1] > fin["sg"][0]),
            "curve": {str(t): {str(m): v for m, v in curve[t].items()} for t in (0, 1)}, "dead_rounds": [sum(dead_rounds[i] for i in range(n) if team[i] == t) for t in (0, 1)],
            "kills": [sum(fin["c"][i][9] for i in range(n) if team[i] == t) for t in (0, 1)], "ev": ev, "engine": d.get("engine")}


if __name__ == "__main__":
    src, tag = sys.argv[1], sys.argv[2]
    idx = load_index()
    out, bad = 0, 0
    with open(f"{V3}/decoded/games_{tag}.jsonl", "w", encoding="utf8", newline="\n") as fh:
        for f in sorted(glob.glob(f"{src}/*.json")):
            m = os.path.basename(f)[:-5]
            if m not in idx: bad += 1; continue
            try:
                d = json.load(open(f, encoding="utf8")); r = analyse(d)
            except Exception as e:
                print("ERR", m, type(e).__name__, e); bad += 1; continue
            r.update({"match": m, **idx[m]})
            r["check_ok"] = r["sg"] == idx[m]["score"]
            fh.write(json.dumps(r, ensure_ascii=False) + "\n"); out += 1
    print(out, "games written;", bad, "skipped")
