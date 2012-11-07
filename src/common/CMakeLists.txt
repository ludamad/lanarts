cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lcommon)

FIND_PACKAGE(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR} )

set( lcommon_src
	geometry.h
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

add_library( lcommon
             ${lcommon_src} )
