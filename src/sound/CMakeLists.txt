cmake_minimum_required( VERSION 2.6 )
PROJECT(project_lsound)

FIND_PACKAGE(SDL REQUIRED)
FIND_PACKAGE(SDL_mixer REQUIRED)

set( LIBS ../../libs)
set( SRC ..)

include_directories(
    "include/lsound"
    ${SDL_INCLUDE_DIR} 
    ${LIBS}/lua
    ${SRC}/luawrap/include
    ${SRC}/common/include
)


# Library target
aux_source_directory("src/" lsound_src) #Find source files
aux_source_directory("src/lua" lua_lsound_src) #Find source files
aux_source_directory("include/lsound" lsound_inc) #Find source files

add_library( lsound
             ${lsound_src} 
             ${lua_lsound_src}
             ${lsound_inc} )

# Test runner target
aux_source_directory(tests lsound_testsrc) 

set(LINKEDLIBS
	lsound
	luawrap
	lcommon
	lua
	${SDL_LIBRARY}
	${SDLMIXER_LIBRARY}
)

#add_executable( lsound_tests main.cpp)

#target_link_libraries(lsound_tests ${LINKEDLIBS})

# Enable tests
#enable_testing()

#add_test(NAME lsound_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/lsound_tests )
