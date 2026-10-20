import json, math, collections
R = 'C:/cambc-solo/'
NAMES = ['Ryth', 'Vesper', 'Vendetta', 'Cassi', 'Issac', 'Stellar', 'Prism', 'Marcella', 'Willow', 'Ren', 'Varg', 'Tavi']
# ability multipliers (x Focus) for spells that deal damage: (slot0, slot1)  from GameRules.md
SPELL = {0: (1.0, 0), 1: (0, 0), 2: (0, 0), 3: (2.0, 0), 4: (0, 0), 5: (2.0, 1.4), 6: (1.5, 1.0), 7: (0, 2.0), 8: (0, 0), 9: (1.25, 0), 10: (0, 0), 11: (0, 2.2)}
STUN = {0: (1, 0), 3: (0, 1), 6: (1, 0), 7: (0, 0), 9: (1, 0), 11: (0, 1)}
RANGED = {1, 3, 5, 6, 8, 11}
UNIT = lambda ty: 3 if ty in (3, 4) else ty  # Cassi & Issac die together: one unit


def load_fights(path=R + 'data/v3/fights.jsonl'):
    rows = []
    for l in open(path, encoding='utf8'):
        r = json.loads(l)
        rows.append(r)
    return rows


def is_ours(r):
    return 'xanarchy' in (r.get('teams') or [])


def unit_deaths(r, t):
    """set of unit ids dead on side t (linked partner counted)."""
    s = set()
    for (i, ty, g) in r['deaths'][str(t)]:
        s.add(UNIT(ty))
    return s


def deaths_gems(r, t):
    return sum(g for (i, ty, g) in r['deaths'][str(t)])


def kill_diff(r, t):
    """units killed by side t minus units lost by side t"""
    return len(unit_deaths(r, 1 - t)) - len(unit_deaths(r, t))


def wilson(k, n, z=1.96):
    if n == 0:
        return (float('nan'), float('nan'))
    p = k / n
    d = 1 + z * z / n
    c = (p + z * z / (2 * n)) / d
    h = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / d
    return (c - h, c + h)
