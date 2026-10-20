"""Make bots/<name> = bots/s1 with extra #defines appended to src/a/params_override.h, then build it.
usage: python3 tools/s1/variant.py <name> P_X=1 P_Y=0 ..."""
import os, shutil, subprocess, sys
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
name, defs = sys.argv[1], sys.argv[2:]
dst = os.path.join(ROOT, "bots", name)
if os.path.exists(os.path.join(dst, "src")): shutil.rmtree(os.path.join(dst, "src"))
shutil.copytree(os.path.join(ROOT, "bots", "s1", "src"), os.path.join(dst, "src"))
with open(os.path.join(dst, "src", "a", "params_override.h"), "a") as f:
    f.write("\n// variant %s\n" % name + "".join("#define %s %s\n" % tuple(d.split("=", 1)) for d in defs))
rc = subprocess.call(["cambc", "build", "--release", name], cwd=ROOT, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
sys.exit("build %s failed" % name if rc else 0)
