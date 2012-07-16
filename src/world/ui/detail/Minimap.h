/*
 * Minimap.h:
 *  Handles drawing & state of a minimap
 */

#ifndef MINIMAP_H_
#define MINIMAP_H_

#include "../../../util/game_basic_structs.h"
#include "../../../display/GLImage.h"

class GameState;

class Minimap {
public:
	Minimap(const BBox& minimap_max_bounds);

	void draw(GameState* gs, float scale);
	BBox minimap_bounds(GameState* gs);

private:
	char* minimap_arr;
	GLimage minimap_buff;
	BBox minimap_max_bounds;
};

#endif /* MINIMAP_H_ */
