cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lcommon)

FIND_PACKAGE(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR} )

set( lcommon_src
  PerfTimer.h
  PerfTimer.cpp
  smartptr.h
  Timer.h
  Timer.cpp
  lcommon_defines.h
  lcommon_defines.cpp
)

add_library( lcommon
             ${lcommon_src} )
