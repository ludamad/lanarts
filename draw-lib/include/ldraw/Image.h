/*
 * Image.h:
 *  Allows defining an image as a region of a shared GLImage
 *  Inherits from DrawableBase, not meant for further inheritance
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <vector>
#include <string>

#include <lcommon/geometry.h>
#include <lcommon/smartptr.h>

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

	Image();

	Image(const std::string& filename, const BBoxF& draw_region = BBoxF(),
			bool rotates = false) {
		initialize(filename, draw_region, rotates);
	}

	Image(const Image& image, const BBoxF& draw_region = BBoxF());

	Image(const Size& size, const BBoxF& draw_region = BBoxF(), bool rotates =
			false) {
		initialize(size, draw_region, rotates);
	}

	virtual SizeF size() const {
		return _draw_region.size();
	}

	int width() const {
		return (int)_draw_region.width();
	}

	int height() const {
		return (int)_draw_region.height();
	}

	BBoxF& draw_region() {
		return _draw_region;
	}

	const BBoxF& draw_region() const {
		return _draw_region;
	}

	bool& rotates() {
		return _rotates;
	}

	void draw(const DrawOptions& options, const PosF& pos) const;
	void initialize(const std::string& filename, const BBoxF& draw_region =
			BBoxF(), bool rotates = false);
	void initialize(const Size& size, const BBoxF& draw_region = BBoxF(),
			bool rotates = false);
	void from_bytes(const BBox& region, char* data);
	void from_bytes(const Size& size, char* data);

	virtual void push_metatable(lua_State* L) const;

private:
	bool _rotates;
	smartptr<GLImage> _image;
	BBoxF _draw_region;
};

std::vector<Image> image_split(const Image& image, const SizeF& size);

// Implemented in lua_drawable.cpp
void lua_pushimage(lua_State* L, const ldraw::Image& image);
const ldraw::Image& lua_getimage(lua_State* L, int idx);
bool lua_checkimage(lua_State* L, int idx);
}

#endif /* IMAGE_H_ */
