#ifndef BUTTONINST_H_
#define BUTTONINST_H_

#include <string>

#include "../objects/GameInst.h"

#include "../util/callback_util.h"

class ButtonInst: public GameInst {
public:
	enum {
		RADIUS = 10
	};
	ButtonInst(const std::string& str, sprite_id sprite, int x, int y,
			const ObjCallback& click_callback, const Colour& hover_colour =
					Colour(255, 0, 0)) :
			GameInst(x, y, RADIUS), str(str), click_callback(click_callback), sprite(
					sprite), hover_colour(hover_colour) {
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

private:
	sprite_id sprite;
	std::string str;
	ObjCallback click_callback;
	Colour hover_colour;
};

#endif /* BUTTONINST_H_ */
