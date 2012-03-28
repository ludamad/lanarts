/*
 * TestInst.h
 *
 *  Created on: Feb 6, 2012
 *      Author: 100397561
 */

#ifndef TESTINST_H_
#define TESTINST_H_

#include "GameInst.h"
#include "../../pathfind/astar_pathfind.h"

class TestInst: public GameInst {
public:
	enum {RADIUS = 10, VISION_SUBSQRS = 1};
	TestInst(int x, int y) :
		GameInst(x,y, RADIUS){}
	virtual ~TestInst(){}
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
private:
	AStarPathFind path;
	std::vector<Pos> p;
};

#endif /* TESTINST_H_ */
