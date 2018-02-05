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
#include "core/gl/VideoDriverGLES20.h"
#include "core/gl/oxgl.h"
#include "STDRenderer.h"
#include "STDMaterial.h"

static SDL_Window* MAIN_WINDOW = nullptr;
static Size RENDER_SIZE;
static SDL_Renderer* MAIN_RENDERER = nullptr;
static oxygine::IVideoDriver* oxygine_video_driver = nullptr;
static oxygine::STDRenderer* oxygine_renderer = nullptr;


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
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    //SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

//    SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 0);

//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);

//    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    int flags = SDL_WINDOW_OPENGL;
    initGLExtensions(SDL_GL_GetProcAddress);

    if (MAIN_WINDOW == nullptr){

        SDL_CreateWindowAndRenderer(
                w, h,
                SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0),
                &MAIN_WINDOW,
                &MAIN_RENDERER
        );
        SDL_SetWindowTitle(MAIN_WINDOW, window_name);

    }   
    if (MAIN_WINDOW == nullptr) {
        fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    fflush(stdout);

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

    oxygine_video_driver = new oxygine::VideoDriverGLES20();
    oxygine::IVideoDriver::instance = oxygine_video_driver;
    oxygine_renderer = new oxygine::STDRenderer(oxygine_video_driver);
    oxygine::STDRenderer::instance = oxygine_renderer;
    oxygine::STDMaterial::instance = new oxygine::STDMaterial(oxygine_renderer);
    oxygine::IVideoDriver::IVideoDriver::instance->setDefaultSettings();
    oxygine::STDRenderer::initialize();
    glShadeModel(GL_SMOOTH);
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
    oxygine::STDRenderer::instance->begin(nullptr);
    auto viewport = oxygine::Rect(
            0, 0, RENDER_SIZE.w, RENDER_SIZE.h
    );
    oxygine_video_driver->setViewport(viewport);
    oxygine_video_driver->clear(oxygine::Color(0,0,0));
    oxygine::Material::setCurrent(nullptr);


    oxygine::STDMaterial& mat = *oxygine::STDMaterial::instance;
    mat.apply(0);

    oxygine::Matrix proj;
    oxygine::Matrix::orthoLH(proj, (float)viewport.getWidth(), (float)viewport.getHeight(), 0.2f, 10000);
    oxygine::Matrix view = oxygine::makeViewMatrix(viewport.getWidth(), viewport.getHeight());
    oxygine::Matrix vp = view * proj;
    mat.setViewProj(vp);
}

void ldraw::display_draw_finish() {
    oxygine::STDRenderer::instance->end();
    oxygine::STDRenderer::instance->drawBatch();
    oxygine::STDMaterial::instance->finish();

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
