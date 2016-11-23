/*
 * fov.h:
 *  Represents a field of view
 */

#ifndef FOV_H_
#define FOV_H_

#include <lcommon/geometry.h>

#include "lanarts_defines.h"

#include "impl/permissive-fov-cpp.h"

class GameState;

const int MAX_FOV_RADIUS = 10;

class fov {
public:
	fov(int radius = MAX_FOV_RADIUS);
	~fov();
	void calculate(GameState* gs, int subradius, int ptx, int pty);
	bool within_fov(int grid_x, int grid_y);
	bool within_fov(const BBox& bbox);
	void matches(int sqr_x, int sqr_y, char* sub_sqrs);
	BBox tiles_covered() {
		return BBox(ptx - radius, pty - radius, ptx + radius, pty + radius);
	}
	fov* clone() const;
private:
	bool has_been_calculated;
	char* sight_mask;
	GameState* gs;
	int radius, diameter;
	int sx, sy;
	int ptx, pty;
	permissive::maskT m;
public:
	//do-not-use
	int isBlocked(short destX, short destY);
	void visit(short destX, short destY);
};
const int VISION_SUBSQRS = 1;
#endif /* FOV_H_ */
