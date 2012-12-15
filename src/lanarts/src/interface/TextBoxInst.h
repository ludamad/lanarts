/*
 * TextBoxInst.h:
 *  Represents an interactive text-box.
 */

#ifndef TEXTBOXINST_H_
#define TEXTBOXINST_H_

#include <string>

#include <common/Timer.h>

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
			const ObjCallback& update_callback = ObjCallback(),
			const ObjCallback& deselect_callback = ObjCallback()) :
			GameInst(bbox.x1, bbox.y1, RADIUS, false), bbox(bbox), update_callback(
					update_callback), deselect_callback(deselect_callback), selected(
					false), valid_string(true), text_field(max_length,
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

	void select(GameState* gs);
	void deselect(GameState* gs);
	void mark_validity(bool valid) {
		valid_string = valid;
	}
	void set_text(const std::string& txt) {
		text_field.set_text(txt);
	}
	const std::string& text() const {
		return text_field.text();
	}

private:
	BBox bbox;
	ObjCallback update_callback, deselect_callback;
	bool selected, valid_string;
	Timer blink_timer;
	TextField text_field;
};

#endif /* TEXTBOXINST_H_ */
