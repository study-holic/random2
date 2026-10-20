"""preflight: local vs platform ground-truth check. Writes STATE.md and state/ground-truth.json. Exit 1 on any FAIL.

python scripts/preflight.py [--fix] [--keep-scratch]
--fix installs the official map files into maps/ (old copies go to maps-prev-<date>/).
"""
import argparse, difflib, glob, hashlib, json, os, re, shutil, subprocess, sys, tempfile, time, urllib.request

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CAMBC = os.path.expandvars(r"%APPDATA%\Python\Python314\Scripts\cambc.exe")
TEAM = "6c59147e-576f-465f-8d00-bdde9ed12f22"
GT = os.path.join(ROOT, "state", "ground-truth.json")
rows = []  # (check, status, text)


def add(name, status, text):
    rows.append((name, status, text))


def run(args, tries=4):
    for i in range(tries):
        p = subprocess.run(args, capture_output=True, text=True, encoding="utf8", errors="replace", cwd=ROOT)
        if p.returncode == 0:
            return p.stdout
        if "429" in (p.stdout + p.stderr) and i < tries - 1:
            time.sleep(3 * (i + 1))
            continue
        raise RuntimeError((p.stderr or p.stdout).strip()[:200])


def cj(*a):
    return json.loads(run([CAMBC, *a, "--json"]))


def sha(b):
    return hashlib.sha256(b.replace(b"\r\n", b"\n")).hexdigest()[:16]


