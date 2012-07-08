/*
 * TextBoxInst.h:
 *  Represents an interactive text-box.
 */

#ifndef TEXTBOXINST_H_
#define TEXTBOXINST_H_

#include <string>

#include "../../util/callback_util.h"

#include "../objects/GameInst.h"

class TextBoxInst: public GameInst {
public:
	enum {
		RADIUS = 10
	};
	TextBoxInst(int x, int y, const std::string defaultstr = std::string(),
			const ObjCallback& enter_callback = ObjCallback()) :
			GameInst(x, y, RADIUS), data(defaultstr), enter_callback(
					enter_callback) {
	}
	virtual ~TextBoxInst() {
	}
	BBox bounds(GameState* gs);
	virtual void init(GameState* gs);
	virtual void step(GameState* gs);
	virtual void draw(GameState* gs);
	virtual void copy_to(GameInst* inst) const;
	virtual TextBoxInst* clone() const;

private:
	std::string data;
	ObjCallback enter_callback;
};

#endif /* TEXTBOXINST_H_ */
