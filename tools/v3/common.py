"""Shared helpers for the v3 data scoping scripts: polite cached GET against the public cambc platform API.
Rate limit: at most 4 requests/second (0.26 s minimum gap). Every response is cached byte-for-byte on disk.
Run with: py -I tools/v3/<script>.py
"""
import json, os, sys, time, urllib.request, urllib.error, datetime

sys.stdout.reconfigure(encoding="utf-8", errors="replace")
BASE = "https://2026a.battlecode.co.uk"
API = BASE + "/api/v1/"
UA = {"User-Agent": "cambc-solo/0.2 (research; read-only; v3 scoping)"}
ROOT = "C:/cambc-solo"
V3 = f"{ROOT}/data/v3"
RAW = f"{V3}/raw"
_last = [0.0]
stats = {"req": 0, "bytes": 0, "r429": 0, "err": 0, "r202": 0}


def now():
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def _gap(min_gap=float(os.environ.get("CAMBC_GAP", "0.26"))):
    d = time.time() - _last[0]
    if d < min_gap:
        time.sleep(min_gap - d)
    _last[0] = time.time()


def get_bytes(url, tries=6, timeout=60):
    """GET url (absolute or relative to API). Returns (status, bytes). Backs off on 429/5xx/202."""
    full = url if url.startswith("http") else API + url
    for k in range(tries):
        _gap()
        try:
            stats["req"] += 1
            with urllib.request.urlopen(urllib.request.Request(full, headers=UA), timeout=timeout) as r:
                if r.status == 202:
                    stats["r202"] += 1; time.sleep(5 * (k + 1)); continue
                b = r.read(); stats["bytes"] += len(b)
                return r.status, b
        except urllib.error.HTTPError as e:
            if e.code in (429, 500, 502, 503, 504) and k < tries - 1:
                stats["r429" if e.code == 429 else "err"] += 1
                ra = e.headers.get("Retry-After")
                time.sleep(float(ra) if ra and ra.replace(".", "").isdigit() else 5 * (k + 1)); continue
            raise
        except (urllib.error.URLError, TimeoutError):
            if k < tries - 1:
                time.sleep(3 * (k + 1)); continue
            raise
    raise RuntimeError("gave up: " + full)


def cached_json(url, path, refresh=False):
    """Fetch API JSON once, cache the raw bytes at path, return parsed JSON."""
    if os.path.exists(path) and not refresh:
        return json.load(open(path, encoding="utf8"))
    os.makedirs(os.path.dirname(path), exist_ok=True)
    _, b = get_bytes(url)
    parsed = json.loads(b)
    with open(path + ".tmp", "wb") as f:
        f.write(b)
    os.replace(path + ".tmp", path)
    return parsed
