cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE DEBUG)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-mfpmath=sse -msse2 -ffp-contract=on")
endif()

option(MSVC_SHARED_RT "MSVC: Build with shared runtime libs (/MD)" ON)
option(MSVC_STHREADED_RT "MSVC: Build with single-threaded static runtime libs (/ML until VS .NET 2003)" OFF)

# Enable tests
enable_testing()

add_subdirectory( dependencies ) 

add_subdirectory( luawrap-lib )
add_subdirectory( common-lib )
add_subdirectory( draw-lib )
add_subdirectory( net-lib )
add_subdirectory( net-lib-udt )
add_subdirectory( sound-lib )

add_subdirectory( lanarts )
