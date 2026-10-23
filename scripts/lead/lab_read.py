"""lab_read.py BUNDLE [seat a|b|ab] [grep] [max]: print LAB lines from a replay bundle's stdout."""
import sys, zipfile, re
f = sys.argv[1]; seats = sys.argv[2] if len(sys.argv) > 2 else "a"; pat = sys.argv[3] if len(sys.argv) > 3 else ""; mx = int(sys.argv[4]) if len(sys.argv) > 4 else 40
z = zipfile.ZipFile(f); n = 0
for s in seats:
    for line in z.read(f"team-{s}.stdout").replace(b"\x00", b"").decode("utf8", "replace").splitlines():
        if line.startswith("LAB") and (not pat or re.search(pat, line)):
            print(s, line[:260]); n += 1
            if n >= mx: sys.exit(0)
