/* ProjectileInst.h:
 *  Represents a projectile attack's trajectory
 */

#ifndef PROJECTILEINST_H_
#define PROJECTILEINST_H_

#include <cstdlib>
#include "GameInst.h"
#include "../../gamestats/Stats.h"

class ProjectileInst: public GameInst {
	enum {RADIUS = 5, DEPTH = 0};
public:
	ProjectileInst(obj_id originator, Attack& attack, int x, int y, int tx, int ty,
			bool bounce = false, int hits = 1, obj_id target = 0);
	~ProjectileInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ProjectileInst* clone() const;

	GameInst* hit_target(GameState* gs);
	obj_id hit_target_id() { return target;}
private:
	Attack attack;
	int range_left;
	int origin_id;
	float rx, ry, vx, vy;
	bool bounce;
	int hits;
	obj_id target;
};

#endif /* PROJECTILEINST_H_ */
