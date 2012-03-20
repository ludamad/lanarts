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

class GameState;

struct EnemyGenSettings {
	int min_xplevel, max_xplevel;
	int nmonsters;
	EnemyGenSettings(int min_xplevel, int max_xplevel, int nmonsters) :
		min_xplevel(min_xplevel), max_xplevel(max_xplevel), nmonsters(nmonsters){

	}
};

//Generates enemy monsters
void generate_enemies(const EnemyGenSettings& rs, MTwist& mt, GeneratedLevel& level, GameState* gs);



#endif /* ENEMYGEN_H_ */
