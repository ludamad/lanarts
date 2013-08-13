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
	Minimap(const Pos& minimap_center);

	void draw(GameState* gs);
	BBox minimap_bounds(GameState* gs, Size size);

private:
	char* minimap_arr;
	int scale;
	ldraw::Image minimap_buff;
	Pos minimap_center;
};

#endif /* MINIMAP_H_ */
