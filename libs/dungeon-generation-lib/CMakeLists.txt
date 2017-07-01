cmake_minimum_required( VERSION 2.8 )
PROJECT(project_lanarts_dungeon_gen)

set( DEPS ../../dependencies)


include_directories(
	include/ldungeon_gen
        ${DEPS}/lua
        ${DEPS}/libxmi
	../common-lib/include
	../draw-lib/include
	../luawrap-lib/include
	${DEPS}/UnitTest++ 
	
)

aux_source_directory( "src/" lanarts_dungeon_gen_src)
aux_source_directory( "src/lua" lanarts_dungeon_gen_src)
aux_source_directory( "include/" lanarts_dungeon_gen_include)
aux_source_directory( "tests/" lanarts_dungeon_gen_tests_src)

add_library( lanarts_dungeon_gen
             STATIC
             ${lanarts_dungeon_gen_src}
             ${lanarts_dungeon_gen_include}
)

#add_executable( lanarts_dungeon_gen_tests
#             ${lanarts_dungeon_gen_tests_src})
#
#target_link_libraries(
#	lanarts_dungeon_gen_tests
#	lanarts_dungeon_gen
#	libxmi
#	lcommon
#	luawrap
#	lua
#	ldraw
#	UnitTest++
#)
#
#enable_testing()
#add_test(NAME lanarts_dungeon_gen_tests COMMAND
#    cd ${CMAKE_CURRENT_SOURCE_DIR} ;
#    ${EXECUTABLE_OUTPUT_PATH}/lanarts_dungeon_gen_tests )
#
