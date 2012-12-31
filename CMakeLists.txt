cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
add_definitions(-DNDEBUG)

option(MSVC_SHARED_RT "MSVC: Build with shared runtime libs (/MD)" ON)
option(MSVC_STHREADED_RT "MSVC: Build with single-threaded static runtime libs (/ML until VS .NET 2003)" OFF)

add_subdirectory( src ) 
add_subdirectory( libs ) 

# Enable tests
enable_testing()
