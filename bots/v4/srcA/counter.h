// counter.h: data-driven counter pick at OUR LAST PICK (builder w7, part 2). The table (counter_table.h) is generated
// by tools/v3/w7_counter_table.py from intel/matches-2026-10-18.csv; the rule below mirrors decide() in that script.
//
// Mechanics (docs/rules/GameRules.md, Drafting): "both teams select champions simultaneously, revealing their picks as
// they go"; a champion "Selected by the opposing team" may still be picked. With no denial a counter loses nothing by
// waiting, so it fires only at the last pick, when team_size - 1 enemy picks are visible
// (cambc::draft::current_draft(team::enemy), sdk/cpp/cambc.h:5045; the draft turn has 10x fuel, BotAPICpp.md:624).
//
// Rule: our first k-1 picks must equal the table's R' (the static list's first k-1) and the static last pick s must
// be in the table. For each candidate c: score = base(c) + mean over seen enemy picks e of I(c, e) (0.1 pp units;
// I = 0 where the cell has fewer than 30 games). Override s with the best c only if, on all data, the gap is
// >= W7_CD_MARGIN and at least W7_CD_INFO of it comes from the enemy's picks (gap - base gap), and the gap is
// positive in both halves of the data. Willow and Vesper are never candidates (their heal/dash are never cast).
#pragma once
#include <cambc.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include "mapid.h"
#include "counter_table.h"

#ifndef W7_CD_MARGIN
#define W7_CD_MARGIN 50   // 5.0 pp
#endif
#ifndef W7_CD_INFO
#define W7_CD_INFO 25     // 2.5 pp
#endif

namespace w7cd {

using D = cambc::champion_draft;

inline bool has(const std::vector<D>& v, D c) { return std::find(v.begin(), v.end(), c) != v.end(); }

inline double score(const ctab::Cand& c, int h, const std::vector<D>& enemy) {
    long sum = 0;
    for (auto e : enemy) sum += c.inter[h][static_cast<int>(e)];
    return c.base[h] + static_cast<double>(sum) / static_cast<double>(enemy.size());
}

// True and `out` set when the table overrides the static last pick. Logs one line at the last pick.
inline bool pick(mapid::MapId id, const std::vector<D>& mine, const std::vector<D>& prefs, unsigned size, D& out) {
    if (mine.size() + 1 != size) return false;
    const ctab::MapTab* t = nullptr;
    for (const auto& m : ctab::k_maps) if (m.id == id) { t = &m; break; }
    if (!t || t->size != size) return false;
    unsigned mask = 0;
    for (auto c : mine) mask |= 1u << static_cast<int>(c);
    if (mask != t->rprime) return false;
    D s{};
    bool found = false;
    for (auto c : prefs) if (!has(mine, c)) { s = c; found = true; break; }
    if (!found || static_cast<int>(s) != t->last) return false;
    std::vector<D> enemy;
    try { enemy = cambc::draft::current_draft(cambc::team::enemy); } catch (...) { return false; }
    if (enemy.empty()) return false;
    const ctab::Cand* cs = nullptr;
    const ctab::Cand* best = nullptr;
    for (int i = 0; i < t->ncand; ++i) {
        const auto& c = t->cands[i];
        if (c.c == t->last) cs = &c;
        if (!best || score(c, 0, enemy) > score(*best, 0, enemy)) best = &c;
    }
    bool fire = false;
    if (cs && best && best != cs) {
        const double g0 = score(*best, 0, enemy) - score(*cs, 0, enemy);
        const double g1 = score(*best, 1, enemy) - score(*cs, 1, enemy);
        const double g2 = score(*best, 2, enemy) - score(*cs, 2, enemy);
        const double bgap = best->base[0] - cs->base[0];
        fire = g0 >= W7_CD_MARGIN && g0 - bgap >= W7_CD_INFO && g1 > 0 && g2 > 0;
    }
    std::cout << "cd7 pick " << mine.size() + 1 << '/' << size << " enemy";
    for (auto e : enemy) std::cout << ' ' << static_cast<int>(e);
    std::cout << " static " << static_cast<int>(t->last) << " best " << (best ? static_cast<int>(best->c) : -1) << (fire ? " FIRE\n" : " keep\n");
    if (!fire) return false;
    out = static_cast<D>(best->c);
    return out != D::willow && out != D::vesper_vale && !has(mine, out);
}

}  // namespace w7cd
