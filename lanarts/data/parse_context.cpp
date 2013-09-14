/*
 * parse_context.cpp:
 *  Global parse context.
 *  This is used so a lua_State does not have to be passed to many methods.
 */

#include <cstdlib>

#include <lcommon/fatal_error.h>

#include "parse_context.h"

static lua_State* __parser_lua_state = NULL;

ParseContext::ParseContext(lua_State* L) {
	if (__parser_lua_state) {
		fatal_error("parse_context.cpp: parser lua state already set! Nesting ParseContext's currently not supported.");
	}
	__parser_lua_state = L;
}

ParseContext::~ParseContext() {
	__parser_lua_state = NULL;
}

lua_State* ParseContext::luastate() {
	if (!__parser_lua_state) {
		fatal_error("parse_context.cpp: parser lua state not set! Use ParseContext in a top level call to ensure this is available.");
	}
	return __parser_lua_state;
}

