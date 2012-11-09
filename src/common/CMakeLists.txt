cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lcommon)

FIND_PACKAGE(SDL REQUIRED)

set( SRC ../)
set( LIBS ../../libs)
include_directories(${SDL_INCLUDE_DIR}
	${LIBS} #/lua 
	${LIBS}/slb/include
)


set( lcommon_src
	geometry.h
	lua_geometry.h
	lua_numarray.h
	lua_geometry.cpp
	lcommon_defines.h
	math.cpp
	math.h
	PerfTimer.cpp
	PerfTimer.h
	range.h
	smartptr.h
	Timer.cpp
	Timer.h
	unittest.cpp
	unittest.h
)

#add_library( lcommon
#             ${lcommon_src} )

set( lcommon_tests
	tests/lua_tests.cpp
	tests/tests.cpp
	tests/tests.h
)

add_executable( lcommon
             ${lcommon_src}
             ${lcommon_tests}
              )

target_link_libraries( 
			lcommon			
			"/home/adomurad/lanarts-git/lanarts_build/libs/lua/liblua.a"
			"/home/adomurad/lanarts-git/lanarts_build/libs/slb/libslb.a"
			#lua
			#slb
)