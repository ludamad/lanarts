file(GLOB_RECURSE lanarts_sources CONFIGURE_DEPENDS *.cpp *.cc)
add_executable(lanarts ${lanarts_sources})
target_include_directories(lanarts PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lanarts PRIVATE
    lcommon ldraw lsound lanarts_dungeon_gen lanarts_net luawrap
    lanarts::lua lpeg lfs luasocket luayaml lua_enet lua_box2d
    yaml-cpp::yaml-cpp rvo2 polypartition UnitTest++
    $<$<NOT:$<BOOL:${EMSCRIPTEN}>>:backward>
    lanarts::warnings)

if(WIN32)
    target_link_libraries(lanarts PRIVATE dbghelp psapi)
    target_link_options(lanarts PRIVATE -static-libgcc -static-libstdc++ -mwindows)
endif()

if(EMSCRIPTEN)
    set_target_properties(lanarts PROPERTIES SUFFIX .html)
    target_link_libraries(lanarts PRIVATE idbfs.js)
    target_link_options(lanarts PRIVATE
        -sMINIFY_HTML=0 -sTOTAL_MEMORY=800MB -sGL_UNSAFE_OPTS=1
        --preload-file ${PROJECT_SOURCE_DIR}/runtime@/
        $<$<CONFIG:Debug>:-sDISABLE_EXCEPTION_CATCHING=0>)
    target_compile_options(lanarts PRIVATE $<$<CONFIG:Debug>:-sDISABLE_EXCEPTION_CATCHING=0>)
endif()

# runtime/compiled/Resources.lua lists the sprites under runtime/spr_*; regenerated on every build
# since sprites can be added without CMake noticing (it takes a few milliseconds).
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(lanarts_resources ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory compiled
    COMMAND Python3::Interpreter compile_images.py compiled/Resources.lua
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/runtime
    COMMENT "Generating runtime/compiled/Resources.lua"
    VERBATIM)
add_dependencies(lanarts lanarts_resources)

# The C++ unit tests are compiled into the game; they run from the runtime directory
add_test(NAME unit_tests COMMAND lanarts engine.StartUnitTests
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/runtime)
