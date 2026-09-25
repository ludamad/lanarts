cmake_minimum_required(VERSION 3.24)
project(lanarts LANGUAGES C CXX)

option(LANARTS_HEADLESS "Build without graphics, sound or input, for simulations and tests" OFF)
option(LANARTS_LUAJIT "Use LuaJIT instead of the bundled Lua 5.1" OFF)
option(LANARTS_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(LANARTS_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# Same floating point on every x86 machine, so networked games stay in sync
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86|AMD64|i.86")
    add_compile_options(-mfpmath=sse -msse2 -ffp-contract=on)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(LanartsDependencies)

enable_testing()

add_subdirectory(dependencies)
add_subdirectory(libs)
add_subdirectory(src)
