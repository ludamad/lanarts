#ifndef LDRAW_FONT_H_
#define LDRAW_FONT_H_

#include <common/smartptr.h>
#include <common/geometry.h>

#include "Colour.h"
#include "DrawOptions.h"

struct font_data;

namespace ldraw {

class Font {
public:
	void initialize(const std::string& filename, int height);
	void render(const DrawOptions& options, const Posf& position, const std::string& str);
private:
	smartptr<font_data> _font;
};

}


#endif /* LDRAW_FONT_H_ */
