/*
 * itemgen.h:
 *  Defines parameters for item generation as well as the generate_items function
 */

#ifndef ITEMGEN_H_
#define ITEMGEN_H_

#include <vector>

#include "dungeon_data.h"
#include "../util/mtwist.h"
#include "GeneratedLevel.h"

class GameState;

void generate_items(const ItemGenSettings& is, MTwist& mt,
		GeneratedLevel& level, GameState* gs);

#endif /* ITEMGEN_H_ */
