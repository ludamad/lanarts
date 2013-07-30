/*
 * lua_general.cpp:
 *  General utility functions.
 */

#include <cstring>

#include <lua.hpp>

#include <lcommon/Timer.h>
#include <lcommon/math_util.h>
#include <lcommon/strformat.h>

#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/calls.h>
#include <lcommon/directory.h>

#include "gamestate/GameState.h"

#include "lua_newapi.h"

#include <lcommon/math_util.h>

static int lapi_table_merge(lua_State* L) {
	lua_pushnil(L);

	while (lua_next(L, 2)) {
		lua_pushvalue(L, -2); // key
		lua_pushvalue(L, -2); // value
		lua_settable(L, 1);

		lua_pop(L, 1);
		// pop value
	}
	lua_pop(L, 1);
	return 0;
}
// Delete different keys
static int lapi_table_remove_nonexisting(lua_State* L) {
	lua_pushnil(L);
	while (lua_next(L, 2)) {
		lua_pushvalue(L, -2); // key
		lua_pushvalue(L, -1); // key
		lua_rawget(L, 1);
		if (lua_isnil(L, -1)) {
			// Set entry to nil, not in other table.
			lua_rawset(L, 2);
		} else {
			lua_pop(L, 2);
		}
		lua_pop(L, 1); // pop value
	}
	lua_pop(L, 1);
	return 0;
}

static bool metacopy(lua_State* L) {
	// Ensure same meta-table, call __copy metamethod if exists
	if (lua_getmetatable(L, 1)) {
		lua_setmetatable(L, 2);
		lua_getmetatable(L, 1);

		// Call __copy metamethod if not nil, skipping the rest of the copying.
		lua_getfield(L, -1, "__copy");
		if (!lua_isnil(L, -1)) {
			lua_pushvalue(L, 1);
			lua_pushvalue(L, 2);
			lua_call(L, 2, 0);
			return true;
		}
		lua_pop(L, 1);
	}
	return false;
}

static int lapi_table_copy(lua_State* L) {
	if (metacopy(L)) {
		return 0;
	}

	lua_pushnil(L);

	while (lua_next(L, 2)) {
		lua_pushvalue(L, -2); // key
		lua_pushvalue(L, -2); // value
		lua_settable(L, 1);

		lua_pop(L, 1);
		// pop value
	}
	lua_pop(L, 1);
	return 0;
}

// Works with plain-data tables and not-too-special tables.
static int lapi_table_deep_copy(lua_State* L) {
	if (metacopy(L)) {
		return 0;
	}

	// Deep copy over members
	lua_pushnil(L);
	while (lua_next(L, 1)) {
		lua_pushvalue(L, -2); // key
		LuaStackValue key(L, -1);
		lua_pushvalue(L, -2); // value
		LuaStackValue value(L, -1);

		key.push();
		lua_rawget(L, 2); // push current value
		if (lua_istable(L, -1) && lua_istable(L, value.index())) {
			// Recursively deep copy
			lua_pushcfunction(L, lapi_table_deep_copy);
			lua_pushvalue(L, -2); // push current value
			value.push();
			lua_call(L, 2, 0);
			lua_pop(L, 1);
		} else {
			lua_pop(L, 1);
			lua_rawset(L, 2);
		}

		lua_pop(L, 1); // pop value
	}
	// Note, calling this directly works only because the arguments are in the same order!
	lapi_table_remove_nonexisting(L);
	return 0;
}

static int lapi_values_aux(lua_State* L) {
	long idx = (long) lua_touserdata(L, lua_upvalueindex(2));
	long len = (long) lua_touserdata(L, lua_upvalueindex(3));

	if (idx > len) {
		return 0;
	}

	lua_pushlightuserdata(L, (void*) (1 + idx));
	lua_replace(L, lua_upvalueindex(2));

	lua_rawgeti(L, lua_upvalueindex(1), idx);
	if (lua_isnil(L, -1)) {
		luaL_error(L,
				"Expected array as parameter to 'values', but retrieved a nil value at index '%d'.",
				idx);
	}
	return 1;
}

