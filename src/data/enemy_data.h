/*
 * enemy_data.h
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#ifndef ENEMY_DATA_H_
#define ENEMY_DATA_H_
#include <cstdlib>
#include "../combat_logic/Stats.h"

struct EnemyType {
	const char* name;
	int sprite_number;
	int radius;
	Stats basestats;
	EnemyType(const char* name, int rad, int spriten, const Stats& stats) :
		name(name), radius(rad), sprite_number(spriten), basestats(stats){
	}
};

extern EnemyType game_enemy_data[];
extern size_t game_enemy_n;

#endif /* ENEMY_DATA_H_ */
