# lyaml with LibYAML bundled, extended for lanarts (see luayaml_push_item in src/lua_api)
add_library(luayaml STATIC
    api.c b64.c dumper.c emitter.c loader.c lyaml.c parser.c reader.c scanner.c writer.c)
target_include_directories(luayaml PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(luayaml PUBLIC lanarts::lua)