def local_map(norm):
    for n in (norm, norm.replace("_", "-")):
        p = os.path.join(ROOT, "maps", n + ".map26a")
        if os.path.exists(p):
            return p
    return os.path.join(ROOT, "maps", norm + ".map26a")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--fix", action="store_true")
    ap.add_argument("--keep-scratch", action="store_true")
    a = ap.parse_args()
    prev = json.load(open(GT)) if os.path.exists(GT) else {}
    gt = {"date": time.strftime("%Y-%m-%d %H:%M"), "tz": time.strftime("%Z")}
    scratch = tempfile.mkdtemp(prefix="cambc-preflight-")

    # a. CLI / SDK
    try:
        cli = run([CAMBC, "--version"]).split()[-1]
        gt["cli"] = cli
        try:
            out = subprocess.run(["py", "-3.14", "-m", "pip", "index", "versions", "cambc"], capture_output=True, text=True, timeout=40).stdout
            latest = re.search(r"cambc \(([\d.]+)\)", out).group(1)
        except Exception:
            latest = None
        tom = re.search(r'sdk_version\s*=\s*"([^"]+)"', open(os.path.join(ROOT, "cambc.toml")).read()).group(1)
        gt["cli_latest"], gt["sdk_toml"] = latest, tom
        run([CAMBC, "init", scratch, "--yes"])
        osdk = open(os.path.join(scratch, "sdk", "cpp", "cambc.h"), "rb").read()
        lsdk = open(os.path.join(ROOT, "sdk", "cpp", "cambc.h"), "rb").read()
        bad = []
        if tom != cli: bad.append(f"cambc.toml {tom} != CLI {cli}")
        if sha(osdk) != sha(lsdk): bad.append("sdk/cpp/cambc.h differs from the CLI's SDK")
        if bad: add("CLI", "FAIL", "; ".join(bad))
        elif latest and latest != cli: add("CLI", "WARN", f"CLI {cli}, PyPI has {latest}")
        elif latest is None: add("CLI", "WARN", f"CLI {cli}; PyPI check failed")
        else: add("CLI", "PASS", f"CLI {cli} = PyPI latest = toml; sdk/ matches")
    except Exception as e:
        add("CLI", "FAIL", f"error: {e}")

    # b. engine
    try:
        ms = cj("matches", "list", "--team", TEAM, "--limit", "30")["matches"]
        eng = None
        for m in ms:
            if m.get("status") == "finished":
                for t in m["teams"]:
                    if t["teamId"] == TEAM: eng = t["engineVersion"]
                if eng: break
        gt["engine"] = eng
        old = prev.get("engine")
        if eng is None: add("Engine", "WARN", "no finished match of ours found")
        elif old and old != eng: add("Engine", "FAIL", f"engine changed {old} -> {eng}: rules may have changed")
        elif not old: add("Engine", "WARN", f"engine {eng}; no baseline recorded (now saved)")
        else: add("Engine", "PASS", f"engine {eng} unchanged")
    except Exception as e:
        add("Engine", "FAIL", f"error: {e}")

    # c. map pool
    try:
        pool = {}
        for ln in run([CAMBC, "maps", "list"]).splitlines():
            m = re.match(r"([0-9a-f-]{36})\s+(\S+)", ln)
            if m: pool[m.group(2)] = m.group(1)
        official = {}
        for f in glob.glob(os.path.join(scratch, "maps", "*.map26a")):
            official[os.path.basename(f)[:-7]] = f
        names = {}
        for raw in official:
            names[re.sub(r"^\d\d", "", raw).replace("-", "_")] = raw
        gt["pool"] = sorted(names)
        gt["pool_ids"] = {re.sub(r"^\d\d", "", k).replace("-", "_"): v for k, v in pool.items()}
        hashes = {}
        missing, diff = [], []
        for n, raw in sorted(names.items()):
            ob = open(official[raw], "rb").read()
            hashes[n] = sha(ob)
            lp = local_map(n)
            if not os.path.exists(lp): missing.append(n)
            elif open(lp, "rb").read() != ob: diff.append(n)
        extra = sorted(os.path.basename(p)[:-7].replace("-", "_") for p in glob.glob(os.path.join(ROOT, "maps", "*.map26a")))
        extra = [e for e in extra if e not in names]
        gt["map_hashes"] = hashes
        gt["map_set_hash"] = hashlib.sha256(json.dumps(hashes, sort_keys=True).encode()).hexdigest()[:12]
        if {re.sub(r"^\d\d", "", k).replace("-", "_") for k in pool} != set(names): add("Maps", "WARN", f"maps list ({len(pool)}) vs pulled ({len(official)}) names differ")
        if missing or extra or diff:
            msg = f"pool {len(names)}; missing {missing}, extra {extra}, different {diff}"
            if a.fix:
                prevd = os.path.join(ROOT, "maps-prev-" + time.strftime("%Y-%m-%d"))
                os.makedirs(prevd, exist_ok=True)
                for n in missing + extra + diff:
                    lp = local_map(n)
                    if os.path.exists(lp): shutil.move(lp, os.path.join(prevd, os.path.basename(lp)))
                for n in missing + diff:
                    shutil.copy(official[names[n]], os.path.join(ROOT, "maps", n + ".map26a"))
                add("Maps", "WARN", "FIXED: " + msg + f" (old in {os.path.basename(prevd)})")
            else:
                add("Maps", "FAIL", msg + " (run --fix)")
        else:
            add("Maps", "PASS", f"{len(names)} maps byte-identical to official; set {gt['map_set_hash']}")
    except Exception as e:
        add("Maps", "FAIL", f"error: {e}")

    # d. rules
    try:
        raw = urllib.request.urlopen("https://2026a.battlecode.co.uk/GameRules.md", timeout=30).read()
        files = sorted(glob.glob(os.path.join(ROOT, "docs", "rules", "GameRules-site-*.md")))
        gt["rules_hash"] = sha(raw)
        msgs, st = [], "PASS"
        newf = os.path.join(ROOT, "docs", "rules", f"GameRules-site-{time.strftime('%Y-%m-%d')}.md")
        if files:
            old = open(files[-1], "rb").read()
            if sha(old) != sha(raw):
                n = sum(1 for l in difflib.unified_diff(old.decode("utf8", "replace").splitlines(), raw.decode("utf8", "replace").splitlines(), lineterm="", n=0) if l[:1] in "+-" and l[:3] not in ("+++", "---"))
                msgs.append(f"GameRules.md changed vs {os.path.basename(files[-1])}: {n} diff lines")
                if not os.path.exists(newf): open(newf, "wb").write(raw)
                msgs.append("saved " + os.path.basename(newf)); st = "WARN"
        else:
            msgs.append("no local copy; saved"); st = "WARN"
            open(newf, "wb").write(raw)
        h = sha(open(os.path.join(ROOT, "sdk", "cpp", "cambc.h"), "rb").read())
        gt["cambc_h_hash"] = h
        if prev.get("cambc_h_hash") and prev["cambc_h_hash"] != h: msgs.append("cambc.h hash changed"); st = "WARN"
        add("Rules", st, "; ".join(msgs) or f"site rules unchanged ({gt['rules_hash']}), cambc.h {h}")
    except Exception as e:
        add("Rules", "FAIL", f"error: {e}")

    # e. champions
    try:
        src = open(os.path.join(ROOT, "sdk", "cpp", "cambc.h"), encoding="utf8").read()
        body = re.search(r"enum class champion_draft[^{]*\{(.*?)\};", src, re.S).group(1)
        body = re.sub(r"///.*", "", body)
        n_enum = len([x for x in body.split(",") if x.strip()])
        ms, cur = [], None
        for _ in range(8):  # newest matches are queued and have no picks yet; page back to 100 played ones
            d = cj("matches", "list", "--kind", "ranked", "--limit", "100", *(["--cursor", cur] if cur else []))
            ms += [m for m in d["matches"] if any(t.get("draftPicks") for t in m["teams"])]
            cur = d.get("nextCursor")
            if len(ms) >= 100 or not cur: break
        ids = [p for m in ms for t in m["teams"] for p in (t.get("draftPicks") or [])]
        mx = max(ids or [0])
        gt["champions_sdk"], gt["champion_max_seen"] = n_enum, mx
        if mx >= n_enum: add("Champions", "FAIL", f"ladder shows id {mx}, SDK enum has {n_enum} (0..{n_enum-1})")
        else: add("Champions", "PASS", f"max id seen {mx} in {len(ms)} ranked matches; SDK has {n_enum}")
    except Exception as e:
        add("Champions", "FAIL", f"error: {e}")

    # f. ladder
    try:
        teams, page, total = [], 1, None
        while True:
            d = cj("ladder", "--limit", "100", "--page", str(page))
            total = d.get("total", total)
            teams += d["teams"]
            if not d["teams"] or (any(t["teamId"] == TEAM for t in teams) and len(teams) >= 100): break
            if len(teams) >= (total or 0): break
            page += 1
        me = next((t for t in teams if t["teamId"] == TEAM), None)
        at = {t["rank"]: round(t["rating"]) for t in teams if t["rank"] in (10, 20, 50, 100)}
        gt["ladder"] = {"total": total, "rank": me and me["rank"], "rating": me and round(me["rating"], 1), "matches": me and me["matches"], "at_rank": at}
        if me: add("Ladder", "PASS", f"{me['name']} rank {me['rank']}/{total}, rating {me['rating']:.0f}, {me['matches']} games; r10 {at.get(10)}, r20 {at.get(20)}, r50 {at.get(50)}, r100 {at.get(100)}")
        else: add("Ladder", "WARN", f"team not found in {len(teams)} listed of {total}")
    except Exception as e:
        add("Ladder", "FAIL", f"error: {e}")

    # g. live bot
    try:
        live = None
        for ln in open(os.path.join(ROOT, "VERSIONS.md"), encoding="utf8"):
            if ln.startswith("## ") and re.search(r"upload|submit", ln, re.I) and not re.search(r"not uploaded|unreleased|not submitted", ln, re.I):
                live = ln[3:].strip()
        gt["live_bot"] = live
        add("Live bot", "PASS" if live else "WARN", (live or "no uploaded version in VERSIONS.md")[:110])
    except Exception as e:
        add("Live bot", "FAIL", f"error: {e}")

    os.makedirs(os.path.dirname(GT), exist_ok=True)
    json.dump(gt, open(GT, "w"), indent=1)
    worst = "FAIL" if any(r[1] == "FAIL" for r in rows) else "WARN" if any(r[1] == "WARN" for r in rows) else "PASS"
    L = [f"# STATE ({gt['date']} {gt['tz']}) overall {worst}", "", "| Check | Status | Detail |", "|---|---|---|"]
    L += [f"| {n} | {s} | {t.replace('|', '/')} |" for n, s, t in rows]
    L += ["", f"Pool ({len(gt.get('pool', []))}): " + ", ".join(gt.get("pool", [])), "Written by scripts/preflight.py; ground truth in state/ground-truth.json."]
    open(os.path.join(ROOT, "STATE.md"), "w", encoding="utf8").write("\n".join(L) + "\n")
    print("\n".join(f"{s:4} {n:9} {t}" for n, s, t in rows))
    print("overall", worst)
    if not a.keep_scratch: shutil.rmtree(scratch, ignore_errors=True)
    sys.exit(1 if worst == "FAIL" else 0)


if __name__ == "__main__":
    main()
