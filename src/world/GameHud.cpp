/*
 * GameHud.cpp
 *
 *  Created on: 2011-11-06
 *      Author: 100400530
 */

#include "GameHud.h"
#include "../util/draw_util.h"
#include "GameState.h"
#include "objects/GameInst.h"
#include "GameInstSet.h"
#include "objects/PlayerInst.h"
#include "../data/tile_data.h"
#include "../data/item_data.h"
#include "../data/sprite_data.h"
#include "../display/display.h"
#include "../gamestats/Inventory.h"

static void draw_player_stats(GameState*gs, PlayerInst* player, int x, int y) {
	Stats& s = player->stats();
	gl_draw_statbar(x, y, 100, 10, s.hp, s.max_hp);
	gl_printf(gs->primary_font(), Colour(0,0,0),
			x+30,y, "%d/%d", s.hp, s.max_hp);
//	gl_draw_rectangle(x, y, 100, 10, Colour(255, 0, 0));
//	gl_draw_rectangle(x, y, 100 * s.hp / s.max_hp, 10, Colour(0, 255, 0));
	gl_draw_statbar(x, y + 15,100, 10, s.mp, s.max_mp,
			Colour(0,0,255), Colour(200,200,200));

	gl_printf(gs->primary_font(), Colour(0,0,0),
			x+30,y+15, "%d/%d", s.mp, s.max_mp);
	gl_draw_statbar(x, y + 30,100, 10, s.xp, s.xpneeded,
			 Colour(255, 215, 11), Colour(169,143,100));

	gl_printf(gs->primary_font(), Colour(0,0,0),
			x+30,y+30, "%d/%d", s.xp, s.xpneeded);
}

static void draw_player_inventory(GameState* gs, PlayerInst* player, int x, int y, int w, int h){
    for(int iy = 0; (iy*TILE_SIZE+TILE_SIZE) < (h-y); iy++){
        for(int ix = 0; (ix*TILE_SIZE+TILE_SIZE) < (w-x+1); ix++){
	    int slot = 5*iy+ix;
            if(slot >= INVENTORY_SIZE) return;
            gl_draw_rectangle((ix*TILE_SIZE)+x, (iy*TILE_SIZE)+y, TILE_SIZE, TILE_SIZE, Colour(43, 43, 43));
            gl_draw_rectangle((ix*TILE_SIZE)+1+x, (iy*TILE_SIZE)+1+y, TILE_SIZE-2, TILE_SIZE-2, Colour(0, 0, 0));
	    if(player->inventory.inv[slot].n > 0){
	      ItemType& itemd = game_item_data[player->inventory.inv[slot].item];
	      image_display(&game_sprite_data[itemd.sprite_number].img,(ix*TILE_SIZE)+x+1,(iy*TILE_SIZE)+y);
	      gl_printf(gs->primary_font(), Colour(255,255,255), x+ix*TILE_SIZE, y+iy*TILE_SIZE, "%d", player->inventory.inv[slot].n);
	    }
		}
    }
}

static void draw_player_actionbar(GameState* gs, PlayerInst* player){
	int w = gs->window_view().width;
	int h = gs->window_view().height;
	
	gl_set_drawing_area(0,0,w,h);
	
	int y = h - TILE_SIZE;
	
	gl_draw_rectangle(0, y, TILE_SIZE*5, TILE_SIZE);
	Colour outline(43,43,43);
	for(int ix = 0; (ix*TILE_SIZE+TILE_SIZE) <= TILE_SIZE*5; ix++){
		if(ix == 0){
			outline = Colour(255,0,0);
		}else {
			outline = Colour(43,43,43);
		}
		gl_draw_rectangle((ix*TILE_SIZE), y, TILE_SIZE, TILE_SIZE, outline);
		gl_draw_rectangle((ix*TILE_SIZE)+1, 1+y, TILE_SIZE-2, TILE_SIZE-2);
// 		if(player->inventory.inv[slot].n > 0){
// 			ItemType& itemd = game_item_data[player->inventory.inv[slot].item];
// 			image_display(&game_sprite_data[itemd.sprite_number].img,(ix*TILE_SIZE)+x+1,(iy*TILE_SIZE)+y);
// 			gl_printf(gs->primary_font(), Colour(255,255,255), x+ix*TILE_SIZE, y+iy*TILE_SIZE, "%d", player->inventory.inv[slot].n);
// 		}
	}
	image_display(&game_sprite_data[SPR_FIREBOLT].img,1, y);
	image_display(&game_sprite_data[SPR_MAGIC_BLAST].img,TILE_SIZE+1, y);
}

static void fill_buff2d(char* buff, int w, int h, int x, int y,
		const Colour& col, int rw = 2, int rh = 2) {
	for (int yy = y; yy < y + rh; yy++)
		for (int xx = x; xx < x + rw; xx++) {
			if (yy > 0 && yy < h && xx > 0 && xx < h) {
				int loc = yy * w + xx;
				buff[loc * 4] = col.b;
				buff[loc * 4 + 1] = col.g;
				buff[loc * 4 + 2] = col.r;
				buff[loc * 4 + 3] = col.a;
			}
		}
}

