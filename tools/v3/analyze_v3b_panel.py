"""v3b real-team panel: reads data/unranked/sumj/<match>.json (cambc matches summary, engine-independent) + matches.jsonl; writes data/unranked/games_v3b.tsv.
Run: py -I tools/v3/analyze_v3b_panel.py   (prints nothing big; tables made by tools/v3/note_v3b_panel.py)"""
import json, os, statistics
U = "C:/cambc-solo/data/unranked"
def runs_at(tl, rounds):
    out = {}; pos = 0; it = iter(tl["runs"]); cur = None; t0 = 0; res = {}
    seq = []; r = 0
    for x in tl["runs"]:
        seq.append((r, int(float(x["value"])))); r += x["duration"]
    for q in rounds:
        v = 0
        for s, val in seq:
            if s <= q: v = val
        res[q] = v
    return res
rows = []
for l in open(f"{U}/matches.jsonl", encoding="utf8"):
    m = json.loads(l)
    if not m["label"].startswith("v3b"): continue
    f = f"{U}/sumj/{m['match']}.json"
    if not os.path.exists(f): continue
    d = json.load(open(f, encoding="utf8"))
    us = "teamA" if m["our_side"] == "a" else "teamB"; th = "teamB" if us == "teamA" else "teamA"
    sg = {t["team"]: int(t["submittedGems"]) for t in d["teams"]}
    tr = {"teamA": [], "teamB": []}; de = {"teamA": [], "teamB": []}
    for e in d["events"]:
        t = e["owner"]["team"] if isinstance(e["owner"], dict) and "team" in e["owner"] else None
        if t not in tr: continue
        if e["kind"] == "gemSubmission":
            for x in e["entries"]:
                if int(float(x["value"])) > 0: tr[t].append((x["timestamp"]["round"], int(float(x["value"]))))
        if e["kind"] == "death":
            for x in e["entries"]: de[t].append(x["timestamp"]["round"])
    tl = {t["owner"]: t for t in d["teamTimelines"] if t["metric"] == "submittedGems"}
    R = d["match"]["roundsCompleted"]
    at = {t: runs_at(tl[t], [100, 200, 300, 500]) for t in tl}
    r = dict(label=m["label"], opp=m["opponent"], map=m["map"], match=m["match"], side=m["our_side"], res=m["result"], rounds=R,
             g_us=sg[us], g_th=sg[th], tr_us=len(tr[us]), tr_th=len(tr[th]), bag_us=sum(b for _, b in tr[us]), bag_th=sum(b for _, b in tr[th]),
             first_us=min((a for a, _ in tr[us]), default=""), first_th=min((a for a, _ in tr[th]), default=""),
             d_us=len(de[us]), d_th=len(de[th]), d_us200=sum(1 for a in de[us] if a < 200), d_th200=sum(1 for a in de[th] if a < 200),
             us100=at[us][100], th100=at[th][100], us200=at[us][200], th200=at[th][200], us300=at[us][300], th300=at[th][300], us500=at[us][500], th500=at[th][500],
             draft="".join(str(x) + "." for x in m["our_draft"]).strip("."), faults=d["diagnostics"]["botFaults"]["teamA" if us == "teamA" else "teamB"])
    rows.append(r)
cols = list(rows[0]); 
with open(f"{U}/games_v3b.tsv", "w", encoding="utf8", newline="\n") as fh:
    fh.write("\t".join(cols) + "\n")
    for r in rows: fh.write("\t".join(str(r[c]) for c in cols) + "\n")
print(len(rows), "games")
