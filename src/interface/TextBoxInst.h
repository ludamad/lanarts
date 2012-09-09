/*
 * TextBoxInst.h:
 *  Represents an interactive text-box.
 */

#ifndef TEXTBOXINST_H_
#define TEXTBOXINST_H_

#include <string>

#include "../objects/GameInst.h"

#include "../util/callback_util.h"

#include "TextField.h"

class TextBoxInst: public GameInst {
public:
	enum {
		RADIUS = 1
	};
	TextBoxInst(const BBox& bbox, int max_length,
			const std::string& defaultstr = std::string(),
			const ObjCallback& enter_callback = ObjCallback()) :
			GameInst(bbox.x1, bbox.y1, RADIUS, false), bbox(bbox), enter_callback(
					enter_callback), selected(false), text_field(max_length,
					defaultstr) {
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
	BBox bbox;
	ObjCallback enter_callback;
	bool selected;
	TextField text_field;
};

#endif /* TEXTBOXINST_H_ */
