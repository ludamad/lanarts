/*
 * Image.cpp:
 *  Allows defining an image as a region of a shared glimage
 */

#include <string>

#include <common/geometry.h>
#include <common/smartptr.h>

#include "DrawOptions.h"

#include "draw/Image.h"
#include "opengl/GLImage.h"

namespace ldraw {

void Image::initialize(const std::string& filename, const BBoxF& draw_region) {
	_image = new GLImage(filename);
	if (draw_region.empty()) {
		_draw_region = BBoxF(0, 0, _image->width, _image->height);
	} else {
		_draw_region = draw_region;
	}
}

void Image::initialize(const Dim& size, const BBoxF& draw_region) {
	_image = new GLImage(size);
	if (draw_region.empty()) {
		_draw_region = BBoxF(0, 0, _image->width, _image->height);
	} else {
		_draw_region = draw_region;
	}
}

void Image::draw(const DrawOptions& options, const Posf& pos) const {
	DrawOptions adjusted_options(options);
	adjusted_options.draw_region = options.draw_region.subregion(
			_draw_region.scaled(width(), height()));
	return _image->draw(options, pos);
}
}
