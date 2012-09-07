/*
 * TextBoxInst.h:
 *  Represents an interactive text-box.
 */

#ifndef TEXTBOXINST_H_
#define TEXTBOXINST_H_

#include <string>

#include "../objects/GameInst.h"

#include "../util/callback_util.h"

#include "DrawCallbackInst.h"

class TextBoxInst: public DrawCallbackInst {
public:
	enum {
		RADIUS = 10
	};
	TextBoxInst(const Pos& p, const ObjCallback& stepcallback,
			const ObjCallback& drawcallback, int max_length,
			const std::string defaultstr = std::string(),
			const ObjCallback& enter_callback = ObjCallback()) :
			DrawCallbackInst(p, stepcallback, drawcallback), data(defaultstr), enter_callback(
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
