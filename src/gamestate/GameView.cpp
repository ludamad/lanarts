/*
 * GameView.cpp:
 *	Represents a section of the game world, bounded by the dimensions of the world, and roughly centred on an object
 */

#include "GameTiles.h"
#include <cmath>
#include <algorithm>
#include "GameView.h"

static const int VIEW_SUBW = 100, VIEW_SUBH = 100;
static const int VIEW_SPEED = 8;


bool GameView::out_of_view_center(int px, int py) {
	int dx = px - x, dy = py - y;
	return (abs(dx) > width / 2 || abs(dy) > height / 2);
}
void GameView::move_towards(int px, int py) {
	int dx = px - x, dy = py - y;
	if (abs(dx) > VIEW_SUBW / 2) {
		if (px > x) {
			x = std::min(px - VIEW_SUBW / 2, x + VIEW_SPEED);
		} else {
			x = std::max(px + VIEW_SUBW / 2, x - VIEW_SPEED);
		}
//		x = std::max(0, std::min(world_width - width, x));
	}
	if (abs(dy) > VIEW_SUBH / 2) {
		if (py > y) {
			y = std::min(py - VIEW_SUBH / 2, y + VIEW_SPEED);
		} else {
			y = std::max(py + VIEW_SUBH / 2, y - VIEW_SPEED);
		}
//		y = std::max(0, std::min(world_height - height, y));
	}
}

BBox GameView::region_covered() const {
	return BBox(Pos(x,y), size());
}

BBox GameView::tile_region_covered() const {
	int min_x = std::max(0, x / TILE_SIZE);
	int min_y = std::max(0, y / TILE_SIZE);
	int max_x = (std::min(world_width, x + width)) / TILE_SIZE;
	int max_y = (std::min(world_height, y + height)) / TILE_SIZE;

	return BBox(min_x, min_y, max_x, max_y);
}

void GameView::sharp_center_on(int px, int py) {
	//if (px < width / 2) {
	//	px = width / 2;
	//} else if (px > world_width - width / 2) {
	//	px = world_width - width / 2;
	//}
	//if (py < height / 2) {
	//	py = height / 2;
	//} else if (py > world_height - height / 2) {
	//	py = world_height - height / 2;
	//}

	x = px - width / 2;
	y = py - height / 2;
}

// Helper functions

#include "GameState.h"

Pos on_screen(GameState* gs, const Pos& p) {
	GameView& view = gs->view();
	return Pos(p.x - view.x, p.y - view.y);
}
BBox on_screen(GameState* gs, const BBox& b) {
	GameView& view = gs->view();
	return b.translated(-view.x, -view.y);

}
PosF on_screen(GameState* gs, const PosF& p) {
	GameView& view = gs->view();
	return PosF(p.x - view.x, p.y - view.y);

}
BBoxF on_screen(GameState* gs, const BBoxF& b) {
	GameView& view = gs->view();
	return b.translated(-view.x, -view.y);
}
