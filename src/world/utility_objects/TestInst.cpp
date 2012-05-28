/*
 * TestInst.cpp
 *
 *  Created on: Feb 6, 2012
 *      Author: 100397561
 */

#include "TestInst.h"
#include "../../data/tile_data.h"
#include "../GameState.h"
#include <typeinfo>

TestInst::TestInst(int x, int y) :
		GameInst(x, y, RADIUS) {
}

void TestInst::init(GameState* gs) {

}
void TestInst::step(GameState* gs) {

	GameView& view = gs->window_view();
	GameInst* player = gs->get_instance(gs->local_playerid());
	int mx = gs->mouse_x() + view.x;
	int my = gs->mouse_y() + view.y;
//	p = path.calculate_AStar_path(gs, player->x / TILE_SIZE,
//			player->y / TILE_SIZE, mx / TILE_SIZE, my / TILE_SIZE);
}

void TestInst::draw(GameState* gs) {
	gl_draw_rectangle(0, 0, 400, 100, Colour(255, 255, 255, 125));
}

void TestInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(TestInst*) inst = *this;
}

TestInst *TestInst::clone() const {
	return new TestInst(*this);
}

