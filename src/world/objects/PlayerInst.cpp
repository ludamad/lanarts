#include "PlayerInst.h"
#include "BulletInst.h"
#include "EnemyInst.h"
#include "ItemInst.h"

#include "../../util/draw_util.h"
#include "../GameState.h"
#include "../../data/sprite_data.h"
#include "../../data/tile_data.h"
#include "../../display/display.h"
#include "../../data/item_data.h"

PlayerInst::~PlayerInst() {
}

void PlayerInst::init(GameState* gs) {
	PlayerController& pc = gs->player_controller();
	pc.register_player(this->id);
	portal = NULL;
}

static bool item_hit(GameInst* self, GameInst* other) {
	return dynamic_cast<ItemInst*>(other) != NULL;
}

static bool enemy_hit(GameInst* self, GameInst* other) {
	return dynamic_cast<EnemyInst*>(other) != NULL;
}

void PlayerInst::move(GameState* gs, int dx, int dy) {
	if (dx == 0 && dy == 0)
		return;

	float mag = effective_stats().movespeed;
	float ddx = dx * mag;
	float ddy = dy * mag;

	EnemyInst* alreadyhitting = NULL;
	gs->object_radius_test(this, (GameInst**) &alreadyhitting, 1, &enemy_hit, x, y);
	if (alreadyhitting){
		if (ddx < 0 == () < 0){

		}
	}

	EnemyInst* target = NULL;
	gs->object_radius_test(this, (GameInst**) &target, 1, &enemy_hit, x + ddx,
			y + ddy);
	gs->tile_radius_test(x+ddx, y+ddy, radius);

	if (!gs->tile_radius_test(x+ddx, y+ddy, radius)) {
		x += ddx;
		y += ddy;
	} else if (!gs->tile_radius_test(x+ddx, y, radius)) {
		x += ddx;
	} else if (!gs->tile_radius_test(x, y+ddy, radius)) {
		y += ddy;
	}

	if (target && !stats().has_cooldown()) {
		if (target->stats().hurt(effective_stats().melee.damage)) {
			gs->remove_instance(target);
		}
		stats().reset_melee_cooldown(effective_stats());
	}
}

