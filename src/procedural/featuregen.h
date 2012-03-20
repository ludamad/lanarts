/*
 * featuregen.h
 *
 *  Created on: Mar 19, 2012
 *      Author: 100397561
 */

#ifndef FEATUREGEN_H_
#define FEATUREGEN_H_

#include "mtwist.h"
#include "GeneratedLevel.h"

class GameState;
struct FeatureGenSettings {
	int nstairs_up, nstairs_down;
	int tileset;
	FeatureGenSettings(int nups, int ndowns, int tileset) :
			nstairs_up(nups), nstairs_down(ndowns), tileset(tileset) {
	}
};

//Generates tiles
void generate_features(const FeatureGenSettings& rs, MTwist& mt, GeneratedLevel& level, GameState* gs);


#endif /* FEATUREGEN_H_ */
