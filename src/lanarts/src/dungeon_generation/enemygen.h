/*
 * enemygen.h:
 *  Enemy spawning routines
 */

#ifndef ENEMYGEN_H_
#define ENEMYGEN_H_

#include <vector>

#include <lcommon/mtwist.h>

#include "lanarts_defines.h"
#include "GeneratedRoom.h"

#include "dungeon_data.h"

class GameState;

//Generate enemy monster after level generation
void generate_enemy_after_level_creation(GameState* gs, enemy_id etype,
		int amount = 1);

int generate_enemy(GameState* gs, GeneratedRoom& level, MTwist& mt, enemy_id etype,
		const Region& r, team_id teamid, int amount);
//Generates enemy monsters during level generation
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt,
		GeneratedRoom& level, GameState* gs);

#endif /* ENEMYGEN_H_ */
