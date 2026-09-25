# Warnings the older lanarts code triggers on modern compilers; applied privately to each library.
add_library(lanarts_warnings INTERFACE)
target_compile_options(lanarts_warnings INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-c++11-narrowing>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-narrowing>
    -Wno-deprecated-declarations)
add_library(lanarts::warnings ALIAS lanarts_warnings)

add_subdirectory(luawrap-lib)
add_subdirectory(common-lib)
if(LANARTS_HEADLESS)
    add_subdirectory(draw-lib-headless)
else()
    add_subdirectory(draw-lib)
endif()
add_subdirectory(dungeon-generation-lib)
add_subdirectory(net-lib)
add_subdirectory(sound-lib)
