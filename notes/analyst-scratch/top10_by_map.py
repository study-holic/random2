"""Top-10 drafts, tempo, and champion rating-adjusted excess per map id.
Run: python -I C:/cambc-solo/notes/analyst-scratch/top10_by_map.py
Writes C:/cambc-solo/notes/top10-by-map.md (generated tables + notes/analyst-scratch/interpretation.md)
and C:/cambc-solo/notes/analyst-scratch/candidates.txt (swap candidates used in the Interpretation section).
Input: C:/cambc-solo/intel/matches-2026-10-18.csv (9,245 ranked games). Standard library only.
"""
import csv, math, statistics, os, collections as C

ROOT = 'C:/cambc-solo'
rows = list(csv.DictReader(open(f'{ROOT}/intel/matches-2026-10-18.csv', encoding='utf-8')))
for r in rows:
    r['ra'] = float(r['rating_a']); r['rb'] = float(r['rating_b'])
    r['rounds'] = int(r['rounds']); r['sa'] = int(r['score_a']); r['sb'] = int(r['score_b'])

def exp_a(ra, rb):  # 400-point logistic expected score for side a
    return 1.0 / (1.0 + 10 ** ((rb - ra) / 400.0))

SHORT = {'CassiIssac': 'C&I'}
def sh(draft):
    return '+'.join(SHORT.get(c, c) for c in draft.split('+'))

# map id -> name (CSV map_name column, 1:1 with id, matches meta-2026-10-18.md section 2)
names = {}
for r in rows: names[r['map_id']] = r['map_name']
mids = sorted(names, key=lambda k: names[k])

# team-sides: one record per (game, side); both sides of every game are used
sides = []  # dict(map,team_id,team,draft,win,e,rating,game_idx)
for i, r in enumerate(rows):
    ea = exp_a(r['ra'], r['rb'])
    for s, tid, tn, rt, dr, e, w in (
        ('a', r['team_a_id'], r['team_a'], r['ra'], r['draft_a'], ea, r['winner'] == 'a'),
        ('b', r['team_b_id'], r['team_b'], r['rb'], r['draft_b'], 1 - ea, r['winner'] == 'b')):
        sides.append(dict(map=r['map_id'], tid=tid, team=tn, rating=rt, draft=dr, e=e,
                          win=1.0 if w else 0.0, gi=i, t=r['finished_at'], sample=r['sample_' + s] == '1'))

# top 10 = highest pre-group rating at the team's LAST game in the window
last = {}
peak = {}
for sd in sides:
    if sd['tid'] not in last or sd['t'] > last[sd['tid']][0]:
        last[sd['tid']] = (sd['t'], sd['rating'], sd['team'])
    peak[sd['tid']] = max(peak.get(sd['tid'], 0), sd['rating'])
top = sorted(last, key=lambda k: -last[k][1])[:10]
topset = set(top)
peak_top = sorted(peak, key=lambda k: -peak[k])[:10]
overlap = len(topset & set(peak_top))
top_desc = ', '.join(f"{last[k][2]} {last[k][1]:.0f}" for k in top)

def fmt_pp(x): return f"{100*x:+.1f}"

out = []
P = out.append

