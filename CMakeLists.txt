# Top-level cmake file. All settings are inherited by children cmake files.
# Recursively includes all children cmake files using add_subdirectory(...)

cmake_minimum_required(VERSION 2.6)

if ($ENV{BUILD_OPTIMIZE})
    message("Building release... no debug symbols will be available.")
    set(CMAKE_BUILD_TYPE Release)
else()
    message("Building debug... performance will not be optimal.")
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (NOT $ENV{BUILD_NO_LUAJIT})
    add_definitions(-DUSE_LUAJIT)
    set(USE_LUAJIT 1)
endif ()

if ($ENV{BUILD_FLAGS})
    add_definitions ($ENV{BUILD_FLAGS})
endif()

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-mfpmath=sse -msse2 -ffp-contract=on")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if ($ENV{BUILD_SANITIZE})
        SET (CMAKE_EXE_LINKER_FLAGS
            "-fsanitize=address"
        )
        SET (CMAKE_EXE_LINKER_FLAGS
            "-fsanitize=undefined"
        )
        add_definitions(
          "-fsanitize=address"  
        )
        add_definitions(
          "-fsanitize=undefined"  
          )
    endif()
else()
    add_definitions(-DNDEBUG)
    if ($ENV{BUILD_PROF_GEN})
        SET (CMAKE_EXE_LINKER_FLAGS
           -fprofile-generate
        )
        add_definitions(
           -fprofile-generate
        )
    elseif( $ENV{BUILD_PROF_USE})
        SET (CMAKE_EXE_LINKER_FLAGS
           -fprofile-use
        )
        add_definitions(
           -fprofile-use
        )
    endif()
endif()

if(WIN32)
    add_definitions("-DWIN32")
    if(CMAKE_COMPILER_IS_GNUCXX)
        add_definitions("-mwindows")
    endif()
endif()

# Enable tests
enable_testing()

add_subdirectory( dependencies ) 
add_subdirectory( libs ) 
add_subdirectory( src )
