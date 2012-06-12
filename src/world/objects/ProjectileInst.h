/* ProjectileInst.h:
 *  Represents a projectile attack's trajectory
 */

#ifndef PROJECTILEINST_H_
#define PROJECTILEINST_H_

#include <cstdlib>

#include "../../gamestats/items.h"
#include "../../gamestats/stats.h"

#include "../../util/callback_util.h"
#include "../../util/game_basic_structs.h"

#include "GameInst.h"

class ProjectileInst: public GameInst {
	enum {
		RADIUS = 5, DEPTH = 0
	};
public:
	ProjectileInst(sprite_id sprite, obj_id originator, float speed, int range,
			int damage, int x, int y, int tx, int ty, bool bounce = false,
			int hits = 1, obj_id target = NONE,
			const ObjCallback& onhit_callback = ObjCallback());
	~ProjectileInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void deinit(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ProjectileInst* clone() const;

	GameInst* hit_target(GameState* gs);
	obj_id hit_target_id() {
		return target_id;
	}
private:
	float rx, ry, vx, vy, speed;
	int damage;

	sprite_id sprite;
	obj_id origin_id, target_id;

	/*Range limiting behaviour*/
	int range_left;

	/*Multi-hit behaviour*/
	bool bounce;
	int hits;

	/*Callback determined behaviour*/
	ObjCallback hit_callback;
};

class _ProjectileInst: public GameInst {
	enum {
		DEPTH = 0
	};
public:
	_ProjectileInst(const Projectile& projectile,
			const EffectiveStats& stats, obj_id origin_id, const Pos& start,
			const Pos& target, float speed, int range, obj_id sole_target = 0, bool bounce = false, int hits = 1);
	~_ProjectileInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void deinit(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual _ProjectileInst* clone() const;

	sprite_id sprite() const;
private:
	static bool bullet_target_hit2(GameInst* self, GameInst* other);

	float rx, ry, vx, vy, speed;
	/* Team alignment, to determine if friendly-firing */
	team_id team;
	/* Origin object, and optional exclusive target*/
	obj_id origin_id, sole_target;

	/* Projectile used */
	Projectile projectile;

	/* Stats at time of projectile creation */
	EffectiveStats stats;

	/* Range left before projectile is destroyed */
	int range_left;

	/*TODO: move to lua*/
	bool bounce;
	int hits;
	float damage_mult;
};

#endif /* PROJECTILEINST_H_ */
