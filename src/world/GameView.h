/*
 * GameView.h
 *
 *  Created on: 2011-11-03
 *      Author: 100397561
 */

#ifndef GAMEVIEW_H_
#define GAMEVIEW_H_

struct GameView {
	int x, y, width, height, world_width, world_height;
	GameView(int x, int y, int width, int height, int world_width, int world_height) :
		x(x), y(y), width(width), height(height), world_width(world_width), world_height(world_height){ }

	bool within_view(int xx, int yy, int w, int h){
		return xx >= x && yy >= y && xx + w <= x + width && yy + h <= y + height;
	}
	void move_towards(int px, int py);
	void center_on(int px, int py){ move_towards(px - width/2, py - height/2); }
	void sharp_center_on(int px, int py);
	void move_delta(int dx, int dy) { move_towards(x+dx, y+dy); }
	void min_tile_within(int& px, int& py) const;
	void max_tile_within(int& px, int& py) const;
};



#endif /* GAMEVIEW_H_ */
