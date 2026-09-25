# Lua 5.1.5. Patched in lparser.c: tail calls are compiled as ordinary calls, so stack traces
# keep every frame.
add_library(lua STATIC
    lapi.c lauxlib.c lbaselib.c lcode.c ldblib.c ldebug.c ldo.c ldump.c lfunc.c lgc.c linit.c
    liolib.c llex.c lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c loslib.c lparser.c lstate.c
    lstring.c lstrlib.c ltable.c ltablib.c ltm.c lundump.c lvm.c lzio.c)
target_include_directories(lua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(UNIX AND NOT EMSCRIPTEN)
    target_compile_definitions(lua PRIVATE LUA_USE_POSIX)
endif()
add_library(lanarts::lua ALIAS lua)
