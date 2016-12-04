/*
 * lua_general.cpp:
 *  General utility functions.
 */

#include <cstring>
#include <vector>
#include <algorithm>

#include <backward.hpp>

#include <lua.hpp>

#include <lcommon/Timer.h>
#include <lcommon/math_util.h>
#include <lcommon/strformat.h>

#include <luawrap/luawrap.h>
#include <lcommon/directory.h>

#include <lcommon/lua_utils.h>

#include "gamestate/GameState.h"

#include "lua_api.h"

#include <lcommon/math_util.h>

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
		if (!lua_isnil(L, -1)) {
			lua_pushvalue(L, 1); // obj
			lua_pushvalue(L, 2); // key
			lua_call(L, 2, 1);
			return 1;
		}
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
		if (!lua_isnil(L, -1)) {
			lua_pushvalue(L, 1); // obj
			lua_pushvalue(L, 2); // key
			lua_pushvalue(L, 3); // value
			lua_call(L, 3, 0);
			return 0;
		}
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

static int lapi_toaddress(lua_State* L) {
	char address[64];
	snprintf(address, 64, "0x%X", lua_topointer(L, 1));
	lua_pushstring(L, address);
	return 1; /* return the table */
}

static void lapi_require_path_add(LuaStackValue path) {
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

static const char* get_source(lua_State* L, int idx) {
	lua_Debug ar;
	lua_getstack(L, idx, &ar);
	lua_getinfo(L, "S", &ar);
	return ar.source;
}

static int lapi_path_resolve(lua_State* L) {
	int stack_idx = lua_gettop(L) < 2 ? 1 : lua_tointeger(L, 2);
	std::string current = get_source(L, stack_idx);
	// Find character index where root dir ends
	int i = current.size() - 1;
	while (i >= 0 && (current[i] == '/' || current[i] == '\\')) {
		i--;
	}
	// Find root dir
	while (i >= 0 && (current[i] != '/' && current[i] != '\\')) {
		i--;
	}
	i--; // Remove separator
	current.resize(std::max(0, i + 1));

	const char* cptr = current.c_str();
	cptr = (*cptr == '@' ? cptr + 1 : cptr);
	if (!*cptr) {
	    cptr = ".";
	}
	// '@' indicates a file path when used in debug info, remove this character if present
	lua_pushfstring(L, "%s/%s", cptr, lua_tostring(L, 1));
	return 1;
}

// Delegates to 'require', readjusts a relative 'dot-form' path
static int lapi_virtual_path_create_relative(lua_State* L) {
	// Resolve backtracks, ie every occurrence of < goes up one directory
	const char* sub_path = lua_tostring(L, 1);
	const char* base_path = lua_tostring(L, 2);
	bool drop_filename = (lua_gettop(L) >= 3 && lua_toboolean(L, 3));

	int backtracks = drop_filename ? 1 : 0;
	while (*sub_path == '<') {
		backtracks++, sub_path++;
	}

	// '@' indicates a file path when used in debug info, remove this character if present
	std::string src = (*sub_path== '@' ? sub_path + 1: sub_path);
	std::string cwd = working_directory();

	// Ensure file extension has been removed
	if (src.size() >= 4 && strncmp(&src[src.size()-4], ".lua", 4) == 0) {
		src.resize(src.size()-4);
	}

	path2dotform(src), path2dotform(cwd);
	// Chop filename from source, and resolve backtracks
	for (int i = 0; i < backtracks; i++) {
		dotform_chop_component(src);
	}
	dotform_chop_trailing_dots(cwd);

	// Make source relative to cwd
	src = cwd + "." + src;
	// Make into cwd by relative path base_path.
	if (*base_path) {
		cwd += ".";
		cwd += base_path;
	}
	cwd += ".";

	const char* src_ptr = src.c_str(), *cwd_ptr = cwd.c_str();

	// Find common substring
	while (*cwd_ptr && *src_ptr == *cwd_ptr) {
		src_ptr++, cwd_ptr++;
	}

	lua_pushstring(L, src_ptr);
	return 1; // Return relative virtual path
}

static int lapi_rand_range(lua_State* L) {
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

static void lapi_random_seed(LuaStackValue seed) {
	GameState* gs = lua_api::gamestate(seed.luastate());
        gs->rng().init_genrand(seed.as<int>());
}


static double lapi_random_gaussian(LuaStackValue average, double std_dev) {
    GameState* gs = lua_api::gamestate(average);
    return gs->rng().guassian(average.to_num(), std_dev, /*n_trials*/ 10);
}

static int lapi_randomf(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	int nargs = lua_gettop(L);
	double min = 0, max = 0;
	if (nargs == 1) {
		// array[1]
		lua_rawgeti(L, 1, 1);
		min = lua_tonumber(L, -1);
		// array[2]
		lua_rawgeti(L, 1, 2);
		max = lua_tonumber(L, -1);
		lua_pop(L, 2);
	} else if (nargs > 1) {
		min = lua_tonumber(L, 1);
		max = lua_tonumber(L, 2);
	}

	lua_pushnumber(L, gs->rng().rand(RangeF(min, max)));
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

static LuaValue lengine_import_internal_raw(LuaStackValue importstring) {
	lua_State* L = importstring.luastate();
	LuaValue module = luawrap::globals(L)["_INTERNAL_IMPORTED"][importstring.to_str()];
	if (module.isnil()) {
		LuaValue loader = luawrap::globals(L)["_INTERNAL_LOADERS"][importstring.to_str()];
		if (loader.isnil()) {
			return loader;
		}
		loader.push();
		lua_call(L, 0, 1);
		return LuaValue::pop_value(L);
	}
	LuaValue ret_table(L);
	ret_table.newtable();
	ret_table[1] = module;
	return ret_table;
}

namespace lua_api {
	void event_projectile_hit(lua_State* L, ProjectileInst* projectile,
			GameInst* target) {
	}

	void register_lua_submodule(lua_State* L, const char* vpath,
			LuaValue module) {
		luawrap::globals(L)["package"]["loaded"][vpath] = module;
	}

	void register_lua_submodule_loader(lua_State* L, const char* vpath, LuaValue loader) {
		luawrap::globals(L)["package"]["preload"][vpath] = loader;
	}

	void pretty_print(LuaField field) {
		lua_State* L = field.luastate();
		luawrap::globals(L)["pretty_print"].push();
		field.push();
		lua_call(L, 1, 0);
	}

        // TODO rename require
	LuaValue import(lua_State* L, const char* virtual_path) {
		luawrap::globals(L)["require"].push();
		lua_pushstring(L, virtual_path);
		lua_call(L, 1, 1);
		return LuaValue::pop_value(L);
	}

	LuaValue register_lua_submodule(lua_State* L, const char* vpath) {
		LuaValue submodule(L);
		submodule.newtable();
		lua_protect_table(submodule); // Ensure that nil access is an error
		register_lua_submodule(L, vpath, submodule);
		return submodule;
	}

	LuaModule register_lua_submodule_as_luamodule(lua_State* L, const char* vpath) {
		LuaModule submodule(L, vpath);
		register_lua_submodule(L, vpath, submodule.proxy);
		return submodule;
	}

	int l_itervalues(lua_State* L) {
		lua_pushvalue(L, 1);
		lua_pushlightuserdata(L, (void*) (1)); // Lua array iteration starts at 1
		lua_pushlightuserdata(L, (void*) (lua_objlen(L, 1))); // End at the object length
		lua_pushcclosure(L, lapi_values_aux, 3);
		return 1;
	}

        std::vector<std::string> lapi_cpp_traceback() {
            // Use our hacked copy of the 'backward' library:
                using namespace backward;
                StackTrace st; st.load_here(255);
                Printer p;
                std::vector<std::string> traceback = p.print(st);
                // Filter the top of the traceback that includes this function:
                while (!traceback.empty()) {
                    std::string back = traceback.back();
                    traceback.pop_back();
                    if (back.find("StackTrace st; st.load_here(255);") != std::string::npos) {
                        break;
                    }
                }
                std::reverse(traceback.begin(), traceback.end());
                return traceback;
        }

	int read_eval_print(lua_State *L);

        static double round(double x) {
            return ::round(x);
        }
	void register_general_api(lua_State* L) {
		LuaValue globals = luawrap::globals(L);
		LuaValue registry = luawrap::registry(L);

		globals["values"].bind_function(l_itervalues);
		globals["newtype"].bind_function(lapi_newtype);
		globals["toaddress"].bind_function(lapi_toaddress);
		globals["rand_range"].bind_function(lapi_rand_range);
		globals["random"].bind_function(lapi_random);
		globals["random_seed"].bind_function(lapi_random_seed);
		globals["randomf"].bind_function(lapi_randomf);
		globals["random_gaussian"].bind_function(lapi_random_gaussian);
		globals["random_subregion"].bind_function(lapi_random_subregion);
		globals["chance"].bind_function(lapi_chance);
		globals["cpp_traceback"].bind_function(lapi_cpp_traceback);

		globals["__read_eval_print"].bind_function(read_eval_print);

		LuaValue lengine = luawrap::ensure_table(globals["LEngine"]);
		lengine["import_internal_raw"].bind_function(lengine_import_internal_raw);

		globals["virtual_path_create_relative"].bind_function(lapi_virtual_path_create_relative);
		globals["require_path_add"].bind_function(lapi_require_path_add);
		globals["path_resolve"].bind_function(lapi_path_resolve);

		LuaValue math_table = luawrap::ensure_table(globals["math"]);
                math_table["round"].bind_function(round);
		LuaValue string_table = luawrap::ensure_table(globals["string"]);
		string_table["split"].bind_function(lapi_string_split);
		string_table["join"].bind_function(lapi_string_join);
		string_table["pack"].bind_function(str_pack);

		// Represents global, mutable data. Only module that is not serialized 'as a constant'.
                lua_api::import(L, "InitialGlobalData").push();
                // Get the initial global data object:
                lua_call(L, 0, 1);
		luawrap::globals(L)["package"]["loaded"]["core.GlobalData"].pop();
	}
}
