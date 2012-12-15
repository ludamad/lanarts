/*
 * AnimatedInst.h:
 *  Represents an object that displays on the screen for a fixed amount of time before removing itself
 */

#ifndef ANIMATEDINST_H_
#define ANIMATEDINST_H_
#include <string>

#include "display/display.h"

#include "GameInst.h"

const int DONT_DRAW_SPRITE = -1;

class AnimatedInst: public GameInst {
public:
	enum {
		RADIUS = 10, VISION_SUBSQRS = 1, DEPTH = -100
	};
	AnimatedInst(const Pos& pos, sprite_id sprite, int animatetime = -1,
			const Posf& speed = Posf(), const Posf& orientation = Posf(),
			int depth = DEPTH, const std::string& text = std::string(),
			Colour textcol = Colour(255, 0, 0)) :
			GameInst(pos.x, pos.y, RADIUS, false, depth), textcol(textcol), rx(
					pos.x), ry(pos.y), vx(speed.x), vy(speed.y), orientx(
					orientation.x), orienty(orientation.y), sprite(sprite), timeleft(
					animatetime), animatetime(animatetime), text(text) {
		LANARTS_ASSERT(animatetime!= 0);
	}
	virtual ~AnimatedInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual AnimatedInst* clone() const;

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);
private:
	Colour textcol;
	float rx, ry, vx, vy, orientx, orienty;
	int sprite, timeleft, animatetime;
	std::string text;
};

inline AnimatedInst* animated_inst(const Pos& pos, const std::string& text,
		const Colour& textcol, int animatetime = -1) {
	return new AnimatedInst(pos, -1, -1, Posf(), Posf(), AnimatedInst::DEPTH,
			text, textcol);
}

#endif /* ANIMATEDINST_H_ */
