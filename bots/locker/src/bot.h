#pragma once
#include <cambc.h>
#include <vector>

// One physical camp group, cached at init so turns never re-query static data.
struct GroupInfo {
    cambc::map::camp_group group;
    std::vector<cambc::vec2> tiles;   // spawn tiles
    cambc::bitboard mask;             // spawn tiles as a bitboard
    cambc::bitboard ring;             // tiles beside the spawn tiles: where to wait without blocking the respawn
    bool heavy = false;               // elk or monolith present
    bool monolith = false;
    int ready_round = 0;              // estimated round the group respawns (0 = assume alive)
};

// Per-turn view of live camps near the champion, queried once.
struct TurnView {
    cambc::vec2 pos{};
    int round = 0;
    std::vector<cambc::camp> camps;   // nearby visible camps with health above zero
};

// Per-champion state survives between turns.
class Bot {
    bool initialised = false;
    int ally_index = 0;                 // slot in shared memory (0..7)
    std::vector<GroupInfo> groups;
    int target = -1;
    bool retreating = false;            // hurt: leave camp range, recall, return at full health
    std::size_t state_cursor = 0;       // rotates the group whose state is refreshed this turn

    // Static map data cached at init.
    cambc::bitboard static_block;       // walls, holes, altars, enemy spawn
    cambc::bitboard camp_static;        // every camp spawn tile
    cambc::bitboard ally_spawn;
    cambc::bitboard altar_goals;        // tiles from which gems can be submitted
    std::vector<cambc::vec2> altars;

    TurnView build_view() const;
    void init();
    void buy_items();
    void update_group_memory(int round);
    int choose_target();
    void play_turn_inner();

public:
    void play_turn();
};