static void draw_rect2d(char* buff, int w, int h, int x, int y, int mx, int my,
		const Colour& col) {
	for (int yy = y; yy < my; yy++)
		for (int xx = x; xx < mx; xx++) {
			if (xx == x || yy == y || xx == mx - 1 || yy == my - 1) {
				int loc = yy * w + xx;
				buff[loc * 4] = col.b;
				buff[loc * 4 + 1] = col.g;
				buff[loc * 4 + 2] = col.r;
				buff[loc * 4 + 3] = col.a;
			}
		}
}

void GameHud::draw_minimap(GameState* gs, int subx, int suby) {
	//Draw a mini version of the contents of gs->tile_grid()
	GameTiles& tiles = gs->tile_grid();
	GameView& view = gs->window_view();

	bool pressed_z = gs->key_down_state(SDLK_z);
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	int tilew = tiles.tile_width(), tileh = tiles.tile_height();
	int ptw = power_of_two(tilew), pth = power_of_two(tileh);
	if (!minimap_arr) {
		minimap_arr = new char[ptw * pth * 4];
	}
	for (int i = 0; i < ptw * pth; i++) {
		minimap_arr[i * 4] = 0;
		minimap_arr[i * 4 + 1] = 0;
		minimap_arr[i * 4 + 2] = 0;
		minimap_arr[i * 4 + 3] = 255;
	}

	for (int y = 0; y < tileh; y++) {
		char* iter = minimap_arr + y * ptw * 4;
		for (int x = 0; x < tilew; x++) {
			int tile = tiles.get(x, y);
			int seen = tiles.seen(x, y) || pressed_z;
			if (seen) {
				if (tile > TILE_STONE_WALL) {/*floor*/
					iter[0] = 255, iter[1] = 255, iter[2] = 255, iter[3] = 255;
				} else { //if (tile == 1){/*wall*/
					iter[0] = 100, iter[1] = 100, iter[2] = 100, iter[3] = 255;
				}
			}
			iter += 4;
		}
	}
	const std::vector<int>& enemy_ids = gs->monster_controller().monster_ids();
	for (int i = 0; i < enemy_ids.size(); i++){
		GameInst* enemy = gs->get_instance(enemy_ids[i]);
		if (enemy){
			int ex = enemy->x/TILE_SIZE;
			int ey = enemy->y/TILE_SIZE;
			int loc = ey* ptw + ex;
			if (!pressed_z && !tiles.seen(ex,ey)) continue;
			minimap_arr[loc * 4] = 0;
			minimap_arr[loc * 4 + 1] = 0;
			minimap_arr[loc * 4 + 2] = 255;
			minimap_arr[loc * 4 + 3] = 255;
		}
	}
	GameInst* inst = gs->get_instance(gs->local_playerid());
	int arr_x = (inst->x / TILE_SIZE), arr_y = (inst->y / TILE_SIZE);
	fill_buff2d(minimap_arr, tilew, tileh, arr_x - arr_x % 2, arr_y - arr_y % 2,
			Colour(255, 180, 99), 2, 2);
	draw_rect2d(minimap_arr, tilew, tileh, min_tilex, min_tiley, max_tilex,
			max_tiley, Colour(255, 180, 99));//
	gl_image_from_bytes(&minimap_buff, ptw, pth, minimap_arr);
	image_display(&minimap_buff, subx, suby);
	//gl_printf(gs->primary_font(),Colour(255,255,255),0,100,"Whatup");
}
void GameHud::draw(GameState* gs) {
	gl_set_drawing_area(x, y, _width, _height);

	PlayerInst* player_inst = (PlayerInst*) gs->get_instance(gs->local_playerid());
	gl_draw_rectangle(0, 0, _width, _height, bg_colour);

	if (player_inst){
		draw_player_stats(gs, player_inst, 32, 32);
        draw_player_inventory(gs, player_inst, 0, INVENTORY_POSITION, _width, _height);
    }
	else {
		return;
		//player = state->get_instance(0);
	}
	draw_minimap(gs, 20, 64+45);
	gl_printf(gs->primary_font(), Colour(255, 215, 11),_width/2-15,10,"Level %d", player_inst->stats().xplevel);
	gl_printf(gs->primary_font(), Colour(255, 215, 11),_width/2-15,64+45+128,"Floor %d", gs->branch_level());
	gl_printf(gs->primary_font(), Colour(255, 215, 11),_width/2-15,64+45+128+15,"Gold %d", player_inst->gold());
	gl_printf(gs->primary_font(), Colour(255, 215, 11),_width/2-50,64+45+128+30,"Melee Damage %d", player_inst->effective_stats().melee.damage);
	gl_printf(gs->primary_font(), Colour(255, 215, 11),_width/2-50,64+45+128+45,"Range Damage %d", player_inst->effective_stats().ranged.damage);
	effect* efx = player_inst->status_effects().get(EFFECT_HASTE);
	if (efx)
		gl_printf(gs->primary_font(), Colour(255, 215, 11),_width/2-50,64+45+128+60,"HASTE %d", efx->t_remaining);
	
	draw_player_actionbar(gs, player_inst);
}

GameHud::GameHud(int x, int y, int width, int height) :
		x(x), y(y), _width(width), _height(height), bg_colour(0, 0, 0), minimap_arr(
				NULL) {
}
GameHud::~GameHud() {
	delete[] minimap_arr;
}
