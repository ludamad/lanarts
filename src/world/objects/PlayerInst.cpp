#include "PlayerInst.h"
#include "BulletInst.h"
#include "EnemyInst.h"
#include "ItemInst.h"
#include "AnimatedInst.h"

#include "../../util/draw_util.h"
#include "../GameState.h"
#include "../../data/sprite_data.h"
#include "../../data/tile_data.h"
#include "../../display/display.h"
#include "../../data/item_data.h"

static const int REST_COOLDOWN = 350;

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

void PlayerInst::use_move_and_melee(GameState* gs) {

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

	if (dx == 0 && dy == 0)
		return;

	float mag = effective_stats().movespeed;
	float ddx = dx * mag;
	float ddy = dy * mag;


	EnemyInst* target = NULL;
	gs->object_radius_test(this, (GameInst**) &target, 1, &enemy_hit, x + ddx,
			y + ddy);

	EnemyInst* alreadyhitting[5] = { 0, 0, 0, 0, 0 };
	gs->object_radius_test(this, (GameInst**) alreadyhitting, 5, &enemy_hit, x,
			y);
	bool already = false;
	for (int i = 0; i < 5; i++) {
		if (alreadyhitting[i]) {
			if (ddx < 0 == ((alreadyhitting[i]->x - x + ddx * 2) < 0)) {
				ddx = 0;
			}
			if (ddy < 0 == ((alreadyhitting[i]->y - y + ddy * 2) < 0)) {
				ddy = 0;
			}
			already = true;
		}
	}

	gs->tile_radius_test(x + ddx, y + ddy, radius);

	if (!gs->tile_radius_test(x + ddx, y + ddy, radius)) {
		x += ddx;
		y += ddy;
	} else if (!gs->tile_radius_test(x + ddx, y, radius)) {
		x += ddx;
	} else if (!gs->tile_radius_test(x, y + ddy, radius)) {
		y += ddy;
	}

	if (ddx == 0 && ddy == 0){
		if (target && !stats().has_cooldown()) {
			if (target->hurt(gs, effective_stats().melee.damage)) {
				stats().gain_xp(target->xpworth());
			}
			stats().reset_melee_cooldown(effective_stats());
			gs->add_instance(new AnimatedInst(target->x, target->y, SPR_SHORT_SWORD, 25));
		}
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

void PlayerInst::use_staircase(GameState* gs){

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
		if ((gs->key_down_state(SDLK_COMMA) || gs->mouse_upwheel())
				&& gs->branch_level() > 1) {
			Pos hitsqr;
			if (gs->tile_radius_test(x, y, RADIUS, false, TILE_STAIR_UP,
					&hitsqr)) {
				int entr_n = scan_entrance(gs->level()->exits, hitsqr);
				LANARTS_ASSERT(
						entr_n >= 0 && entr_n < gs->level()->entrances.size());
				portal = &gs->level()->exits[entr_n];
				gs->branch_level()--;gs
				->set_generate_flag();
			}
		}
}

void PlayerInst::use_spell(GameState* gs){
	GameView& view = gs->window_view();

	bool mouse_within = gs->mouse_x() < gs->window_view().width;

	int rmx = view.x + gs->mouse_x(), rmy = view.y + gs->mouse_y();

	if (gs->mouse_right_click() && mouse_within) {
		int px = x, py = y;
		x = rmx, y = rmy;
		if (stats().mp >= 50 && !gs->solid_test(this)
				&& gs->object_visible_test(this)) {
			stats().mp -= 50;
			canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
		} else {
			x = px, y = py;
		}
	}

	static bool spell = false;

	if (gs->key_press_state(SDLK_SPACE)){
		spell = !spell;
	}
	bool cast = false;
	int tx, ty;
	if (gs->key_down_state(SDLK_j)){
		obj_id tid = gs->monster_controller().targetted;
		if (tid){
			GameInst* target = gs->get_instance(tid);
			if (target){
				cast = true;
				tx = target->x;
				ty = target->y;
			}
		}
	} else if (gs->mouse_left_click()){
		cast = true;
		tx = gs->mouse_x() + view.x;
		ty = gs->mouse_y() + view.y;
	}

	if (cast && !stats().has_cooldown()){
		Attack atk(effective_stats().ranged);
		bool bounce = true;
		int hits = 0;
		int mpcost = 10;
		if (spell){
			atk.projectile_sprite = SPR_MAGIC_BLAST;
			atk.projectile_speed /= 1.75;
			atk.damage *= 2;
			bounce = false;
			hits = 3;
			mpcost = 20;
		}
		if (mpcost <= stats().mp){

			GameInst* bullet = new BulletInst(id, atk, x, y, tx, ty, bounce, hits);
			gs->add_instance(bullet);

			canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
			if (spell)
				base_stats.cooldown = effective_stats().ranged.cooldown*1.4;
			else
				base_stats.cooldown = effective_stats().ranged.cooldown;

		}
	}

	if (gs->mouse_left_click() && !mouse_within) {
		int posx = (gs->mouse_x() - gs->window_view().width) / TILE_SIZE;
		int posy = (gs->mouse_y() - INVENTORY_POSITION) / TILE_SIZE;
		int slot = 5 * posy + posx;
		if (slot >= 0 && slot < INVENTORY_SIZE
				&& inventory.inv[slot].n > 0) {
			int item = inventory.inv[slot].item;
			game_item_data[item].action(this);
			inventory.inv[slot].n--;
			canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
		}
	}

}
void PlayerInst::step(GameState* gs) {

	GameView& view = gs->window_view();

	//Stats/effect step
	stats().step();
	effects.step();

	if (stats().hp <= 0) {
		gs->branch_level() = 1;
		gs->set_generate_flag();
		return;
	}
	if (stats().hurt_cooldown > 0)
		canrestcooldown = std::max(canrestcooldown, REST_COOLDOWN);
	canrestcooldown--;
	if (canrestcooldown < 0)
		canrestcooldown = 0;

	bool resting = false;
	if (gs->key_down_state(SDLK_r) && canrestcooldown == 0) {
		resting = true;
		stats().raise_hp(stats().hpregen * 5);
		stats().raise_mp(stats().mpregen * 5);
	}

	if (!resting)
		use_move_and_melee(gs);

	if (!resting) {
		use_staircase(gs);

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
		for (int i = 0; i < 9; i++) {
			if (gs->key_press_state(SDLK_1 + i)) {
				if (inventory.inv[i].n > 0) {
					int item = inventory.inv[i].item;
					game_item_data[item].action(this);
					inventory.inv[i].n--;
				}
			}
		}

		if (gs->key_press_state(SDLK_k)) {
			gs->monster_controller().shift_target(gs);
		}
		use_spell(gs);
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

	if (effects.get(EFFECT_HASTE)) {
		effect* e = effects.get(EFFECT_HASTE);
		float s = e->t_remaining/200;
		if (s> 1) s = 1;
		Colour blue(255*(1-s), 255*(1-s), 255);
		image_display(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y, blue);
	}else if (stats().hurt_cooldown > 0) {
		float s = 1 - stats().hurt_alpha() ;
		Colour red(255, 255*s, 255*s);
		image_display(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y, red);
	}else {
		image_display(&img, x - img.width / 2 - view.x,
				y - img.height / 2 - view.y);
	}


	//for (int i = 0; i < 10; i++)
//	gl_printf(gs->primary_font(), Colour(255,255,255), x-10-view.x, y-30-view.y, "id=%d", this->id);
//	gl_printf(gs->primary_font(), Colour(255,255,255), gs->mouse_x(), gs->mouse_y(),
//			"mx=%d,my=%d\nx=%d,y=%d",
//			gs->mouse_x(), gs->mouse_y(),
//			gs->mouse_x()+view.x, gs->mouse_y()+view.y);

//	gs->monster_controller().paths[0].draw(gs);
}
