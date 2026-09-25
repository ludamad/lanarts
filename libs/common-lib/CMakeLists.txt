# Shared utilities: geometry, timers, serialisation, random numbers
file(GLOB lcommon_sources CONFIGURE_DEPENDS src/*.cpp src/lua/*.cpp)
add_library(lcommon STATIC ${lcommon_sources})
target_include_directories(lcommon PUBLIC include PRIVATE include/lcommon)
target_link_libraries(lcommon PUBLIC luawrap lanarts::sdl UnitTest++ PRIVATE lanarts::warnings)
# Headless builds replace the SDL calls with stubs (lcommon/sdl_headless_support.h)
target_compile_definitions(lcommon PUBLIC $<$<BOOL:${LANARTS_HEADLESS}>:BUILD_HEADLESS>)
