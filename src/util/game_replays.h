/*
 * game_replays.h:
 *  Utilities for storing and replaying game replays
 */

#ifndef GAME_REPLAYS_H_
#define GAME_REPLAYS_H_

#include <deque>
#include "../gamestate/GameAction.h"

class GameState;

void load_actions(GameState* gs, std::deque<GameAction>& actions);
void save_actions(GameState* gs, std::deque<GameAction>& actions);

void load_init(GameState* gs, int& seed, class_id& classtype);
void save_init(GameState* gs, int seed, class_id classtype);


#endif /* GAME_REPLAYS_H_ */
