cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lcommon)

#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)
#add_definitions(-DNDEBUG)

FIND_PACKAGE(SDL REQUIRED)

set( SRC ../)
set( LIBS ../../libs)
include_directories(${SDL_INCLUDE_DIR}
	${LIBS} #/lua 
	${LIBS}/slb/include
)


set( lcommon_src
    perf_timer.h
    fatal_error.h
    fatal_error.cpp
	geometry.cpp
	geometry.h
	lcommon_assert.h
	math.cpp
	math.h
	PerfTimer.cpp
	PerfTimer.h
	SerializeBuffer.cpp
	SerializeBuffer.h
	Range.h
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
    lua/slb.h
    lua/luacpp_wrap_numeric.h
    lua/luacpp_wrap.h
	
	random/mtwist.cpp
	random/mtwist.h
)

add_library( lcommon
             ${lcommon_src} )

AUX_SOURCE_DIRECTORY(tests lcommon_tests) 



add_executable( lcommon_tests
             ${lcommon_src}
             ${lcommon_tests}
              )

target_link_libraries( 
			lcommon_tests
			lua
			slb
)
