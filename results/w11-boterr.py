"""Count 'bot error' lines on our side per result file: python results/w11-boterr.py a.json [b.json ...] [--opps x,y] [--salts 0]"""
import json, sys, zipfile, collections
args = [a for a in sys.argv[1:] if not a.startswith("--")]
opts = dict(a[2:].split("=") for a in sys.argv[1:] if a.startswith("--"))
for f in args:
    d = json.load(open(f)); n = g = 0; kinds = collections.Counter()
    for r in d["results"]:
        if "opps" in opts and r["opp"] not in opts["opps"].split(","): continue
        if "salts" in opts and str(r["salt"]) not in opts["salts"].split(","): continue
        g += 1
        try: out = zipfile.ZipFile(r["replay"]).read("team-a.stdout" if r["side"] == "A" else "team-b.stdout").decode(errors="replace")
        except Exception as e: kinds["unreadable"] += 1; continue
        for l in out.splitlines():
            if "bot error" in l: n += 1; kinds[l.strip()[:70]] += 1
    print(f"{f}: games {g}, bot error lines {n} ({n / max(1, g):.2f}/game)", dict(kinds))
