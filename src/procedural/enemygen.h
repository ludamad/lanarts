/*
 * enemygen.h:
 *  Enemy spawning routines
 */

#ifndef ENEMYGEN_H_
#define ENEMYGEN_H_

#include <vector>

#include "../util/game_basic_structs.h"
#include "../util/mtwist.h"

#include "GeneratedLevel.h"

class GameState;
struct EnemyGenChance {
	int guaranteed_spawns; //# of guaranteed generations of this enemy
	int generate_chance; //Proportion of chance to generate. Actual spawn chance relative to sum of all spawn chances.
	enemy_id enemytype;
	int generate_group_chance; //Chance that, if generating, generate group. Out of 100.
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
//Generates enemy monsters during level generation
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt,
		GeneratedLevel& level, GameState* gs);

#endif /* ENEMYGEN_H_ */
