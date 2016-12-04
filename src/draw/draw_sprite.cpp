/*
 * draw_sprite.cpp:
 *  Core image drawing and display functions
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include <ldraw/DrawOptions.h>

#include "gamestate/GameView.h"

#include <lcommon/math_util.h>

#include "draw_sprite.h"

#include "SpriteEntry.h"

void draw_sprite(const GameView& view, sprite_id sprite, int x, int y,
		float dx, float dy, float frame, const Colour& c) {
	using namespace ldraw;

	float PI = 3.1415921f;
	float direction = PI * 2.5f + atan2f(dy, dx);

	SpriteEntry& entry = game_sprite_data.at(sprite);
	entry.sprite.draw(DrawOptions().colour(c).angle(direction).frame(frame), Pos(x-view.x,y-view.y));
}

void draw_sprite(sprite_id sprite, int x, int y, const Colour& c) {
	using namespace ldraw;

	SpriteEntry& entry = game_sprite_data.at(sprite);
	entry.sprite.draw(DrawOptions().colour(c.multiply(entry.drawcolour)), Pos(x,y));
}

void draw_sprite(const GameView& view, sprite_id sprite, int x, int y,
		const Colour& c) {
	draw_sprite(sprite, x - view.x, y - view.y, c);
}
