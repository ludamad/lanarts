/*
 * itemgen.h:
 *  Defines parameters for item generation as well as the generate_items function
 */

#ifndef ITEMGEN_H_
#define ITEMGEN_H_

#include <vector>

#include <lcommon/mtwist.h>
#include "GeneratedLevel.h"
#include "dungeon_data.h"

class GameState;

const ItemGenChance& generate_item_choice(MTwist& mt,
		itemgenlist_id itemgenlist);

void generate_items(const ItemGenSettings& is, MTwist& mt,
		GeneratedLevel& level, GameState* gs);

#endif /* ITEMGEN_H_ */