// Try getters, then methods, then inherited __index
static int lapi_newtype_index(lua_State* L) {
	// Upvalue 1: Getters
	lua_pushvalue(L, 2); // key
	lua_gettable(L, lua_upvalueindex(1)); // Push getter

	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, 1); // obj
		lua_call(L, 1, 1); // Call getter with obj
		return 1;
	}
	// Upvalue 2: Metatable
	lua_pushvalue(L, 2); // key
	lua_rawget(L, lua_upvalueindex(2)); // metatable table

	if (!lua_isnil(L, -1)) {
		return 1;
	}
	// Upvalue 3: Inherited metatable
	if (!lua_isnil(L, lua_upvalueindex(3))) {
		lua_getfield(L, lua_upvalueindex(3), "__index");
		lua_pushvalue(L, 1); // obj
		lua_pushvalue(L, 2); // key
		lua_call(L, 2, 1);
		return 1;
	}

	luawrap::globals(L)["tostring"].push();
	std::string tostring = luawrap::call<std::string>(L, LuaStackValue(L, 1));

	return luaL_error(L,
			"Class object '%s': Cannot read '%s', member does not exist!\n",
			tostring.c_str(), lua_tostring(L, 2));
}

static int lapi_newtype_newindex(lua_State* L) {
	// Upvalue 1: Setters
	lua_pushvalue(L, 2); // key
	lua_gettable(L, lua_upvalueindex(1)); // Push setter

	if (!lua_isnil(L, -1)) {
		lua_pushvalue(L, 1); // obj
		lua_pushvalue(L, 3); // value
		lua_call(L, 2, 0); // Call setter with obj, value
		return 0;
	}
	// Upvalue 2: Inherited metatable
	if (!lua_isnil(L, lua_upvalueindex(2))) {
		lua_getfield(L, lua_upvalueindex(2), "__newindex");
		lua_pushvalue(L, 1); // obj
		lua_pushvalue(L, 2); // key
		lua_pushvalue(L, 3); // value
		lua_call(L, 3, 0);
		return 0;
	}

	// Try object itself:
	lua_settop(L, 3);
	lua_rawset(L, 1);
	return 0;
}

static int lapi_newtype_create(lua_State* L) {
	int nargs = lua_gettop(L);

	lua_newtable(L);

	LuaSpecialValue metatable(L, lua_upvalueindex(1));
	metatable["init"].push(); // push the init function

	lua_pushvalue(L, -2); // push the new instance as 1st arg

	metatable.push();
	lua_setmetatable(L, -2); // set the new instance's metatable

	if (lua_isnil(L, -2)) { // Function is nil
		return 1; // Just return new instance now
	}

	for (int i = 1; i <= nargs; i++) {
		lua_pushvalue(L, i); // Passed arguments
	}

	lua_call(L, nargs + 1, 0);

	return 1;
}

static int lapi_newtype(lua_State* L) {
	lua_newtable(L);
	// new type metatable
	LuaStackValue newtype(L, -1);

	newtype.push();
	lua_pushcclosure(L, lapi_newtype_create, 1);
	newtype["create"].pop();

	luawrap::ensure_table(newtype["set"]);
	luawrap::ensure_table(newtype["get"]);

	if (!lua_isnil(L, 1)) {
		LuaStackValue parameters(L, 1);

		parameters["parent"].push();
		lua_setmetatable(L, -2); // set new type metatable's metatable

		newtype["typename"] = parameters["typename"];
		newtype["parent"] = parameters["parent"];
	} else {
		newtype["typename"] = "<UserDefinedType>";
	}

	newtype["get"].push(); // Upvalue 1: getters
	newtype.push(); // Upvalue 2: metatable
	newtype["parent"].push(); // Upvalue 3: parent metatable
	lua_pushcclosure(L, lapi_newtype_index, 3);
	newtype["__index"].pop();

	newtype["set"].push(); // Upvalue 1: setters
	newtype["parent"].push(); // Upvalue 2: parent metatable
	lua_pushcclosure(L, lapi_newtype_newindex, 2);
	newtype["__newindex"].pop();

	return 1; // return new type metatable
}

