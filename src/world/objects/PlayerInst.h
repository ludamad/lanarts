#ifndef PLAYERINST_H_
#define PLAYERINST_H_

#include "GameInst.h"
#include "../../logic/Stats.h"

class PlayerInst : public GameInst {
	enum {RADIUS = 10};
public:
	PlayerInst(int x, int y) :
		GameInst(x,y, RADIUS), base_stats(100,100), cooldown(0) {}
	virtual ~PlayerInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);

	Stats& stats(){ return base_stats; }
private:
	Stats base_stats;
	int cooldown;
};


#endif /* PLAYERINST_H_ */
