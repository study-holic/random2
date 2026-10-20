#pragma once
// v4: v2's fuel-lean structure plus v3-quick's enemy rules, S-08 squads (squads.cpp), squad micro with shield and
// stuns (micro.cpp), and the banking rules of S-13.
#include <cambc.h>
#include <array>
#include <cstdint>
#include <vector>

// One physical camp group, cached at init so turns never re-query static data.
struct GroupInfo {
    cambc::map::camp_group group;
    std::vector<cambc::vec2> tiles;   // spawn tiles
    cambc::bitboard mask;             // spawn tiles as a bitboard
    mutable cambc::bitboard ring_;    // tiles beside the spawn tiles: where to wait without blocking the respawn (lazy, see ring())
    mutable bool ring_ready = false;
    const cambc::bitboard& ring() const;   // FIX_LAZY_RING: built on first use (was 3k to 5k fuel per group in init)
    bool heavy = false;               // elk or monolith present
    bool monolith = false;
    bool squad_target = false;        // an Elk or three or more wolves: farmed by a squad
    int wolves = 0;
    int gems = 0;
    int deaths = 0;                   // FIX_DEATH_LOOP
    int ready_round = 0;              // estimated round the group respawns (0 = assume alive)
    int excluded_until = 0;           // gave up here: skip this group until this round
    int contested_round = -1000;      // last round 2+ enemy champions were seen within 4 tiles of a spawn tile
    int dist_ally = 0;                // min distance from a spawn tile to our spawn centre
    int dist_enemy = 0;               // same for the enemy spawn centre
    bool cluster = false;             // EXP-030: a gem cluster and no damaging camp (wolf, Elk, Monolith)
    bool has_gem_cluster = false;     // EXP-030: at least one gem cluster (mixed groups too)
    int cluster_seen = -1000;         // EXP-030: last round a mixed group was seen with only clusters alive
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
inline constexpr int kSquadClaimByte = 6;

// Per-champion state survives between turns.
class Bot {
public:
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
    // Stall fix (EXP-026/EXP-021 re-run): retreat ping-pong between two danger-zone tiles, and a lock never attacked.
    cambc::vec2 pos_prev{-1, -1}, pos_prev2{-1, -1};
    int retreat_osc = 0;                // consecutive A-B-A-B turns while retreating
    int retreat_walk_until = 0;         // until this round the retreat walks home through the danger zone
    int lock_wait = 0;                  // turns spent in submit range of a locked altar without attacking the lock
    int carry_since = -1;               // round the current bag was first carried
    bool was_submitting = false;        // P_SHOP_RECALL: a tribute channel was running last turn
    bool remote_shop = false;           // P_FIELD_SHOP or MapPlan::tavi_focus_field: this champion's passive is remote_shop
    bool tavi_focus_field = false;      // w8 build C (MapPlan::tavi_focus_field, set in Plan::init): buy_items_w8
    int field_shop_off_until = 0;       // w8: a field purchase threw; no field buying before this round
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
    // EXP-071 threat layer (threat.cpp)
    bool regrouping = false;            // outnumbered: walking to or holding at allies
    int regroup_start = -1000;
    int last_foe_round = -1000;         // last round an enemy was within P_THREAT_RADIUS
    int bank_altar = -1;                // bank-first: the altar being walked to (sticky, no flipping with enemy moves)
    int flee_budget = 0;                // rounds bank-first has acted for this bag (capped at 8 x P_REGROUP_ROUNDS)
    bool bag_fled = false;              // bank-first moved a carrier away from enemies: hold for P_REGROUP_ROUNDS
    std::vector<int> foe_seen;          // per group: last round any enemy champion was seen near a spawn tile (kept out of GroupInfo: its size changes fuel use, EXP-082)
    int allies6 = 0;                    // allies (not me) within 6 tiles, this turn

    // Static map data cached at init.
    cambc::bitboard static_block;       // walls, holes, altars, enemy spawn
    cambc::bitboard camp_static;        // every camp spawn tile
    cambc::bitboard ally_spawn;
    cambc::bitboard altar_goals;        // tiles from which gems can be submitted
    std::vector<cambc::vec2> altars;
    std::vector<int> lock_until;        // per altar: an enemy lock was seen, avoid it until this round
    std::vector<int> altar_skip_until;  // per altar: stuck there, avoid it until this round
    std::vector<int> altar_foes;        // EXP-023: enemy champions seen within 5 tiles of each altar
    std::vector<int> altar_foes_round;  // round of that sighting
    std::vector<cambc::vec2> spawn_tiles;
    bool cluster_map = false;           // EXP-030: three or more gem-cluster groups on the map
    int fb_origin = -1;                 // EXP-081: squad group this champion left for a fallback group
    int open_group = -1;                // EXP-079: this champion's assigned first group (P_OPENING_ASSIGN)
    bool rows_sched = false;            // EXP-093: fixed pack schedule active (rows.cpp)
    int my_pack = -1;                   // EXP-093: owned pack group index
    std::vector<int> mates;             // EXP-093: roster indices of the unit mates
    std::vector<int> pack_list;         // EXP-093: packs the unit rotates over
    bool bank_spawn_done = false;       // EXP-096: this bank trip has passed the spawn (heal, buy)
    bool idle_rows = false;             // EXP-095: all own packs claimed by others
    int last_shop = -1000;              // EXP-095: last round on the spawn
    int takes = 0;                      // EXP-093: finished takes (rotation counter)
    bool rows = false;                  // EXP-093: rows commander (50x50, six wolf packs, no Elk)
    bool open_done = false;             // EXP-079: first gems collected, assignment released

