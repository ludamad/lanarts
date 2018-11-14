/*
 * AnimatedInst.h:
 *  Represents an object that displays on the screen for a fixed amount of time before removing itself
 */

#ifndef ANIMATEDINST_H_
#define ANIMATEDINST_H_
#include <string>
#include <cmath>
#include <ldraw/Font.h>

#include "draw/draw_sprite.h"

#include "GameInst.h"

const int DONT_DRAW_SPRITE = -1;


inline static void assert_normal_text(const char* c) {
	while (*c) {
		LANARTS_ASSERT(*c >= 0 && *c < 128);
		c++;
	}
}

class AnimatedInst: public GameInst {
public:
	enum {
		RADIUS = 10, DEPTH = -100
	};
	AnimatedInst(const Pos& pos, sprite_id sprite, int animatetime = -1,
			const PosF& speed = PosF(), const PosF& orientation = PosF(),
			int depth = DEPTH, const std::string& text = std::string(),
			Colour textcol = Colour(255, 0, 0)) :
			GameInst(pos.x, pos.y, RADIUS, false, depth), textcol(textcol),
                                        vx(speed.x), vy(speed.y), orientx(
					orientation.x), orienty(orientation.y), sprite(sprite), timeleft(
					animatetime), animatetime(animatetime), text(text) {
		animateframe = -1;
		assert_normal_text(text.c_str());
		vx = round(vx * 256.0f) / 256.0f;
		vy = round(vy * 256.0f) / 256.0f;
		LANARTS_ASSERT(animatetime!= 0);
	}
	virtual ~AnimatedInst();
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual AnimatedInst* clone() const;
	void frame(float frame) {
		animateframe = frame;
	}

	virtual void serialize(GameState* gs, SerializeBuffer& serializer);
	virtual void deserialize(GameState* gs, SerializeBuffer& serializer);

private:
	Colour textcol;
	float vx, vy, orientx, orienty;
	int sprite, timeleft, animatetime;
	float animateframe;
	std::string text;
public:
    ldraw::Font* font = nullptr;
    bool should_center_font = false;
};

inline AnimatedInst* animated_inst(const Pos& pos, const std::string& text,
		const Colour& textcol, int animatetime = -1) {
	return new AnimatedInst(pos, -1, -1, PosF(), PosF(), AnimatedInst::DEPTH,
			text, textcol);
}

#endif /* ANIMATEDINST_H_ */
