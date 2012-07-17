/*
 * GameAction.h:
 *  Represents an action taken by a player instance.
 *  Formatted for sending over network.
 */

#ifndef GAMEACTION_H_
#define GAMEACTION_H_
#include <cstdio>

#include "../util/game_basic_structs.h"

class GameInst;
class GameState;

struct GameAction {
	enum action_t {
		MOVE,
		USE_SPELL,
		USE_REST,
		USE_ENTRANCE,
		USE_EXIT,
		USE_ITEM,
		USE_WEAPON,
		PICKUP_ITEM,
		DROP_ITEM,
		DEEQUIP_ITEM,
		REPOSITION_ITEM,
		CHOSE_SPELL
	};
	GameAction() {
	}
	GameAction(obj_id origin, action_t act, int frame, int level,
			int use_id = 0, int action_x = 0, int action_y = 0,
			int use_id2 = 0);
	obj_id origin;
	action_t act;
	int frame, level;
	int use_id, use_id2;
	int action_x, action_y;
};

GameAction game_action(GameState* gs, GameInst* origin, GameAction::action_t action,
		int use_id = 0, int action_x = 0, int action_y = 0, int use_id2 = 0);

void to_action_file(FILE* f, const GameAction& action);
GameAction from_action_file(FILE* f);

#endif /* GAMEACTION_H_ */
