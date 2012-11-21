/*
 * Image.h:
 *  Allows defining an image as a region of a shared GLImage
 *  Inherits from DrawableBase, not meant for further inheritance
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <vector>
#include <string>

#include <common/geometry.h>
#include <common/smartptr.h>

#include "DrawOptions.h"
#include "DrawableBase.h"

#include "ldraw_assert.h"

struct GLImage;
struct lua_State;

#include <cstdio>

namespace ldraw {

class Image: public DrawableBase {
public:
	using DrawableBase::draw;

	Image() {
	}
	Image(const std::string& filename, const BBoxF& draw_region = BBoxF()) {
		initialize(filename, draw_region);
	}
	Image(const Image& image, const BBoxF& draw_region = BBoxF());

	Image(const Dim& size, const BBoxF& draw_region = BBoxF()) {
		initialize(size, draw_region);
	}

	virtual DimF size() const {
		return _draw_region.size();
	}

	int width() const {
		return _draw_region.width();
	}
	int height() const {
		return _draw_region.height();
	}
	BBoxF& draw_region() {
		return _draw_region;
	}

	const BBoxF& draw_region() const {
		return _draw_region;
	}

	void draw(const DrawOptions& options, const Posf& pos) const;

	void initialize(const std::string& filename, const BBoxF& draw_region =
			BBoxF());
	void initialize(const Dim& size, const BBoxF& draw_region = BBoxF());

	/* Pushes metatable to be used with a userdata object, which encapsulates a Drawable.
	 * Implemented in lua_image.cpp
	 */
	virtual void push_metatable(lua_State* L) const;

private:
	smartptr<GLImage> _image;
	BBoxF _draw_region;
};

std::vector<Image> image_split(const Image& image, const DimF& size);

// Implemented in lua_drawable.cpp
void lua_pushimage(lua_State* L, const ldraw::Image& image);
const ldraw::Image& lua_getimage(lua_State* L, int idx);
bool lua_checkimage(lua_State* L, int idx);

}

#endif /* IMAGE_H_ */
