"""Count caught 'bot error' lines on our side per map in a mapgate JSON's replay bundles. usage: py -I w7_boterr.py run.json [...]"""
import json, sys, zipfile, collections
for f in sys.argv[1:]:
    d = json.load(open(f)); c = collections.Counter(); n = collections.Counter()
    for r in d["results"]:
        try:
            z = zipfile.ZipFile(r["replay"])
            out = z.read("team-a.stdout" if r["side"] == "A" else "team-b.stdout").replace(b"\0", b"").decode("utf-8", "replace")
        except Exception:
            continue
        c[r["map"]] += out.count("bot error"); n[r["map"]] += 1
    print(f, " ".join(f"{m}:{c[m]}/{n[m]}" for m in sorted(n)), "total", sum(c.values()), "/", sum(n.values()))
