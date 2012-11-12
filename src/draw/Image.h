/*
 * Image.h:
 *  Allows defining an image as a region of a shared GLImage
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <string>

#include <common/geometry.h>
#include <common/smartptr.h>

#include "DrawOptions.h"

struct GLImage;
struct lua_State;

#include <cstdio>

namespace ldraw {

class Image {
public:
	void draw(const DrawOptions& options, const Posf& pos) const;
	void draw(const Posf& pos) const {
		draw(DrawOptions(), pos);
	}

	Image() {
	}
	Image(const std::string& filename, const BBoxF& draw_region = BBoxF()) {
		initialize(filename, draw_region);
	}
	Image(const Dim& size, const BBoxF& draw_region = BBoxF()) {
		initialize(size, draw_region);
	}

	float width() const {
		return _draw_region.width();
	}
	float height() const {
		return _draw_region.height();
	}
	DimF size() const {
		return _draw_region.size();
	}

	BBoxF& draw_region() {
		return _draw_region;
	}

	void initialize(const std::string& filename, const BBoxF& draw_region =
			BBoxF());
	void initialize(const Dim& size, const BBoxF& draw_region = BBoxF());

private:
	smartptr<GLImage> _image;
	BBoxF _draw_region;
};

}

#endif /* IMAGE_H_ */
