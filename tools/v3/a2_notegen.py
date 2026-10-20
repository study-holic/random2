"""Prints markdown snippets for notes/matchups.md from the data/v3/matchups outputs."""
import json, numpy as np, pandas as pd, sys
R = 'C:/cambc-solo/data/v3/matchups/'
CH = ['Ryth', 'Vesper', 'Vendetta', 'CassiIssac', 'Stellar', 'Prism', 'Marcella', 'Willow', 'Ren', 'Varg', 'Tavi']
SH = ['Ryth', 'Vesp', 'Vend', 'C&I', 'Stel', 'Prism', 'Marc', 'Will', 'Ren', 'Varg', 'Tavi']
which = sys.argv[1]
if which == 'main':
    m = pd.read_csv(R + 'champ_main.csv')
    for _, r in m.iterrows():
        flag = '*' if (r.lo > 0 or r.hi < 0) else ''
        print('| %s | %.0f%% | %d | %+.1f%s [%+.1f, %+.1f] |' % (r.champ, 100 * r.share_any, r.games_excl, r.exc_pp, flag, r.lo, r.hi))
if which == 'matrix':
    mean = np.load(R + 'draft_matrix_mean.npy')
    pr = pd.read_csv(R + 'draft_pairs.csv')
    sig = {(r.a, r.b): (r.sig, r.bh05) for _, r in pr.iterrows()}
    print('| row v col | ' + ' | '.join(SH[1:]) + ' |')
    print('|---|' + '---|' * 10)
    for i in range(10):
        cells = []
        for j in range(1, 11):
            if j <= i:
                cells.append('')
                continue
            v = 100 * mean[i, j]
            s = sig[(CH[i], CH[j])]
            cells.append('%+.1f%s' % (v, '**' if s[1] else ('*' if s[0] else '')))
        print('| %s | ' % SH[i] + ' | '.join(cells) + ' |')
if which == 'pairs':
    p = pd.read_csv(R + 'draft_fullpairs.csv')
    for _, r in p.head(int(sys.argv[2])).iterrows():
        print('| %s | %s | %d | %.1f%% | %+.1f [%+.1f, %+.1f] | %d |' % (r.draft1.replace('CassiIssac', 'C&I'), r.draft2, r.games, 100 * r.wr1, r.exc_pp, r.lo, r.hi, r.teams))