static int lapi_string_split(lua_State* L) {
	size_t str_size;
	const char* tail = luaL_checklstring(L, 1, &str_size);
	const char* end = tail + str_size;

	size_t sep_size = 0;
	const char* sep = luaL_checklstring(L, 2, &sep_size);
	int i = 1;

	/* push result table */
	lua_newtable(L);

	/* repeat for each separator */
	while (tail < end) {
		const char* head = tail;
		/* find separator */
		while (strncmp(head, sep, sep_size) != 0) {
			head++;
			if (head >= end) {
				goto label_LeaveLoop;
			}
		}

		lua_pushlstring(L, tail, head - tail); /* push substring */
		lua_rawseti(L, -2, i++);
		tail = head + sep_size; /* skip separator */
	}

	label_LeaveLoop:

	/* push last substring */
	lua_pushstring(L, tail);
	lua_rawseti(L, -2, i);

	return 1; /* return the table */
}

/*
 * Joins array elements with a given string, eg (" "):join( {"hello", "world"} ) => "hello world"
 */
static std::string lapi_string_join(const char* joiner, LuaStackValue table) {
	lua_State* L = table.luastate();

	std::string return_string;

	int joiner_len = strlen(joiner), table_len = table.objlen();
	for (int i = 1; i <= table_len; i++) {
		// Append element
		table[i].push();
		size_t elem_size;
		const char* elem = lua_tolstring(L, -1, &elem_size);
		return_string.append(elem, elem_size);
		lua_pop(L, 1);

		// Append joiner, unless at end
		if (i != table_len) {
			return_string.append(joiner, joiner_len);
		}
	}
	return return_string;
}

static int lapi_setglobal(lua_State* L) {
	bool prev = lua_api::globals_get_mutability(L);

	lua_api::globals_set_mutability(L, true);
	lua_settable(L, LUA_GLOBALSINDEX); // use params 1 & 2
	lua_api::globals_set_mutability(L, prev);

	return 1; /* return the table */
}

static int lapi_toaddress(lua_State* L) {
	char address[64];
	snprintf(address, 64, "0x%X", lua_topointer(L, 1));
	lua_pushstring(L, address);
	return 1; /* return the table */
}

static void lapi_add_search_path(LuaStackValue path) {
	lua_api::add_search_path(path.luastate(), path.to_str());
}

// Replace '/' or '\' by '.', placing it in 'dot-form'
// Remove any pre-existing '.', as this would mess with the path handing algorithms.
static void path2dotform(std::string& str) {
	std::string dotform;
	int i = 0;
	// Remove any separators or dots at beginning
	for (; i < str.size(); i++) {
		if (str[i] != '\\' && str[i] != '/' && str[i] != '.') {
			break;
		}
	}
	// Replace separators, remove dots
	for (; i < str.size(); i++) {
		if (str[i] == '\\' || str[i] == '/') {
			dotform += '.';
		} else if (str[i] != '.') {
			dotform += str[i];
		}
	}
	str = dotform;
}

static void dotform_chop_until_dot(std::string& str) {
	while (!str.empty() && str[str.size()-1] != '.') {
		str.resize(str.size() - 1);
	}
}

static void dotform_chop_trailing_dots(std::string& str) {
	// Chop trailing dots from cwd
	while (!str.empty() && str[str.size()-1] == '.') {
		str.resize(str.size() - 1);
	}
}

static void dotform_chop_component(std::string& str) {
	dotform_chop_until_dot(str);
	dotform_chop_trailing_dots(str);
}

