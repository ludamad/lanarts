#ifndef BUTTONINST_H_
#define BUTTONINST_H_

#include <string>

#include "../../util/callback_util.h"

#include "../objects/GameInst.h"

class ButtonInst: public GameInst {
public:
	enum {
		RADIUS = 10
	};
	ButtonInst(const std::string& str, int x, int y,
			const ObjCallback& click_callback) :
			GameInst(x, y, RADIUS), str(str), click_callback(click_callback) {
	}
	virtual ~ButtonInst() {
	}
	BBox bounds(GameState* gs);
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual ButtonInst* clone() const;

private:
	std::string str;
	ObjCallback click_callback;
};

#endif /* BUTTONINST_H_ */
