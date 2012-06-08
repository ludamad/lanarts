/* ProjectileInst.h:
 *  Represents a projectile attack's trajectory
 */

#include "ProjectileInst.h"
#include "EnemyInst.h"
#include "PlayerInst.h"
#include "../GameState.h"
#include <cmath>
#include <cstdio>
#include "../../data/sprite_data.h"
#include "../../util/world/collision_util.h"
#include "../../util/math_util.h"
#include "../utility_objects/AnimatedInst.h"
#include <typeinfo>

static bool bullet_target_hit(GameInst* self, GameInst* other) {
	return ((ProjectileInst*)self)->hit_target_id() == other->id;
}

ProjectileInst::~ProjectileInst() {

}

ProjectileInst::ProjectileInst(sprite_id sprite, obj_id originator, float speed,
		int range, int damage, int x, int y, int tx, int ty, bool bounce,
		int hits, obj_id target, const ObjCallback& onhit_callback) :
		GameInst(x, y, RADIUS, false), rx(x), ry(y), speed(speed), damage(
				damage), sprite(sprite), origin_id(originator), target_id(
				target), range_left(range), bounce(bounce), hits(hits), hit_callback(
				onhit_callback) {
	int dx = tx - x, dy = ty - y;
	double abs = distance_between(Pos(x, y), Pos(tx, ty));
	vx = dx * speed / abs, vy = dy * speed / abs;
}

void ProjectileInst::step(GameState* gs) {
	Pos tile_hit;
	int newx = (int)round(rx + vx); //update based on rounding of true float
	int newy = (int)round(ry + vy);
	bool collides = gs->tile_radius_test(newx, newy, radius, true, -1,
			&tile_hit);
	if (bounce) {
		bool hitsx = gs->tile_radius_test(newx, y, radius, true, -1);
		bool hitsy = gs->tile_radius_test(x, newy, radius, true, -1);
		if (hitsy || hitsx || collides) {
			if (hitsx) {
				vx = -vx;
			}
			if (hitsy) {
				vy = -vy;
			}
			if (!hitsy && !hitsx) {
				vx = -vx;
				vy = -vy;
			}

			//    return;
		}
	} else if (collides) {
		gs->remove_instance(this);
	}
	x = (int)round(rx += vx); //update based on rounding of true float
	y = (int)round(ry += vy);

	range_left -= speed;

	bool hit = false;
	GameInst* colobj = NULL;
	GameInst* origin = gs->get_instance(origin_id);
	if (dynamic_cast<PlayerInst*>(origin)) {
		if (target_id)
			gs->object_radius_test(this, &colobj, 1, &bullet_target_hit);
		else
			gs->object_radius_test(this, &colobj, 1, &enemy_colfilter);
		if (colobj) {
			EnemyInst* e = (EnemyInst*)colobj;
			char buffstr[32];
			snprintf(buffstr, 32, "%d", damage);
			float rx = vx / speed * .5;
			float ry = vy / speed * .5;
			gs->add_instance(
					new AnimatedInst(e->x - 5 + rx * 5, e->y + ry * 5, -1, 25,
							rx, ry, buffstr));

			if (e->hurt(gs, damage)) {
				PlayerInst* p = (PlayerInst*)origin;
				p->gain_xp(gs, e->xpworth());

				if (p->is_local_player()) {
					snprintf(buffstr, 32, "%d XP", e->xpworth());
					gs->add_instance(
							new AnimatedInst(e->x, e->y, -1, 25, 0, 0, buffstr,
									Colour(255, 215, 11)));
				} else
					gs->skip_next_id();
			}
		}
	} else {
		gs->object_radius_test(this, &colobj, 1, &player_colfilter);
		if (colobj) {
			Stats& s = ((PlayerInst*)colobj)->stats();
			if (!gs->game_settings().invincible)
				s.hurt(damage);
			char dmgstr[32];
			snprintf(dmgstr, 32, "%d", damage);
			float rx = vx / speed * .5;
			float ry = vy / speed * .5;
			gs->add_instance(
					new AnimatedInst(colobj->x - 5 + rx * 5, colobj->y + ry * 5,
							-1, 25, rx, ry, dmgstr));
		}
	}
	if (colobj || range_left <= 0) {
		hits--;
		if (hits >= 0 && colobj) {
			MonsterController& mc = gs->monster_controller();
			int mindist = 200;
			if (target_id == 0)
				damage /= 2;
			target_id = NONE; //Clear target
			for (int i = 0; i < mc.monster_ids().size(); i++) {
				obj_id mid = mc.monster_ids()[i];
				GameInst* enemy = gs->get_instance(mid);
				if (enemy && enemy != colobj) {

					int dx = enemy->x - x, dy = enemy->y - y;
					double abs = sqrt(dx * dx + dy * dy);
					if (abs < 1)
						abs = 1;
					if (abs < mindist) {
						target_id = mid;
						mindist = abs;
						vx = dx * speed / abs, vy = dy * speed / abs;
					}
				}
			}
		}
		if (hits == 0 || target_id == 0) {
			gs->add_instance(new AnimatedInst(x, y, sprite, 15));
			gs->remove_instance(this);
		}
	}

}

void ProjectileInst::draw(GameState* gs) {
	GameView& view = gs->window_view();
	SpriteEntry& spr = game_sprite_data[sprite];
	GLimage& img = game_sprite_data[sprite].img();
	int w = img.width, h = img.height;
	int xx = x - w / 2, yy = y - h / 2;

	if (!view.within_view(xx, yy, w, h))
		return;
	if (!gs->object_visible_test(this))
		return;

	gl_draw_sprite_entry(view, spr, x - TILE_SIZE / 2, y - TILE_SIZE / 2, vx,
			vy, 0);
//	gl_draw_image(view, img, x - TILE_SIZE / 2, y - TILE_SIZE / 2);

//	gl_printf(gs->primary_font(), Colour(255,255,255), x - view.x, y-25 -view.y, "id=%d", id);
	//gl_draw_circle(view, x, y, RADIUS, (origin_id == gs->local_playerid()) ? Colour(0, 255, 0) : Colour(255, 215, 11));

}

ProjectileInst *ProjectileInst::clone() const {
	return new ProjectileInst(*this);
}

void ProjectileInst::deinit(GameState* gs){
	hit_callback.call(gs, this);
}

GameInst* ProjectileInst::hit_target(GameState *gs) {
	return gs->get_instance(target_id);
}

void ProjectileInst::copy_to(GameInst *inst) const {
	LANARTS_ASSERT(typeid(*this) == typeid(*inst));
	*(ProjectileInst*)inst = *this;
}
