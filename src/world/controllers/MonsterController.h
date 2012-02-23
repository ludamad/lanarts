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
	void pre_step(GameState* gs);
	void register_enemy(obj_id monster) { mids.push_back(monster); }

	const std::vector<obj_id>& monster_ids(){ return mids;}
	//vector of paths to player instances
private:
	std::vector<PathInfo> paths;
	void set_behaviours(GameState* gs);
	//vector of monster ids
	std::vector<obj_id> mids;
};

#endif /* MONSTERCONTROLLER_H_ */
