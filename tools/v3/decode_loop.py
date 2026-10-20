"""Keep decoding downloaded replays until the downloader has finished and nothing is left (resumable).
Usage: py -I tools/v3/decode_loop.py <ids.txt> <out dir> [--workers 1] [--stride 13]
Stride 13 = round-level snapshots (about 1 s per game); stride 1 = full action log (about 8-12 s per game).
"""
import os, sys, subprocess, time, argparse

ap = argparse.ArgumentParser()
ap.add_argument("ids")
ap.add_argument("out")
ap.add_argument("--workers", type=int, default=1)
ap.add_argument("--stride", type=int, default=13)
a = ap.parse_args()
ids = [l.strip() for l in open(a.ids) if l.strip()]
REP = "C:/cambc-solo/data/v3/replays"


def pending():
    return [i for i in ids if os.path.exists(f"{REP}/{i}.zip") and not os.path.exists(f"{a.out}/{i}.json")]


idle = 0
while True:
    p = pending()
    if not p:
        left = [i for i in ids if not os.path.exists(f"{REP}/{i}.zip")]
        idle += 1
        if not left or idle > 180:  # nothing left to download, or 30 min without new files
            break
        time.sleep(10)
        continue
    idle = 0
    ch = f"C:/cambc-solo/data/v3/lists/_chunk_{os.path.basename(a.out)}.txt"
    with open(ch, "w", newline="\n") as f:
        f.write("\n".join(p[:100]) + "\n")
    subprocess.run([sys.executable, "-I", "C:/cambc-solo/tools/v3/decode_batch.py", ch, "--workers", str(a.workers), "--stride", str(a.stride), "--out", a.out], stdin=subprocess.DEVNULL)
print("decode loop finished", time.strftime("%H:%M:%S"))
