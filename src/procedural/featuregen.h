/*
 * featuregen.h
 *  Defines parameters for feature generation (eg tile generation, staircase placement),
 *  as well as the generate_features function
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
