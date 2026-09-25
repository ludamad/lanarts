# GNU libxmi 1.2 (polygon rasterisation for the dungeon generator); unmaintained upstream
file(GLOB libxmi_sources CONFIGURE_DEPENDS *.c)
add_library(libxmi STATIC ${libxmi_sources})
target_include_directories(libxmi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
