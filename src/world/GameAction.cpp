/*
 * GameAction.cpp:
 *  Represents an action taken by a player instance.
 *  Formatted for sending over network.
 */

#include "GameAction.h"
#include "GameState.h"
#include "GameLevelState.h"

#include "objects/GameInst.h"

GameAction::GameAction(obj_id origin, action_t act, int frame, int level,
		int use_id, int action_x, int action_y, int use_id2) :
		origin(origin), act(act), frame(frame), level(level), use_id(use_id), use_id2(
				use_id2), action_x(action_x), action_y(action_y) {

}
void to_action_file(FILE* f, const GameAction& action) {
	fwrite(&action, sizeof(GameAction), 1, f);
}
GameAction game_action(GameState* gs, GameInst* origin,
		GameAction::action_t action, int use_id, int action_x, int action_y,
		int use_id2) {
	return GameAction(origin->id, action, gs->frame(),
			gs->get_level()->level_number, use_id, action_x, action_y, use_id2);
}

GameAction from_action_file(FILE* f) {
	char data[sizeof(GameAction)];
	fread(data, sizeof(GameAction), 1, f);
	return *(GameAction*) data;
}

