// Per-map profiles. The map is identified once (draft phase, or gameplay init if the draft ran in another instance)
// with the fingerprints v10 used: width, height, team size, and on 50x50 maps the camp structure (six three-wolf packs
// and no Elk = rows). Every profile below reproduces v10 behaviour; edit a profile to change one map's strategy.
#include <cambc.h>
#include "profile.h"
#include "params.h"
#include <algorithm>

using D = cambc::champion_draft;
using CT = cambc::champion_type;

namespace {

bool g_identified = false;

template <std::size_t N>
constexpr void set_draft(MapProfile& p, const D (&list)[N]) {
    static_assert(N <= kDraftMax);
    p.draft_n = static_cast<int>(N);
    for (std::size_t i = 0; i < N; ++i) p.draft[i] = list[i];
}

constexpr D small[] = {D::vendetta, D::cassi_and_issac, D::stellar, D::ryth, D::varg, D::tavi};
constexpr D large[] = {D::vendetta, D::cassi_and_issac, D::varg, D::ryth, D::tavi, D::ren, D::marcella, D::stellar};
constexpr D rows_prefs[] = {D::cassi_and_issac, D::varg, D::tavi, D::stellar, D::ryth, D::vendetta};
// EXP-105: interlock drops Cassi and Issac; Varg and Ryth move up.
constexpr D il_prefs[] = {D::vendetta, D::varg, D::ryth, D::tavi, D::ren, D::marcella, D::stellar};
// EXP-115 (packs): Cassi and Issac, Stellar, Varg lead.
constexpr D packs_prefs[] = {D::cassi_and_issac, D::stellar, D::varg, D::vendetta, D::cassi_and_issac, D::varg, D::ryth, D::tavi, D::stellar, D::ren};

// v10 behaviour, every field. Per-map differences are applied by make_profile().
constexpr MapProfile base_profile() {
    MapProfile p;
    // purchases: Focus for Cassi, Stellar, Prism; health-first for Varg, Ryth, Vendetta only when P_HEALTH_FIRST_TANKS
    for (int t = 0; t < kChampionTypes; ++t) {
        const auto& k = kit_of(static_cast<CT>(t));
        p.buy[t].main_focus = k.main_focus;
        p.buy[t].health_first = P_HEALTH_FIRST_TANKS && k.tank;
    }
    // squads: squad 0 = vanguard (Vendetta, Issac, Cassi), squad 1 = everyone else
    for (int r = 0; r < static_cast<int>(Role::count); ++r) p.squad_of_role[r] = 1;
    p.squad_of_role[static_cast<int>(Role::vanguard)] = 0;
    p.squad_min_roster = P_SQUAD_MIN_ROSTER;
    p.squad_min_size = P_SQUAD_MIN_SIZE;
    p.elk_min_squad = P_ELK_MIN_SQUAD;
    p.capture_by_health = P_CAPTURE_BY_HEALTH;
    // target valuation
    p.bank_bag = P_BANK_BAG;
    p.rows_bag = P_ROWS_BAG;
    p.cluster_reach = P_CLUSTER_REACH;
    p.cluster_min_groups = P_CLUSTER_MIN_GROUPS;
    p.pack_wolves = P_PACK_WOLVES;
    p.own_half_rounds = P_OWN_HALF_ROUNDS;
    p.wait_weight = P_WAIT_WEIGHT;
    p.claim_bonus = P_CLAIM_BONUS;
    // combat stance
    p.retreat_hurt = P_RETREAT_HURT;
    p.retreat_calm = P_RETREAT_CALM;
    p.retreat_tank = P_RETREAT_TANK;
    p.retreat_incoming_mult = P_RETREAT_INCOMING_MULT;
    p.retreat_incoming_base = P_RETREAT_INCOMING_BASE;
    p.recall_done_field = P_RECALL_DONE_FIELD;
    p.recall_done_spawn = P_RECALL_DONE_SPAWN;
    p.spawn_leave_hp = P_SPAWN_LEAVE_HP;
    p.near_spawn = P_NEAR_SPAWN;
    p.shield_hp = P_VARG_SHIELD_HP;
    // kit actions
    p.willow_heal = P_WILLOW_HEAL;
    p.willow_heal_below = P_WILLOW_HEAL_BELOW;
    p.dash_escape = P_DASH_ESCAPE;
    return p;
}

constexpr MapProfile make_profile(MapId id, int team_size) {
    MapProfile p = base_profile();
    p.id = id;
    switch (id) {
        case MapId::collision: p.name = "collision"; set_draft(p, small); break;
        case MapId::expanse:   p.name = "expanse";   set_draft(p, small); break;
        case MapId::corners:   p.name = "corners";   set_draft(p, large); break;
        case MapId::elkring:   p.name = "elkring";   set_draft(p, large); break;
        case MapId::lanes:     p.name = "lanes";     set_draft(p, large); break;
        case MapId::monolith:  p.name = "monolith";  set_draft(p, large); break;
        case MapId::interlock: p.name = "interlock"; set_draft(p, il_prefs); break;
        case MapId::packs:     p.name = "packs";     set_draft(p, packs_prefs); break;
        case MapId::rows:      p.name = "rows";      set_draft(p, rows_prefs); p.rows_mode = true; break;
        default:               p.name = "generic";   if (team_size <= 3) set_draft(p, small); else set_draft(p, large); break;
    }
    // Willow main stat follows the heal: Focus scales the heal (3 x Focus).
    if (p.willow_heal) p.buy[static_cast<int>(CT::willow)].main_focus = true;
    return p;
}

// Fingerprints. size 0 = unknown (gameplay phase): any size matches.
MapId classify(int w, int h, int size, bool rows_fp) {
    auto sz = [&](int n) { return size == 0 || size == n; };
    if (rows_fp) return MapId::rows;
    if (w == 41 && h == 41 && sz(3)) return MapId::collision;
    if (w == 71 && h == 71 && sz(5)) return MapId::corners;
    if (w == 30 && h == 30 && sz(5)) return MapId::elkring;
    if (w == 80 && h == 80 && sz(3)) return MapId::expanse;
    if (w == 40 && h == 40 && sz(4) && P_IL_DRAFT) return MapId::interlock;
    if (w == 90 && h == 30 && sz(5)) return MapId::lanes;
    if (w == 50 && h == 50 && sz(5)) return MapId::monolith;
    if (w == 31 && h == 31 && sz(3)) return MapId::packs;
    return MapId::unknown;
}

// One constexpr profile per map plus the two generic ones (team size <= 3 or larger): identification only picks a pointer.
constexpr MapProfile kProfiles[] = {
    make_profile(MapId::unknown, 3), make_profile(MapId::collision, 0), make_profile(MapId::corners, 0),
    make_profile(MapId::elkring, 0), make_profile(MapId::expanse, 0), make_profile(MapId::interlock, 0),
    make_profile(MapId::lanes, 0), make_profile(MapId::monolith, 0), make_profile(MapId::packs, 0),
    make_profile(MapId::rows, 0), make_profile(MapId::unknown, 5),
};

int profile_index(MapId id, int size) { return id == MapId::unknown ? (size <= 3 ? 0 : 10) : static_cast<int>(id); }

void install(MapId id, int size) {
    g_profile_ptr = &kProfiles[profile_index(id, size)];
    g_identified = true;
}

}  // namespace

