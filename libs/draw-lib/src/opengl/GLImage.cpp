/*
 * GLImage.cpp:
 *  A convenient OpenGL image wrapper
 */

#include <lcommon/math_util.h>
#include <lcommon/fatal_error.h>
#include <stdexcept>
#include <fstream>

#ifdef WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

#include "GLImage.h"

GLImage::~GLImage() {
}

static Size quick_and_dirty_image_size(std::string fn) {
	if(fn.substr(fn.find_last_of(".") + 1) == "png") {
		std::ifstream in(fn);
		unsigned int width, height;

		in.seekg(16);
		in.read((char *)&width, 4);
		in.read((char *)&height, 4);

		width = ntohl(width);
		height = ntohl(height);
		return Size {width, height};

	}
	throw std::runtime_error("Cannot find image size of non-png!");
}

void GLImage::initialize(const std::string& filename) {
	if (filename.empty()) {
		return;
	}

	/* Load the image using SDL_image library */
	Size size = quick_and_dirty_image_size(filename);

	width = size.w;
	height = size.h;
}

static void gl_image_from_bytes(GLImage& img, const Size& size, char* data) {
	img.width = size.w, img.height = size.h;
}

void GLImage::subimage_from_bytes(const BBox& region, char* data) {
}

void GLImage::image_from_bytes(const Size& size, char* data) {
	gl_image_from_bytes(*this, size, data);
}

void GLImage::draw(const ldraw::DrawOptions& options, const PosF& pos) {
}


void GLImage::start_batch_draw() {
}

void GLImage::batch_draw(const BBoxF& bbox, const PosF& pos) {
}

void GLImage::end_batch_draw() {
}
