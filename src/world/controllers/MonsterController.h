/*
 * MonsterController.h:
 *  Centralized location of all pathing decisions of monsters, with collision avoidance
 */

#ifndef MONSTERCONTROLLER_H_
#define MONSTERCONTROLLER_H_

#include <vector>

#include "../../collision_avoidance/CollisionAvoidance.h"

#include "../../pathfind/pathfind.h"
#include "../../pathfind/astar_pathfind.h"

#include "../objects/GameInst.h"
#include "../objects/EnemyInst.h"


struct EnemyOfInterest {
	EnemyInst* e;
	int closest_player_index;
	EnemyOfInterest(EnemyInst* e, int player_index) :
			e(e), closest_player_index(player_index){
	}
};

namespace RVO {
	struct RVOSimulator;
}

struct GameLevelState;

class MonsterController {
public:
	MonsterController(bool wander = true);
	~MonsterController();

	const std::vector<obj_id>& monster_ids(){ return mids;}

	void pre_step(GameState* gs);
	void post_draw(GameState* gs);

	void register_enemy(GameInst* enemy);
	void deregister_enemy(EnemyInst* enemy);

    void shift_target(GameState *gs);

    void resize_paths(int size){
    	if (paths.size() > size){
    		for (int i = size; i < paths.size(); i++)
    			delete paths[i];
    	}
    	paths.resize(size, NULL);

    }
    size_t number_monsters() { return mids.size(); }
    obj_id current_target(){ return targetted; }
    void clear();
    //Copy everything but RVO::Simulator
    void partial_copy_to(MonsterController& mc) const;
    //Fill out RVO::Simulator
    void finish_copy(GameLevelState* level);

private:

    void set_monster_headings(GameState *gs, std::vector<EnemyOfInterest> & eois);
    void update_position(GameState* gs, EnemyInst* e);
    void update_velocity(GameState* gs, EnemyInst* e);
    /*returns an index into the player_simids vector*/
    int find_player_to_target(GameState* gs, EnemyInst* e);
    void process_players(GameState* gs);
    void monster_wandering(GameState *gs, EnemyInst *e);
    void monster_follow_path(GameState *gs, EnemyInst *e);
    void monster_get_to_stairs(GameState *gs, EnemyInst *e);

    AStarPathFind astarcontext;
    obj_id targetted;
    std::vector<PathInfo*> paths;
	std::vector<obj_id> mids;
	std::vector<int> player_simids;

	CollisionAvoidance coll_avoid;
	bool monsters_wandering_flag;
};




#endif /* MONSTERCONTROLLER_H_ */
