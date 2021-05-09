/*
 * GLImage.cpp:
 *  A convenient OpenGL image wrapper
 */

#include <lcommon/math_util.h>
#include <lcommon/fatal_error.h>

#include "imgui.h"

#include <SDL_opengl.h>

//Surpress some multiple definition warnings:
#undef GL_GLEXT_VERSION
#include <SDL_image.h>

#include "ldraw_assert.h"

#include "GLImage.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct StbImage {
    GLuint texture;
    int width, height;
};

// Simple helper function to load an image into a OpenGL texture with common settings
StbImage load_texture(const char* filename) {
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == nullptr) {
        return {0, 0, 0};
    }

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    return {image_texture, image_width, image_height};
}
/* Utility function for conversion between SDL surfaces and GL surfaces */
//static GLuint SDL_GL_LoadTexture(SDL_Surface *surface, GLfloat *texcoord) {
//    load_texture()
//	GLuint texture;
//	int w, h;
//	SDL_Surface *image;
//	SDL_Rect area;
//	Uint32 saved_flags;
//	Uint8 saved_alpha;
//
//	/* Use the surface width and height expanded to powers of 2 */
//	w = power_of_two_round(surface->w);
//	h = power_of_two_round(surface->h);
//	texcoord[0] = 0.0f; /* Min X */
//	texcoord[1] = 0.0f; /* Min Y */
//	texcoord[2] = (GLfloat)surface->w / w; /* Max X */
//	texcoord[3] = (GLfloat)surface->h / h; /* Max Y */
//
//	image = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
//#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
//			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
//#else
//			0xFF000000,
//			0x00FF0000,
//			0x0000FF00,
//			0x000000FF
//#endif
//			);
//	if (image == NULL) {
//		return 0;
//	}
//
//	/* Save the alpha blending attributes */
//	SDL_GetSurfaceAlphaMod(surface, &saved_alpha);
//	SDL_SetSurfaceAlphaMod(surface, 255);
//        SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
//
//	/* Copy the surface into the GL texture image */
//	area.x = 0;
//	area.y = 0;
//	area.w = surface->w;
//	area.h = surface->h;
//	SDL_BlitSurface(surface, &area, image, &area);
//
//	/* Restore the alpha blending attributes */
//    SDL_SetSurfaceAlphaMod(surface, saved_alpha);
//
//	/* Create an OpenGL texture for the image */
//	glGenTextures(1, &texture);
//	glBindTexture(GL_TEXTURE_2D, texture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
//			image->pixels);
//	SDL_FreeSurface(image); /* No longer needed */
//
//	return texture;
//}

GLImage::~GLImage() {
	if (texture) {
		glDeleteTextures(1, &texture);
	}
}

void GLImage::initialize(const std::string& filename) {
	if (filename.empty() || texture != 0) {
		return;
	}

	/* Load the image using SDL_image library */
    StbImage image = load_texture(filename.c_str());
	if (image.texture == 0) {
		printf("Image '%s' could not be loaded\n", filename.c_str());
		fatal_error();
	}

	width = image.width;
	height = image.height;

	/* Convert the image into an OpenGL texture */
	GLfloat texcoord[4];
	texture = image.texture;
    texcoord[0] = 0.0f; /* Min X */
    texcoord[1] = 0.0f; /* Min Y */
    texcoord[2] = ((GLfloat)image.width) / power_of_two_round(image.width); /* Max X */
    texcoord[3] = ((GLfloat)image.height) / power_of_two_round(image.height); /* Max Y */
        texw = texh = 0;
	if (texture != NULL) {
                texw = texcoord[2];
                texh = texcoord[3];
        // If images cannot be loaded by OpenGL, error, except if in headless mode:
	} else if (getenv("LANARTS_HEADLESS") == nullptr) {
		printf("Texture from image '%s' (%dx%d) could not be loaded (%s)\n",
				filename.c_str(), width, height, SDL_GetError());
		fatal_error(); // Don't fatal error for now! TODO conditional on environment variable?
	}
}

static void gl_subimage_from_bytes(GLImage& img, const BBox& region, char* data,
		int type) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, img.texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexSubImage2D(GL_TEXTURE_2D, 0, region.x1, region.y1, region.width(),
			region.height(), type, GL_UNSIGNED_BYTE, data);
	glDisable(GL_TEXTURE_2D);
}

static void gl_image_from_bytes(GLImage& img, const Size& size, char* data,
		int type) {
	bool was_init = img.texture;
	// allocate a texture name
	if (!was_init)
		glGenTextures(1, &img.texture);

	int ptw = power_of_two_round(size.w), pth = power_of_two_round(size.h);
	ptw = std::max(4, ptw);
	pth = std::max(4, pth);

	img.width = size.w, img.height = size.h;
	img.texw = size.w / ((float)ptw);
	img.texh = size.h / ((float)pth);
	if (size.w == 0 || size.h == 0)
		return;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, img.texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	if (!was_init)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ptw, pth, 0, type,
				GL_UNSIGNED_BYTE, NULL);
	if (data) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.w, size.h, type,
				GL_UNSIGNED_BYTE, data);
	}
	glDisable(GL_TEXTURE_2D);
}

void GLImage::subimage_from_bytes(const BBox& region, char* data, int type) {
	gl_subimage_from_bytes(*this, region, data, type);
}

void GLImage::image_from_bytes(const Size& size, char* data, int type) {
	gl_image_from_bytes(*this, size, data, type);
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

	QuadF quad(adjusted, options.draw_angle);

	Colour colour = options.draw_colour.clamp();
    BBoxF region = draw_region.scaled(1.0f / width, 1.0f / height);
	QuadF q = quad.translated(pos);

//	ImGui::GetForegroundDrawList()->AddQuadFilled(
//		ImVec2(draw_region.x1, draw_region.y1),
//		ImVec2(draw_region.x2, draw_region.y1),
//		ImVec2(draw_region.x2, draw_region.y2),
//		ImVec2(draw_region.x1, draw_region.y2),
//		colour.as_rgba()
//	);
//
//	ImGui::GetForegroundDrawList()->AddImage(
//         (void*)(unsigned long)texture,
//         ImVec2(draw_region.x1, draw_region.y1),
//         ImVec2(draw_region.x2, draw_region.y2)
//    );
    //printf("[%f %f %f %f]\n", q.pos[0].x, q.pos[0].y, q.pos[3].x, q.pos[3].y);
	ImGui::GetForegroundDrawList()->AddImageQuad(
		(void*)(unsigned long)texture,
        ImVec2(q.pos[0].x, q.pos[0].y),
        ImVec2(q.pos[1].x, q.pos[1].y),
        ImVec2(q.pos[2].x, q.pos[2].y),
        ImVec2(q.pos[3].x, q.pos[3].y),
		ImVec2(region.x1, region.y1),
		ImVec2(region.x2, region.y1),
		ImVec2(region.x2, region.y2),
		ImVec2(region.x1, region.y2),
		colour.as_rgba()
	);
//	gl_draw_image(texture, options.draw_colour.clamp(),
//			draw_region.scaled(texw / width, texh / height),
//			quad.translated(pos));
}