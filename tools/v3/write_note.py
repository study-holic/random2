"""Compose notes/v3-data-scope.md from cached data (top50.json, groups_ranked.jsonl, raw/groups, lists/index.jsonl, pilot outputs). Run: py -I tools/v3/write_note.py"""
import json, glob, csv, collections, os, statistics, sys
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
V3 = "C:/cambc-solo/data/v3"
top = json.load(open(f"{V3}/top50.json", encoding="utf8"))["top50"]; R = {t["teamId"]: t["rank"] for t in top}
cur = {r["map_id"]: r["map_name"] for r in csv.DictReader(open("C:/cambc-solo/intel/matches-2026-10-18.csv", encoding="utf8"))}
gl = [g for g in (json.loads(l) for l in open(f"{V3}/groups_ranked.jsonl", encoding="utf8")) if g["status"] == "finished"]
per = collections.Counter()
for g in gl:
    for s in g["sides"]:
        if s["teamId"] in R: per[s["teamId"]] += 1
pt = sorted(per.values())
patch = collections.defaultdict(collections.Counter); ids = collections.defaultdict(set)
for f in glob.glob(f"{V3}/raw/groups/*.json"):
    for m in json.load(open(f, encoding="utf8"))["matches"]:
        p = (m.get("patchId") or "none")[:5]; patch[p]["n"] += 1; patch[p]["cur"] += m["mapId"] in cur; ids[p].add(m["mapId"])
        patch[p]["endReason"] += m.get("endReason") is not None; patch[p]["draft"] += bool(m["sides"][0].get("draft"))
nm = lambda t: t["name"].replace("|", "/")[:22]
rows = []
for i in range(25):
    a, b = top[i], top[i + 25]
    rows.append(f"| {a['rank']} | {nm(a)} ({a['teamId'][:6]}) | {a['rating']:.0f} | {per[a['teamId']]} | {b['rank']} | {nm(b)} ({b['teamId'][:6]}) | {b['rating']:.0f} | {per[b['teamId']]} |")
P = lambda p, k: f"{patch[p][k]:,}"
tbl = "\n".join(rows)
note = open("C:/cambc-solo/tools/v3/note_template.md", encoding="utf8").read()
note = note.replace("{{TOP50}}", tbl).replace("{{PT_MIN}}", str(pt[0])).replace("{{PT_MED}}", f"{statistics.median(pt):.0f}").replace("{{PT_MAX}}", str(pt[-1]))
for p in ("none", "0.0.2", "0.0.3"):
    note = note.replace("{{N_" + p + "}}", P(p, "n")).replace("{{CUR_" + p + "}}", P(p, "cur")).replace("{{IDS_" + p + "}}", str(len(ids[p])))
    note = note.replace("{{ER_" + p + "}}", f"{100 * patch[p]['endReason'] / patch[p]['n']:.0f}%").replace("{{DR_" + p + "}}", f"{100 * patch[p]['draft'] / patch[p]['n']:.0f}%")
open("C:/cambc-solo/notes/v3-data-scope.md", "w", encoding="utf8").write(note)
print(len(note.splitlines()), "lines")
