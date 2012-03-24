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

class MonsterController {
public:
	MonsterController();
	~MonsterController();

	const std::vector<obj_id>& monster_ids(){ return mids;}

	void pre_step(GameState* gs);
	void post_draw(GameState* gs);
	void register_enemy(obj_id monster) { mids.push_back(monster); }
	void shift_target(GameState* gs);

	void clear();
	void set_monster_headings(GameState* gs, std::vector<EnemyOfInterest>& eois);
public:
	obj_id targetted;
	//vector of paths to player instances
	std::vector<PathInfo> paths;
	//vector of monster ids
	std::vector<obj_id> mids;
};

#endif /* MONSTERCONTROLLER_H_ */
