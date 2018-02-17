cmake_minimum_required( VERSION 2.6 )

PROJECT(project_ldraw)

include(FindFreetype)

set( SRC ../)
set( DEPS ../../dependencies)


include_directories(
	include/ldraw/
	"${SRC}/common-lib/include"
	"${SRC}/luawrap-lib/include"
	${DEPS}/UnitTest++
	${DEPS}/lua
	${FREETYPE_INCLUDE_DIRS} 
)

file(GLOB ldraw_src RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
        src/*
        src/lua/*
        src/opengl/*
        include/ldraw/*
)

file(GLOB ldraw_tests RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} 
    tests/*
)

add_library( ldraw
		STATIC
             ${ldraw_src} )

set(LINKEDDEPS
	ldraw
	luawrap
	lcommon
	lua
	${FREETYPE_LIBRARIES}
)

add_definitions(--std=c++14)


#add_executable( ldraw_ex ${ldraw_examples} )
add_executable( ldraw_tests ${ldraw_tests} ${ldraw_src} )
#
target_link_libraries( ldraw_tests ${LINKEDDEPS} )
target_link_libraries( ldraw_tests ${LINKEDDEPS} UnitTest++ )
#
#enable_testing()
#add_test(NAME ldraw_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/ldraw_tests )
