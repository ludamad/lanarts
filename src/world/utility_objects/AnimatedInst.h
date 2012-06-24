/*
 * AnimatedInst.h:
 *  Represents an object that displays on the screen for a fixed amount of time before removing itself
 */

#ifndef ANIMATEDINST_H_
#define ANIMATEDINST_H_
#include <string>

#include "../../display/display.h"

#include "../objects/GameInst.h"

const int DONT_DRAW_SPRITE = -1;

class AnimatedInst: public GameInst {
public:
	enum {
		RADIUS = 10, VISION_SUBSQRS = 1, DEPTH = -100
	};
	AnimatedInst(int x, int y, sprite_id sprite, int animatetime = -1,
			float vx = 0, float vy = 0, int depth = DEPTH,
			const std::string& text = std::string(),
			Colour textcol = Colour(255, 0, 0)) :
			GameInst(x, y, RADIUS, false, depth), textcol(textcol), rx(x), ry(
					y), vx(vx), vy(vy), sprite(sprite), timeleft(animatetime), animatetime(
					animatetime), text(text) {
	}
	virtual ~AnimatedInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual AnimatedInst* clone() const;

private:
	Colour textcol;
	float rx, ry, vx, vy;
	int sprite, timeleft, animatetime;
	std::string text;
};

#endif /* ANIMATEDINST_H_ */
