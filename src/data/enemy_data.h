/*
 * enemy_data.h
 *	Definitions for the enemy type, these are filled through the parsed yaml file.
 */

#ifndef ENEMY_DATA_H_
#define ENEMY_DATA_H_
#include <cstdlib>
#include "../gamestats/Stats.h"
#include <vector>

struct EnemyType {
	const char* name;
	int sprite_number;
	int radius;
	int xpaward;
	Stats basestats;
	EnemyType(){
	}
	EnemyType(const char* name, int rad, int xpaward, int spriten, const Stats& stats) :
		name(name), radius(rad), xpaward(xpaward), sprite_number(spriten), basestats(stats){
	}
};

extern std::vector<EnemyType> game_enemy_data;

#endif /* ENEMY_DATA_H_ */
