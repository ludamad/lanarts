/*
 * GameView.cpp:
 *	Represents a section of the game world, bounded by the dimensions of the world, and roughly centred on an object
 */

#include "GameView.h"
#include <cmath>
#include <algorithm>
#include "GameTiles.h"

static const int VIEW_SUBW = 100, VIEW_SUBH = 100;
static const int VIEW_SPEED = 8;

bool GameView::out_of_view_center(int px, int py) {
	int dx = px - x, dy = py - y;
	return (abs(dx) > width / 2 || abs(dy) > height / 2);
}
void GameView::move_towards(int px, int py) {
	int dx = px - x, dy = py - y;
	if (abs(dx) > VIEW_SUBW / 2) {
		if (px > x)
			x = std::min(px - VIEW_SUBW / 2, x + VIEW_SPEED);
		else
			x = std::max(px + VIEW_SUBW / 2, x - VIEW_SPEED);
		x = std::max(0, std::min(world_width - width, x));
	}
	if (abs(dy) > VIEW_SUBH / 2) {
		if (py > y)
			y = std::min(py - VIEW_SUBH / 2, y + VIEW_SPEED);
		else
			y = std::max(py + VIEW_SUBH / 2, y - VIEW_SPEED);
		y = std::max(0, std::min(world_height - height, y));
	}
}

void GameView::min_tile_within(int& px, int& py) const {
	px = std::max(0, x / TILE_SIZE);
	py = std::max(0, y / TILE_SIZE);
}

void GameView::max_tile_within(int& px, int& py) const {
	px = (std::min(world_width, x + width)) / TILE_SIZE;
	py = (std::min(world_height, y + height)) / TILE_SIZE;
}

void GameView::sharp_center_on(int px, int py) {
	if (px < width / 2)
		px = width / 2;
	else if (px > world_width - width / 2)
		px = world_width - width / 2;
	if (py < height / 2)
		py = height / 2;
	else if (py > world_height - height / 2)
		py = world_height - height / 2;

	x = px - width / 2;
	y = py - height / 2;
}
