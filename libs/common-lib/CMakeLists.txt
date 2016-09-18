cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lcommon)

set( SRC "..")
set( DEPS ../../dependencies)
include_directories(
	include/lcommon/
	${DEPS}/UnitTest++ 
	${DEPS}/lua 
	${DEPS}/sdl2/include
	"${SRC}/luawrap-lib/include"
)
add_definitions( -std=c++98 )

file(GLOB lcommon_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    src/* 
    src/lua/* 
    include/common/* 
)


add_library( lcommon
             STATIC
             ${lcommon_src} )

AUX_SOURCE_DIRECTORY(tests lcommon_tests) 

add_executable( lcommon_tests
             ${lcommon_tests}
              )

target_link_libraries( 
			lcommon_tests
			lcommon
			UnitTest++
			luawrap
			lua
)

# Enable tests
enable_testing()

add_test(NAME lcommon_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/lcommon_tests )
