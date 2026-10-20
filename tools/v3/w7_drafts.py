"""w7: per-map draft evidence from intel/matches-2026-10-18.csv (9,245 ranked games, drafts sorted alphabetically,
so pick order is not recoverable). Rating-adjusted excess = win - Elo expectation (400 scale) from pre-group ratings.
usage: py -I tools/v3/w7_drafts.py [map ...]"""
import csv, sys, math, collections
sys.stdout.reconfigure(encoding="utf-8", errors="replace")
ROWS = list(csv.DictReader(open("C:/cambc-solo/intel/matches-2026-10-18.csv", encoding="utf-8")))

def sides(rows):
    for r in rows:
        try: ra, rb = float(r["rating_a"]), float(r["rating_b"])
        except ValueError: continue
        ea = 1 / (1 + 10 ** ((rb - ra) / 400))
        wa = 1.0 if r["winner"] == "a" else 0.0
        yield dict(map=r["map_name"], team=r["team_a"], rating=ra, opp_rating=rb, draft=frozenset(r["draft_a"].split("+")), opp=frozenset(r["draft_b"].split("+")), win=wa, exp=ea, top=r["sample_a"] == "1")
        yield dict(map=r["map_name"], team=r["team_b"], rating=rb, opp_rating=ra, draft=frozenset(r["draft_b"].split("+")), opp=frozenset(r["draft_a"].split("+")), win=1 - wa, exp=1 - ea, top=r["sample_b"] == "1")

S = list(sides(ROWS))
def fmt(d): return "+".join(sorted(d))
if __name__ == "__main__":
    maps = sys.argv[1:] or sorted({s["map"] for s in S})
    K = 30  # shrinkage prior strength (games)
    for m in maps:
        g = [s for s in S if s["map"] == m]
        size = collections.Counter(len(s["draft"]) for s in g).most_common(1)[0][0]
        print(f"== {m} size {size} side-games {len(g)}")
        by = collections.defaultdict(list)
        for s in g: by[s["draft"]].append(s)
        rows = []
        for d, L in by.items():
            n = len(L); ex = sum(x["win"] - x["exp"] for x in L) / n
            wr = sum(x["win"] for x in L) / n
            se = 0.5 / math.sqrt(n)
            rows.append((ex * n / (n + K), n, wr, ex, ex / se, len({x["team"] for x in L}), sum(x["rating"] for x in L) / n, d))
        rows.sort(key=lambda t: -t[0])
        for sh, n, wr, ex, z, teams, rt, d in rows[:8]:
            if n >= 10: print(f"  {fmt(d):<40} n={n:4d} wr={wr:.2f} ex={ex*100:+5.1f} z={z:+4.1f} shrunk={sh*100:+5.1f} teams={teams} rating={rt:.0f}")
        # champion-level excess
        champs = sorted({c for s in g for c in s["draft"]})
        line = []
        for c in champs:
            L = [x for x in g if c in x["draft"]]
            if len(L) >= 40:
                ex = sum(x["win"] - x["exp"] for x in L) / len(L)
                line.append(f"{c}:{len(L)}:{ex*100:+.1f}")
        print("  champ ", " ".join(line))
