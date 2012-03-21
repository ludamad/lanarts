/*
 * enemy_data.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#ifndef ENEMY_DATA_CPP_
#define ENEMY_DATA_CPP_

#include "enemy_data.h"
#include "sprite_data.h"

EnemyType game_enemy_data[] = {
		EnemyType("Zombie", 14, SPR_ZOMBIE, Stats(2, 40,100, 1, 0, 70, 25 /*melee reach*/, 100, 4)),
		EnemyType("Storm Elemental", 14, SPR_STORM_ELE, Stats(1, 40,100, 0, 1, 70, 0/*no melee reach*/, 100, 4))
};

size_t game_enemy_n = sizeof(game_enemy_data)/sizeof(EnemyType);



#endif /* ENEMY_DATA_CPP_ */
