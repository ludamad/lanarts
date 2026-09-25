# Third-party libraries.
#
# Upstream releases are fetched at configure time, pinned by SHA-256. Most of them either ship no
# CMake build or one that modern CMake rejects, so only their sources are fetched and the targets
# are declared here. What stays in this directory is either patched for lanarts (lua, lua-yaml,
# lua-box2d, rvo2) or no longer maintained upstream (libxmi).

include(FetchContent)

# Fetches sources only; the targets are declared below. A macro, so <name>_SOURCE_DIR reaches the caller.
macro(lanarts_fetch_sources name url sha256)
    FetchContent_Declare(${name} URL ${url} URL_HASH SHA256=${sha256} SOURCE_SUBDIR no-cmake-build)
    FetchContent_MakeAvailable(${name})
endmacro()

# ---------------------------------------------------------------------------------------------
# Lua 5.1: the bundled copy, or LuaJIT. Both are exposed as lanarts::lua.
# ---------------------------------------------------------------------------------------------
if(LANARTS_LUAJIT)
    include(LanartsLuaJIT)
else()
    add_subdirectory(lua)
endif()

# ---------------------------------------------------------------------------------------------
# Lua modules
# ---------------------------------------------------------------------------------------------
lanarts_fetch_sources(lpeg
    https://www.inf.puc-rio.br/~roberto/lpeg/lpeg-1.1.0.tar.gz
    4b155d67d2246c1ffa7ad7bc466c1ea899bbc40fef0257cc9c03cecbaed4352a)
add_library(lpeg STATIC
    ${lpeg_SOURCE_DIR}/lpcap.c ${lpeg_SOURCE_DIR}/lpcode.c ${lpeg_SOURCE_DIR}/lpcset.c
    ${lpeg_SOURCE_DIR}/lpprint.c ${lpeg_SOURCE_DIR}/lptree.c ${lpeg_SOURCE_DIR}/lpvm.c)
target_link_libraries(lpeg PUBLIC lanarts::lua)

lanarts_fetch_sources(luafilesystem
    https://github.com/lunarmodules/luafilesystem/archive/refs/tags/v1_9_0.tar.gz
    1142c1876e999b3e28d1c236bf21ffd9b023018e336ac25120fb5373aade1450)
add_library(lfs STATIC ${luafilesystem_SOURCE_DIR}/src/lfs.c)
target_include_directories(lfs PUBLIC ${luafilesystem_SOURCE_DIR}/src)
target_link_libraries(lfs PUBLIC lanarts::lua)

lanarts_fetch_sources(luasocket
    https://github.com/lunarmodules/luasocket/archive/refs/tags/v3.1.0.tar.gz
    bf033aeb9e62bcaa8d007df68c119c966418e8c9ef7e4f2d7e96bddeca9cca6e)
set(_ls ${luasocket_SOURCE_DIR}/src)
# socket.core and mime.core; the Lua halves live in runtime/dependencies/socket
add_library(luasocket STATIC
    ${_ls}/luasocket.c ${_ls}/timeout.c ${_ls}/buffer.c ${_ls}/io.c ${_ls}/auxiliar.c
    ${_ls}/compat.c ${_ls}/options.c ${_ls}/inet.c ${_ls}/except.c ${_ls}/select.c
    ${_ls}/tcp.c ${_ls}/udp.c ${_ls}/mime.c
    $<IF:$<BOOL:${WIN32}>,${_ls}/wsocket.c,${_ls}/usocket.c>)
target_include_directories(luasocket PUBLIC ${_ls})
target_link_libraries(luasocket PUBLIC lanarts::lua $<$<BOOL:${WIN32}>:ws2_32>)

# ---------------------------------------------------------------------------------------------
# Networking
# ---------------------------------------------------------------------------------------------
lanarts_fetch_sources(enet
    https://github.com/lsalzman/enet/archive/refs/tags/v1.3.18.tar.gz
    28603c895f9ed24a846478180ee72c7376b39b4bb1287b73877e5eae7d96b0dd)
add_library(enet STATIC
    ${enet_SOURCE_DIR}/callbacks.c ${enet_SOURCE_DIR}/compress.c ${enet_SOURCE_DIR}/host.c
    ${enet_SOURCE_DIR}/list.c ${enet_SOURCE_DIR}/packet.c ${enet_SOURCE_DIR}/peer.c
    ${enet_SOURCE_DIR}/protocol.c ${enet_SOURCE_DIR}/unix.c ${enet_SOURCE_DIR}/win32.c)
target_include_directories(enet PUBLIC ${enet_SOURCE_DIR}/include)
if(WIN32)
    target_link_libraries(enet PUBLIC winmm ws2_32)
