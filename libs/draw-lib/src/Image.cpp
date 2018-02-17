/*
 * Image.cpp:
 *  Allows defining an image as a region of a shared glimage
 */

#include <cmath>
#include <string>

#include <lcommon/geometry.h>
#include <lcommon/smartptr.h>

#include "DrawOptions.h"

#include "Image.h"
#include "GLImage.h"

#include "opengl/GLGridAtlas.h"

//static GLGridAtlas _atlas(32, 32, 50, 50);
//static int n_alloc;

static BBoxF fullimagebounds(const smartptr<GLImage>& image) {
	return BBoxF(0, 0, image->width, image->height);
}

namespace ldraw {

Image::Image() :
		_rotates(false) {
}

void Image::initialize(const std::string& filename, const BBoxF& draw_region,
		bool rotates) {
	_image.set(new GLImage(filename));
	if (draw_region.empty()) {
		_draw_region = BBoxF(0, 0, _image->width, _image->height);
	} else {
		_draw_region = draw_region;
	}
//    if (_draw_region.width() == 32 && _draw_region.height() == 32) {
//        _atlas.allocate(filename, draw_region, _image, _draw_region);
//        n_alloc++;
//        printf("GOT %d ALLOCATED\n", n_alloc);
//    }
	_rotates = rotates;
}

Image::Image(const Image & image, const BBoxF & draw_region) :
		_image(image._image), _draw_region(draw_region) {
	if (_draw_region.empty()) {
		_draw_region = image.draw_region();
	}
	_rotates = image._rotates;
}

void Image::initialize(const Size& size, const BBoxF& draw_region,
		bool rotates) {
	_image.set(new GLImage(size));
	if (draw_region.empty()) {
		_draw_region = fullimagebounds(_image);
	} else {
		_draw_region = draw_region;
	}
	_rotates = rotates;
}

void Image::batch_draw(const PosF& pos) const {
#ifdef EMSCRIPTEN
	_image->draw(_draw_region, pos);
#else
	_image->batch_draw(_draw_region, pos);
#endif
}

void Image::draw(const DrawOptions& options, const PosF& pos) const {
	DrawOptions adjusted_options(options);
	BBoxF region(options.draw_region);
	if (region == BBoxF()) {
		region = fullimagebounds(_image);
	}

	adjusted_options.draw_region = region.subregion(
			_draw_region.scaled(1.0f / _image->width, 1.0f / _image->height));
	adjusted_options.draw_frame = 0; // GLImage does not accept this

	if (!_rotates) {
		adjusted_options.draw_angle = 0.0f;
	}

	return _image->draw(adjusted_options, pos);
}

void Image::from_bytes(const BBox& region, char* data) {
	LDRAW_ASSERT(!_image.empty());
	_image->subimage_from_bytes(region, data);

}

void Image::from_bytes(const Size& size, char* data) {
	if (_image.empty()) {
		_image = smartptr<GLImage>(new GLImage(size));
	}
	_image->image_from_bytes(size, data);
	_draw_region = BBoxF(0, 0, _image->width, _image->height);
}

std::vector<Image> image_split(const Image & image, const SizeF & size) {
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
