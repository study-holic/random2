#pragma once
// Tung clone: a sparring bot imitating Emperor Tung (strategy/tung-clone.md). Squads farm the own half of the map,
// a tank holds the camp, ranged allies stand at distance 3, and one champion captures while the rest move on.
#include <cambc.h>
#include <vector>

// One physical camp group, cached at init so turns never re-query static data.
struct GroupInfo {
    cambc::map::camp_group group;
    std::vector<cambc::camp> camps;   // camp handles, cached
    std::vector<cambc::vec2> tiles;   // spawn tiles
    cambc::bitboard mask;             // spawn tiles as a bitboard
    cambc::bitboard ring;             // tiles beside the spawn tiles: where to wait without blocking the respawn
    bool heavy = false;               // elk or monolith present
    bool monolith = false;
    bool own_half = false;            // nearest tile closer to our spawn region than to the enemy's
    bool tie_half = false;            // equally close (used only when no group is strictly ours)
    int camp_count = 0;
    int wolves = 0;
    int ready_round = 0;              // estimated round the group respawns (0 = assume alive)
};

// A live camp together with the index of its group and its health, read once per turn.
struct LiveCamp {
    cambc::camp camp;
    int group;
    float health;
};

// Per-turn view of live camps near the champion and of the target group, queried once.
struct TurnView {
    cambc::vec2 pos{};
    int round = 0;
    std::vector<LiveCamp> camps;
    cambc::bitboard visible;
};

// Tiles within reach of a live damaging monster (range plus one tile margin).
cambc::bitboard danger_zone(const TurnView& v);

// Per-champion state survives between turns.
class Bot {
    bool initialised = false;
    int ally_index = 0;                 // position in the ally list (champion id order)
    int squad = 0;                      // fixed at draft time by champion type
    int ally_count = 0;
    std::vector<GroupInfo> groups;
    int target = -1;                    // group this champion works on (follows the squad claim)
    int holding = -1;                   // group whose capture this champion is standing on
    bool retreating = false;            // hurt: leave camp range, recall, return at full health
    std::size_t state_cursor = 0;       // rotates the group whose state is refreshed this turn

    // Static map data cached at init.
    cambc::bitboard static_block;       // walls, holes, altars, enemy spawn
    cambc::bitboard camp_static;        // every camp spawn tile
    cambc::bitboard ally_spawn;
    cambc::bitboard altar_goals;        // tiles from which gems can be submitted
    std::vector<cambc::vec2> altars;
    std::vector<cambc::vec2> spawn_tiles;
    std::vector<cambc::champion> roster;   // every ally in id order
    bool is_melee = false;

    bool go(const cambc::bitboard& goals, const TurnView& v);
    TurnView build_view() const;
    void init();
    void buy_items();
    void update_group_memory(int round, const cambc::bitboard& visible);

    // Squad logic (farm.cpp).
    int choose_target(const TurnView& v, int exclude);
    void write_claim(int group);
    int read_claim() const;
    bool refresh_group(int index, int round, const cambc::bitboard& visible);
    bool group_cleared(const TurnView& v, int index) const;
    bool tank_role(const std::vector<cambc::champion>& allies) const;
    int capture_rank(const std::vector<cambc::champion>& allies, int group) const;
    void farm(const TurnView& v, bool is_tank, const std::vector<cambc::champion>& allies);
    void resolve_inner(const TurnView& v, const std::vector<cambc::champion>& allies);
    void resolve_target(const TurnView& v, const std::vector<cambc::champion>& allies);
    void play_turn_inner();

public:
    void play_turn();
};

int squad_of(cambc::champion_type type, int champions);
