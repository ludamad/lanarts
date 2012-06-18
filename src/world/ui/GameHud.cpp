/*
 * GameHud.cpp
 *  Various components of the overlay display & side bar
 *  Includes inventory manipulation
 */

#include "../../display/display.h"

#include "../../data/tile_data.h"
#include "../../data/item_data.h"
#include "../../data/sprite_data.h"
#include "../../data/weapon_data.h"

#include "../../gamestats/Inventory.h"

#include "../../util/math_util.h"

#include "../GameState.h"
#include "../GameInstSet.h"

#include "../objects/GameInst.h"
#include "../objects/PlayerInst.h"

#include "GameHud.h"

const int INVENTORY_POSITION = 342;

static void draw_player_statbars(GameState*gs, PlayerInst* player, int x,
		int y) {
	ClassStats& class_stats = player->class_stats();
	CoreStats& core = player->effective_stats().core;

	gl_draw_statbar(x, y, 100, 10, core.hp, core.max_hp);
	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y, "%d/%d", core.hp,
			core.max_hp);

	gl_draw_statbar(x, y + 15, 100, 10, core.mp, core.max_mp, Colour(0, 0, 255),
			Colour(200, 200, 200));

	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y + 15, "%d/%d",
			core.mp, core.max_mp);
	gl_draw_statbar(x, y + 30, 100, 10, class_stats.xp, class_stats.xpneeded,
			Colour(255, 215, 11), Colour(169, 143, 100));

	gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 30, y + 30, "%d/%d",
			class_stats.xp, class_stats.xpneeded);

	float ratio = player->rest_cooldown() / float(REST_COOLDOWN);
	Colour col(200 * ratio, 200 * (1.0f - ratio), 0);
	gl_draw_rectangle(x, y + 45, 100, 10, col);

	if (player->rest_cooldown() == 0)
		gl_printf(gs->primary_font(), Colour(0, 0, 0), x + 25, y + 44,
				"can rest", player->rest_cooldown() * 100 / REST_COOLDOWN);
}

static void draw_player_inventory_slot(GameState* gs, _ItemSlot& itemslot,
		int x, int y) {
	if (itemslot.amount > 0) {
		ItemEntry& itemd = itemslot.item.item_entry();
		GLimage& itemimg = game_sprite_data[itemd.sprite_number].img();
		gl_draw_image(itemimg, x, y);
		gl_printf(gs->primary_font(), Colour(255, 255, 255), x + 1, y + 1, "%d",
				itemslot.amount);
	}
}
static void draw_player_inventory(GameState* gs, PlayerInst* player, int inv_x,
		int inv_y, int w, int h, int slot_selected) {
	_Inventory& inv = player->inventory();

	int slot = 0;
	for (int y = 0; y < h; y += TILE_SIZE) {
		for (int x = 0; x < w; x += TILE_SIZE) {
			if (slot >= INVENTORY_SIZE)
				break;

			_ItemSlot& itemslot = inv.get(slot);

			Colour outline(43, 43, 43);
			if (itemslot.amount > 0 && slot != slot_selected)
				outline = Colour(120, 115, 110);

			int slot_x = inv_x + x, slot_y = inv_y + y;
			gl_draw_rectangle_outline(slot_x, slot_y, TILE_SIZE, TILE_SIZE,
					outline);
			if (slot != slot_selected)
				draw_player_inventory_slot(gs, itemslot, slot_x, slot_y);

			slot++;
		}
	}

	if (slot_selected != -1) {
		draw_player_inventory_slot(gs, inv.get(slot_selected),
				gs->mouse_x() - TILE_SIZE / 2, gs->mouse_y() - TILE_SIZE / 2);
	}
}

static Colour outline_col(bool cond) {
	return cond ? Colour(50, 205, 50) : Colour(43, 43, 43);
}

static void draw_player_weapon_actionbar(GameState* gs, PlayerInst* player,
		int x, int y) {
	Colour outline = outline_col(player->spell_selected() == -1);
	_Equipment& equipment = player->equipment();

	if (equipment.projectile == -1) {
		gl_draw_rectangle_outline(x, y, TILE_SIZE, TILE_SIZE, outline);
	} else {
		gl_draw_rectangle_outline(x, y, TILE_SIZE * 2, TILE_SIZE, outline);
		ProjectileEntry& ptype = equipment.projectile.projectile_entry();
		gl_draw_image(game_sprite_data[ptype.item_sprite].img(), x + TILE_SIZE,
				y);
		gl_printf(gs->primary_font(), Colour(255, 255, 255), x + TILE_SIZE + 1,
				y + 1, "%d", player->equipment().projectile_amnt);
	}

	WeaponEntry& wentry = player->weapon_type().weapon_entry();
	gl_draw_image(game_sprite_data[wentry.item_sprite].img(), x, y);
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
	return minimapbox;
}

void GameHud::step(GameState *gs) {
}

