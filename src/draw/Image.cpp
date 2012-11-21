/*
 * Image.cpp:
 *  Allows defining an image as a region of a shared glimage
 */

#include <cmath>
#include <string>

#include <common/geometry.h>
#include <common/smartptr.h>

#include "DrawOptions.h"

#include "draw/Image.h"
#include "opengl/GLImage.h"

static BBoxF fullimagebounds(const smartptr<GLImage>& image) {
	return BBoxF(0, 0, image->width, image->height);
}

namespace ldraw {

void Image::initialize(const std::string& filename, const BBoxF& draw_region) {
	_image = new GLImage(filename);
	if (draw_region.empty()) {
		_draw_region = BBoxF(0, 0, _image->width, _image->height);
	} else {
		_draw_region = draw_region;
	}
}

Image::Image(const Image & image, const BBoxF & draw_region) :
		_image(image._image), _draw_region(draw_region) {
	if (_draw_region.empty()) {
		_draw_region = image.draw_region();
	}
}

void Image::initialize(const Dim& size, const BBoxF& draw_region) {
	_image = new GLImage(size);
	if (draw_region.empty()) {
		_draw_region = fullimagebounds(_image);
	} else {
		_draw_region = draw_region;
	}
}

void Image::draw(const DrawOptions& options, const Posf& pos) const {
	DrawOptions adjusted_options(options);
	BBoxF region(options.draw_region);
	if (region == BBoxF()) {
		region = fullimagebounds(_image);
	}

	adjusted_options.draw_region = region.subregion(
			_draw_region.scaled(1.0f / width(), 1.0f / height()));
	adjusted_options.draw_region = _draw_region;
	return _image->draw(adjusted_options, pos);
}

std::vector<Image> image_split(const Image & image, const DimF & size) {
	std::vector<Image> results;

	BBoxF bounds(image.draw_region());
	float w = bounds.width(), h = bounds.height();

	int ncols = ceil(w / size.w);
	int nrows = ceil(h / size.h);

	for (int row = 0; row < nrows; row++) {
		for (int col = 0; col < ncols; col++) {
			float x = bounds.x1 + col * size.w;
			float y = bounds.y1 + row * size.h;

			BBoxF region(x, y, std::min(bounds.x2, x + size.w),
					std::min(bounds.y2, y + size.h));
			results.push_back(Image(image, region));
		}
	}

	return results;
}
}
