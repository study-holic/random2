"""Download the unranked baseline groups listed in data/unranked/groups.csv through the signed-in cambc CLI.
For each group: `cambc match-groups show <id> --json`; when all five matches are finished, `cambc match-groups download <id> -o data/unranked/dl/<label>`
(a zip of five per-match bundles, each holding replay.repl26a plus our bot's stdout/fuel log). The five bundles are saved as data/v3/replays/<matchid>.zip
(shared with the ranked pipeline) and one row per match goes to data/unranked/matches.jsonl.
Usage: py -I tools/v3/fetch_unranked.py     (resumable; rerun until every group is finished)
"""
import csv, io, json, os, re, subprocess, sys, time, zipfile

CAMBC = os.environ.get("CAMBC") or (os.path.expandvars(r"%APPDATA%\Python\Python314\Scripts\cambc.exe") if os.name == "nt" else (__import__("shutil").which("cambc") or "cambc"))
U = "C:/cambc-solo/data/unranked"
REP = "C:/cambc-solo/data/v3/replays"
OURS = "6c59147e-576f-465f-8d00-bdde9ed12f22"
os.makedirs(f"{U}/dl", exist_ok=True)
os.makedirs(REP, exist_ok=True)


def cli(*args, tries=8):
    for k in range(tries):
        r = subprocess.run([CAMBC, *args], capture_output=True, text=True, timeout=180, stdin=subprocess.DEVNULL)
        msg = r.stdout + r.stderr
        if r.returncode == 0:
            return r.stdout
        m = re.search(r"retry after (\d+)", msg)
        if "429" in msg or "409" in msg:
            time.sleep(int(m.group(1)) + 1 if m else 2 + k)
            continue
        raise RuntimeError(msg[:300])
    raise RuntimeError("gave up after retries")


rows = list(csv.DictReader(open(f"{U}/groups.csv", encoding="utf8")))
have = {}
if os.path.exists(f"{U}/matches.jsonl"):
    for l in open(f"{U}/matches.jsonl", encoding="utf8"):
        d = json.loads(l)
        have[d["group"]] = True
out = open(f"{U}/matches.jsonl", "a", encoding="utf8", newline="\n")
pending = 0
for r in rows:
    gid = r["group_id"]
    if gid in have:
        continue
    G = json.loads(cli("match-groups", "show", gid, "--json"))
    ms = G["matches"]
    if len(ms) < 5 or any(m["status"] != "finished" for m in ms):
        pending += 1
        print("not finished:", r["label"], [m["status"] for m in ms])
        continue
    dst = f"{U}/dl/{r['label']}"
    if not os.path.exists(dst):
        cli("match-groups", "download", gid, "-o", dst)
    z = zipfile.ZipFile(dst)
    for info in z.infolist():
        mid = re.search(r"[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}", info.filename).group(0)
        open(f"{REP}/{mid}.zip", "wb").write(z.read(info.filename))
    for m in ms:
        us = next(t for t in m["teams"] if t["teamId"] == OURS)
        them = next(t for t in m["teams"] if t["teamId"] != OURS)
        out.write(json.dumps({"label": r["label"], "opponent": r["opponent"], "group": gid, "match": m["matchId"], "position": m["position"], "map": m["map"], "mapId": m["mapId"],
                              "our_side": us["side"], "result": us["result"], "our_draft": us["draftPicks"], "opp_draft": them["draftPicks"], "opp_name": them["name"], "opp_id": them["teamId"],
                              "finished": m["finishedAt"], "engine": us.get("engineVersion")}, ensure_ascii=False) + "\n")
    out.flush()
    print("downloaded", r["label"], flush=True)
    time.sleep(1.0)
print("pending groups:", pending)
