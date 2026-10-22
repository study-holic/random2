// EXP-088 tactical policy (behind P_TACTICS, default off). Local rules that take over a champion's action only while an
// enemy champion is within P_TACT_RADIUS (or while an ally nearby is in contact), and release the moment none is:
//   a. focus fire: allies within P_TACT_CLUSTER of me share one target, chosen from the visible roster by a pure
//      function (enemy in range of the most of the cluster, ties by lowest health, then position), so no shared bytes;
//   b. ability timing: with the focus in range of a ready spell/stun and enough allies in the cluster, cast (stun
//      first) before the basic attack;
//   c. ranged spacing: a champion with range 3 or more keeps Chebyshev distance P_TACT_SPACING from enemy stunners
//      unless the focus is in range and the cluster is large enough to burst;
//   d. support: an idle ally within 8 of a contact (an ally of ours with an enemy within 4) walks to within 4 of it.
// A lone champion (no ally within the cluster radius) and a heavy carrier return false so the v4.14 logic runs.
#include <cambc.h>
#include "bot.h"
#include "util.h"
#include "params.h"
#include <algorithm>

using cambc::vec2;
using CT = cambc::champion_type;

namespace {

struct Foe {
    const cambc::champion* c;
    vec2 pos;
    float hp;
    bool stunner;
};

struct Mate {
    vec2 pos;
    int range;
};

bool has_stun(CT t) {
    return t == CT::ryth || t == CT::cassi || t == CT::prism || t == CT::ren || t == CT::tavi;
}

// A ready damage, area or stun spell that can target champions and reaches `p`.
bool spell_reaches(vec2 p) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (const auto& ab : me.abilities()) {
        if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || ab.is_cc()) || !ab.can_target_enemies()) continue;
        try { if (ab.valid_tiles().is_set(p)) return true; } catch (...) {}
    }
    return false;
}

// Stun first, then damage: one cast on the focus target.
bool cast_stun_then_burst(const cambc::champion& foe, vec2 p) {
    auto& me = cambc::self();
    if (!me.can_cast()) return false;
    for (int pass = 0; pass < 2; ++pass)
        for (const auto& ab : me.abilities()) {
            const bool cc = ab.is_cc();
            if ((pass == 0) != cc) continue;
            if (!ab.is_ready() || !(ab.is_damage() || ab.is_aoe_damage() || cc) || !ab.can_target_enemies()) continue;
            try { if (ab.valid_tiles().is_set(p) && ab.use(foe)) return true; } catch (...) {}
        }
    return false;
}

}  // namespace

