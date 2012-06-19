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
		DEPTH = 0
	};
public:
	ProjectileInst(const Projectile& projectile,
			const EffectiveAttackStats& atkstats, obj_id origin_id, const Pos& start,
			const Pos& target, float speed, int range, obj_id sole_target = 0, bool bounce = false, int hits = 1);
	~ProjectileInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void deinit(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ProjectileInst* clone() const;

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
	EffectiveAttackStats atkstats;

	/* Range left before projectile is destroyed */
	int range_left;

	/*TODO: move to lua*/
	bool bounce;
	int hits;
	float damage_mult;
};

#endif /* PROJECTILEINST_H_ */
