/*
 * enemygen.h
 *
 *  Created on: Mar 20, 2012
 *      Author: 100397561
 */

#ifndef ENEMYGEN_H_
#define ENEMYGEN_H_

#include "../util/mtwist.h"
#include "GeneratedLevel.h"
#include <vector>
#include "../util/game_basic_structs.h"

class GameState;
/*
struct EnemyGenSettings {
	int min_xplevel, max_xplevel;
	int nmonsters;
	EnemyGenSettings(int min_xplevel, int max_xplevel, int nmonsters) :
		min_xplevel(min_xplevel), max_xplevel(max_xplevel), nmonsters(nmonsters){

	}
};*/

struct EnemyGenChance {
	int guaranteed;
	int genchance;//Out of 100%
	int enemytype;
	int groupchance;//Out of 100%
	Range groupsize;

};
struct EnemyGenSettings {
	std::vector<EnemyGenChance> enemy_chances;
	Range num_monsters;
	EnemyGenSettings(const std::vector<EnemyGenChance>& enemies, int min_monsters, int max_monsters) :
		enemy_chances(enemies), num_monsters(min_monsters, max_monsters){
	}
};

//Generate enemy monster after level generation
void post_generate_enemy(GameState* gs, enemy_id etype, int amount = 1);
//Generates enemy monsters
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt, GeneratedLevel& level, GameState* gs);


//Generates enemy monsters
//void generate_enemies(const MonsterGenSettings& rs, MTwist& mt, GeneratedLevel& level, GameState* gs);



#endif /* ENEMYGEN_H_ */
