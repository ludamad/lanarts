/*
 * MonsterController.h
 *
 *  Created on: Feb 20, 2012
 *      Author: 100397561
 */

#ifndef MONSTERCONTROLLER_H_
#define MONSTERCONTROLLER_H_

#include <vector>
#include "../objects/GameInst.h"
#include "../objects/EnemyInst.h"
#include "../../pathfind/pathfind.h"
#include "../../pathfind/astar_pathfind.h"


struct EnemyOfInterest {
	EnemyInst* e;
	int closest_player_index;
	int dist_to_player_sqr;
	EnemyOfInterest(EnemyInst* e, int player_index, int distsqr) :
			e(e), closest_player_index(player_index), dist_to_player_sqr(distsqr) {
	}
	bool operator<(const EnemyOfInterest& eoi) const {
		return dist_to_player_sqr < eoi.dist_to_player_sqr;
	}
};

namespace RVO {
	struct RVOSimulator;
}

class MonsterController {
public:
	MonsterController();
	~MonsterController();

	const std::vector<obj_id>& monster_ids(){ return mids;}

	void pre_step(GameState* gs);
	void post_draw(GameState* gs);
	void register_enemy(GameInst* enemy);
    void shift_target(GameState *gs);
    void clear();
    void set_monster_headings(GameState *gs, std::vector<EnemyOfInterest> & eois);
    void update_position(GameState* gs, EnemyInst* e);
    void update_velocity(GameState* gs, EnemyInst* e);
public:
    void monster_wandering(GameState *gs, EnemyInst *e);
    void monster_follow_path(GameState *gs, EnemyInst *e);
    void monster_get_to_stairs(GameState *gs, EnemyInst *e);
    bool must_initialize;
    AStarPathFind astarcontext;
    obj_id targetted;
    std::vector<PathInfo> room_paths;
    std::vector<PathInfo> paths;
	std::vector<obj_id> mids;

	RVO::RVOSimulator* simulator;
};




#endif /* MONSTERCONTROLLER_H_ */
