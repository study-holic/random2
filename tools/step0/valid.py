import sys,glob,json,collections; sys.path.insert(0,"C:/cambc-solo/tools/v3"); sys.path.insert(0,"C:/cambc-solo/tools/step0")
import killers as K
conf=collections.Counter()
for f in glob.glob("C:/cambc-solo/data/top10/full/*.json"):
    d=json.load(open(f)); ex={(e["r"],e["v"]):e for e in K.deaths_exact(d)}
    for e in K.deaths_nearby(d):
        x=ex.get((e["r"],e["v"]))
        if x: conf[("exact champ" if x["k"] is not None else "exact none","near champ" if e["k"] is not None else "near none")]+=1
print(conf)
