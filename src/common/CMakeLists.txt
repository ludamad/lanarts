cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lcommon)

set(CMAKE_BUILD_TYPE Debug)

FIND_PACKAGE(SDL REQUIRED)

set( SRC ../)
set( LIBS ../../libs)
include_directories(${SDL_INCLUDE_DIR}
	${LIBS} #/lua 
	${LIBS}/slb/include
)


set( lcommon_src
    func_timer.h
	geometry.cpp
	geometry.h
	lcommon_defines.h
	math.cpp
	math.h
	PerfTimer.cpp
	PerfTimer.h
	SerializeBuffer.cpp
	SerializeBuffer.h
	range.h
	smartptr.h
	Timer.cpp
	Timer.h
	unittest.cpp
	unittest.h
	strformat.cpp
	strformat.h
	
	lua/lua_geometry.h
	lua/lua_geometry.cpp
	lua/lua_lcommon.h
	lua/lua_lcommon.cpp
    lua/lua_range.cpp
    lua/lua_range.h
    lua/LuaValue.h
    lua/LuaValue.cpp
    lua/lua_serialize.h
    lua/lua_serialize.cpp
    lua/lua_timer.cpp
    lua/lua_timer.h
    lua/lua_unittest.cpp
    lua/lua_unittest.h
    lua/lua_vector.h
    lua/slb_funcproperty.h
    lua/slb_mutabletable.h
    lua/slb_numvectortype.h
    lua/slb_valuetype.h
	
)

add_library( lcommon
             ${lcommon_src} )

set( lcommon_tests
	tests/lcommon_tests.cpp
	tests/lcommon_tests.h
	tests/lua_geometry_tests.cpp
	tests/lua_range_tests.cpp
	tests/lua_vector_tests.cpp
	tests/luavalue_tests.cpp
    tests/lua_serialize_tests.cpp
	tests/strformat_tests.cpp
)

add_executable( lcommon_tests
             ${lcommon_src}
             ${lcommon_tests}
              )

target_link_libraries( 
			lcommon_tests
			
			"/home/adomurad/lanarts-git/lanarts_build/libs/slb/libslb.a"
			"/home/adomurad/lanarts-git/lanarts_build/libs/lua/liblua.a"
#			lua
#			slb
)
