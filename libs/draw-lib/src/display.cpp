/*
 * display.cpp:
 *  Control display options, and drawing operation start/end.
 */

#include <SDL.h>

#undef GL_GLEXT_VERSION
#ifndef __EMSCRIPTEN__
#include <GL/gl3w.h>
#endif
#include <SDL_opengl.h>

#include <lcommon/geometry.h>
#include <lcommon/perf_timer.h>

#include "display.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"

#include "opengl/gl_extensions.h"

static SDL_Window *MAIN_WINDOW = NULL;
static SDL_GLContext MAIN_GL_CONTEXT = NULL;
static Size RENDER_SIZE;
//
//static
//ImVec2          DisplayPos;             // Top-left position of the viewport to render (== top-left of the orthogonal projection matrix to use) (== GetMainViewport()->Pos for the main viewport, == (0.0) in most single-viewport applications)
//ImVec2          DisplaySize;

////Set up the coordinate system x1 -> x2, y2 -> y1
//static void gl_set_world_region(double x1, double y1, double x2, double y2) {
//    //Set projection
//    glMatrixMode(GL_PROJECTION);
//    //glPushMatrix();
//    glLoadIdentity();
//    glOrtho(x1, x2, y2, y1, 0.0, 1.0);
//}
//
//static void gl_set_window_region(int x, int y, int wa, int ha) {
//    int w = 0, h = 0;
//    SDL_GetWindowSize(MAIN_WINDOW, &w, &h);
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    glViewport(x, h - ha - y, wa, ha);
//    gl_set_world_region(0, 0, wa, ha);
//}
//

static void gl_sdl_initialize(const char *window_name, int w, int h, bool fullscreen) {
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    if (MAIN_WINDOW == NULL) {

        // SDL_CreateWindowAndRenderer(
        //         w, h,
        //         SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0),
        //         &MAIN_WINDOW,
        //         &MAIN_RENDERER
        // );
        MAIN_WINDOW = SDL_CreateWindow(
                window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h,
                SDL_WINDOW_OPENGL | (fullscreen ? SDL_WINDOW_FULLSCREEN /*_DESKTOP*/ : 0)
        );
        // Create an OpenGL context associated with the window.
        MAIN_GL_CONTEXT = SDL_GL_CreateContext(MAIN_WINDOW);
        SDL_GL_MakeCurrent(MAIN_WINDOW, MAIN_GL_CONTEXT);
        // Enable vsync with catchup
        if (SDL_GL_SetSwapInterval(-1) != 0) {
            // Enable vsync without catchup if not supported
//            SDL_GL_SetSwapInterval(1);
            SDL_GL_SetSwapInterval(0);
        }
    }
    if (MAIN_WINDOW == NULL) {
        fprintf(stderr, "Couldn't set GL mode: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    // Initialize OpenGL loader
#ifndef __EMSCRIPTEN__
    bool err = gl3wInit();
#else
    bool err = false; // Initialization not needed for emscripten
#endif
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // TODO how to handle this properly? have to defer control to imguiio
//    ImGuiIO &io = ImGui::GetIO();
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(MAIN_WINDOW, MAIN_GL_CONTEXT);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

//// Set up sane 2D drawing defaults
//static void _gl_sdl_initialize(const char *window_name, int w, int h, bool fullscreen) {
////    if (sdl_gl_init() < 0) {
////        return;
////    }
//    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//        return;
//    }
//
//    glDisable(GL_TEXTURE_2D);
//
//    if (!gl_set_vsync(false)) {
//        printf("Disabling vsync not supported, please do this in graphics card settings for best performance.\n");
//    }
//
//    gl_set_window_region(0, 0, w, h);
//
//    /* This allows alpha blending of 2D textures with the scene */
//    glEnable(GL_BLEND);
//    //glEnable(GL_TEXTURE_2D);
//    glDisable(GL_DEPTH_TEST);
//
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//    //Set up the coordinate system 0 -> w, 0 -> h
//    gl_set_world_region(0.0, 0.0, (GLdouble) RENDER_SIZE.w, (GLdouble) RENDER_SIZE.h);
//}

static void gl_set_fullscreen(bool fullscreen) {
    Uint32 flags = SDL_GetWindowFlags(MAIN_WINDOW);
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else {
        flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    SDL_SetWindowFullscreen(MAIN_WINDOW, flags);
}

void ldraw::display_initialize(const char *window_name,
                               const Size &draw_area_size, bool fullscreen) {
    ////TODO: Allow passing video flags, esp. SDL_NOFRAME
    RENDER_SIZE = draw_area_size;
    gl_sdl_initialize(window_name, RENDER_SIZE.w, RENDER_SIZE.h,
                      fullscreen);
}

void ldraw::display_set_fullscreen(bool fullscreen) {
    gl_set_fullscreen(fullscreen);
}

void ldraw::display_set_window_region(const BBoxF &bbox) {
//    gl_set_window_region(bbox.x1, bbox.y1, bbox.width(), bbox.height());
}

void ldraw::display_set_world_region(const BBoxF &bbox) {
//    gl_set_world_region(bbox.x1, bbox.y1, bbox.x2, bbox.y2);
}

bool ldraw::display_is_fullscreen() {
    return (SDL_GetWindowFlags(MAIN_WINDOW) & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN)) != 0;
}

void ldraw::display_draw_start() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(MAIN_WINDOW);
    ImGui::NewFrame();
}

void ldraw::display_draw_finish() {
    // Rendering
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(MAIN_WINDOW);
}

Size ldraw::screen_size() {
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    // Get screen size:
    int w = display_mode.w, h = display_mode.h;
    return Size(w, h);
}

Size ldraw::display_size() {
    return RENDER_SIZE;
}

// Solely for when box2d clobbers it
void ldraw::reset_blend_func() {
//    /* This allows alpha blending of 2D textures with the scene */
//    glEnable(GL_BLEND);
//    //glEnable(GL_TEXTURE_2D);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

Size ldraw::window_size() {
    int w = 0, h = 0;
    SDL_GetWindowSize(MAIN_WINDOW, &w, &h);
    return Size(w, h);
}

void ldraw::process_event(const SDL_Event* event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}