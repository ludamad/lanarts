cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

#add_definitions(-DNDEBUG)

set(CMAKE_ECLIPSE_MAKE_ARGUMENTS -j4)

option(MSVC_SHARED_RT "MSVC: Build with shared runtime libs (/MD)" ON)
option(MSVC_STHREADED_RT "MSVC: Build with single-threaded static runtime libs (/ML until VS .NET 2003)" OFF)

add_subdirectory( src ) 
add_subdirectory( libs ) 
