/*
 * DrawableMock.h:
 *  A drawable that sets a string and the draw options it was passed through pointers.
 */

#ifndef DRAWABLEMOCK_H_
#define DRAWABLEMOCK_H_

#include <string>
#include "../DrawableBase.h"
#include "../DrawOptions.h"

namespace ldraw {
namespace test {

class DrawableMock: public DrawableBase {
public:
	using DrawableBase::draw;

	DrawableMock(const std::string& value, std::string* str,
			DrawOptions* options = NULL, Posf* posf= NULL) :
			value(value), _str(str), _options(options), _posf(posf) {
	}
	virtual void draw(const DrawOptions& options, const Posf& pos) const {
		if (_str) {
			*_str = value;
		}
		if (_options) {
			*_options = options;
		}
		if (_posf) {
			*_posf = pos;
		}
	}
private:
	std::string value;
	std::string* _str;
	DrawOptions* _options;
	Posf* _posf;
};

}
}

#endif /* DRAWABLEMOCK_H_ */
