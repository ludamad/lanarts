/*
 * display.cpp:
 *  Control display options, and drawing operation start/end.
 */

#include <SDL.h>
#undef GL_GLEXT_VERSION
#include <SDL_opengl.h>

#include <lcommon/geometry.h>
#include <lcommon/perf_timer.h>

#include "display.h"

#include "opengl/gl_extensions.h"

static SDL_Window* MAIN_WINDOW = NULL;
static Size RENDER_SIZE;
static SDL_Renderer* MAIN_RENDERER = NULL;

//Set up the coordinate system x1 -> x2, y2 -> y1
static void gl_set_world_region(double x1, double y1, double x2, double y2) {
    //Set projection
    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    glLoadIdentity();
    glOrtho(x1, x2, y2, y1, 0.0, 1.0);
}

static void gl_set_window_region(int x, int y, int wa, int ha) {
    int w = 0, h = 0;
    SDL_GetWindowSize(MAIN_WINDOW, &w, &h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(x, h - ha - y , wa, ha);
    gl_set_world_region(0, 0, wa, ha);
}


// Set up sane 2D drawing defaults
static void gl_sdl_initialize(const char* window_name, int w, int h, bool fullscreen) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return;
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    if (MAIN_WINDOW == NULL){

       // SDL_CreateWindowAndRenderer(
       //         w, h,
       //         SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0),
       //         &MAIN_WINDOW,
       //         &MAIN_RENDERER
       // );
      MAIN_WINDOW = SDL_CreateWindow(
        window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, 
        SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)
      );
      // Create an OpenGL context associated with the window.
      SDL_GL_CreateContext(MAIN_WINDOW);
    }   
    if (MAIN_WINDOW == NULL) {
        fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    glDisable(GL_TEXTURE_2D);

    if (!gl_set_vsync(false) ) {
        printf("Disabling vsync not supported, please do this in graphics card settings for best performance.\n");
    }

    gl_set_window_region(0, 0, w, h);

    /* This allows alpha blending of 2D textures with the scene */
    glEnable(GL_BLEND);
    //glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Set up the coordinate system 0 -> w, 0 -> h
    gl_set_world_region(0.0, 0.0, (GLdouble)RENDER_SIZE.w, (GLdouble)RENDER_SIZE.h);
}

static void gl_set_fullscreen(bool fullscreen) {
    Uint32 flags = SDL_GetWindowFlags(MAIN_WINDOW); 
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else {
        flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL_SetWindowFullscreen(MAIN_WINDOW, flags);
}

void ldraw::display_initialize(const char* window_name,
        const Size& draw_area_size, bool fullscreen) {
    ////TODO: Allow passing video flags, esp. SDL_NOFRAME
    RENDER_SIZE = draw_area_size;
    gl_sdl_initialize(window_name, RENDER_SIZE.w, RENDER_SIZE.h,
            fullscreen);
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
    return (SDL_GetWindowFlags(MAIN_WINDOW) & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN)) != 0;
}

void ldraw::display_draw_start() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void ldraw::display_draw_finish() {
    perf_timer_begin("SDL_GL_SwapBuffers");
    SDL_GL_SwapWindow(MAIN_WINDOW);
    perf_timer_end("SDL_GL_SwapBuffers");
}

Size ldraw::screen_size() {
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    // Get screen size:
    int w = display_mode.w, h = display_mode.h;
    return Size(w,h);
}
Size ldraw::display_size() {
    return RENDER_SIZE;
}

// Solely for when box2d clobbers it
void ldraw::reset_blend_func() {
    /* This allows alpha blending of 2D textures with the scene */
    glEnable(GL_BLEND);
    //glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Size ldraw::window_size() {
    int w = 0, h = 0;
    SDL_GetWindowSize(MAIN_WINDOW, &w, &h);
    return Size(w,h);
}