# ---------- table 1: top-10 drafts + tempo ----------
t1 = []
TOT = [0, 0.0]
cand = []  # for interpretation
tot_top_sides = 0
top_champ_share = {}  # (map, champ) -> (n_with, n_total)
for m in mids:
    ts = [s for s in sides if s['map'] == m and s['tid'] in topset]
    tot_top_sides += len(ts)
    byd = C.defaultdict(list)
    for s in ts: byd[s['draft']].append(s)
    ordered = sorted(byd.items(), key=lambda kv: -len(kv[1]))
    cells = []
    for d, L in ordered[:3]:
        n = len(L); w = sum(x['win'] for x in L) / n; e = sum(x['e'] for x in L) / n
        flag = '†' if n < 30 else ''
        cells.append(f"{sh(d)}: n={n}{flag}, {100*w:.0f}% ({fmt_pp(w-e)})")
    rest = len(ts) - sum(len(L) for _, L in ordered[:3])
    nd = len(byd)
    cells.append(f"other {nd-3 if nd>3 else 0} drafts: n={rest}")
    ntot = len(ts); wt = sum(x['win'] for x in ts) / ntot; et = sum(x['e'] for x in ts) / ntot
    # champion adoption by top 10 on this map
    cc = C.Counter()
    for s in ts:
        for c in s['draft'].split('+'): cc[c] += 1
        top_champ_share_key = None
    for c, k in cc.items(): top_champ_share[(m, c)] = (k, ntot)
    # tempo: games WON by a top-10 team (each game once)
    wins = [s for s in ts if s['win'] == 1.0]
    rr = []; n2000 = 0; n1000 = 0; noth = 0
    for s in wins:
        g = rows[s['gi']]
        sc = g['sa'] if g['winner'] == 'a' else g['sb']
        rr.append(g['rounds'])
        if g['rounds'] == 2000: n2000 += 1
        elif sc >= 1000: n1000 += 1
        else: noth += 1
    nw = len(wins)
    q = statistics.quantiles(rr, n=4, method='inclusive') if nw >= 2 else [float('nan')] * 3
    r1000 = [x for x, s in zip(rr, wins) if x < 2000]
    med1000 = statistics.median(r1000) if r1000 else float('nan')
    fl = '†' if nw < 30 else ''
    tempo = (f"wins n={nw}{fl}: median {statistics.median(rr):.0f}, IQR {q[0]:.0f}-{q[2]:.0f}<br>"
             f"round 2000: {n2000} ({100*n2000/nw:.0f}%); 1000 gems first: {n1000} ({100*n1000/nw:.0f}%)"
             + (f"; other: {noth}" if noth else '') + f"<br>median of 1000-gem wins: {med1000:.0f}")
    adopt = ', '.join(f"{SHORT.get(c,c)} {100*k/ntot:.0f}%" for c, k in sorted(cc.items(), key=lambda kv: -kv[1]) if k / ntot >= 0.10)
    # all-team tempo for context (every game on the map, winner side)
    ag = [g for g in rows if g['map_id'] == m]
    ar = [g['rounds'] for g in ag]
    a2000 = sum(1 for x in ar if x == 2000)
    qa = statistics.quantiles(ar, n=4, method='inclusive')
    tempo += f"<br>all teams: n={len(ag)}, median {statistics.median(ar):.0f}, IQR {qa[0]:.0f}-{qa[2]:.0f}, round 2000: {100*a2000/len(ag):.0f}%"
    TOT[0] += ntot; TOT[1] += sum(x['win'] - x['e'] for x in ts)
    t1.append(f"| {names[m]} `{m[:8]}` | {ntot}{'†' if ntot<30 else ''}; {100*wt:.0f}% ({fmt_pp(wt-et)})<br>pick share: {adopt} | "
              + '<br>'.join(cells) + f" | {tempo} |")

# ---------- table 2: champion excess, all teams ----------
t2 = []
allchamps = sorted({c for s in sides for c in s['draft'].split('+')})
for m in mids:
    ms = [s for s in sides if s['map'] == m]
    stat = {}
    for c in allchamps:
        L = [s for s in ms if c in s['draft'].split('+')]
        if not L: continue
        n = len(L); res = sum(s['win'] - s['e'] for s in L)
        var = sum(s['e'] * (1 - s['e']) for s in L)
        z = res / math.sqrt(var) if var > 0 else 0.0
        stat[c] = (n, res / n, z, n / len(ms))
    elig = {c: v for c, v in stat.items() if v[0] >= 30}
    rk = sorted(elig, key=lambda c: -elig[c][1])
    pos = rk[:3]; neg = rk[::-1][:2]
    f = lambda c: f"{c} {fmt_pp(elig[c][1])} (n={elig[c][0]}, z={elig[c][2]:+.1f})"
    small = [c for c in stat if stat[c][0] < 30]
    t2.append(f"| {names[m]} `{m[:8]}` | {len(ms)} | " + '<br>'.join(f(c) for c in pos) + ' | ' + '<br>'.join(f(c) for c in neg)
              + f" | {', '.join(f'{c} n={stat[c][0]}' for c in small) or '-'} |")
    # candidates: positive excess, top-10 adoption
    for c, (n, ex, z, sh_all) in stat.items():
        k, nt = top_champ_share.get((m, c), (0, 0))
        nt = nt or sum(1 for s in ms if s['tid'] in topset)
        cand.append((names[m], c, n, ex, z, sh_all, k, nt))

# ---------- cross-check vs meta (sides of teams that were ever a sample team) ----------
chk = []
samp = {s['tid'] for s in sides if s['sample']}
for m in mids[:1]:
    ms = [s for s in sides if s['map'] == m and s['tid'] in samp]
    L = [s for s in ms if 'Prism' in s['draft'].split('+')]
    chk.append(f"{names[m]} Prism, sides of sample teams: excess {100*sum(s['win']-s['e'] for s in L)/len(L):+.1f} (n={len(L)} of {len(ms)} sides)")

