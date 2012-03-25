/*
 * AnimatedInst.h
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */

#ifndef ANIMATEDINST_H_
#define ANIMATEDINST_H_
#include "GameInst.h"
#include <string>


const int DONT_DRAW_SPRITE = -1;

class AnimatedInst: public GameInst {
public:
	enum {RADIUS = 10, VISION_SUBSQRS = 1};
	AnimatedInst(int x, int y, int sprite, int animatetime, std::string text = std::string()) :
		GameInst(x,y, RADIUS, false), sprite(sprite), timeleft(animatetime), animatetime(animatetime), text(text){}
	virtual ~AnimatedInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
private:
	std::string text;
	int sprite, timeleft, animatetime;
};


#endif /* ANIMATEDINST_H_ */
