/*
 * GameHud.cpp
 *
 *  Created on: 2011-11-06
 *      Author: 100400530
 */

#include "../../gamestats/Inventory.h"

#include "../../util/math_util.h"

#include "../../display/display.h"

#include "../../data/tile_data.h"
#include "../../data/item_data.h"
#include "../../data/sprite_data.h"
#include "../../data/weapon_data.h"

#include "../GameState.h"
#include "../GameInstSet.h"

#include "../objects/GameInst.h"
#include "../objects/PlayerInst.h"

#include "GameHud.h"

const int INVENTORY_POSITION = 327;

static void draw_player_stats(GameState*gs, PlayerInst* player, int x, int y) {
	Stats& s = player->stats();
	gl_draw_statbar(x, y, 100, 10, s.hp, s.max_hp);
	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y, "%d/%d", s.hp,
			s.max_hp);
//	gl_draw_rectangle(x, y, 100, 10, Colour(255, 0, 0));
//	gl_draw_rectangle(x, y, 100 * s.hp / s.max_hp, 10, Colour(0, 255, 0));
	gl_draw_statbar(x, y + 15, 100, 10, s.mp, s.max_mp, Colour(0, 0, 255),
			Colour(200, 200, 200));

	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y + 15, "%d/%d",
			s.mp, s.max_mp);
	gl_draw_statbar(x, y + 30, 100, 10, s.xp, s.xpneeded, Colour(255, 215, 11),
			Colour(169, 143, 100));

	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y + 30, "%d/%d",
			s.xp, s.xpneeded);

	float ratio = player->rest_cooldown() / float(REST_COOLDOWN);
	Colour col(200 * ratio, 200 * (1.0f - ratio), 0);
	gl_draw_rectangle(x, y + 45, 100, 10, col);

	if (player->rest_cooldown() == 0)
		gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 25, y + 44,
				"can rest", player->rest_cooldown() * 100 / REST_COOLDOWN);
}

static void draw_player_inventory(GameState* gs, PlayerInst* player, int inv_x,
		int inv_y, int w, int h) {
	Inventory& inv = player->get_inventory();

	int chat_w = (w - inv_x + 1) - TILE_SIZE, chat_h = (h - inv_y) - TILE_SIZE;

	int slot = 0;
	for (int y = 0; y < chat_h; y += TILE_SIZE) {
		for (int x = 0; x < chat_w; x += TILE_SIZE) {
			if (slot >= INVENTORY_SIZE)
				return;

			ItemSlot& itemslot = inv.get(slot);
			Colour outline(43, 43, 43);
			Pos p(inv_x + x, inv_y + y);

			if (itemslot.amount > 0)
				outline = Colour(120, 115, 110);

			gl_draw_rectangle_outline(x + inv_x, y + inv_y, TILE_SIZE,
					TILE_SIZE, outline);

			if (itemslot.amount > 0) {
				ItemEntry& itemd = game_item_data[itemslot.item];
				GLimage& itemimg = game_sprite_data[itemd.sprite_number].img();
				gl_draw_image(itemimg, p.x, p.y);
				gl_printf(gs->primary_font(), Colour(255, 255, 255), p.x + 1,
						p.y + 1, "%d", itemslot.amount);
			}

			slot++;
		}
	}
}

static Colour outline_col(bool cond) {
	return cond ? Colour(50, 205, 50) : Colour(43, 43, 43);
}

