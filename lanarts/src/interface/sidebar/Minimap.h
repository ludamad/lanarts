/*
 * Minimap.h:
 *  Handles drawing & state of a minimap
 */

#ifndef MINIMAP_H_
#define MINIMAP_H_

#include <ldraw/Image.h>
#include "lanarts_defines.h"

class GameState;

class Minimap {
public:
	Minimap(const BBox& minimap_max_bounds);

	void draw(GameState* gs);
	BBox minimap_bounds(GameState* gs);

private:
	char* minimap_arr;
	int scale;
	ldraw::Image minimap_buff;
	BBox minimap_max_bounds;
};

#endif /* MINIMAP_H_ */
