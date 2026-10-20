#pragma once
// Shared helpers: geometry, strength table, attack and cast wrappers, and the obstacle-aware step.
#include <cambc.h>
#include <vector>

struct TurnView;

inline constexpr int kMonsterRange = 3;   // rules: every monster attacks at range 3

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
