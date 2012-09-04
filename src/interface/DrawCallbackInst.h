/*
 * DrawCallbackInst.h:
 *  Object with generic step & draw event callbacks, meant for drawing something every frame
 */

#ifndef DRAWCALLBACKINST_H_
#define DRAWCALLBACKINST_H_

#include <string>

#include "../objects/GameInst.h"

#include "../util/callback_util.h"

class DrawCallbackInst: public GameInst {
public:
	enum {
		RADIUS = 1
	};
	DrawCallbackInst(const Pos& p, const ObjCallback& stepcallback,
			const ObjCallback& drawcallback) :
			GameInst(p.x, p.y, RADIUS, false), stepcallback(stepcallback), drawcallback(
					drawcallback) {

	}
	virtual void step(GameState* gs) {
		stepcallback.call(gs, this);
	}
	virtual void draw(GameState* gs) {
		drawcallback.call(gs, this);
	}

private:
	ObjCallback stepcallback, drawcallback;
};

#endif /* DRAWCALLBACKINST_H_ */
