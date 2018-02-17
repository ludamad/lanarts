/*
 * GLImage.cpp:
 *  A convenient OpenGL image wrapper
 */

#include <lcommon/math_util.h>
#include <lcommon/fatal_error.h>

#include <SDL_opengl.h>

//Surpress some multiple definition warnings:
#undef GL_GLEXT_VERSION
#include <SDL_image.h>

#include "ldraw_assert.h"

#include "GLImage.h"

#include "core/gl/VideoDriverGLES20.h"
#include "STDRenderer.h"
#include "STDMaterial.h"

GLImage::~GLImage() {
}

void GLImage::initialize(const std::string& filename) {
	if (filename.empty() || texture_) {
		return;
	}

	using namespace oxygine;
	file::buffer bf;
	file::read(filename, bf);
	Image mt;
	if (!mt.init(bf, true)) {
		printf("Texture from image '%s' (%dx%d) could not be loaded\n",
			   filename.c_str(), width, height);
		fatal_error(); // Don't fatal error for now! TODO conditional on environment variable?
	}
	texture_ = IVideoDriver::instance->createTexture();
	texture_->init(mt.lock(), false);
	texture_->apply();

	width = texture_->getWidth();
	height = texture_->getHeight();
}

static void gl_subimage_from_bytes(GLImage& img, const BBox& region, char* data,
		int type) {
	oxygine::ImageData image_data(region.width(), region.height(),
		region.width() * 4, oxygine::TF_R8G8B8A8, data);
	img.texture_->updateRegion(region.x1, region.y1, image_data);
}

void GLImage::subimage_from_bytes(const BBox& region, char* data, int type) {
	gl_subimage_from_bytes(*this, region, data, type);
}

void GLImage::image_from_bytes(const Size& size, char* data, int type) {
	oxygine::ImageData image_data(size.w, size.h, size.w * 4, oxygine::TF_R8G8B8A8, data);
	using namespace oxygine;
	texture_ = IVideoDriver::instance->createTexture();
	texture_->init(image_data);
	texture_->apply();

	width = texture_->getWidth();
	height = texture_->getHeight();
}

void GLImage::draw(const ldraw::DrawOptions& options, const PosF& pos) {
	BBoxF draw_region(0, 0, width, height);

	//Assert so unused settings don't pass through silently
	LDRAW_ASSERT(options.draw_frame == 0.0f);

	if (!options.draw_region.empty()) {
		LDRAW_ASSERT(
				options.draw_region.x1 >= 0 && options.draw_region.y1 >= 0);
		LDRAW_ASSERT(options.draw_region.x2 <= width);
		LDRAW_ASSERT(options.draw_region.y2 <= height);

		draw_region = options.draw_region;
	}
	if (draw_region.empty()) {
		return;
	}

	BBoxF adjusted = adjusted_for_origin(BBoxF(PosF(), draw_region.size()),
										 options.draw_origin).scaled(options.draw_scale);

	oxygine::RenderState rs;
	rs.material = oxygine::STDMaterial::instance;
	if (options.draw_angle) {
		rs.transform.rotate(options.draw_angle);
	}
	oxygine::RectF clip = oxygine::RectF::huge();
	rs.clip = &clip;
	auto colour = options.draw_colour.clamp();
	BBoxF img_region = draw_region.scaled(1.0 / width, 1.0 / height);
	BBoxF region = adjusted.translated(pos);
	oxygine::STDMaterial::instance->doRender(texture_, oxygine::Color(colour.r, colour.g, colour.b, colour.a),
											 oxygine::blend_alpha,
											 oxygine::RectF(img_region.x1, img_region.y1, img_region.width(),
															img_region.height()),
											 oxygine::RectF(region.x1, region.y1, region.width(),
															region.height()), rs);

//	oxygine::STDRenderer::instance->drawBatch();
}


void GLImage::start_batch_draw() {
}

void GLImage::batch_draw(const BBoxF& bbox, const PosF& pos) {
	draw(bbox, pos);
}