"""census_read.py BUNDLE: summarise the CENSUS records in a replay bundle (both teams)."""
import json, sys, zipfile, collections

z = zipfile.ZipFile(sys.argv[1])
for t in ("a", "b"):
    raw = z.read(f"team-{t}.stdout").replace(b"\x00", b"").decode("utf8", "replace")
    c = collections.Counter(); extra = []
    for line in raw.splitlines():
        if not line.startswith("CENSUS "):
            continue
        d = json.loads(line[7:]); c[d["rec"]] += 1
        if d["rec"] in ("pick", "pick_error", "done", "error", "map", "play"):
            extra.append({k: v for k, v in d.items() if k not in ("altars", "spawn_ally", "spawn_enemy")})
    print(t, dict(c))
    for e in extra[:14]:
        print("  ", json.dumps(e)[:200])
