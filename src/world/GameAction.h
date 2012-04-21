/*
 * GameAction.h
 *
 *  Created on: Mar 28, 2012
 *      Author: 100397561
 */

#ifndef GAMEACTION_H_
#define GAMEACTION_H_
#include <cstdio>
#include "objects/GameInst.h"

struct GameAction {
	enum action_t {
		MOVE_IN_DIRECTION, SPELL,
		REST,
		USE_ENTRANCE,
		USE_EXIT,
		USE_ITEM,
		PICKUP_ITEM
	};
	GameAction(){}
	GameAction(obj_id origin, action_t act, int frame, int level, int use_id = 0, int action_x = 0,
			int action_y = 0);
	obj_id origin;
	action_t act;
	int frame, level;
	int use_id;
	int action_x, action_y;
};

void to_action_file(FILE* f, const GameAction& action);
GameAction from_action_file(FILE* f);

#endif /* GAMEACTION_H_ */