// Delegates to 'require', readjusts a relative 'dot-form' path
static int lapi_require_relative(lua_State* L) {
	// Resolve backtracks, ie every occurrence of < goes up one directory
	const char* req_arg = lua_tostring(L, 1);
	int backtracks = 0;
	while (*req_arg == '<') {
		backtracks++, req_arg++;
	}

	// Lookup caller's source location
	lua_Debug debug;
	lua_getstack(L, 1, &debug);
	lua_getinfo(L, "S", &debug);

	// '@' indicates a relative path, remove this character if present
	std::string src = (*debug.source == '@' ? debug.source + 1: debug.source);
	std::string cwd = working_directory();

	path2dotform(src), path2dotform(cwd);
	// Chop filename from source, and resolve backtracks
	for (int i = 0; i < backtracks + 1; i++) {
		dotform_chop_component(src);
	}
	dotform_chop_trailing_dots(cwd);

	// Try to make all paths relative to the modules directory.
	cwd += 'modules';

	const char* src_ptr = src.c_str(), *cwd_ptr = cwd.c_str();

	// Find common substring
	while (*cwd_ptr && *src_ptr == *cwd_ptr) {
		src_ptr++, cwd_ptr++;
	}

	std::string require_string = src_ptr;
	require_string += ".";
	require_string += req_arg;

	luawrap::globals(L)["require"].push();
	lua_pushstring(L, require_string.c_str());
	lua_call(L, 1, 1);

	return 1; // Return module
}

static int lapi_random(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int nargs = lua_gettop(L);
	int min = 0, max = 0;
	if (nargs == 1) {
		// array[1]
		lua_rawgeti(L, 1, 1);
		min = lua_tointeger(L, -1);
		// array[2]
		lua_rawgeti(L, 1, 2);
		max = lua_tointeger(L, -1);
		lua_pop(L, 2);
	} else if (nargs > 1) {
		min = lua_tointeger(L, 1);
		max = lua_tointeger(L, 2);
	}

	lua_pushnumber(L, gs->rng().rand(min, max + 1));
	return 1;
}

static BBox lapi_random_subregion(LuaStackValue lbox, Size size) {
	BBox bbox = lbox.as<BBox>();
	GameState* gs = lua_api::gamestate(lbox);

	int x = gs->rng().rand(bbox.x1, bbox.x2 - size.w);
	int y = gs->rng().rand(bbox.y1, bbox.y2 - size.h);

	return BBox(Pos(x, y), size);
}

static bool lapi_chance(LuaStackValue val) {
	GameState* gs = lua_api::gamestate(val);
	return gs->rng().rand(RangeF(0, 1)) < val.to_num();
}

namespace lua_api {
	void event_projectile_hit(lua_State* L, ProjectileInst* projectile,
			GameInst* target) {
	}

	int l_itervalues(lua_State* L) {
		lua_pushvalue(L, 1);
		lua_pushlightuserdata(L, (void*) (1)); // Lua array iteration starts at 1
		lua_pushlightuserdata(L, (void*) (lua_objlen(L, 1))); // End at the object length
		lua_pushcclosure(L, lapi_values_aux, 3);
		return 1;
	}

	void register_general_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		globals["values"].bind_function(l_itervalues);
		globals["direction"].bind_function(compute_direction);
		globals["distance"].bind_function(distance_between);
		globals["newtype"].bind_function(lapi_newtype);
		globals["setglobal"].bind_function(lapi_setglobal);
		globals["toaddress"].bind_function(lapi_toaddress);
		globals["random"].bind_function(lapi_random);
		globals["random_subregion"].bind_function(lapi_random_subregion);
		globals["chance"].bind_function(lapi_chance);

		globals["require_relative"].bind_function(lapi_require_relative);
		globals["require_path_add"].bind_function(lapi_add_search_path);

		LuaValue table = luawrap::ensure_table(globals["table"]);
		table["merge"].bind_function(lapi_table_merge);
		table["copy"].bind_function(lapi_table_copy);
		table["deep_copy"].bind_function(lapi_table_deep_copy);
		table["remove_nonexisting"].bind_function(lapi_table_remove_nonexisting);

		LuaValue string_table = luawrap::ensure_table(globals["string"]);
		string_table["split"].bind_function(lapi_string_split);
		string_table["join"].bind_function(lapi_string_join);
		string_table["pack"].bind_function(str_pack);
	}
}
