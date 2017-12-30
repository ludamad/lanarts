/*
 * display.cpp:
 *  Control display options, and drawing operation start/end.
 */

#undef GL_GLEXT_VERSION

#include <lcommon/geometry.h>

#include "display.h"

static Size RENDER_SIZE;
static Size WINDOW_SIZE;
static bool IS_FULLSCREEN;

//Set up the coordinate system x1 -> x2, y2 -> y1
static void gl_set_world_region(double x1, double y1, double x2, double y2) {
}

static void gl_set_window_region(int x, int y, int wa, int ha) {
}


// Set up sane 2D drawing defaults
static void gl_sdl_initialize(const char* window_name, int w, int h, bool fullscreen) {
    WINDOW_SIZE = Size {w,h};
}

static void gl_set_fullscreen(bool fullscreen) {
    IS_FULLSCREEN = fullscreen;
}

void ldraw::display_initialize(const char* window_name,
        const Size& draw_area_size, bool fullscreen) {
    WINDOW_SIZE = draw_area_size;
    IS_FULLSCREEN = fullscreen;
}

void ldraw::display_set_fullscreen(bool fullscreen) {
    gl_set_fullscreen(fullscreen);
}

void ldraw::display_set_window_region(const BBoxF & bbox) {
    gl_set_window_region(bbox.x1, bbox.y1, bbox.width(), bbox.height());
}

void ldraw::display_set_world_region(const BBoxF & bbox) {
    gl_set_world_region(bbox.x1, bbox.y1, bbox.x2, bbox.y2);
}

bool ldraw::display_is_fullscreen() {
    return IS_FULLSCREEN;
}

void ldraw::display_draw_start() {
}
void ldraw::display_draw_finish() {
}

Size ldraw::screen_size() {
    return WINDOW_SIZE;
}
Size ldraw::display_size() {
    return RENDER_SIZE;
}

// Solely for when box2d clobbers it
void ldraw::reset_blend_func() {
}

Size ldraw::window_size() {
    return WINDOW_SIZE;
}
