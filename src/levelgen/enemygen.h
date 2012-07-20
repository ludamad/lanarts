/*
 * enemygen.h:
 *  Enemy spawning routines
 */

#ifndef ENEMYGEN_H_
#define ENEMYGEN_H_

#include <vector>

#include "../data/dungeon_data.h"

#include "../lanarts_defines.h"
#include "../util/mtwist.h"

#include "GeneratedLevel.h"

class GameState;

//Generate enemy monster after level generation
void generate_enemy_after_level_creation(GameState* gs, enemy_id etype, int amount = 1);
//Generates enemy monsters during level generation
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt,
		GeneratedLevel& level, GameState* gs);

#endif /* ENEMYGEN_H_ */
