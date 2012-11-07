/*
 * Image.h:
 *  Allows defining an image as a region of a shared GLImage
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include <common/geometry.h>
#include <common/smartptr.h>

#include "opengl/GLImage.h"

namespace ldraw {

class Image {
public:
	Image();
	float width() {
		return _draw_region.width();
	}
	float height() {
		return _draw_region.height();
	}
	DimF size() {
		return _draw_region.size();
	}
	void draw(const DrawOptions& options = DrawOptions());
private:
	smartptr<GLImage> _image;
	BBoxF _draw_region;
};

}

#endif /* IMAGE_H_ */
