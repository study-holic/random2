"""equiv.py SET_X SET_Y: game-by-game comparison of two baseline sets with the same bots (map, side): identical results or not."""
import csv, sys
def load(s):
    return {(r["map"], r["a_side"]): r for r in csv.DictReader(open(f"/home/claude/runs/{s}/games.csv"))}
x, y = load(sys.argv[1]), load(sys.argv[2])
keys = sorted(set(x) & set(y))
fields = ("a_gems", "b_gems", "a_win", "rounds", "a_deaths", "b_deaths", "a_fuel_exh")
same = [k for k in keys if all(x[k][f] == y[k][f] for f in fields)]
diff = [k for k in keys if k not in same]
print(f"{sys.argv[1]} vs {sys.argv[2]}: {len(keys)} paired games, {len(same)} identical on {', '.join(fields)}; {len(diff)} differ")
for k in diff[:10]:
    print("  ", k, {f: (x[k][f], y[k][f]) for f in fields if x[k][f] != y[k][f]})
