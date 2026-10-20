#pragma once
// Shared helpers: geometry, strength table, attack and cast wrappers, and the obstacle-aware step.
#include <cambc.h>
#include <vector>

struct TurnView;

inline constexpr int kMonsterRange = 3;   // wolf range; engine 1.0: elk 4, monolith 5, golem 4 (use camp_range)
// FIX_CAMP_RANGE: the attack range of a camp from the API (cached by kind; engine 1.0 raised elk to 4 and monolith to 5).
#ifndef P_CAMP_RANGE
#define P_CAMP_RANGE 2   // 0 off, 1 all kinds from the API, 2 monolith and golem only (ablation: elk 4 hurt collision/eight/expanse h2h)
#endif
inline int camp_range(const cambc::camp& c) {
    if (!P_CAMP_RANGE) return kMonsterRange;
    if (P_CAMP_RANGE == 2 && !c.is_monolith() && !c.is_golem()) return kMonsterRange;
    static int cache[16] = {0};
    const int k = static_cast<int>(c.kind());
    if (k < 0 || k >= 16) return kMonsterRange;
    if (cache[k] == 0) { try { cache[k] = static_cast<int>(c.attack_range()); } catch (...) { cache[k] = -1; } }
    return cache[k] > 0 ? cache[k] : kMonsterRange;
}

// FIX_UNSEEN_HIT (P_UNSEEN_HIT): health lost with no enemy champion in view and no visible camp in range means a camp we
// cannot see is hitting us (corridors: camps 3 tiles away are out of sight while they hit). A retreat then walks home
// instead of recalling: the hit interrupts every recall, which was a death loop.
#ifndef P_UNSEEN_HIT
#define P_UNSEEN_HIT 1   // 0 off, 1 on
#endif
#ifndef P_UNSEEN_MEMORY
#define P_UNSEEN_MEMORY 3   // rounds an unseen hit is remembered
#endif

// FIX_STAND_SIGHT (P_STAND_SIGHT): a ranged stand tile must see its prey (corridors: tiles at range 3 sit behind a bush
// row, the prey drops out of sight there, and the squad stepped in and out without attacking).
#ifndef P_STAND_SIGHT
#define P_STAND_SIGHT 1   // 0 off, 1 on
#endif

bool enemy_champion_near();   // P_SAVE_CC: an enemy champion within 8 tiles (util.cpp)
int tile_distance(cambc::vec2 a, cambc::vec2 b);
cambc::bitboard tiles_around(cambc::vec2 position, int radius);
int camp_strength(const cambc::camp& c);
int min_distance(cambc::vec2 pos, const std::vector<cambc::vec2>& tiles);

// Damage per round that live camp monsters can deal to a champion standing on `pos`.
double incoming_damage(const TurnView& v, cambc::vec2 pos);
// Tiles within reach of a live damaging monster (range plus one tile margin).
cambc::bitboard danger_zone(const TurnView& v);

// Drop champions whose handle is no longer in vision (killed this turn, or out of sight after a move): reading a
// position or health from such a handle throws engine error 97.
void keep_visible(std::vector<cambc::champion>& list);

bool attack_if_in_range(const cambc::unit& target);
void reset_and_attack_again(const cambc::unit& target);
// Cast a damage or stun spell on a camp in spell range (S-14: pure stuns too).
bool cast_on_camp(const cambc::camp& camp);
// Varg: shield whenever ready while a live damaging camp is within 4 tiles, or when hurt.
void cast_shield(const TurnView& v);

// Step towards the closest goal tile. Camp spawn tiles stay obstacles unless they are goals.
// FIX_MONO_ZONE: tiles within a Monolith's range (5 on engine 1.0); path steps avoid them unless we already stand inside.
struct MonoZone { cambc::vec2 pos; cambc::bitboard zone; };
extern std::vector<MonoZone> g_mono_list;
extern bool g_mono_zone_on;
bool move_towards_closest(const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                          cambc::bitboard goals);

#ifndef P_ROWS_DBG
#define P_ROWS_DBG 0
#endif
#if P_ROWS_DBG
#include <string>
extern std::string g_dbg;   // EXP-095 diagnostic trace of this turn's decisions
#define DBG(x) do { g_dbg += (x); } while (0)
#else
#define DBG(x) do { } while (0)
#endif
