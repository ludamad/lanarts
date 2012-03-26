/*
 * enemygen.h
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#ifndef ENEMYGEN_H_
#define ENEMYGEN_H_

#include "mtwist.h"
#include "GeneratedLevel.h"
#include <vector>

class GameState;

struct EnemyGenSettings {
	int min_xplevel, max_xplevel;
	int nmonsters;
	EnemyGenSettings(int min_xplevel, int max_xplevel, int nmonsters) :
		min_xplevel(min_xplevel), max_xplevel(max_xplevel), nmonsters(nmonsters){

	}
};

struct EnemyGenChance {
	int genchance;//Out of 100%
	int enemytype;
	int groupchance;//Out of 100%
	int groupmin, groupmax;

};
struct MonsterGenSettings {
	std::vector<EnemyGenChance> enemy_chances;
	int min_monsters, max_monsters;
	MonsterGenSettings(const std::vector<EnemyGenChance>& enemies, int min_monsters, int max_monster) :
		enemy_chances(enemies), min_monsters(min_monsters), max_monsters(max_monsters){
	}
};

//Generates enemy monsters
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt, GeneratedLevel& level, GameState* gs);


//Generates enemy monsters
void generate_enemies(const MonsterGenSettings& rs, MTwist& mt, GeneratedLevel& level, GameState* gs);



#endif /* ENEMYGEN_H_ */
