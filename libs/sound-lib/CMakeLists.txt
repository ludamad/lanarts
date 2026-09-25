# Sound effects and music through SDL_mixer
file(GLOB lsound_sources CONFIGURE_DEPENDS src/*.cpp src/lua/*.cpp)
add_library(lsound STATIC ${lsound_sources})
target_include_directories(lsound PUBLIC include PRIVATE include/lsound)
target_link_libraries(lsound PUBLIC lcommon PRIVATE lanarts::warnings)
