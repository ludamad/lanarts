#ifndef PLAYERINST_H_
#define PLAYERINST_H_

#include "GameInst.h"
#include "../../gamestats/Stats.h"
#include "../../fov/fov.h"
#include "../../pathfind/pathfind.h"

class PlayerInst : public GameInst {
public:
	enum {RADIUS = 10, VISION_SUBSQRS = 1};
	PlayerInst(int x, int y) :
		GameInst(x,y, RADIUS), base_stats(100,100,20, 5){}
	virtual ~PlayerInst();
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	Stats& stats(){ return base_stats; }
private:
	Stats base_stats;
};


#endif /* PLAYERINST_H_ */
