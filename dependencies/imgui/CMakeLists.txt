cmake_minimum_required(VERSION 2.8.12)
project(imgui)


set(root ${CMAKE_CURRENT_SOURCE_DIR})
set(phdrs ${root}/imgui.h)
set(hdrs ${root}/stb_rect_pack.h ${root}/stb_textedit.h ${root}/stb_truetype.h)
set(srcs ${root}/imgui.cpp  ${root}/backends/imgui_impl_sdl.cpp ${root}/imgui_draw.cpp  ${root}/imgui_tables.cpp  ${root}/imgui_widgets.cpp  ${root}/backends/imgui_impl_opengl3.cpp)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
else()
    set(srcs ${srcs} ${root}/GL/gl3w.c)
endif()

add_library(imgui STATIC ${srcs})
set(IMGUI_SOURCES ${srcs})

include_directories(imgui 
    ${root}
    "${root}/../sdl2/include"
)
add_definitions(-DIMGUI_DISABLE_INCLUDE_IMCONFIG_H)

