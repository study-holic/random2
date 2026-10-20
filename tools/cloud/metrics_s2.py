"""S2 minimal metrics (v5-common.md field names) from a mapgate run kept with --keep.
Usage: python3 tools/cloud/metrics_s2.py results/<run>.json [--by-map]
Per game and side (us = the mapgate bot, opp = the panel bot), from the run-batch statistics JSON:
  locks_bought  buyConsumable events of the altar lock
  locks_placed  our bot's stdout tokens "LK r<round>" (P_S2_ALTAR_LOCK prints one per placement; opp side: n/a)
  gear200       shards bought by round 200 per body (+1 per Power emblem formed, which counts as 6), mean over bodies
  gold100/200   unspent gold per body at rounds 100 and 200 (gold timeline)
  deaths, gems, gems_opp, win
"""
import json, os, sys, zipfile, collections


def gold_at(runs, rnd):
    t = 0
    for r in runs:
        if t + r["duration"] > rnd:
            return float(r["value"])
        t += r["duration"]
    return float(runs[-1]["value"]) if runs else 0.0


def side_metrics(d, team):
    bodies = [c for c in d["roster"]["champions"] if c["team"] == team]
    ids = {c["id"] for c in bodies}
    gear = collections.Counter()
    lb = 0
    for ev in d["events"]:
        o = ev["owner"]
        if o.get("id") not in ids:
            continue
        for x in ev["entries"]:
            r = x["timestamp"]["round"]
            if ev["kind"] == "buyItem" and r <= 200:
                gear[o["id"]] += int(float(x["value"]))
            elif ev["kind"] == "itemCombination" and r <= 200 and "Power Emblem" in str(x["target"].get("name", "")):
                gear[o["id"]] += 1
            elif ev["kind"] == "buyConsumable" and "ock" in json.dumps(x.get("target")):
                lb += 1
    g100 = g200 = 0.0
    for tl in d["championTimelines"]:
        if tl["metric"] == "gold" and tl["owner"].get("id") in ids:
            g100 += gold_at(tl["runs"], 100)
            g200 += gold_at(tl["runs"], 200)
    n = max(1, len(bodies))
    deaths = sum(len(ev["entries"]) for ev in d["events"] if ev["kind"] == "death" and ev["owner"].get("team") == team)
    return {"locks_bought": lb, "gear200": sum(gear.values()) / n, "gold100": g100 / n, "gold200": g200 / n, "deaths": deaths}


def stdout_count(replay, side, token="LK"):
    try:
        with zipfile.ZipFile(replay) as z:
            name = "team-a.stdout" if side == "A" else "team-b.stdout"
            txt = z.read(name).decode("utf8", "replace") if name in z.namelist() else ""
        return txt.count(token + " r")
    except Exception:
        return None


def main():
    run = json.load(open(sys.argv[1]))
    by_map = "--by-map" in sys.argv
    rows = []
    for r in run["results"]:
        stat = r["replay"][:-len(".repl26a")] + ".json"
        if not os.path.exists(stat):
            continue
        d = json.load(open(stat))
        us, them = ("teamA", "teamB") if r["side"] == "A" else ("teamB", "teamA")
        m = {"map": r["map"], "win": r["win"], "gems": r.get("gems", 0), "gems_opp": r.get("gems_opp", 0)}
        for k, v in side_metrics(d, us).items(): m[k] = v
        for k, v in side_metrics(d, them).items(): m["opp_" + k] = v
        m["locks_placed"] = stdout_count(r["replay"], r["side"]) or 0
        rows.append(m)
    if not rows:
        print("no statistics JSON found (run mapgate with --keep)"); return
    keys = ["win", "gems", "gems_opp", "deaths", "opp_deaths", "locks_bought", "locks_placed", "opp_locks_bought",
            "gear200", "opp_gear200", "gold100", "opp_gold100", "gold200", "opp_gold200"]
    def line(lbl, rs):
        n = len(rs)
        return f"{lbl:<20} n{n:>4} " + " ".join(f"{k}={sum(float(x[k]) for x in rs) / n:.2f}" for k in keys)
    print(line(run["bot"] + " vs " + ",".join(run["panel"]), rows))
    if by_map:
        for mp in sorted({x["map"] for x in rows}):
            print(line(mp, [x for x in rows if x["map"] == mp]))


if __name__ == "__main__":
    main()
