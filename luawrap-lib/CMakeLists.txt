cmake_minimum_required( VERSION 2.6 )
PROJECT(project_luawrap)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
#add_definitions(-DNDEBUG)

set( DEPS ../dependencies)

add_definitions( -std=c++98 )

include_directories(
	"include/"
	${DEPS}/lua 
	${DEPS}/UnitTest++ 
)

# Library target
aux_source_directory("src/" luawrap_src) #Find source files
aux_source_directory("include/luawrap/" luawrap_inc) #Find source files

add_library( luawrap
             STATIC
             ${luawrap_src} 
             ${luawrap_inc} )

# Test runner target
aux_source_directory(tests luawrap_testsrc) 

add_executable( luawrap_tests ${luawrap_testsrc} )

target_link_libraries(luawrap_tests luawrap lua UnitTest++)

# Enable tests
enable_testing()

add_test(NAME luawrap_tests COMMAND ${EXECUTABLE_OUTPUT_PATH}/luawrap_tests )
