/*
 * GameView.h:
 *	Represents a section of the game world, bounded by the dimensions of the world, and roughly centered on an object
 */

#ifndef GAMEVIEW_H_
#define GAMEVIEW_H_

#include <lcommon/geometry.h>

struct GameView {
        enum {
            VIEW_SPEED = 10
        };
	int x, y, width, height, world_width, world_height;

	GameView(int x = 0, int y = 0, int width = 0, int height = 0, int world_width = 0,
			int world_height = 0) :
			x(x), y(y), width(width), height(height), world_width(world_width), world_height(
					world_height) {
	}

	bool within_view(int xx, int yy, int w, int h) {
		return xx + w >= x && yy + h >= y && xx <= x + width && yy <= y + height;
	}
	bool within_view(const BBox& bbox) {
		return within_view(bbox.x1, bbox.y1, bbox.width(), bbox.height());
	}
	void move_towards(int px, int py, int view_speed = VIEW_SPEED);
	void move_towards(const Pos& p, int view_speed = VIEW_SPEED) {
		move_towards(p.x, p.y, view_speed);
	}

	void center_on(int px, int py, int view_speed = VIEW_SPEED) {
		move_towards(px - width / 2, py - height / 2, view_speed);
	}
	void center_on(const Pos& p, int view_speed = VIEW_SPEED) {
		center_on(p.x, p.y);
	}

	void sharp_center_on(int px, int py);
	void sharp_center_on(const Pos& p) {
		sharp_center_on(p.x, p.y);
	}

	void sharp_move(int dx, int dy);
	void sharp_move(const Pos& p) {
		sharp_move(p.x, p.y);
	}
	void move_delta(int dx, int dy) {
		move_towards(x + dx, y + dy);
	}

	BBox region_covered() const;
	BBox tile_region_covered() const;

	/*Are we outside of the centre of the view enough to warrant snapping the view ?*/
	bool out_of_view_center(int px, int py);

	Size size() const {
		return Size(width, height);
	}
};

class GameState;

// Helper functions

Pos on_screen(GameState* gs, const Pos& p);
BBox on_screen(GameState* gs, const BBox& p);
PosF on_screen(GameState* gs, const PosF& p);
BBoxF on_screen(GameState* gs, const BBoxF& p);

#endif /* GAMEVIEW_H_ */
