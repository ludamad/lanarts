/*
 * GLImage.h:
 *  A convenient OpenGL image wrapper
 */

#ifndef GLIMAGE_H_
#define GLIMAGE_H_

#include <string>

#include <lcommon/geometry.h>

#include "DrawOptions.h"

struct GLImage {
	GLImage() {
	}
	GLImage(const std::string& filename) {
		initialize(filename);
	}
	GLImage(const Size& size) {
		initialize(size);
	}
	~GLImage();

	void initialize(const std::string& filename);
	void initialize(const Size& size) {
		image_from_bytes(size, NULL);
	}

	void subimage_from_bytes(const BBox& region, char* data);
	void image_from_bytes(const Size& size, char* data);

	void draw(const ldraw::DrawOptions& options, const PosF& pos);
	void draw(const PosF& pos) {
		draw(ldraw::DrawOptions(), pos);
	}
	static void start_batch_draw();
	void batch_draw(const BBoxF& region, const PosF& pos);
        static void end_batch_draw();

	int width, height;
};

#endif /* GLIMAGE_H_ */
