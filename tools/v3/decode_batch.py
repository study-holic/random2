"""Run extract_pilot.mjs over a replay list with N parallel node workers (shards), round-level by default.
Usage: py -I tools/v3/decode_batch.py <ids.txt> --workers 3 [--stride 13] [--out data/v3/decoded/s13]
Replays are read from data/v3/replays/<id>.zip; outputs <out>/<id>.json (resumable). Keep workers <= 3 while simulations run.
"""
import os, sys, subprocess, argparse
ap = argparse.ArgumentParser(); ap.add_argument("ids"); ap.add_argument("--workers", type=int, default=3); ap.add_argument("--stride", type=int, default=13)
ap.add_argument("--out", default="C:/cambc-solo/data/v3/decoded/s13"); a = ap.parse_args()
ids = [l.strip() for l in open(a.ids) if l.strip()]
files = [f"C:/cambc-solo/data/v3/replays/{i}.zip" for i in ids if os.path.exists(f"C:/cambc-solo/data/v3/replays/{i}.zip") and not os.path.exists(f"{a.out}/{i}.json")]
print(len(files), "to decode;", a.workers, "workers", flush=True)
os.makedirs(a.out, exist_ok=True)
node = "C:/Program Files/nodejs/node.exe"; js = "C:/cambc-solo/tools/v3/extract_pilot.mjs"
ps = [subprocess.Popen([node, "--no-warnings", js, *files[w::a.workers], "--out", a.out, "--stride", str(a.stride)], cwd=os.path.expanduser("~")) for w in range(a.workers)]
sys.exit(max(p.wait() for p in ps))
