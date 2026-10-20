# Sweep35 analysis (tools/cloud/sweep35.md). Inputs: mapgate JSONs run with --keep (statistics JSON beside each replay) and
# the stall decoder output (tools/cloud/stalls.mjs). Per game: tributed gems; deaths by killer kind (camp = monster,
# champion); per-turn fuel peak after round 1 and turns >= 90,000 / >= 100,000 (team-?.fuel); stall and altar-stall
# champion-rounds; zero-gem flag.
# usage: python3 -I tools/cloud/sweep35.py <stalls.jsonl> <out.json> <mapgate.json ...>
import json, os, struct, sys, zipfile, collections


def fuel_turns(bundle, side, nch):
    with zipfile.ZipFile(bundle) as z:
        b = z.read("team-%s.fuel" % side.lower())
    v = struct.unpack_from("<%dQ" % ((len(b) - 13) // 8), b, 13)
    late = v[13 + nch:]
    return max(late, default=0), sum(1 for x in late if x >= 90000), sum(1 for x in late if x >= 100000)


def deaths(stat, side):
    us = "teamA" if side == "A" else "teamB"
    d = json.load(open(stat))
    m = c = 0
    for ev in d["events"]:
        if ev["kind"] == "death" and ev["owner"].get("team") == us and ev["owner"].get("kind") == "champion":
            for e in ev["entries"]:
                if e["target"]["kind"] == "camp": m += 1
                else: c += 1
    return m, c


def main():
    stalls = {}
    for l in open(sys.argv[1]):
        r = json.loads(l)
        if "error" not in r: stalls[(r["bot"], r["opp"], r["map"], r["side"], r["salt"])] = r
    rows = []
    for f in sys.argv[3:]:
        mg = json.load(open(f))
        for g in mg["results"]:
            if "error" in g: rows.append({"bot": mg["bot"], "opp": g["opp"], "map": g["map"], "side": g["side"], "salt": g["salt"], "error": g["error"]}); continue
            rep = g["replay"]; stat = rep[:-len(".repl26a")] + ".json"
            fpk, f90, f100 = fuel_turns(rep, g["side"], g["nch"])
            dm, dc = deaths(stat, g["side"]) if os.path.exists(stat) else (None, None)
            s = stalls.get((mg["bot"], g["opp"], g["map"], g["side"], g["salt"]), {}).get("us", {})
            rows.append({"bot": mg["bot"], "opp": g["opp"], "map": g["map"], "side": g["side"], "salt": g["salt"], "win": g["win"],
                         "gems": g["gems"], "gems_opp": g["gems_opp"], "round": g["round"], "deaths_monster": dm, "deaths_champion": dc,
                         "fuel_peak": fpk, "turns_90k": f90, "turns_100k": f100, "fuel_exhausted": g.get("fuel_late", 0) + g.get("fuel_init", 0),
                         "stall_rounds": s.get("stall_rounds"), "altar_stall_rounds": s.get("altar_stall_rounds"),
                         "worst_stall": s.get("worst_stall"), "worst_at": s.get("worst_at"), "zero_gem": g["gems"] == 0, "faults": g["faults"]})
    json.dump(rows, open(sys.argv[2], "w"))
    print(len(rows), "rows ->", sys.argv[2])


if __name__ == "__main__":
    main()
