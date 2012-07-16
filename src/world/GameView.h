/*
 * GameView.h:
 *	Represents a section of the game world, bounded by the dimensions of the world, and roughly centred on an object
 */

#ifndef GAMEVIEW_H_
#define GAMEVIEW_H_

#include "../util/game_basic_structs.h"

struct GameView {
	int x, y, width, height, world_width, world_height;
	GameView(int x, int y, int width, int height, int world_width = 0, int world_height = 0) :
		x(x), y(y), width(width), height(height), world_width(world_width), world_height(world_height){ }


	bool within_view(int xx, int yy, int w, int h){
		return xx + w >= x  && yy + h >= y && xx <= x + width && yy <= y + height;
	}
	bool within_view(const BBox& bbox){
		return within_view(bbox.x1, bbox.y1, bbox.width(), bbox.height());
	}
	void move_towards(int px, int py);

	void center_on(int px, int py){ move_towards(px - width/2, py - height/2); }
	void sharp_center_on(int px, int py);
	void move_delta(int dx, int dy) { move_towards(x+dx, y+dy); }
	void min_tile_within(int& px, int& py) const;
	void max_tile_within(int& px, int& py) const;

	/*Are we outside of the centre of the view enough to warrant snapping the view ?*/
	bool out_of_view_center(int px, int py);

	void move_towards(const Pos& p){
		move_towards(p.x,p.y);
	}
	BBox tiles_covered(){
		BBox ret;
		min_tile_within(ret.x1,ret.y1);
		max_tile_within(ret.x2,ret.y2);
		return ret;
	}
};



#endif /* GAMEVIEW_H_ */
