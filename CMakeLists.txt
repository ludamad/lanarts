cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)


option(MSVC_SHARED_RT "MSVC: Build with shared runtime libs (/MD)" ON)
option(MSVC_STHREADED_RT "MSVC: Build with single-threaded static runtime libs (/ML until VS .NET 2003)" OFF)

add_subdirectory( src ) 
add_subdirectory( src_yaml/yaml-cpp )
add_subdirectory( src_rvo2/RVO2/rvo2 )
add_subdirectory( src_net/net )
add_subdirectory( src_lua/lua )
