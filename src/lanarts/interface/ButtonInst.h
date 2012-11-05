#ifndef BUTTONINST_H_
#define BUTTONINST_H_

#include <string>

#include "../display/colour_constants.h"

#include "../objects/GameInst.h"

#include "../util/callback_util.h"

class ButtonInst: public GameInst {
public:
	enum {
		RADIUS = 10
	};
	ButtonInst(const std::string& str, sprite_id sprite, int x, int y,
			const ObjCallback& click_callback,
			const ObjCallback& draw_callback = ObjCallback(),
			const Colour& draw_colour = Colour(255, 255, 255),
			const Colour& hover_colour = COL_MUTED_RED) :
			GameInst(x, y, RADIUS), str(str), draw_callback(draw_callback), click_callback(
					click_callback), sprite(sprite), _draw_colour(draw_colour), _hover_colour(
					hover_colour) {
	}
	virtual ~ButtonInst() {
	}
	BBox text_bounds(GameState* gs);
	BBox sprite_bounds(GameState* gs);
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ButtonInst* clone() const;

	Colour& draw_colour() {
		return _draw_colour;
	}
	Colour& hover_colour() {
		return _hover_colour;
	}

private:
	sprite_id sprite;
	std::string str;
	ObjCallback click_callback, draw_callback;
	Colour _draw_colour;
	Colour _hover_colour;
};

inline ButtonInst* button_inst(const std::string& str, int x, int y,
		const ObjCallback& click_callback) {
	return new ButtonInst(str, -1, x, y, click_callback);
}
#endif /* BUTTONINST_H_ */
