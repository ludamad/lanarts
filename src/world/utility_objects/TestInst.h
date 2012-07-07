/*
 * TestInst.h
 *
 *  Created on: Feb 6, 2012
 *      Author: 100397561
 */

#ifndef TESTINST_H_
#define TESTINST_H_

#include "../objects/GameInst.h"
#include "../../pathfind/astar_pathfind.h"

class TestInst: public GameInst {
public:
	enum {RADIUS = 10, VISION_SUBSQRS = 1};
	TestInst(int x, int y);
	virtual ~TestInst(){}
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual TestInst* clone() const;
private:
	AStarPathFind path;
	std::vector<Pos> p;
};


#endif /* TESTINST_H_ */
