#pragma once
// v4: v2's fuel-lean structure plus v3-quick's enemy rules, S-08 squads (squads.cpp), squad micro with shield and
// stuns (micro.cpp), and the banking rules of S-13.
#include <cambc.h>
#include <cstdint>
#include <vector>

// One physical camp group, cached at init so turns never re-query static data.
struct GroupInfo {
    cambc::map::camp_group group;
    std::vector<cambc::vec2> tiles;   // spawn tiles
    cambc::bitboard mask;             // spawn tiles as a bitboard
    cambc::bitboard ring;             // tiles beside the spawn tiles: where to wait without blocking the respawn
    bool heavy = false;               // elk or monolith present
    bool monolith = false;
    bool squad_target = false;        // an Elk or three or more wolves: farmed by a squad
    int wolves = 0;
    int gems = 0;
    int ready_round = 0;              // estimated round the group respawns (0 = assume alive)
    int excluded_until = 0;           // gave up here: skip this group until this round
    int contested_round = -1000;      // last round 2+ enemy champions were seen within 4 tiles of a spawn tile
    int dist_ally = 0;                // min distance from a spawn tile to our spawn centre
    int dist_enemy = 0;               // same for the enemy spawn centre
};

// A live camp with its group index and health, read once per turn.
struct LiveCamp {
    cambc::camp camp;
    int group;
    float health;
};

// Per-turn view of live camps near the champion (and of the target group), queried once.
struct TurnView {
    cambc::vec2 pos{};
    int round = 0;
    std::vector<LiveCamp> camps;
    cambc::bitboard visible;
};

// Shared memory: bytes 0..5 hold each champion's target + 1, bytes 6 and 7 each squad's claimed group + 1.
// Ambush (EXP-020, maps with 4 or 5 picks only): while the ambush runs, bytes 0 (last round an enemy champion was
// seen, /2 + 64), 1 (round the stack arrived, /2 + 64) and 5 (bit 7: ambush over, bits 0..4: enemies seen) are
// reused; per-champion target writes pause until the ambush is over.
inline constexpr int kSquadClaimByte = 6;
inline constexpr int kByteLastSeen = 0, kByteArrived = 1, kByteFlags = 5;

// Per-champion state survives between turns.
class Bot {
    bool initialised = false;
    int ally_index = 0;                 // slot in shared memory (0..5), position in the roster
    int squad = 0;
    bool is_melee = false;
    bool squad_capable = true;          // the squad has a melee champion to tank
    std::vector<GroupInfo> groups;
    std::vector<cambc::champion> roster;
    std::vector<int> roster_squad;
    std::vector<bool> roster_melee;
    std::vector<int> capture_key;       // capture order inside a squad (lowest first)
    int target = -1;
    int holding = -1;                   // squad group whose capture this champion stands on
    int hold_since = 0;
    bool retreating = false;            // hurt: leave camp range, recall, return at full health
    int carry_since = -1;               // round the current bag was first carried
    bool banking = false;               // decided to bank: keep going until the bag is empty
    int wait_rounds = 0;                // rounds spent waiting beside a respawning group
    int wait_tank = 0;
    int wait_cohesion = 0;              // rounds spent waiting for squad members to catch up                  // ranged: rounds spent waiting for a melee ally to engage
    std::size_t state_cursor = 0;       // rotates the group whose state is refreshed this turn
    // Stuck detector (EXP-018): rounds alive at the same position, not tributing, capturing or attacking.
    cambc::vec2 last_pos{-1, -1};
    int still_rounds = 0;
    bool could_attack = false;          // can_attack() at the start of the turn (attack detection)
    bool attacked = false;
    int walk_home = 0;                  // stuck walk: rounds left walking towards our spawn
    cambc::vec2 walk_from{0, 0};
    // Ambush opening (ambush.cpp).
    bool ambush_enabled = false;        // this map turns the ambush on (D <= 25, 4 or 5 picks)
    bool ambush_over = false;
    bool ambush_arrived = false;        // this champion reached the staging tile
    bool ambush_way_done = true;
    int ambush_group = -1;              // the enemy's first group
    int ambush_d = -1;                  // distance from our spawn centre to that group
    int ambush_hold = 0;                // rounds held for a slower member
    int ambush_idle = 0;                // rounds the travel step failed
    cambc::vec2 ambush_stage{0, 0};     // tile beside the group nearest to us
    cambc::vec2 ambush_way{0, 0};       // waypoint on our side of the map
    std::uint8_t ambush_killed = 0;     // enemy champions seen dead (index mask)
    std::vector<cambc::champion> enemy_roster;
    cambc::bitboard enemy_spawn;

