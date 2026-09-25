# C++ <-> Lua binding helpers
file(GLOB luawrap_sources CONFIGURE_DEPENDS src/*.cpp)
add_library(luawrap STATIC ${luawrap_sources})
target_include_directories(luawrap PUBLIC include)
target_link_libraries(luawrap PUBLIC lanarts::lua PRIVATE lanarts::warnings)
