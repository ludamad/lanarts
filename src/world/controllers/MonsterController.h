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
#include "../../pathfind/pathfind.h"

class MonsterController {
public:
	MonsterController();
	~MonsterController();

	const std::vector<obj_id>& monster_ids(){ return mids;}

	void pre_step(GameState* gs);
	void register_enemy(obj_id monster) { mids.push_back(monster); }

	void clear();
public:
	//vector of paths to player instances
	std::vector<PathInfo> paths;
	//vector of monster ids
	std::vector<obj_id> mids;
};

#endif /* MONSTERCONTROLLER_H_ */
