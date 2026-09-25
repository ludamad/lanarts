# Platform libraries: SDL2 (+ image, mixer), OpenGL and FreeType.
#
# Desktop builds use the system packages (Homebrew, apt, MSYS2 ...). Emscripten builds use the
# matching emscripten ports. Headless builds only need the SDL headers, for SDL types that appear
# in shared code; nothing from SDL is linked.
#
# Targets defined here:
#   lanarts::sdl       SDL2, SDL2_image and SDL2_mixer
#   lanarts::opengl    OpenGL (also linked when headless: lua-box2d's debug drawer uses it)
#   lanarts::freetype  FreeType (empty when headless)

add_library(lanarts_sdl INTERFACE)
add_library(lanarts::sdl ALIAS lanarts_sdl)
add_library(lanarts_opengl INTERFACE)
add_library(lanarts::opengl ALIAS lanarts_opengl)
add_library(lanarts_freetype INTERFACE)
add_library(lanarts::freetype ALIAS lanarts_freetype)

if(EMSCRIPTEN)
    set(_ports -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_MIXER=2)
    target_compile_options(lanarts_sdl INTERFACE ${_ports})
    target_link_options(lanarts_sdl INTERFACE ${_ports} "-sSDL2_IMAGE_FORMATS=[\"png\"]")
    target_compile_options(lanarts_freetype INTERFACE -sUSE_FREETYPE=1)
    target_link_options(lanarts_freetype INTERFACE -sUSE_FREETYPE=1)
    return()
endif()

find_package(OpenGL REQUIRED)
target_link_libraries(lanarts_opengl INTERFACE OpenGL::GL)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

if(LANARTS_HEADLESS)
    target_include_directories(lanarts_sdl INTERFACE
        $<TARGET_PROPERTY:SDL2::SDL2,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:SDL2_image::SDL2_image,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:SDL2_mixer::SDL2_mixer,INTERFACE_INCLUDE_DIRECTORIES>)
    return()
endif()

target_link_libraries(lanarts_sdl INTERFACE SDL2::SDL2 SDL2_image::SDL2_image SDL2_mixer::SDL2_mixer)
if(TARGET SDL2::SDL2main)
    target_link_libraries(lanarts_sdl INTERFACE SDL2::SDL2main)
endif()

# Prefer the system FreeType; build a pinned release when there is none.
include(FetchContent)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
FetchContent_Declare(freetype
    URL https://download.savannah.gnu.org/releases/freetype/freetype-2.14.3.tar.xz
    URL_HASH SHA256=36bc4f1cc413335368ee656c42afca65c5a3987e8768cc28cf11ba775e785a5f
    FIND_PACKAGE_ARGS NAMES Freetype)
FetchContent_MakeAvailable(freetype)
if(TARGET Freetype::Freetype)
    target_link_libraries(lanarts_freetype INTERFACE Freetype::Freetype)
else()
    target_link_libraries(lanarts_freetype INTERFACE freetype)
endif()
