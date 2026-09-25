# LuaJIT 2.1 (rolling release, pinned by commit), built with its own makefile and exposed as
# lanarts::lua. Not available for Emscripten or MSVC.
include(ExternalProject)

if(EMSCRIPTEN)
    message(FATAL_ERROR "LANARTS_LUAJIT is not supported for Emscripten builds")
endif()

set(_luajit_env "")
if(APPLE)
    # LuaJIT's makefile refuses to build for macOS without a deployment target
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
        set(_luajit_env ${CMAKE_COMMAND} -E env MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
    else()
        set(_luajit_env ${CMAKE_COMMAND} -E env MACOSX_DEPLOYMENT_TARGET=11.0)
    endif()
endif()

ExternalProject_Add(luajit_build
    URL https://github.com/LuaJIT/LuaJIT/archive/c6ffc141a8762b41703f9287d63d93622a13dd8f.tar.gz
    URL_HASH SHA256=6e5fec07750add912e7c3eae0c194d24cd6d023714e1f04a0298a5b4819e4457
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE ON
    BUILD_COMMAND ${_luajit_env} make -C src libluajit.a CC=${CMAKE_C_COMPILER} BUILDMODE=static
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS <SOURCE_DIR>/src/libluajit.a)
ExternalProject_Get_Property(luajit_build SOURCE_DIR)

# The include directory must exist at configure time for the imported target
file(MAKE_DIRECTORY ${SOURCE_DIR}/src)
add_library(luajit STATIC IMPORTED GLOBAL)
set_target_properties(luajit PROPERTIES
    IMPORTED_LOCATION ${SOURCE_DIR}/src/libluajit.a
    INTERFACE_INCLUDE_DIRECTORIES ${SOURCE_DIR}/src
    INTERFACE_COMPILE_DEFINITIONS USE_LUAJIT
    INTERFACE_LINK_LIBRARIES "${CMAKE_DL_LIBS}")
add_dependencies(luajit luajit_build)
add_library(lanarts::lua ALIAS luajit)