# ---------- write ----------
EXC = 100 * TOT[1] / TOT[0]
hdr = f"""# Top-10 drafts, tempo and champion excess by map (18 Oct 2026, 09:20Z-19:13Z)

Source: `intel/matches-2026-10-18.csv`, {len(rows)} ranked games, 17 current map ids (script at bottom). Written 21:47 BST 18 Oct.

## Definitions
- **Maps**: keyed by map id (first 8 hex shown). Name = CSV `map_name`, 1:1 with id and identical to `intel/meta-2026-10-18.md` section 2. `mapid (1).h` has only the 13 older maps, so it is not used.
- **Top 10**: the 10 team ids with the highest pre-group rating at their own last game in the window ({top_desc}). Using peak rating instead gives the same set for {overlap}/10 teams. Their games are taken from both sides (a top-10 team may sit on side a or b); {tot_top_sides} top-10 team-games in total (23-40 per map, so every top-10 draft cell is below 30 games; read them as anecdotes). Selecting on final rating biases the top-10 excess upward (overall {EXC:+.1f} pp over {tot_top_sides} team-games).
- **Draft strings** are alphabetical in the data, so pick order is not visible; a draft is the champion set. C&I = CassiIssac.
- **Expected score** = 1/(1+10^((opp-own)/400)) from the pre-group ratings in the CSV. **Excess** = actual minus expected, in percentage points. Draft rows: `n, win% (excess)`.
- **Champion excess (all teams)**: every team-side of every game (both sides, {len(sides)} team-sides), not only top 10. z = sum(actual-expected) / sqrt(sum e(1-e)); the two sides of one game are treated as independent, which slightly overstates |z|. Ranked among champions with n >= 30 on that map; champions below 30 are listed in the last column.
- **Tempo**: games won by a top-10 team (each game counted once). Round 2000 = game ended at the round cap and the lead decided it; "1000 gems first" = finished before 2000 with winner score >= 1000. Winner score is submitted gems.
- † marks any figure from fewer than 30 games. Sample: the CSV holds only teams in the top-50 and 1,450-1,600 samples and their opponents, so "all teams" means all teams in these games.

## 1. Top-10 drafts and tempo (answers a and b)
Columns: top-10 team-games (n; win%; excess) and champion pick share (>=10%); three most-used top-10 drafts; tempo of top-10 wins, then all teams for context.

| Map | Top-10 all | Top-10 drafts | Tempo of top-10 wins |
|---|---|---|---|
"""
body1 = '\n'.join(t1)
mid = """

## 2. Champion rating-adjusted win excess, all teams (answers c)
Top 3 positive and top 2 negative among champions with n >= 30 on the map (n = team-sides drafting the champion).

| Map | Team-sides | Top 3 positive (pp, n, z) | Top 2 negative | Champions with n < 30 (excluded) |
|---|---|---|---|---|
"""
body2 = '\n'.join(t2)
interp = open(f'{ROOT}/notes/analyst-scratch/interpretation.md', encoding='utf-8').read() if os.path.exists(f'{ROOT}/notes/analyst-scratch/interpretation.md') else '## Interpretation (labelled)\n(pending)\n'
ftr = "\n\nScript: `C:/cambc-solo/notes/analyst-scratch/top10_by_map.py` (run `python -I`; reads the CSV, regenerates this file and `candidates.txt`). Cross-check against the meta file's sample-side method: " + '; '.join(chk) + " (meta: -25.3 with 927 side-games).\n"
open(f'{ROOT}/notes/top10-by-map.md', 'w', encoding='utf-8').write(hdr + body1 + mid + body2 + '\n\n' + interp + ftr)

# candidates file
with open(f'{ROOT}/notes/analyst-scratch/candidates.txt', 'w', encoding='utf-8') as f:
    f.write('map champion n_all excess_pp z share_all top10_n top10_total top10_share\n')
    for (mn, c, n, ex, z, sa, k, nt) in sorted(cand, key=lambda x: (x[0], -x[3])):
        if n >= 30:
            f.write(f"{mn} {c} {n} {100*ex:+.1f} {z:+.1f} {100*sa:.0f}% {k}/{nt} {100*k/nt:.0f}%\n")
print('done', len(out))