static int scan_entrance(const std::vector<GameLevelPortal>& portals,
		const Pos& tilepos) {
	for (int i = 0; i < portals.size(); i++) {
		if (portals[i].entrancesqr == tilepos) {
			return i;
		}
	}
	return -1;
}
void PlayerInst::step(GameState* gs) {

	GameView& view = gs->window_view();

	//Stats/effect step
	stats().step();
	effects.step();
	
	//HP/MP regen
	if (gs->frame() % stats().hpregen == 0) {
		if (++stats().hp > stats().max_hp)
			stats().hp = stats().max_hp;
	}
	if (gs->frame() % stats().mpregen == 0) {
		if (++stats().mp > stats().max_mp)
			stats().mp = stats().max_mp;
	}

	if (stats().hp <= 0) {
		gs->branch_level() = 1;
		gs->set_generate_flag();
		return;
	}


	//Arrow/wasd movement
	int dx = 0, dy = 0;
	if (gs->key_down_state(SDLK_UP) || gs->key_down_state(SDLK_w)) {
		dy -= 1;
	}
	if (gs->key_down_state(SDLK_RIGHT) || gs->key_down_state(SDLK_d)) {
		dx += 1;
	}
	if (gs->key_down_state(SDLK_DOWN) || gs->key_down_state(SDLK_s)) {
		dy += 1;
	}
	if (gs->key_down_state(SDLK_LEFT) || gs->key_down_state(SDLK_a)) {
		dx -= 1;
	}
	move(gs, dx, dy);
	
	
	//Up/down stairs
	if (gs->key_down_state(SDLK_PERIOD) || gs->mouse_downwheel()) {
		Pos hitsqr;
		if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_DOWN,
				&hitsqr)) {
			int entr_n = scan_entrance(gs->level()->entrances, hitsqr);
			LANARTS_ASSERT(
					entr_n >= 0 && entr_n < gs->level()->entrances.size());
			portal = &gs->level()->entrances[entr_n];
			gs->branch_level()++;gs
			->set_generate_flag();
		}
	}
	if ((gs->key_down_state(SDLK_COMMA) || gs->mouse_upwheel()) && gs->branch_level() > 1) {
		Pos hitsqr;
		if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_UP, &hitsqr)) {
			int entr_n = scan_entrance(gs->level()->exits, hitsqr);
			LANARTS_ASSERT(
					entr_n >= 0 && entr_n < gs->level()->entrances.size());
			portal = &gs->level()->exits[entr_n];
			gs->branch_level()--;gs
			->set_generate_flag();
		}
	}
	
	//Item pickup
	ItemInst* item = NULL;
	if (gs->object_radius_test(this, (GameInst**) &item, 1, &item_hit)) {
		gs->remove_instance(item);
		if (item->item_type() == ITEM_GOLD) {
			money += 10;
		} else {
			inventory.add(item->item_type(), 1);
		}
	}

	bool mouse_within = gs->mouse_x() < gs->window_view().width;
	if (gs->key_press_state(SDLK_1)) {
		if (inventory.inv[0].n > 0) {
			int item = inventory.inv[0].item;
			game_item_data[item].action(this);
			inventory.inv[0].n--;
		}
	}

	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();
	if (gs->mouse_right_click() && mouse_within) {
		int px = x, py = y;
		x = rmx, y = rmy;
		if (stats().mp >= 50 && !gs->solid_test(this)
				&& gs->object_visible_test(this)) {
			stats().mp -= 50;
		} else {
			x = px, y = py;
		}
	}

	if (gs->key_press_state(SDLK_k)) {
		gs->monster_controller().shift_target(gs);
	}
	if (gs->key_down_state(SDLK_j) && !base_stats.has_cooldown()) {
		obj_id tid = gs->monster_controller().targetted;
		GameInst* target = gs->get_instance(tid);
		if (tid && target ) {
			if (stats().mp >= 10) {
				stats().mp -= 10;
				GameInst* bullet = new BulletInst(id, stats().ranged, x, y,
						target->x, target->y);
				gs->add_instance(bullet);
				base_stats.reset_ranged_cooldown(effective_stats());
			}
		}
	} else if (gs->mouse_left_down() && mouse_within && !base_stats.has_cooldown()) {
		if (stats().mp >= 10) {
			stats().mp -= 10;
			GameInst* bullet = new BulletInst(id, stats().ranged, x, y, rmx,
					rmy);
			gs->add_instance(bullet);
			base_stats.reset_ranged_cooldown(effective_stats());
		}
	} else if (gs->mouse_left_click() && !mouse_within) {
		int posx = (gs->mouse_x() - gs->window_view().width) / TILE_SIZE;
		int posy = (gs->mouse_y() - INVENTORY_POSITION) / TILE_SIZE;
		int slot = 5 * posy + posx;
		if (slot >= 0 && slot < INVENTORY_SIZE && slot <inventory.inv[slot].n > 0) {
			int item = inventory.inv[slot].item;
			game_item_data[item].action(this);
			inventory.inv[slot].n--;
		}
	}

	/*if (gs->mouse_right_down()) {
	 int nx = gs->mouse_x() + view.x, ny = gs->mouse_y() + view.y;
	 view.center_on(nx, ny);
	 } else*/
	view.center_on(last_x, last_y);
}

void PlayerInst::draw(GameState* gs) {
	GameView& view = gs->window_view();
	GLImage& img = spr_player.img;
	bool b = gs->tile_radius_test(x, y, RADIUS);
	//gl_draw_rectangle(view, x-10,y-20,20,5, b ? Colour(255,0,0) : Colour(0,255,0));
	//gl_draw_circle(view, x,y,RADIUS);
	if (stats().hp < stats().max_hp)
		gl_draw_statbar(view, x - 10, y - 20, 20, 5, stats().hp,
				stats().max_hp);

	if (stats().hurt_cooldown > 0) {
		float s = 1 - stats().hurt_alpha() / 2;
		Colour red(255, s, s);
		image_display(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y, red);
	} else
		image_display(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y);
	//for (int i = 0; i < 10; i++)
//	gl_printf(gs->primary_font(), Colour(255,255,255), x-10-view.x, y-30-view.y, "id=%d", this->id);
//	gl_printf(gs->primary_font(), Colour(255,255,255), gs->mouse_x(), gs->mouse_y(),
//			"mx=%d,my=%d\nx=%d,y=%d",
//			gs->mouse_x(), gs->mouse_y(),
//			gs->mouse_x()+view.x, gs->mouse_y()+view.y);

//	gs->monster_controller().paths[0].draw(gs);
}
