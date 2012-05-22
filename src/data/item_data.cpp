#include "item_data.h"
#include "sprite_data.h"
#include "../world/objects/GameInst.h"
#include "../world/objects/PlayerInst.h"
#include <cstring>

int get_item_by_name(const char* name){
	for (int i = 0; i < game_item_data.size(); i++){
		if (strcmp(name, game_item_data[i].name) == 0){
			return i;
		}
	}

	LANARTS_ASSERT(false);
	return 0;
}
