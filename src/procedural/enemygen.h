/*
 * enemygen.h:
 *  Enemy spawning routines
 */

#ifndef ENEMYGEN_H_
#define ENEMYGEN_H_

#include "../util/mtwist.h"
#include "GeneratedLevel.h"
#include <vector>
#include "../util/game_basic_structs.h"

class GameState;
struct EnemyGenChance {
	int guaranteed;
	int genchance; //Out of 100%
	int enemytype;
	int groupchance; //Out of 100%
	Range groupsize;
};
struct EnemyGenSettings {
	std::vector<EnemyGenChance> enemy_chances;
	Range num_monsters;
	bool wandering;
	EnemyGenSettings () : wandering(true){

	}
};

//Generate enemy monster after level generation
void generate_enemy_after_level_creation(GameState* gs, enemy_id etype, int amount = 1);
//Generates enemy monsters
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt,
		GeneratedLevel& level, GameState* gs);

//Generates enemy monsters
//void generate_enemies(const MonsterGenSettings& rs, MTwist& mt, GeneratedLevel& level, GameState* gs);

#endif /* ENEMYGEN_H_ */
