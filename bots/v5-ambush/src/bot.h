#pragma once
// v5: one stack (stack.cpp), optional ambush wipe (ambush.cpp, kAmbush). v4: v2's fuel-lean structure plus v3-quick's enemy rules, S-08 squads (squads.cpp), squad micro with shield and
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

// Variant switch: false = v5-stack, true = v5-ambush (ambush.cpp).
inline constexpr bool kAmbush = true;

// Shared memory (v5): byte 0 enemy kill mask, byte 2 last round an enemy was seen (/2), byte 3 stack home call
// (/2 + 1), byte 6 the stack's claimed group + 1, byte 7 ambush state (bits 0..1: 0 none seen, 1 seen, 2 wipe over)
// and focus target (bits 2..5: enemy index + 1).
inline constexpr int kSquadClaimByte = 6;
inline constexpr int kByteKills = 0, kByteSeen = 2, kByteHome = 3, kByteState = 7;

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
    int wait_cohesion = 0;              // rounds spent holding for stack members (stack.cpp)
    int wait_stack = 0;                 // rounds spent waiting beside a cleared group for the capturer
    bool stack_banking = false;         // the stack is on a bank run
    int bank_since = 0;
    // Ambush (ambush.cpp)
    std::vector<cambc::champion> enemy_roster;
    cambc::bitboard enemy_spawn;
    int ambush_group = -1;              // group on the enemy half nearest to the enemy spawn
    bool ambush_done = false;
    int focus_unseen = 0;               // rounds the focus target has been out of vision
    cambc::vec2 focus_last{};
    float focus_last_hp = 1.0f;
    bool home_call = false;
    bool focus_seen = false;            // this champion has seen the current focus target
    int home_stamp = -1;                // last home call handled
    std::size_t state_cursor = 0;       // rotates the group whose state is refreshed this turn

    // Static map data cached at init.
    cambc::bitboard static_block;       // walls, holes, altars, enemy spawn
    cambc::bitboard camp_static;        // every camp spawn tile
    cambc::bitboard ally_spawn;
    cambc::bitboard altar_goals;        // tiles from which gems can be submitted
    std::vector<cambc::vec2> altars;
    std::vector<int> lock_until;        // per altar: an enemy lock was seen, avoid it until this round
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

    // stack.cpp
    bool stay_with_stack(const TurnView& v, const std::vector<cambc::champion>& allies,
                         const std::vector<cambc::vec2>& dest);
    void update_stack_bank(const TurnView& v, const std::vector<cambc::champion>& allies);
    bool escort(const TurnView& v, const std::vector<cambc::champion>& allies);
    std::uint8_t shared_byte(int i) const;
    void set_shared_byte(int i, std::uint8_t value);
    // ambush.cpp
    int ambush_state() const;
    bool ambush_on() const;             // kAmbush and the wipe is not over
    void ambush_init(const cambc::vec2& enemy_centre);
    void ambush_bookkeeping(const TurnView& v, const std::vector<cambc::champion>& enemies,
                            const std::vector<cambc::champion>& allies);
    bool wipe_turn(const TurnView& v, const std::vector<cambc::champion>& enemies,
                   const std::vector<cambc::champion>& allies);

public:
    void play_turn();
};
