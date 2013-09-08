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
	BBox minimap_bounds(GameState* gs) const;
	BBox drawn_tile_region(GameState* gs, const GameView& view) const;
	Pos minimap_xy_to_world_xy(GameState* gs, const GameView& view, Pos xy) const;
	BBox image_draw_region(GameState* gs, const GameView& view) const;

private:
	char* minimap_arr;
	int scale;
	ldraw::Image minimap_buff;
	Pos minimap_center;
};

#endif /* MINIMAP_H_ */
