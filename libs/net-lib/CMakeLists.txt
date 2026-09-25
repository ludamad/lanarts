# Client/server connections over ENet
file(GLOB lanarts_net_sources CONFIGURE_DEPENDS src/*.cpp)
add_library(lanarts_net STATIC ${lanarts_net_sources})
target_include_directories(lanarts_net PUBLIC include)
target_link_libraries(lanarts_net PUBLIC lcommon PRIVATE enet lanarts::warnings)
