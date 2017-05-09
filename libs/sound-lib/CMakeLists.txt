cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lsound)

set( DEPS ../../dependencies)
set( SRC ..)

include_directories(
    "include/lsound"
    "${DEPS}/lua"
    "${DEPS}/sdl2/include"
    "${DEPS}/sdl2-mixer"
    "${SRC}/luawrap-lib/include"
    "${SRC}/draw-lib/include"
    "${SRC}/common-lib/include"
)


# Library target
aux_source_directory("src/" lsound_src) #Find source files
aux_source_directory("src/lua" lua_lsound_src) #Find source files
aux_source_directory("include/lsound" lsound_inc) #Find source files

add_library( lsound
             STATIC
             ${lsound_src} 
             ${lua_lsound_src}
             ${lsound_inc} )

# Test runner target
aux_source_directory(tests lsound_testsrc) 

set(LINKEDLIBS
	lsound
        ldraw
	luawrap
	lcommon
	lua
        SDL2-static
        SDL2-mixer
)

#add_executable( lsound_tests main.cpp)
#
#target_link_libraries(lsound_tests ${LINKEDLIBS})
#
# Enable tests
#enable_testing()

#add_test(NAME lsound_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/lsound_tests )
