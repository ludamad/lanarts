/*
 * featuregen.h:
 *  Defines parameters for feature generation (eg tile generation, staircase placement),
 *  as well as the generate_features function
 */

#ifndef FEATUREGEN_H_
#define FEATUREGEN_H_

#include "../util/mtwist.h"

#include "../lanarts_defines.h"
#include "GeneratedLevel.h"

#include "dungeon_data.h"

class GameState;

//Generates tiles
void generate_features(const FeatureGenSettings& rs, MTwist& mt,
		GeneratedLevel& level, GameState* gs);

#endif /* FEATUREGEN_H_ */