    // Static map data cached at init.
    cambc::bitboard static_block;       // walls, holes, altars, enemy spawn
    cambc::bitboard camp_static;        // every camp spawn tile
    cambc::bitboard ally_spawn;
    cambc::bitboard altar_goals;        // tiles from which gems can be submitted
    std::vector<cambc::vec2> altars;
    std::vector<int> lock_until;        // per altar: an enemy lock was seen, avoid it until this round
    std::vector<int> altar_skip_until;  // per altar: stuck there, avoid it until this round
    std::vector<cambc::vec2> spawn_tiles;

    TurnView build_view() const;
    void init();
    void buy_items();
    bool refresh_group(int index, int round, const cambc::bitboard& visible);
    void update_group_memory(const TurnView& v);
    bool bank(const TurnView& v, const std::vector<cambc::champion>& enemies,
              const std::vector<cambc::champion>& allies);
    void play_turn_inner();
    // enemies.cpp
    bool enemy_response(const TurnView& v, const std::vector<cambc::champion>& enemies,
                        const std::vector<cambc::champion>& allies);

    // stuck.cpp (EXP-018)
    void update_contested(const TurnView& v, const std::vector<cambc::champion>& enemies);
    bool contested(int index, int round) const;
    bool leave_contested(const TurnView& v);
    void update_stuck();
    bool handle_stuck(const TurnView& v);
    cambc::bitboard altar_goal_tiles(int round) const;

    // squads.cpp
    void assign_squads();
    int roster_index(const cambc::champion& c) const;
    int read_squad_claim() const;
    void write_squad_claim(int group);
    bool is_squad_group(int index) const;
    bool claim_usable(int claim, int round, int exclude) const;
    int choose_target(const TurnView& v, int exclude);
    void exclude_group(int index, int round);
    bool group_cleared(const TurnView& v, int index) const;
    int capture_rank(const std::vector<cambc::champion>& allies, int group) const;
    void resolve_target(const TurnView& v, const std::vector<cambc::champion>& allies);

    // micro.cpp
    void farm(const TurnView& v, const std::vector<cambc::champion>& allies);
    void farm_squad(const TurnView& v, const std::vector<cambc::champion>& allies, const LiveCamp* prey);
    void farm_solo(const TurnView& v, const LiveCamp* prey);
    bool engaged_at_squad_target(const TurnView& v) const;
    bool wait_for_squad(const TurnView& v, const std::vector<cambc::champion>& allies);

    // ambush.cpp (EXP-020)
    void ambush_init(cambc::vec2 ally_centre);
    bool ambush_active() const { return ambush_enabled && !ambush_over; }
    void ambush_update(const TurnView& v, const std::vector<cambc::champion>& enemies,
                       const std::vector<cambc::champion>& allies);
    bool ambush_turn(const TurnView& v, const std::vector<cambc::champion>& enemies,
                     const std::vector<cambc::champion>& allies);
    bool ambush_travel(const TurnView& v, const std::vector<cambc::champion>& allies);
    void ambush_focus(const TurnView& v, const cambc::champion& foe, cambc::vec2 centre);
    std::uint8_t shared_byte(int i) const;
    void set_shared_byte(int i, std::uint8_t value);

public:
    void play_turn();
};