static int get_itemslotn(GameState* gs, int x, int y) {
	int posx = (x - gs->window_view().width) / TILE_SIZE;
	int posy = (gs->mouse_y() - INVENTORY_POSITION) / TILE_SIZE;
	int slot = 5 * posy + posx;
	if (slot < 0 || slot >= INVENTORY_SIZE)
		return -1;
	return slot;
}

void GameHud::queue_io_actions(GameState* gs, PlayerInst* player,
		std::deque<GameAction>& queued_actions) {
	bool mouse_within_view = gs->mouse_x() < gs->window_view().width;
	int level = gs->get_level()->roomid, frame = gs->frame();

	_Inventory inv = player->inventory();

	for (int i = 0; i < this->queued_actions.size(); i++) {
		queued_actions.push_back(this->queued_actions[i]);
	}
	this->queued_actions.clear();

	if (gs->mouse_right_click() && mouse_within_view) {
		int action_bar_x = 0, action_bar_y = gs->window_view().height
				- TILE_SIZE;
		int posx = (gs->mouse_x() - action_bar_x) / TILE_SIZE;
		int posy = (gs->mouse_y() - action_bar_y) / TILE_SIZE;

		_Equipment& equipment = player->equipment();
		if (posy == 0 && posx == 0)
			queued_actions.push_back(
					GameAction(player->id, GameAction::DEEQUIP_ITEM, frame,
							level, ItemEntry::WEAPON));
		else if (posy == 0 && posx == 1)
			queued_actions.push_back(
					GameAction(player->id, GameAction::DEEQUIP_ITEM, frame,
							level, ItemEntry::PROJECTILE));
	}

	if (gs->mouse_left_click() && !mouse_within_view) {
		int slot = get_itemslotn(gs, gs->mouse_x(), gs->mouse_y());
		if (slot >= 0 && slot < INVENTORY_SIZE && inv.get(slot).amount > 0) {
			queued_actions.push_back(
					GameAction(player->id, GameAction::USE_ITEM, frame, level,
							slot, player->x, player->y));
		}
	}

//	// Drop item
//	const int ITEM_DROP_RATE = 2; //steps
//	bool within_rate = gs->mouse_right_click()
//			|| (gs->mouse_right_down() && frame % ITEM_DROP_RATE == 0);
//
//	if (within_rate && !mouse_within_view) {
//		int slot = get_itemslotn(gs, gs->mouse_x(), gs->mouse_y());
//
//		if (slot != -1 && inv.get(slot).amount > 0) {
//			queued_actions.push_back(
//					GameAction(player->id, GameAction::DROP_ITEM, frame, level,
//							slot));
//		}
//	}
}

const int SPELL_MAX = 2;
bool GameHud::handle_event(GameState* gs, SDL_Event* event) {
	int level = gs->get_level()->roomid, frame = gs->frame();

	bool mouse_within_view = gs->mouse_x() < gs->window_view().width;
	PlayerInst* player = (PlayerInst*)gs->get_instance(gs->local_playerid());
	if (!player)
		return false;

	_Inventory inv = player->inventory();

	bool mleft = event->button.button == SDL_BUTTON_LEFT;
	bool mright = event->button.button == SDL_BUTTON_RIGHT;

	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
		if (mleft && mouse_within_view) {
			int action_bar_x = 0, action_bar_y = gs->window_view().height
					- TILE_SIZE;
			int posx = (gs->mouse_x() - action_bar_x) / TILE_SIZE;
			int posy = (gs->mouse_y() - action_bar_y) / TILE_SIZE;

			_Equipment& equipment = player->equipment();
			if (posy == 0) {
				if (posx == 0) {
					player->spell_selected() = -1;
					return true;
				} else if (posx == 1 && equipment.has_projectile()) {
					player->spell_selected() = -1;
					return true;
				} else if (posx > 1 && posx - 2 < SPELL_MAX) {
					player->spell_selected() = posx - 2;
					return true;
				}
			}
		}
		if (mright && !mouse_within_view) {
			int slot = get_itemslotn(gs, gs->mouse_x(), gs->mouse_y());
			if (slot != -1 && inv.get(slot).amount > 0) {
				item_slot_selected = slot;
				return true;
			}
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (item_slot_selected != -1) {
			int slot = get_itemslotn(gs, gs->mouse_x(), gs->mouse_y());
			//TODO: fix selection outside of the inventory
			if (slot == -1 || slot == item_slot_selected) {
				queued_actions.push_back(
						GameAction(player->id, GameAction::DROP_ITEM, frame,
								level, item_slot_selected));
			} else {
				this->queued_actions.push_back(
						GameAction(player->id, GameAction::REPOSITION_ITEM,
								frame, level, item_slot_selected, 0, 0, slot));
			}
			return true;
		}
		break;
	}
	return false;
}