const MapProfile* g_profile_ptr = &kProfiles[0];

const MapProfile& identify_in_draft() {
    if (g_identified) return *g_profile_ptr;
    int w = 0, h = 0;
    try { w = cambc::map::width(); h = cambc::map::height(); } catch (...) { w = h = 0; }
    const int size = static_cast<int>(cambc::draft::team_size());
    bool rows_fp = false;
    try { if (P_ROWS_CMD && w == 50 && h == 50) {   // EXP-093: six wolf packs, no Elk (the only camp_groups scan, 50x50 only)
        int packs = 0; bool elk = false;
        for (auto g : cambc::map::camp_groups()) {
            int wolves = 0; bool mono = false;
            for (const auto& c : g.camps()) { if (c.is_elk()) elk = true; if (c.is_wolf()) ++wolves; if (c.is_monolith()) mono = true; }
            if (wolves >= 3 && !mono) ++packs;
        }
        rows_fp = !elk && packs == 6;
    } } catch (...) { rows_fp = false; }
    install(classify(w, h, size, rows_fp), size);
    return *g_profile_ptr;
}

void identify_prelim(int w, int h) {
    if (g_identified) return;
    g_profile_ptr = &kProfiles[profile_index(classify(w, h, 0, false), 0)];   // not final: 50x50 stays monolith until the rows fingerprint
}

const MapProfile& identify_in_init(bool any_elk, int three_wolf_packs, int w, int h) {
    if (g_identified) return *g_profile_ptr;
    const int size = 0;   // no SDK call in the gameplay phase: any size matches
    const bool rows_fp = P_ROWS_CMD && !any_elk && w == 50 && h == 50 && three_wolf_packs == 6;
    install(classify(w, h, size, rows_fp), size);
    return *g_profile_ptr;
}
