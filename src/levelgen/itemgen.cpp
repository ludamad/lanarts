/*
 * itemgen.cpp:
 *  Defines parameters for item generation as well as the generate_items function
 */

#include "../gamestate/GameState.h"
#include "../gamestate/GameTiles.h"

#include "../objects/ItemInst.h"

#include "../stats/items/ItemEntry.h"
#include "../stats/itemgen_data.h"

#include "itemgen.h"

const ItemGenChance& generate_item_choice(MTwist& mt,
		itemgenlist_id itemgenlist) {
	ItemGenList& entry = game_itemgenlist_data.at(itemgenlist);
	std::vector<ItemGenChance>& items = entry.items;
	int total_chance = 0;

	for (int i = 0; i < items.size(); i++) {
		total_chance += items[i].genchance;
	}

	int item_roll = mt.rand(total_chance);
	int itemn;
	for (itemn = 0; itemn < items.size(); itemn++) {
		item_roll -= items[itemn].genchance;
		if (item_roll < 0) {
			break;
		}
	}
	return items[itemn];
}

void generate_items(const ItemGenSettings& is, MTwist& mt,
		GeneratedLevel& level, GameState* gs) {
	GameTiles& tiles = gs->tiles();
	int start_x = (tiles.tile_width() - level.width()) / 2;
	int start_y = (tiles.tile_height() - level.height()) / 2;

	int amount = mt.rand(is.num_items);

	if (is.itemgenlist <= -1)
		return;

	//generate gold
	for (int i = 0; i < amount; i++) {
		Pos ipos = generate_location(mt, level);
		int ix = (ipos.x + start_x) * TILE_SIZE + TILE_SIZE / 2;
		int iy = (ipos.y + start_y) * TILE_SIZE + TILE_SIZE / 2;

		const ItemGenChance& igc = generate_item_choice(mt, is.itemgenlist);
		int quantity = mt.rand(igc.quantity);
		gs->add_instance(new ItemInst(Item(igc.itemtype, quantity), ix, iy));
		level.at(ipos).has_instance = true;
	}
}
