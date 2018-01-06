# Top-level cmake file. All settings are inherited by children cmake files.
# Recursively includes all children cmake files using add_subdirectory(...)

cmake_minimum_required(VERSION 2.6)

if ($ENV{BUILD_OPTIMIZE})
    message("Building release... no debug symbols will be available.")
    set(CMAKE_BUILD_TYPE Relwithdebinfo)
    #set(CMAKE_BUILD_TYPE Release)
else()
    message("Building debug... performance will not be optimal.")
    set(CMAKE_BUILD_TYPE Debug)
endif()

if ($ENV{BUILD_LUAJIT})
    add_definitions(-DUSE_LUAJIT)
    message("Using LuaJIT, performance will increase, debugging (especially C++ stack traces) will suffer.")
    set(USE_LUAJIT 1)
else ()
    message("Using Lua 5.1, use -lj to use luajit if raw Lua performance is inadequate.")
endif ()

if ($ENV{BUILD_FLAGS})
    add_definitions ($ENV{BUILD_FLAGS})
endif()

add_definitions(-static-libstdc++)
## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-mfpmath=sse -msse2 -ffp-contract=on")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if ($ENV{BUILD_SANITIZE})
        SET (CMAKE_EXE_LINKER_FLAGS
            "-fsanitize=address -fsanitize=undefined"
        )
        add_definitions(
          "-fsanitize=address"  
        )
        add_definitions(
          "-fsanitize=undefined"  
          )
      endif()
else()
    #add_definitions(-DNDEBUG -g)
    add_definitions(-g)
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
     SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Enable tests
enable_testing()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s ALLOW_MEMORY_GROWTH=1 -s LEGACY_GL_EMULATION=1 ")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/runtime@/")
endif()


add_subdirectory( dependencies ) 
add_subdirectory( libs ) 
add_subdirectory( src )

