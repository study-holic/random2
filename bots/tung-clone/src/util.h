#pragma once
// Shared helpers: geometry, strength table, attack and cast wrappers, and the obstacle-aware step.
#include <cambc.h>
#include <vector>

struct TurnView;

int tile_distance(cambc::vec2 a, cambc::vec2 b);
cambc::bitboard tiles_around(cambc::vec2 position, int radius);
int camp_strength(const cambc::camp& c);

// Distance from `pos` to the nearest tile in `tiles`.
int min_distance(cambc::vec2 pos, const std::vector<cambc::vec2>& tiles);

bool attack_if_in_range(const cambc::unit& target);
void reset_and_attack_again(const cambc::unit& target);
bool cast_on_camp(const cambc::camp& camp);

// Step towards the closest goal tile. Camp spawn tiles stay obstacles unless they are goals.
bool move_towards_closest(const cambc::bitboard& static_block, const cambc::bitboard& camp_static,
                          cambc::bitboard goals, const cambc::bitboard* avoid = nullptr);