else()
    include(CheckSymbolExists)
    include(CheckStructHasMember)
    include(CheckTypeSize)
    check_symbol_exists(fcntl "fcntl.h" HAS_FCNTL)
    check_symbol_exists(poll "poll.h" HAS_POLL)
    check_symbol_exists(getaddrinfo "netdb.h" HAS_GETADDRINFO)
    check_symbol_exists(getnameinfo "netdb.h" HAS_GETNAMEINFO)
    check_symbol_exists(gethostbyname_r "netdb.h" HAS_GETHOSTBYNAME_R)
    check_symbol_exists(gethostbyaddr_r "netdb.h" HAS_GETHOSTBYADDR_R)
    check_symbol_exists(inet_pton "arpa/inet.h" HAS_INET_PTON)
    check_symbol_exists(inet_ntop "arpa/inet.h" HAS_INET_NTOP)
    check_struct_has_member("struct msghdr" msg_flags "sys/types.h;sys/socket.h" HAS_MSGHDR_FLAGS)
    set(CMAKE_EXTRA_INCLUDE_FILES sys/types.h sys/socket.h)
    check_type_size(socklen_t HAS_SOCKLEN_T BUILTIN_TYPES_ONLY)
    unset(CMAKE_EXTRA_INCLUDE_FILES)
    foreach(feature FCNTL POLL GETADDRINFO GETNAMEINFO GETHOSTBYNAME_R GETHOSTBYADDR_R
                    INET_PTON INET_NTOP MSGHDR_FLAGS SOCKLEN_T)
        if(HAS_${feature})
            target_compile_definitions(enet PRIVATE HAS_${feature}=1)
        endif()
    endforeach()
endif()

lanarts_fetch_sources(lua_enet
    https://github.com/leafo/lua-enet/archive/refs/tags/v1.2.tar.gz
    b46d4623d00be7514356fc20c2f580113b9b4264cebc932e5f4d071c244c7901)
add_library(lua_enet STATIC ${lua_enet_SOURCE_DIR}/enet.c)
target_link_libraries(lua_enet PUBLIC enet lanarts::lua)

# ---------------------------------------------------------------------------------------------
# C++ libraries
# ---------------------------------------------------------------------------------------------
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
set(YAML_CPP_FORMAT_SOURCE OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(yaml-cpp
    URL https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.9.0.tar.gz
    URL_HASH SHA256=25cb043240f828a8c51beb830569634bc7ac603978e0f69d6b63558dadefd49a)
FetchContent_MakeAvailable(yaml-cpp)

lanarts_fetch_sources(polypartition
    https://github.com/ivanfratric/polypartition/archive/b000a4a2a72b46e1305fb6e95b080448d7c12049.tar.gz
    d5370ea2dd03e3e3590fe94145bd01fc29dd5cbb01aca6d3443132385e4b89f6)
add_library(polypartition STATIC ${polypartition_SOURCE_DIR}/src/polypartition.cpp)
target_include_directories(polypartition PUBLIC ${polypartition_SOURCE_DIR}/src)

lanarts_fetch_sources(unittest_cpp
    https://github.com/unittest-cpp/unittest-cpp/archive/refs/tags/v2.0.0.tar.gz
    74852198877dc2fdebdc4e5e9bd074018bf8ee03a13de139bfe41f4585b2f5b9)
file(GLOB _utpp_sources CONFIGURE_DEPENDS ${unittest_cpp_SOURCE_DIR}/UnitTest++/*.cpp)
if(WIN32)
    file(GLOB _utpp_platform CONFIGURE_DEPENDS ${unittest_cpp_SOURCE_DIR}/UnitTest++/Win32/*.cpp)
else()
    file(GLOB _utpp_platform CONFIGURE_DEPENDS ${unittest_cpp_SOURCE_DIR}/UnitTest++/Posix/*.cpp)
endif()
add_library(UnitTest++ STATIC ${_utpp_sources} ${_utpp_platform})
target_include_directories(UnitTest++ PUBLIC ${unittest_cpp_SOURCE_DIR})

lanarts_fetch_sources(backward_cpp
    https://github.com/bombela/backward-cpp/archive/refs/tags/v1.6.tar.gz
    c654d0923d43f1cea23d086729673498e4741fb2457e806cfaeaea7b20c97c10)
# Installs the crash handler that prints a stack trace
add_library(backward OBJECT ${backward_cpp_SOURCE_DIR}/backward.cpp)
target_include_directories(backward PUBLIC ${backward_cpp_SOURCE_DIR})

if(NOT LANARTS_HEADLESS)
    lanarts_fetch_sources(imgui
        https://github.com/ocornut/imgui/archive/refs/tags/v1.92.9.tar.gz
        af97ed649182c39314320514a672b82008ab462b9293fe23d37b30bfa5d05519)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
    target_link_libraries(imgui PUBLIC lanarts::sdl lanarts::opengl)
endif()

# ---------------------------------------------------------------------------------------------
# Kept in this repository
# ---------------------------------------------------------------------------------------------
add_subdirectory(lua-yaml)
add_subdirectory(rvo2)
add_subdirectory(lua-box2d)
add_subdirectory(libxmi)
