/*
 * Team.h:
 *   Represents a list of CombatGameInst's on the same team for a specific level.
 *   Handle's field of view for a team, as well as factors such as whether two CombatGameInst's see each other.
 */

#ifndef TEAM_H_
#define TEAM_H_

#include <set>
#include <lcommon/geometry.h>
#include "lanarts_defines.h"

class SerializeBuffer;
class GameState;
class CombatGameInst;
class PlayerInst;

// TODO remember to split object_visible and object_drawwise_visible (or such) into separate concepts
// Do not want to have game decisions based on drawing differences.
// TODO find better home for some of these

CombatGameInst* get_nearest_enemy(GameState* gs, CombatGameInst* inst);
CombatGameInst* get_nearest_visible_enemy(GameState* gs, CombatGameInst* inst);
CombatGameInst* get_nearest_ally(GameState* gs, CombatGameInst* inst);
//CombatGameInst* get_nearest_on_team(GameState* gs, _Team& team, CombatGameInst* inst, float* smallest_sqr_dist = NULL,
//        CombatGameInst** closest_game_inst = NULL);

// The raw direction towards an object, not accounting for collaborative motion planning (which takes this as input).
PosF get_direction_towards(GameState* gs, CombatGameInst* from, CombatGameInst* to, float max_speed);

bool are_allies(CombatGameInst* inst1, CombatGameInst* inst2);
bool are_enemies(CombatGameInst* inst1, CombatGameInst* inst2);

bool is_visible(GameState* gs, CombatGameInst* viewer, BBox area);
bool is_visible(GameState* gs, CombatGameInst* viewer, CombatGameInst* observed);

struct TeamPlayerData {
    // The gold accumulated by players:
    money_t gold = 0; 
    std::vector<PlayerInst*> players;
    void serialize(GameState* gs, SerializeBuffer& buffer);
    void deserialize(GameState* gs, SerializeBuffer& buffer);
};

// Feature work: Allow mixed teams of EnemyInst's and PlayerInst's.
// All the team objects for all the teams on a given level.
// Currently, all teams are enemies to each other.
// Facilitates deciding things such as:
//   - What's the nearest enemy/ally a certain object can see?
//   - How to path towards a certain enemy/ally?
typedef std::vector<CombatGameInst*> TeamLevelData;
struct Team {
    std::vector<TeamLevelData> per_level_data;
    // If the team has players:
    TeamPlayerData player_data;
    void serialize(GameState* gs, SerializeBuffer& buffer);
    void deserialize(GameState* gs, SerializeBuffer& buffer);
};

struct TeamData {
    std::vector<Team> teams;
    void _ensure(team_id team);
    void _ensure(level_id level, team_id team);
    void add(CombatGameInst* obj);
    void remove(CombatGameInst* obj);
    void serialize(GameState* gs, SerializeBuffer& buffer);
    void deserialize(GameState* gs, SerializeBuffer& buffer);
};

// Currently just have two teams:
constexpr int PLAYER_TEAM = 0;
#endif
