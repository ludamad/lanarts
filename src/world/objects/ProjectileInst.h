/* ProjectileInst.h:
 *  Represents a projectile attack's trajectory
 */

#ifndef PROJECTILEINST_H_
#define PROJECTILEINST_H_

#include <cstdlib>

#include "../../gamestats/stats.h"

#include "../../util/callback_util.h"

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

#endif /* PROJECTILEINST_H_ */