    TurnView build_view() const;
    void init();
    struct MonoPos { cambc::vec2 pos; int range; };
    std::vector<MonoPos> init_monos;
    int last_tgt = -1; bool was_alive = true;
    int init_phase = 0, init_gi = 0, init_unreachable = 0;
    cambc::vec2 init_ally_centre{0, 0}, init_enemy_centre{0, 0};
    bool init_any_elk = false;
    int init_cluster_groups = 0;
    cambc::distance_field init_field;
    cambc::bitboard init_reach;
    static bool init_fuel_ok(std::uint64_t need) { return cambc::fuel_remaining() >= need; }
    void buy_items();
    void buy_items_w8();   // w8 build C, only where MapPlan::tavi_focus_field is set
    bool refresh_group(int index, int round, const cambc::bitboard& visible);
    void update_group_memory(const TurnView& v);
    bool bank(const TurnView& v, const std::vector<cambc::champion>& enemies,
              const std::vector<cambc::champion>& allies);
    void play_turn_inner();
    // enemies.cpp
    bool enemy_response(const TurnView& v, const std::vector<cambc::champion>& enemies,
                        const std::vector<cambc::champion>& allies);

    // threat.cpp (EXP-071)
    bool threat_layer(const TurnView& v, const std::vector<cambc::champion>& enemies,
                      const std::vector<cambc::champion>& allies);
    int step_to_friends(const std::vector<cambc::vec2>& foes, const std::vector<cambc::champion>& allies);
    bool flee_step(const TurnView& v, const std::vector<cambc::vec2>& foes);
    bool leashed(int index, int round) const;

    // stuck.cpp (EXP-018)
    void update_contested(const TurnView& v, const std::vector<cambc::champion>& enemies);
    bool contested(int index, int round) const;
    bool leave_contested(const TurnView& v);
    void update_stuck();
    bool handle_stuck(const TurnView& v);
    cambc::bitboard altar_goal_tiles(int round) const;
    int choose_altar(const TurnView& v, const std::vector<cambc::champion>& enemies, int& contest);   // EXP-023

    // squads.cpp
    void assign_squads();
    int roster_index(const cambc::champion& c) const;
    int read_squad_claim() const;
    void write_squad_claim(int group);
    bool is_squad_group(int index) const;
    bool claim_usable(int claim, int round, int exclude) const;
    int choose_target(const TurnView& v, int exclude);
    // EXP-030 (cluster.cpp): cluster-first opening on maps with three or more gem-cluster groups.
    bool cluster_opening(int round) const;
    bool harmless_now(int index, int round) const;
    void note_mixed_clusters(const TurnView& v);
    int choose_cluster(const TurnView& v, int exclude, const std::uint8_t* mem);
    bool opening_bank(const TurnView& v, int gems) const;
    int fallback_group(const TurnView& v);                    // EXP-081
    float squad_tank_frac(const std::vector<cambc::champion>& allies) const;   // EXP-081
    void init_opening_assign(const cambc::distance_field& field, const cambc::bitboard& reach);                            // EXP-079
    int assigned_group(const TurnView& v, int exclude, const std::uint8_t* mem);   // EXP-079
    void exclude_group(int index, int round);
    bool group_cleared(const TurnView& v, int index) const;
    int capture_rank(const std::vector<cambc::champion>& allies, int group) const;
    void resolve_target(const TurnView& v, const std::vector<cambc::champion>& allies);

    // rows.cpp (EXP-093)
    void init_rows_sched();
    void resolve_rows(const TurnView& v, const std::vector<cambc::champion>& allies);
    int rows_ready(int k, int round, const std::array<std::uint8_t, 8>& mem) const;   // EXP-095
    bool rows_claimed(int k, const std::array<std::uint8_t, 8>& mem, bool lower_only) const;
    int rows_free_ready(int round) const;
    void farm_rows(const TurnView& v, const std::vector<cambc::champion>& allies);

    // micro.cpp
    void farm(const TurnView& v, const std::vector<cambc::champion>& allies);
    void farm_squad(const TurnView& v, const std::vector<cambc::champion>& allies, const LiveCamp* prey);
    void farm_solo(const TurnView& v, const LiveCamp* prey);
    bool engaged_at_squad_target(const TurnView& v) const;
    bool fresher_mate_near(const TurnView& v, const std::vector<cambc::champion>& allies) const;
    bool wait_for_squad(const TurnView& v, const std::vector<cambc::champion>& allies);

public:
    void play_turn();
};
