#pragma once
// microstella-next: per-map profiles and the champion-kit table. Every strategic knob a decision reads lives in
// MapProfile; the default profile of every map reproduces the live v10 (bots/scr-packs) exactly (EQUIVALENCE mode).
// Later strategy changes are edits to a profile (profile.cpp) or new kit modules (kit.cpp), measured against this base.
#include <cambc.h>
#include <array>
#include <cstdint>
#include <vector>

// Roles replace champion-type checks in squad assignment: squad 0 = vanguard (Vendetta, Issac, Cassi), squad 1 = the rest.
enum class Role : std::uint8_t { vanguard, bruiser, caster, ranged, healer, count };

// Static facts about a champion type (kit.cpp). Profiles may override how a kit is used, never what a champion is.
struct ChampionKit {
    Role role;
    bool main_focus;       // default main stat is Focus (else Strength)
    bool tank;             // candidate for the health-first purchase policy
    bool shields;          // casts its self-shield by the cast_shield rule (Varg)
    bool pair_follower;    // moves with its Issac on the rows schedule (Cassi)
};
// Indexed by champion_type: ryth, vesper_vale, vendetta, cassi, issac, stellar, prism, marcella, willow, ren, varg, tavi.
// Fields: role, main_focus, tank, shields, pair_follower.
inline constexpr ChampionKit kKitTable[12] = {
    {Role::bruiser,  false, true,  false, false},   // ryth
    {Role::ranged,   false, false, false, false},   // vesper_vale
    {Role::vanguard, false, true,  false, false},   // vendetta
    {Role::vanguard, true,  false, false, true},    // cassi
    {Role::vanguard, false, false, false, false},   // issac
    {Role::caster,   true,  false, false, false},   // stellar
    {Role::caster,   true,  false, false, false},   // prism
    {Role::bruiser,  false, false, false, false},   // marcella
    {Role::healer,   false, false, false, false},   // willow
    {Role::bruiser,  false, false, false, false},   // ren
    {Role::bruiser,  false, true,  true,  false},   // varg
    {Role::ranged,   false, false, false, false},   // tavi
};
constexpr const ChampionKit& kit_of(cambc::champion_type type) { return kKitTable[static_cast<int>(type)]; }

enum class MapId : std::uint8_t { unknown, collision, corners, elkring, expanse, interlock, lanes, monolith, packs, rows };

struct BuyPolicy {
    bool main_focus = false;     // main stat Focus (Monster Emblem, Power shards of Focus) instead of Strength
    bool health_first = false;   // Power shards of Health first, until five are owned
};

inline constexpr int kChampionTypes = 12;   // champion_type values 0..11
inline constexpr int kDraftMax = 10;

struct MapProfile {
    MapId id = MapId::unknown;
    const char* name = "generic";

    // --- draft: preference list, first entry not yet picked wins; then the lowest unpicked pick id
    std::array<cambc::champion_draft, kDraftMax> draft{};
    int draft_n = 0;

    // --- purchases per champion type
    std::array<BuyPolicy, kChampionTypes> buy{};

    // --- squads: membership by role, split rules
    std::array<int, static_cast<int>(Role::count)> squad_of_role{};
    int squad_min_roster = 0;      // champions needed for the two-squad split by role
    int squad_min_size = 0;        // a squad smaller than this triggers the melee-first redeal
    int elk_min_squad = 0;         // squad members (with a melee champion) before Elk groups are squad targets
    bool capture_by_health = true;

    // --- target valuation
    int bank_bag = 0;              // gems carried before banking
    int rows_bag = 0;              // same, rows mode
    int cluster_reach = 0;         // cluster-first opening: tiles from the champion
    int cluster_min_groups = 0;    // gem-cluster groups needed for the cluster-first opening
    int pack_wolves = 0;           // wolves in a pack that becomes a squad target on Elk-free maps
    int own_half_rounds = 0;       // before this round only own-half groups are chosen
    double wait_weight = 0;        // target score: tiles per round of respawn wait
    int claim_bonus = 0;           // target score bonus for the squad's claimed group
    bool rows_mode = false;        // rows commander (fixed pack schedule, Power-main-only buying)

    // --- combat stance
    float retreat_hurt = 0, retreat_calm = 0, retreat_tank = 0;
    double retreat_incoming_mult = 0, retreat_incoming_base = 0;
    float recall_done_field = 0, recall_done_spawn = 0, spawn_leave_hp = 0;
    int near_spawn = 0;            // retreat walks in when the spawn is within this many tiles
    float shield_hp = 0;           // Varg shields below this health fraction

    // --- kit actions (default off: equivalence mode)
    bool willow_heal = false;      // Willow casts heal on the most damaged ally within range (kit.cpp), buys Focus
    float willow_heal_below = 0.65f;
    int willow_heal_range = 4;
    bool dash_escape = false;      // reserved: decision code does not call kit_dash_escape yet

    const BuyPolicy& buy_of(cambc::champion_type t) const { return buy[static_cast<int>(t)]; }
};

// The active profile. Defaults to the generic (v10) profile until identified; identification happens once.
extern const MapProfile* g_profile_ptr;   // points into a constexpr table in profile.cpp: identification copies nothing
inline const MapProfile& prof() { return *g_profile_ptr; }

// Draft phase: identifies from width, height, team size; camp groups are scanned only on 50x50 maps (rows vs monolith).
const MapProfile& identify_in_draft();
// Gameplay init: identifies from data Bot::init already holds (no extra camp_groups call). Cached: a profile found
// in the draft phase is kept.
// Gameplay init, first step: size-only profile (rows fingerprint not yet known), so squads can be dealt in v10's position.
void identify_prelim(int width, int height);
const MapProfile& identify_in_init(bool any_elk, int three_wolf_packs, int width, int height);

// kit.cpp -------------------------------------------------------------------------------------------------------
// Willow: heal the most damaged ally (health fraction below profile threshold) within range. True when cast.
bool kit_willow_heal();
// Dash away from `threat`, towards `home` tiles, to a free legal tile. True when dashed. Not called by decisions yet.
bool kit_dash_escape(cambc::vec2 threat, const cambc::bitboard& blocked_static, const std::vector<cambc::vec2>& home);
