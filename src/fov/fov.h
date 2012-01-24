/*
 * fov.h
 *
 *  Created on: 2011-11-05
 *      Author: 100397561
 */

#ifndef FOV_H_
#define FOV_H_

#include "impl/permissive-fov-cpp.h"
#include "../world/GameState.h"

class fov {
public:
	fov(GameState* gs, int radius, int ptx, int pty, int sub_squares = 1);
	~fov();
public://do-not-use
    int isBlocked(short destX, short destY);
    void visit(short destX, short destY);
public:
    bool within_fov(int grid_x, int grid_y);
    void matches(int sqr_x, int sqr_y, char* sub_sqrs);
private:
    char* sight_mask;
    GameState* gs;
    int radius, radsub;
    int ptx, pty, sub_squares;
    permissive::maskT m;
};
#endif /* FOV_H_ */
