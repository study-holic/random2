"""Single collector (~1 req/s) for ranked match groups of two samples: (a) current top 50, (b) teams rated 1450-1600.

Platform JSON only: /api/v1/ladder, /api/v1/match-groups?team= (to discover group ids), /api/v1/match-groups/{id}.
Never downloads replays. Backs off on 429, 5xx and HTTP 202 (queue full).
Listings round-robin over sample teams (newest first) until <listUntil>; after that, ranked group details are
fetched newest first, alternating samples; when the detail queue is empty, another listing round runs.
Usage: python -I collect.py <listUntilISO> <deadlineISO>
"""
import json, os, sys, time, glob, urllib.request, urllib.error, datetime

sys.stdout.reconfigure(encoding="utf-8", errors="replace")
API = "https://2026a.battlecode.co.uk/api/v1/"
UA = {"User-Agent": "cambc-solo/0.1 (research; read-only)"}
ROOT = "C:/cambc-solo"
list_until, deadline = sys.argv[1], sys.argv[2]
D = f"{ROOT}/data"; LIST = f"{D}/listings"; GRP = f"{D}/groups"; LOG = f"{ROOT}/logs/collect.log"
for p in (LIST, GRP, f"{ROOT}/logs"): os.makedirs(p, exist_ok=True)
now = lambda: datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
stats = {"req": 0, "r202": 0, "r429": 0, "err": 0}


def log(msg):
    line = f"{now()} {msg}"; print(line, flush=True)
    with open(LOG, "a", encoding="utf8") as f: f.write(line + "\n")


def get(url, tries=6):
    for k in range(tries):
        try:
            stats["req"] += 1
            with urllib.request.urlopen(urllib.request.Request(API + url, headers=UA), timeout=40) as r:
                if r.status == 202:
                    stats["r202"] += 1; time.sleep(10 * (k + 1)); continue
                return r.read()
        except urllib.error.HTTPError as e:
            if e.code in (429, 500, 502, 503, 504) and k < tries - 1:
                stats["r429" if e.code == 429 else "err"] += 1
                ra = e.headers.get("Retry-After")
                time.sleep(float(ra) if ra and ra.replace(".", "").isdigit() else 10 * (k + 1)); continue
            raise
        except (urllib.error.URLError, TimeoutError):
            if k < tries - 1: time.sleep(5 * (k + 1)); continue
            raise
    raise RuntimeError("gave up after retries (202/429)")


# ladder snapshot
if not os.path.exists(f"{D}/ladder.json"):
    teams, page = [], 1
    while True:
        d = json.loads(get(f"ladder?page={page}&pageSize=100&category=all")); teams += d["teams"]
        if len(d["teams"]) < 100 or page > 30: break
        page += 1; time.sleep(1)
    json.dump({"retrievedAt": now(), "teams": teams}, open(f"{D}/ladder.json", "w", encoding="utf8"))
L = json.load(open(f"{D}/ladder.json", encoding="utf8"))["teams"]
A = {t["teamId"]: t["name"] for t in L[:50]}
B = {t["teamId"]: t["name"] for t in L if 1450 <= t["rating"] <= 1600}
json.dump({"A": A, "B": B}, open(f"{D}/samples.json", "w", encoding="utf8"), ensure_ascii=False, indent=1)
log(f"samples: A={len(A)} B={len(B)}")

state = {}
for tid in list(A) + [t for t in B if t not in A]:
    p = f"{LIST}/{tid}.jsonl"; seen, oldest = set(), None
    if os.path.exists(p):
        for line in open(p, encoding="utf8"):
            g = json.loads(line); seen.add(g["id"])
            if oldest is None or g["createdAt"] < oldest["createdAt"]: oldest = g
    state[tid] = {"p": p, "seen": seen, "cursor": f"{oldest['createdAt']}_{oldest['id']}" if oldest else None, "more": True}

queue = {"a": {}, "b": {}}


def enqueue(g):
    if g.get("kind") != "ranked" or g.get("status") != "finished" or os.path.exists(f"{GRP}/{g['id']}.json"): return
    ids = [s["teamId"] for s in g["sides"]]
    if any(i in A for i in ids): queue["a"][g["id"]] = g["createdAt"]
    if any(i in B for i in ids): queue["b"][g["id"]] = g["createdAt"]


for s in state.values():
    if os.path.exists(s["p"]):
        for line in open(s["p"], encoding="utf8"): enqueue(json.loads(line))


def listing_round():
    n = 0
    for tid, s in state.items():
        if not s["more"] or now() >= deadline: continue
        try:
            d = json.loads(get(f"match-groups?scope=all&team={tid}" + (f"&before={s['cursor']}" if s["cursor"] else "")))
        except Exception as e:
            stats["err"] += 1; log(f"ERROR list {tid}: {type(e).__name__}: {e}"); time.sleep(1); continue
        gs = d.get("groups", [])
        with open(s["p"], "a", encoding="utf8") as f:
            for g in gs:
                if g["id"] not in s["seen"] and g.get("status") == "finished":
                    s["seen"].add(g["id"]); f.write(json.dumps(g) + "\n"); enqueue(g)
        if gs: s["cursor"] = f"{gs[-1]['createdAt']}_{gs[-1]['id']}"
        s["more"] = bool(gs) and bool(d.get("more")); n += 1
        time.sleep(1.0)
    return n


rnd, ok, turn = 0, 0, 0
while now() < deadline:
    pend = {k: [g for g in v if not os.path.exists(f"{GRP}/{g}.json")] for k, v in queue.items()}
    if now() < list_until or not (pend["a"] or pend["b"]):
        rnd += 1
        if listing_round() == 0: break
        log(f"listing round {rnd}: queued a={len(queue['a'])} b={len(queue['b'])} details={ok} {stats}")
        continue
    for _ in range(60):  # a batch of details, alternating samples, newest first
        if now() >= deadline: break
        k = "ab"[turn % 2] if pend["a"] and pend["b"] else ("a" if pend["a"] else "b"); turn += 1
        if not pend[k]: break
        gid = max(pend[k], key=lambda g: queue[k][g]); pend[k].remove(gid)
        if os.path.exists(f"{GRP}/{gid}.json"): continue
        try:
            raw = get(f"match-groups/{gid}"); json.loads(raw)
            open(f"{GRP}/{gid}.json.tmp", "wb").write(raw); os.replace(f"{GRP}/{gid}.json.tmp", f"{GRP}/{gid}.json"); ok += 1
        except Exception as e:
            stats["err"] += 1; log(f"ERROR detail {gid}: {type(e).__name__}: {e}")
        time.sleep(1.0)
    log(f"details={ok} pending a={len([g for g in queue['a'] if not os.path.exists(f'{GRP}/{g}.json')])} "
        f"b={len([g for g in queue['b'] if not os.path.exists(f'{GRP}/{g}.json')])} {stats}")
log(f"collector finished details={ok} {stats}")
