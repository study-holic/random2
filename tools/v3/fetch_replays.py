"""Download replay bundles for a list of match ids through the cambc CLI (uses the CLI's own `cambc login` session; no tokens are read or stored here).
The platform answers HTTP 429 "retry after N seconds" at roughly 0.5 downloads/s sustained, and the CLI itself stalls about 5.5 s on those.
This script runs a few CLI calls in parallel behind one global start-rate limiter and a shared pause after any 429, then retries the id.
Stores validated zips as data/v3/replays/<id>.zip (never executed; only read as bytes by tools/v3/decode.mjs). Resumable.
Usage: py -I tools/v3/fetch_replays.py data/v3/lists/T1.txt [--limit N] [--rate 1] [--threads 3]
"""
import os, sys, subprocess, time, argparse, glob, shutil, threading, re
from concurrent.futures import ThreadPoolExecutor

ap = argparse.ArgumentParser()
ap.add_argument("ids")
ap.add_argument("--limit", type=int, default=0)
ap.add_argument("--rate", type=float, default=1.0)
ap.add_argument("--threads", type=int, default=3)
a = ap.parse_args()
CAMBC = os.path.expandvars("%APPDATA%/Python/Python314/Scripts/cambc.exe")
OUT = "C:/cambc-solo/data/v3/replays"
os.makedirs(OUT, exist_ok=True)
ids = [l.strip() for l in open(a.ids) if l.strip()]
ids = [i for i in ids if not os.path.exists(f"{OUT}/{i}.zip")]
if a.limit:
    ids = ids[:a.limit]
lock = threading.Lock()
nxt = [0.0]
pause = [0.0]
cnt = {"ok": 0, "fail": 0, "r429": 0}
t_start = time.time()
RETRY = re.compile(r"429.*retry after (\d+)")
BUSY = re.compile(r"HTTP 409")


def one(mid):
    for attempt in range(8):
        with lock:  # global start-rate limiter plus shared pause after a 429
            now = time.time()
            start = max(nxt[0], pause[0], now)
            nxt[0] = start + 1 / a.rate
        if start > now:
            time.sleep(start - now)
        tmp = f"{OUT}/_tmp/{mid}"
        shutil.rmtree(tmp, ignore_errors=True)
        os.makedirs(tmp)
        try:
            r = subprocess.run([CAMBC, "matches", "download", mid, "-o", f"{tmp}/{mid}.bundle"], capture_output=True, text=True, timeout=180, stdin=subprocess.DEVNULL)
            ok = [p for p in glob.glob(f"{tmp}/**/*", recursive=True) if os.path.isfile(p) and open(p, "rb").read(2) == b"PK"]
            msg = r.stdout + r.stderr
            if ok and not r.returncode:
                shutil.move(ok[0], f"{OUT}/{mid}.zip")
                with lock:
                    cnt["ok"] += 1
                    if cnt["ok"] % 100 == 0:
                        print(time.strftime("%H:%M:%S"), "downloaded", cnt["ok"], "failed", cnt["fail"], "429s", cnt["r429"], round(cnt["ok"] / (time.time() - t_start), 2), "/s", flush=True)
                return
            if BUSY.search(msg):  # the platform allows one download at a time per account
                time.sleep(1.5)
                continue
            m = RETRY.search(msg)
            if m:
                with lock:
                    cnt["r429"] += 1
                    pause[0] = max(pause[0], time.time() + int(m.group(1)) + 1)
                continue
            with lock:
                cnt["fail"] += 1
            print("FAIL", mid, r.returncode, msg[:160].replace(chr(10), " "), flush=True)
            return
        except Exception as e:
            with lock:
                cnt["fail"] += 1
            print("FAIL", mid, type(e).__name__, e, flush=True)
            return
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
    with lock:
        cnt["fail"] += 1
    print("FAIL (429 x8)", mid, flush=True)


with ThreadPoolExecutor(a.threads) as ex:
    list(ex.map(one, ids))
print("done: downloaded", cnt["ok"], "failed", cnt["fail"], "429s", cnt["r429"], round(time.time() - t_start), "s", flush=True)