static void world2minimapbuffer(GameState* gs, char* buff, const BBox& world_portion, int w, int h,
		int ptw, int pth) {
	GameTiles& tiles = gs->tile_grid();
	GameView& view = gs->window_view();

	bool minimap_reveal = gs->key_down_state(SDLK_z)
			|| gs->key_down_state(SDLK_BACKQUOTE);


	int stairs_down = get_tile_by_name("stairs_down");
	int stairs_up = get_tile_by_name("stairs_up");
	for (int y = 0; y < h; y++) {
		char* iter = buff + y * ptw * 4;
		for (int x = 0; x < w; x++) {
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
	const std::vector<obj_id>& enemy_ids =
			gs->monster_controller().monster_ids();
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

			buff[loc * 4] = 0;
			buff[loc * 4 + 1] = 0;
			buff[loc * 4 + 2] = 255;
			buff[loc * 4 + 3] = 255;
		}
	}
//
//	int min_tilex, min_tiley;
//	int max_tilex, max_tiley;
//
//	view.min_tile_within(min_tilex, min_tiley);
//	view.max_tile_within(max_tilex, max_tiley);
}

const int MINIMAP_SIZEMAX = 128;

void GameHud::draw_minimap(GameState* gs, const BBox& bbox, float scale) {
//Draw a mini version of the contents of gs->tile_grid()
	GameTiles& tiles = gs->tile_grid();
	GameView& view = gs->window_view();

	int min_tilex, min_tiley;
	int max_tilex, max_tiley;

	view.min_tile_within(min_tilex, min_tiley);
	view.max_tile_within(max_tilex, max_tiley);

	int minimap_w = bbox.width(), minimap_h = bbox.height();
	int minimap_x = bbox.x1 + (128 - minimap_w)/2, minimap_y = bbox.y1 + (128 - minimap_w)/2;
	int ptw = power_of_two(MINIMAP_SIZEMAX), pth = power_of_two(MINIMAP_SIZEMAX);
	if (!minimap_arr) {
		minimap_arr = new char[ptw * pth * 4];
	}
	for (int i = 0; i < ptw * pth; i++) {
		minimap_arr[i * 4] = 0;
		minimap_arr[i * 4 + 1] = 0;
		minimap_arr[i * 4 + 2] = 0;
		minimap_arr[i * 4 + 3] = 255;
	}

	world2minimapbuffer(gs, minimap_arr, BBox(), minimap_w, minimap_h, ptw, pth);

	GameInst* inst = gs->get_instance(gs->local_playerid());
	if (inst) {
		int arr_x = (inst->x / TILE_SIZE), arr_y = (inst->y / TILE_SIZE);
		fill_buff2d(minimap_arr, ptw, pth, arr_x - arr_x % 2, arr_y - arr_y % 2,
				Colour(255, 180, 99), 2, 2);
		draw_rect2d(minimap_arr, ptw, pth, min_tilex, min_tiley, max_tilex,
				max_tiley, Colour(255, 180, 99)); //
	}

	gl_image_from_bytes(minimap_buff, ptw, pth, minimap_arr);

	gl_draw_image(minimap_buff, minimap_x, minimap_y);
}
void GameHud::draw(GameState* gs) {
	int minimap_relposx = 20, minimap_relposy = 64 + 45;
	minimapbox = BBox(x + minimap_relposx, y + minimap_relposy,
			x + minimap_relposx + gs->get_level()->tile_width(),
			y + minimap_relposy + gs->get_level()->tile_height());

	gl_set_drawing_area(x, y, _width, _height);
	gl_draw_rectangle(0, 0, _width, _height, bg_colour);

	PlayerInst* player_inst = (PlayerInst*)gs->get_instance(
			gs->local_playerid());
	if (!player_inst)
		return;

	ClassStats& class_stats = player_inst->class_stats();
	CoreStats& core = player_inst->effective_stats().core;

	draw_player_statbars(gs, player_inst, 32, 32);

	draw_minimap(gs, minimap_bbox().translated(-x, -y), 2.0);

	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 15, 10,
			"Level %d", class_stats.xplevel);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 40,
			64 + 45 + 128, "Floor %d", gs->get_level()->level_number);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 40,
			64 + 45 + 128 + 15, "Gold %d", player_inst->gold());
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 50,
			64 + 45 + 128 + 30, "Strength   %d", core.strength);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 50,
			64 + 45 + 128 + 45, "Magic      %d", core.magic);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 50,
			64 + 45 + 128 + 60, "Defence  %d", core.defence);
	gl_printf(gs->primary_font(), Colour(255, 215, 11), _width / 2 - 50,
			64 + 45 + 128 + 75, "Willpower  %d", core.willpower);
	draw_player_actionbar(gs, player_inst);

	GameView& view = gs->window_view();
	gl_set_drawing_area(0, 0, x + _width, y + _height);

	int inv_w = _width, inv_h = _height - y - TILE_SIZE;
	draw_player_inventory(gs, player_inst, x, y + INVENTORY_POSITION, inv_w,
			inv_h, item_slot_selected);

}

GameHud::GameHud(int x, int y, int width, int height) :
		x(x), y(y), _width(width), _height(height), bg_colour(0, 0, 0), minimap_arr(
				NULL) {
	item_slot_selected = -1;
}
GameHud::~GameHud() {
	delete[] minimap_arr;
}
