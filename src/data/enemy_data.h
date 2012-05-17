/*
 * enemy_data.h
 *	Definitions for the enemy type, these are filled through the parsed yaml file.
 */

#ifndef ENEMY_DATA_H_
#define ENEMY_DATA_H_
#include <cstdlib>
#include <vector>
#include <string>

#include "../lua/LuaValue.h"
#include "../gamestats/Stats.h"

struct EnemyType {
	const char* name;
	int radius;
	int xpaward;
	int sprite_number;
	Stats basestats;

	LuaValue init_event;
	LuaValue step_event;

	EnemyType(){
	}
	EnemyType(const char* name, int rad, int xpaward, int spriten, const Stats& stats, const std::string& step) :
		name(name), radius(rad), xpaward(xpaward), sprite_number(spriten), basestats(stats), step_event(step){
	}
};

extern std::vector<EnemyType> game_enemy_data;

#endif /* ENEMY_DATA_H_ */
