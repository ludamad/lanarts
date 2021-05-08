cmake_minimum_required(VERSION 2.8.12)
project(imgui)


set(root ${CMAKE_CURRENT_SOURCE_DIR})
set(phdrs ${root}/imgui.h)
set(hdrs ${root}/stb_rect_pack.h ${root}/stb_textedit.h ${root}/stb_truetype.h)
set(srcs ${root}/imgui.cpp)
add_library(imgui STATIC ${srcs})
set(IMGUI_SOURCES ${srcs})

include_directories(imgui ${root})
add_definitions(-DIMGUI_DISABLE_INCLUDE_IMCONFIG_H)

