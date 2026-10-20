// plan.cpp: builds the current map's MapPlan once per process and dispatches the hooks.
#include "plan.h"
#include "bot.h"
#include "w7_drafts.h"
#include <iostream>
#if __has_include("salt.h")
#include "salt.h"   // written by scripts/mapgate.py --salts: a noise sample of the same bot
#endif
#ifndef P_SALT
#define P_SALT 0
#endif

namespace Plan {

void setup_ambush(MapPlan& p);
void setup_collision(MapPlan& p);
void setup_contest(MapPlan& p);
void setup_corners(MapPlan& p);
void setup_eight(MapPlan& p);
void setup_elkring(MapPlan& p);
void setup_expanse(MapPlan& p);
void setup_fragment(MapPlan& p);
void setup_interlock(MapPlan& p);
void setup_lanes(MapPlan& p);
void setup_monolith(MapPlan& p);
void setup_packs(MapPlan& p);
void setup_pilgrimage(MapPlan& p);
void setup_rows(MapPlan& p);
void setup_rushbait(MapPlan& p);
void setup_stampede(MapPlan& p);
void setup_swirl(MapPlan& p);

MapPlan& current() {
    static MapPlan plan = [] {
        MapPlan p;
        p.id = mapid::current_map();
        using mapid::MapId;
        switch (p.id) {
        case MapId::ambush: setup_ambush(p); break;
        case MapId::collision_old: setup_collision(p); break;
        case MapId::collision_v2: setup_collision(p); break;
        case MapId::contest: setup_contest(p); break;
        case MapId::corners_old: setup_corners(p); break;
        case MapId::corners_v2: setup_corners(p); break;
        case MapId::eight: setup_eight(p); break;
        case MapId::elkring: setup_elkring(p); break;
        case MapId::expanse_old: setup_expanse(p); break;
        case MapId::expanse_v2: setup_expanse(p); break;
        case MapId::fragment: setup_fragment(p); break;
        case MapId::interlock_old: setup_interlock(p); break;
        case MapId::interlock_v2: setup_interlock(p); break;
        case MapId::lanes_old: setup_lanes(p); break;
        case MapId::lanes_v2: setup_lanes(p); break;
        case MapId::monolith: setup_monolith(p); break;
        case MapId::packs_old: setup_packs(p); break;
        case MapId::packs_v2: setup_packs(p); break;
        case MapId::pilgrimage: setup_pilgrimage(p); break;
        case MapId::rows: setup_rows(p); break;
        case MapId::rushbait: setup_rushbait(p); break;
        case MapId::stampede: setup_stampede(p); break;
        case MapId::swirl: setup_swirl(p); break;
        default: break;
        }
        if (const auto* d = w7::draft_for(p.id)) p.draft = *d;   // w7: static per-map drafts (w7_drafts.h)
        p.counter = w7::counter_on(p.id);                          // w7 part 2 flag (off unless W7_CD_ALL or listed)
        std::cout << "map " << mapid::map_name(p.id) << '\n';   // once per process; checks identification
        return p;
    }();
    return plan;
}

const std::vector<cambc::champion_draft>* draft() {
    auto& p = current();
    return p.draft.empty() ? nullptr : &p.draft;
}

void init(Bot& b) {
    if (P_SALT > 0) {   // burn a fixed amount of first-turn fuel: fuel-dependent code then takes another trajectory
        volatile unsigned sink = 0;
        for (unsigned i = 0; i < P_SALT * 4000u; ++i) sink = sink + i;
    }
    if (current().tavi_focus_field) {   // w8 build C (map flag): Tavi buys the Focus damage build where he stands
        b.tavi_focus_field = true;
        b.remote_shop = cambc::self().passive() == cambc::passive_type::remote_shop;
    }
    if (auto f = current().init) f(b);
}

bool turn(Bot& b, Phase phase, const TurnView& v, Champs& enemies, Champs& allies) {
    auto& p = current();
    auto f = phase == Phase::pre ? p.pre : p.mid;
    return f ? f(b, v, enemies, allies) : false;
}

void target(Bot& b, const TurnView& v, int& chosen) { if (auto f = current().target) f(b, v, chosen); }

bool buy(Bot& b) { auto f = current().buy; return f ? f(b) : false; }

}  // namespace Plan
