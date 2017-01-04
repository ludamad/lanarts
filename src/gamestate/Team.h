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
#include "GameState.h"
#include "objects/CombatGameInst.h"

class PlayerInst;

// Associated with an object in the team.
// Used to aid decisions made by hostile and ally creatures with respect to
// the CombatGameInst this represents.
struct _TeamMemberData {
    // path_to_object:
    //   Used for decisions about pathing to the object.
    //   For major team members (players, AI controlling player-like characters, classes that summon a major companion)
    //   this is updated every step and perfect pathing towards the object is used.
    //   For minor team members (every other NPC) this is not used.
    FloodFillPaths paths_to_object;
    // field_of_view:
    //   Used for decisions about whether one object sees another.
    //   This is used for all allies of the player, although most allies do not provide the player
    //   with their full field of view.
    //   This is used to decide if a major team member sees an enemy, but other NPCs have more primitive sight code.
    fov* field_of_view = NULL;
};

struct _Team {
    // Identifies the team.
    team_id id;
    // Contains all the object id's, mapped to ephermal data used for team-based decisions.
    std::map<obj_id, _TeamMemberData> members;
};

// TODO remember to split object_visible and object_drawwise_visible (or such) into separate concepts
// Do not want to have game decisions based on drawing differences.

// All the team objects for all the teams on a given level.
// Currently, all teams are enemies to each other.
// Allows for deciding things such as:
//   - What's the nearest enemy/ally a certain object can see?
//   - How to path towards a certain enemy/ally?
class LevelTeamData {
public:
    // Requirements: The level holding this team should be the current level of focus in GameState.
    fov* get_fov_of(GameState* gs, CombatGameInst* inst);
    CombatGameInst* get_nearest_enemy(GameState* gs, CombatGameInst* inst);
    CombatGameInst* get_nearest_ally(GameState* gs, CombatGameInst* inst);
    // The raw direction towards an object, not accounting for
    PosF get_direction_towards(GameState* gs, CombatGameInst* from, CombatGameInst* to, float max_speed);

    bool are_allies(CombatGameInst* inst1, CombatGameInst* inst2);
    bool are_enemies(CombatGameInst* inst1, CombatGameInst* inst2);

    bool are_tiles_visible(GameState* gs, CombatGameInst* viewer, BBox tile_span);
    bool is_visible(GameState* gs, CombatGameInst* viewer, BBox area);
    bool is_visible(GameState* gs, CombatGameInst* viewer, CombatGameInst* observed);

    void remove_instance(GameState* gs, CombatGameInst* inst);
    void add_instance(GameState* gs, CombatGameInst* inst);
    template <typename Function>
    void for_all_enemies(GameState* gs, team_id id, Function&& f) {
        for (auto& team : _teams) {
            if (id == team.id) {
                continue;
            }
            _for_all_on_team(gs, team, f);
        }
    }

    template <typename Function>
    void for_all_on_team(GameState* gs, team_id id, Function&& f) {
        for (auto& team : _teams) {
            if (id != team.id) {
                continue;
            }
            _for_all_on_team(gs, team, f);
        }
    }
private:
    template <typename Function>
    void _for_all_on_team(GameState* gs, _Team& team, Function&& f) {
        for (auto& entry : team.members) {
            f(gs->get_instance(entry.first));
        }
    }
    std::vector<_Team> _teams;
    _Team* _get_team_of(CombatGameInst* inst);
    _TeamMemberData* _get_member_data_of(CombatGameInst* inst);
    void _get_nearest_on_team(GameState* gs, _Team& team, CombatGameInst* inst, float* smallest_sqr_dist = NULL,
            CombatGameInst** closest_game_inst = NULL);
};

typedef std::set<CombatGameInst*> TeamLevelData;

struct PlayerDataEntry2 {
    std::string player_name;
    MultiframeActionQueue action_queue;
    GameInstRef player_inst;
    class_id classtype;
    int net_id;

    PlayerInst* player() const;

    PlayerDataEntry(const std::string& player_name, GameInst* player_inst,
            class_id classtype, int net_id) :
            player_name(player_name), player_inst(player_inst), classtype(
                    classtype), net_id(net_id) {
    }
};

struct Team {
    std::vector<TeamLevelData> per_level_data;
    std::vector<PlayerDataEntry2> player_data;
};

struct TeamData {
    std::vector<Team> teams;
    void add(CombatGameInst* obj) {
        teams[obj->team][obj->current_floor].add(obj);
    }
    void remove(CombatGameInst* obj) {
        teams[obj->team][obj->current_floor].remove(obj);
    }
};

#endif