bool Bot::tactics(const TurnView& v, const std::vector<cambc::champion>& enemies,
                  const std::vector<cambc::champion>& allies) {
    auto& me = cambc::self();
    constexpr int R = P_TACT_RADIUS, K = P_TACT_CLUSTER;
    if (static_cast<int>(me.gem_count()) >= P_TACT_SUPPORT_BAG) return false;   // heavy carriers bank (legacy logic)

    // Snapshot the visible roster once: handles of units killed this turn throw when read.
    std::vector<Foe> foes;
    for (const auto& e : enemies) {
        try {
            const float hp = e.health();
            if (hp <= 0) continue;
            foes.push_back({&e, e.position(), hp, has_stun(e.champion_name())});
        } catch (...) {}
    }
    std::vector<Mate> mates;
    for (const auto& a : allies) {
        if (a == me) continue;
        try { mates.push_back({a.position(), static_cast<int>(a.attack_range())}); } catch (...) {}
    }
    int near = 0;
    for (const auto& f : foes) if (tile_distance(v.pos, f.pos) <= R) ++near;

    // d. Support: no enemy within R of me, an ally within 8 has one within 4 (the walk continues a few rounds).
    if (near == 0) {
        const Mate* contact = nullptr;
        for (const auto& m : mates) {
            if (tile_distance(v.pos, m.pos) > 8) continue;
            bool touched = false;
            for (const auto& f : foes) if (tile_distance(m.pos, f.pos) <= 4) { touched = true; break; }
            if (!touched) continue;
            if (contact == nullptr || tile_distance(v.pos, m.pos) < tile_distance(v.pos, contact->pos)
                || (tile_distance(v.pos, m.pos) == tile_distance(v.pos, contact->pos)
                    && (m.pos.x < contact->pos.x || (m.pos.x == contact->pos.x && m.pos.y < contact->pos.y))))
                contact = &m;
        }
        if (contact != nullptr) { tact_seen = v.round; tact_contact = contact->pos; }
        if (tact_contact.x < 0 || v.round - tact_seen > P_TACT_SUPPORT_HOLD) return false;
        if (tile_distance(v.pos, tact_contact) <= K) return false;
        auto goals = tiles_around(tact_contact, K) & ~static_block & ~camp_static & ~danger_zone(v);
        return move_towards_closest(static_block, camp_static, goals);
    }
    tact_seen = v.round;
    tact_contact = vec2(-1, -1);

    // Cluster: me and the allies within K tiles.
    std::vector<Mate> cluster;
    cluster.push_back({v.pos, static_cast<int>(me.attack_range())});
    for (const auto& m : mates) if (tile_distance(v.pos, m.pos) <= K) cluster.push_back(m);
    const int nmates = static_cast<int>(cluster.size()) - 1;

    // a. Focus: the enemy within reach of the most of the cluster (attack range; if none in range, range + 1), ties by
    // lowest health, then by position. Same inputs give the same choice on every champion.
    const Foe* focus = nullptr;
    int best_cnt = -1, best_reach = -1;
    for (const auto& f : foes) {
        if (tile_distance(v.pos, f.pos) > R) continue;
        int cnt = 0, reach = 0;
        for (const auto& m : cluster) {
            const int d = tile_distance(m.pos, f.pos);
            if (d <= m.range) ++cnt;
            if (d <= m.range + 1) ++reach;
        }
        bool better;
        if (focus == nullptr) better = true;
        else if (cnt != best_cnt) better = cnt > best_cnt;
        else if (cnt == 0 && reach != best_reach) better = reach > best_reach;
        else if (f.hp != focus->hp) better = f.hp < focus->hp;
        else better = f.pos.x < focus->pos.x || (f.pos.x == focus->pos.x && f.pos.y < focus->pos.y);
        if (better) { focus = &f; best_cnt = cnt; best_reach = reach; }
    }
    if (focus == nullptr) return false;

    const int range = std::max(1, static_cast<int>(me.attack_range()));
    const bool focus_in_range = me.attack_tiles().is_set(focus->pos) || spell_reaches(focus->pos);
    const bool burst_on = focus_in_range && nmates >= P_TACT_CAST_ALLIES;

    // c. Ranged spacing: step to the neighbouring tile that gains distance from enemy stunners.
    if (range >= 3 && me.can_move() && !burst_on) {
        auto stun_dist = [&](vec2 p) {
            int d = 1 << 20;
            for (const auto& f : foes) if (f.stunner) d = std::min(d, tile_distance(p, f.pos));
            return d;
        };
        const int here = stun_dist(v.pos);
        if (here < P_TACT_SPACING) {
            const auto open = tiles_around(v.pos, 1) & ~static_block & ~camp_static & ~danger_zone(v)
                & ~cambc::vision::champion_tiles() & ~cambc::vision::camp_tiles();
            vec2 best = v.pos;
            int best_d = here, best_mate = 1 << 20;
            for (auto t : open.positions()) {
                if (t == v.pos) continue;
                const int d = stun_dist(t);
                int md = 0;
                for (const auto& m : mates) if (tile_distance(v.pos, m.pos) <= K) md += tile_distance(t, m.pos);
                if (d > best_d || (d == best_d && best_d > here && md < best_mate)) { best = t; best_d = d; best_mate = md; }
            }
            if (!(best == v.pos)) {
                cambc::bitboard goal; goal.clear(); goal.set(best, true);
                if (move_towards_closest(static_block, camp_static, goal)) {
                    // A step does not forbid an attack from the new tile.
                    for (const auto& f : foes)
                        if (me.attack_tiles().is_set(f.pos) && attack_if_in_range(*f.c)) { reset_and_attack_again(*f.c); break; }
                    return true;
                }
            }
        }
    }

    // A lone champion keeps the v4.14 rules (flee to an ally or the spawn, fight back).
    if (nmates == 0) return false;

    // b. Ability timing: stun, then burst, before the basic attack.
    const cambc::champion& foe = *focus->c;
    bool gone = false, acted = false;
    if (burst_on && cast_stun_then_burst(foe, focus->pos)) { acted = true; gone = !foe.is_visible(); }

    // Close the gap to the focus when it is within a short chase (never across the whole map).
    if (!gone && !me.attack_tiles().is_set(focus->pos) && me.can_move()
        && tile_distance(v.pos, focus->pos) <= range + 3) {
        auto goals = tiles_around(focus->pos, range);
        goals.set(focus->pos, false);
        goals &= ~camp_static & ~danger_zone(v);
        if (range >= 2) {   // ranged: stand at full range when a free tile exists
            auto ring = goals & ~tiles_around(focus->pos, range - 1);
            if (!ring.is_empty()) goals = ring;
        }
        if (!goals.is_empty() && move_towards_closest(static_block, camp_static, goals)) acted = true;
    }

    // Basic attack: the focus, else the weakest enemy in range.
    if (!gone && foe.is_visible() && me.attack_tiles().is_set(focus->pos)) {
        if (attack_if_in_range(foe)) { reset_and_attack_again(foe); return true; }
    }
    const Foe* other = nullptr;
    for (const auto& f : foes) {
        if (f.c == focus->c || !f.c->is_visible() || !me.attack_tiles().is_set(f.pos)) continue;
        if (other == nullptr || f.hp < other->hp) other = &f;
    }
    if (other != nullptr && attack_if_in_range(*other->c)) { reset_and_attack_again(*other->c); return true; }
    return acted;   // nothing done this turn: release to the v4.14 logic (no waiting state)
}
