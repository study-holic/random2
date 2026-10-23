"""census_build.py: read runs/census/replays/*.repl26a -> data/census/<map>.jsonl, data/ENGINE-FACTS.json, data/census-groups.json.
Global records (settings, passive constants, champions, items, consumables) must be identical on every map and both seats;
any difference is reported. Prints a summary of 20 lines or fewer."""
import json, os, sys, zipfile, collections, glob

ROOT = "/home/claude"
src = sorted(glob.glob(f"{ROOT}/runs/census/replays/*.repl26a"))
os.makedirs(f"{ROOT}/data/census", exist_ok=True)
glob_recs = collections.defaultdict(dict)  # rec -> key -> json string -> [maps]
maps = {}
incomplete = []
for path in src:
    name = os.path.basename(path).split(".")[0][2:].replace("-", "_")
    z = zipfile.ZipFile(path)
    per_seat = {}
    for t in ("a", "b"):
        raw = z.read(f"team-{t}.stdout").replace(b"\x00", b"").decode("utf8", "replace")
        recs = [json.loads(l[7:]) for l in raw.splitlines() if l.startswith("CENSUS ")]
        per_seat[t] = recs
    with open(f"{ROOT}/data/census/{name}.jsonl", "w") as f:
        for t, recs in per_seat.items():
            for r in recs:
                f.write(json.dumps(dict(seat=t, **r)) + "\n")
    recs = per_seat["a"]
    kinds = collections.Counter(r["rec"] for r in recs)
    if not kinds.get("done"):
        incomplete.append(name)
    for t, rs in per_seat.items():
        for r in rs:
            k = r["rec"]
            if k in ("settings", "passive_constants"):
                glob_recs[k].setdefault("all", collections.defaultdict(list))[json.dumps(r, sort_keys=True)].append(f"{name}/{t}")
            elif k in ("champion", "item", "consumable"):
                glob_recs[k].setdefault(r.get("type", r.get("i")), collections.defaultdict(list))[json.dumps(r, sort_keys=True)].append(f"{name}/{t}")
    mrec = next(r for r in recs if r["rec"] == "map")
    plays = [r for r in recs if r["rec"] == "play"]
    picks = [r["champ"] for r in recs if r["rec"] == "pick"]
    groups = [r for r in recs if r["rec"] == "group"]
    gb = [r for r in per_seat["b"] if r["rec"] == "group"]
    maps[name] = dict(width=mrec["width"], height=mrec["height"], symmetry=mrec["symmetry"], team_size=mrec["team_size"],
                      picks=picks, bodies=len(plays), altars=mrec["altars"], walls=mrec["walls"], holes=mrec["holes"], bushes=mrec["bushes"],
                      spawn_a=plays[0]["spawn_ally"] if plays else None, spawn_b=plays[0]["spawn_enemy"] if plays else None,
                      groups=[dict(i=g["i"], gems=g["gems"], gold=g["gold"], respawn=g["respawn"], spawn_tiles=g["spawn_tiles"],
                                   camps=[dict(kind=c["kind"], pos=c["pos"]) for c in g["camps"]]) for g in groups],
                      group_lists_match_between_seats=(json.dumps([(g["gems"], g["gold"], g["respawn"]) for g in groups]) ==
                                                       json.dumps([(g["gems"], g["gold"], g["respawn"]) for g in gb])))
# camp base stats by kind across every map
camp_kinds = collections.defaultdict(collections.Counter)
for path in src:
    name = os.path.basename(path).split(".")[0][2:].replace("-", "_")
    for l in open(f"{ROOT}/data/census/{name}.jsonl"):
        r = json.loads(l)
        if r["rec"] == "group" and r["seat"] == "a":
            for c in r["camps"]:
                camp_kinds[c["kind"]][json.dumps(c["base"], sort_keys=True)] += 1

def single(rec):
    out, conflicts = {}, []
    for key, variants in glob_recs[rec].items():
        if len(variants) > 1:
            conflicts.append((rec, key, {v[:120]: len(m) for v, m in variants.items()}))
        best = max(variants.items(), key=lambda kv: len(kv[1]))[0]
        d = json.loads(best); d.pop("seat", None); d.pop("rec", None)
        out[str(key)] = d
    return out, conflicts

facts, conflicts = {}, []
for rec in ("settings", "passive_constants", "champion", "item", "consumable"):
    v, c = single(rec); conflicts += c
    facts[rec] = v["all"] if rec in ("settings", "passive_constants") else v
facts["camp_base_stats"] = {k: [json.loads(s) | {"n_camps": n} for s, n in c.items()] for k, c in camp_kinds.items()}
facts["provenance"] = dict(engine="1.0.3", cli="cambc 3.0.5", sdk="3.0.5", source="census bot (bots/census) on all 40 platform maps, 23 Oct 2026",
                           label="KNOWN (engine output)")
json.dump(facts, open(f"{ROOT}/data/ENGINE-FACTS.json", "w"), indent=1, sort_keys=True)
json.dump(maps, open(f"{ROOT}/data/census-groups.json", "w"), indent=0)

print("maps", len(maps), "incomplete", incomplete)
print("conflicts across maps/seats:", len(conflicts))
for c in conflicts[:5]:
    print("  ", str(c)[:200])
print("team_size by map:", dict(collections.Counter(m["team_size"] for m in maps.values())))
print("bodies - team_size (Cassi+Issac adds one):", dict(collections.Counter(m["bodies"] - m["team_size"] for m in maps.values())))
print("seat group lists equal:", sum(m["group_lists_match_between_seats"] for m in maps.values()), "of", len(maps))
print("camp kinds:", {k: sum(c.values()) for k, c in camp_kinds.items()}, "variants:", {k: len(c) for k, c in camp_kinds.items()})
print("maps with golems:", sorted(n for n, m in maps.items() if any(c["kind"] == "golem" for g in m["groups"] for c in g["camps"])))
print("settings:", json.dumps(facts["settings"])[:600])