static void draw_player_weapon_actionbar(GameState* gs, PlayerInst* player,
		int x, int y) {
	Colour outline = outline_col(player->spell_selected() == -1);
	Equipment& equipment = player->get_equipment();

	if (equipment.projectile == -1) {
		gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE, outline);
	} else {
		gl_draw_rectangle_outline(x, y, TILE_SIZE * 2, TILE_SIZE, outline);
		ProjectileEntry& ptype =
				game_projectile_data[player->get_equipment().projectile];
		gl_draw_image(game_sprite_data[ptype.item_sprite].img(), x + TILE_SIZE,
				y);
		gl_printf(gs->primary_font(), Colour(255, 255, 255), x + TILE_SIZE + 1,
				y + 1, "%d", player->get_equipment().projectile_amnt);
	}

	WeaponEntry& wtype = game_weapon_data[player->weapon_type()];
	gl_draw_image(game_sprite_data[wtype.item_sprite].img(), x, y);
}
static void draw_player_actionbar(GameState* gs, PlayerInst* player) {
	int w = gs->window_view().width;
	int h = gs->window_view().height;

	gl_set_drawing_area(0, 0, w, h);

	int sx = TILE_SIZE * 2;
	int sy = h - TILE_SIZE;

	draw_player_weapon_actionbar(gs, player, 0, sy);

	int spellidx = 0;
	for (int x = sx; x < w; x += TILE_SIZE) {
		bool is_selected = spellidx++ == player->spell_selected();
		Colour outline = outline_col(is_selected);
		if (!is_selected && spellidx < 3)
			outline = Colour(120, 115, 110);
		gl_draw_rectangle_outline(x, sy, TILE_SIZE, TILE_SIZE, outline);
	}
	//TODO: Unhardcode this already !!
	gl_draw_image(game_sprite_data[get_sprite_by_name("fire bolt")].img(), sx,
			sy);
	gl_draw_image(game_sprite_data[get_sprite_by_name("magic blast")].img(),
			sx + TILE_SIZE, sy);
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

BBox GameHud::minimap_bbox() {
	return BBox(x + 20, y + 64 + 45, x + 20 + 128, y + 64 + 45 + 128);
}

void GameHud::step(GameState *gs) {
}

void GameHud::queue_io_actions(GameState* gs, PlayerInst* player,
		std::deque<GameAction>& queued_actions) {
	bool mouse_within_view = gs->mouse_x() < gs->window_view().width;
	int level = gs->level()->roomid, frame = gs->frame();

	Inventory inv = player->get_inventory();

	if (gs->mouse_left_click() && mouse_within_view) {
	}

	if (gs->mouse_left_click() && !mouse_within_view) {
		int posx = (gs->mouse_x() - gs->window_view().width) / TILE_SIZE;
		int posy = (gs->mouse_y() - INVENTORY_POSITION) / TILE_SIZE;
		int slot = 5 * posy + posx;
		if (slot >= 0 && slot < INVENTORY_SIZE && inv.get(slot).amount > 0) {
			queued_actions.push_back(
					GameAction(player->id, GameAction::USE_ITEM, frame, level,
							slot, player->x, player->y));
		}
	}

	// Drop item
	const int ITEM_DROP_RATE = 2; //steps
	bool within_rate = gs->mouse_right_click()
			|| (gs->mouse_right_down() && frame % ITEM_DROP_RATE == 0);

	if (within_rate && frame && !mouse_within_view) {
		int posx = (gs->mouse_x() - gs->window_view().width) / TILE_SIZE;
		int posy = (gs->mouse_y() - INVENTORY_POSITION) / TILE_SIZE;
		int slot = 5 * posy + posx;
		if (slot >= 0 && slot < INVENTORY_SIZE && inv.get(slot).amount > 0) {
			queued_actions.push_back(
					GameAction(player->id, GameAction::DROP_ITEM, frame, level,
							slot));
		}
	}
}

void GameHud::draw_minimap(GameState* gs, const BBox& bbox) {
	//Draw a mini version of the contents of gs->tile_grid()
	GameTiles& tiles = gs->tile_grid();
	GameView& view = gs->window_view();

	bool minimap_reveal = gs->key_down_state(SDLK_z)
			|| gs->key_down_state(SDLK_BACKQUOTE);
	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	int minimap_x = bbox.x1, minimap_y = bbox.y1;
	int minimap_w = 128 /*bbox.width()*/, minimap_h = 128 /*bbox.height()*/;
	int ptw = power_of_two(minimap_w), pth = power_of_two(minimap_h);
	if (!minimap_arr) {
		minimap_arr = new char[ptw * pth * 4];
	}
	for (int i = 0; i < ptw * pth; i++) {
		minimap_arr[i * 4] = 0;
		minimap_arr[i * 4 + 1] = 0;
		minimap_arr[i * 4 + 2] = 0;
		minimap_arr[i * 4 + 3] = 255;
	}

	int stairs_down = get_tile_by_name("stairs_down");
	int stairs_up = get_tile_by_name("stairs_up");
	for (int y = 0; y < minimap_h; y++) {
		char* iter = minimap_arr + y * ptw * 4;
		for (int x = 0; x < minimap_w; x++) {
			int tile = tiles.get(x, y).tile;
			int seen = tiles.is_seen(x, y) || minimap_reveal;
			if (seen) {
				if (tile == stairs_down || tile == stairs_up) {
					iter[0] = 255, iter[1] = 0, iter[2] = 0, iter[3] = 255;
				} else if (!tiles.is_solid(x, y)) {/*floor*/
					iter[0] = 255, iter[1] = 255, iter[2] = 255, iter[3] = 255;
				} else { //if (tile == 1){/*wall*/
					iter[0] = 100, iter[1] = 100, iter[2] = 100, iter[3] = 255;
				}
			}
			iter += 4;
		}
	}
	const std::vector<int>& enemy_ids = gs->monster_controller().monster_ids();
	for (int i = 0; i < enemy_ids.size(); i++) {
		GameInst* enemy = gs->get_instance(enemy_ids[i]);
		if (enemy) {
			int ex = enemy->x / TILE_SIZE;
			int ey = enemy->y / TILE_SIZE;
			int loc = ey * ptw + ex;
			if (!minimap_reveal && !tiles.is_seen(ex, ey))
				continue;
			if (!minimap_reveal && !gs->object_visible_test(enemy))
				continue;

			minimap_arr[loc * 4] = 0;
			minimap_arr[loc * 4 + 1] = 0;
			minimap_arr[loc * 4 + 2] = 255;
			minimap_arr[loc * 4 + 3] = 255;
		}
	}
	GameInst* inst = gs->get_instance(gs->local_playerid());
	if (inst) {
		int arr_x = (inst->x / TILE_SIZE), arr_y = (inst->y / TILE_SIZE);
		fill_buff2d(minimap_arr, minimap_w, minimap_h, arr_x - arr_x % 2,
				arr_y - arr_y % 2, Colour(255, 180, 99), 2, 2);
		draw_rect2d(minimap_arr, minimap_w, minimap_h, min_tilex, min_tiley,
				max_tilex, max_tiley, Colour(255, 180, 99)); //
	}

	gl_image_from_bytes(minimap_buff, ptw, pth, minimap_arr);

	gl_draw_image(minimap_buff, minimap_x, minimap_y);
}
void GameHud::draw(GameState* gs) {
	gl_set_drawing_area(x, y, _width, _height);

	PlayerInst* player_inst = (PlayerInst*) gs->get_instance(
			gs->local_playerid());
	Stats effective_stats = player_inst->effective_stats(gs->get_luastate());

	gl_draw_rectangle(0, 0, _width, _height, bg_colour);

	draw_minimap(gs, minimap_bbox().translated(-x, -y));
	if (player_inst) {
		draw_player_stats(gs, player_inst, 32, 32);
		draw_player_inventory(gs, player_inst, 0, INVENTORY_POSITION, _width,
				_height);
	} else {
		return;
		//player = state->get_instance(0);
	}
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 15, 10,
			"Level %d", effective_stats.xplevel);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 40,
			64 + 45 + 128, "Floor %d", gs->level()->level_number);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 40,
			64 + 45 + 128 + 15, "Gold %d", player_inst->gold());
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 50,
			64 + 45 + 128 + 30, "Strength   %d", effective_stats.strength);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 50,
			64 + 45 + 128 + 45, "Magic      %d", effective_stats.magic);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 50,
			64 + 45 + 128 + 60, "Defence  %d", effective_stats.defence);
	draw_player_actionbar(gs, player_inst);
}

GameHud::GameHud(int x, int y, int width, int height) :
		x(x), y(y), _width(width), _height(height), bg_colour(0, 0, 0), minimap_arr(
				NULL) {
}
GameHud::~GameHud() {
	delete[] minimap_arr;
}
