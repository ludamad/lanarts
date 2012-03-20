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
		EnemyType("Zombie", 14, SPR_ZOMBIE, Stats(100,100, 5)),
		EnemyType("Storm Elemental", 14, SPR_STORM_ELE, Stats(100,100, 5))
};

size_t game_enemy_n = sizeof(game_enemy_data)/sizeof(EnemyType);



#endif /* ENEMY_DATA_CPP_ */
