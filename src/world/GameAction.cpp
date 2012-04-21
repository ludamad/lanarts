/*
 * GameAction.cpp
 *
 *  Created on: Mar 28, 2012
 *      Author: 100397561
 */

#include "GameAction.h"

GameAction::GameAction(obj_id origin, action_t act, int frame, int level, int use_id, int action_x,
		int action_y)
	: origin(origin), act(act), frame(frame), level(level), use_id(use_id), action_x(action_x), action_y(action_y){

}
void to_action_file(FILE* f, const GameAction& action){
	fwrite(&action, sizeof(GameAction), 1, f);
}
GameAction from_action_file(FILE* f){
	char data[sizeof(GameAction)];
	fread(data, sizeof(GameAction), 1, f);
	return *(GameAction*)data;
}
