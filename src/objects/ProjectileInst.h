/* ProjectileInst.h:
 *  Represents a projectile attack's trajectory
 */

#ifndef PROJECTILEINST_H_
#define PROJECTILEINST_H_

#include <cstdlib>

#include "stats/items/items.h"
#include "stats/stats.h"

#include "util/callback_util.h"
#include "lanarts_defines.h"

#include "GameInst.h"

class ProjectileInst: public GameInst {
	enum {
		DEPTH = 0
	};
public:
	ProjectileInst(const Item& projectile,
			const EffectiveAttackStats& atkstats, obj_id origin_id, const Pos& start,
			const Pos& target, float speed, int range, obj_id sole_target = 0, bool bounce = false, int hits = 1, bool pass_through = false);
	~ProjectileInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void init(GameState* gs);
	virtual void deinit(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ProjectileInst* clone() const;

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);

	virtual void update_position(float newx, float newy) {
            rx = newx;
            ry = newy;
            x = (int) rx;
            y = (int) ry;
        }
	sprite_id sprite() const;
	float rx, ry, vx, vy, speed;
private:
	static bool bullet_target_hit2(GameInst* self, GameInst* other);
	/* Stats at time of projectile creation */
	EffectiveAttackStats atkstats;
	/* Origin object, and optional exclusive target*/
	obj_id origin_id, sole_target;

	/* Projectile used */
	Item projectile;

	/* Range left before projectile is destroyed */
	int range_left;

	int frame = 0;
	/*TODO: move to lua*/
	bool bounce;
	int hits;
	float damage_mult;
	bool pass_through;


};

#endif /* PROJECTILEINST_H_ */
