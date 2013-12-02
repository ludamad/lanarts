cmake_minimum_required(VERSION 2.6)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/luajit)
    add_definitions(-DUSE_LUAJIT)
    set(USE_LUAJIT 1)
endif ()

set(CMAKE_BUILD_TYPE RELEASE)
add_definitions(-DNDEBUG)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-mfpmath=sse -msse2 -ffp-contract=on")
    #add_definitions(-Wuninitialized -fsanitize=address)
    #    SET (CMAKE_EXE_LINKER_FLAGS -fsanitize=address)
endif()

if(WIN32)
    add_definitions("-DWIN32")

    if(CMAKE_COMPILER_IS_GNUCXX)
        add_definitions("-mwindows")
    endif()
    
endif()

option(MSVC_SHARED_RT "MSVC: Build with shared runtime libs (/MD)" ON)
option(MSVC_STHREADED_RT "MSVC: Build with single-threaded static runtime libs (/ML until VS .NET 2003)" OFF)

# Enable tests
enable_testing()

add_subdirectory( dependencies ) 

add_subdirectory( luawrap-lib )
add_subdirectory( common-lib )
add_subdirectory( dungeon-generation-lib )
add_subdirectory( draw-lib )
add_subdirectory( net-lib )
add_subdirectory( sound-lib )

add_subdirectory( src )
