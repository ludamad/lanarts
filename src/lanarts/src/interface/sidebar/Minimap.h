/*
 * Minimap.h:
 *  Handles drawing & state of a minimap
 */

#ifndef MINIMAP_H_
#define MINIMAP_H_

#include "../../display/GLImage.h"
#include "../../lanarts_defines.h"

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
