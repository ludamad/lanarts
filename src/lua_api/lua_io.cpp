/*
 * lua_io.cpp:
 *  Bindings for checking IO states
 */

#include <lua.hpp>
#include <luawrap/luawrap.h>
#include <luawrap/functions.h>
#include <luawrap/members.h>
#include <luawrap/types.h>

#include <lcommon/directory.h>

#include <lsound/lua_lsound.h>

#include <SDL.h>

#include "gamestate/GameState.h"

#include "interface/TextField.h"

#include "lua_api.h"

/* START of TextField bindings */

static void textfield_step(TextField& textfield) {
	textfield.step();
}

static void textfield_clear(TextField& textfield) {
	textfield.clear();
}

static void textfield_clear_keystate(TextField& textfield) {
	textfield.clear_keystate();
}

static bool textfield_handle_event(TextField& textfield, SDL_Event* event) {
	return textfield.handle_event(event);
}

static int lua_newtextinput(lua_State* L) {
	const char* text = lua_gettop(L) >= 2 ? lua_tostring(L, 2) : "";
	luawrap::push(L, TextField(lua_tointeger(L, 1), text));
	return 1;
}

static std::string textfield_get_text(const LuaStackValue& obj, const LuaStackValue& key) {
	return obj.as<TextField>().text();
}

static void textfield_set_text(const LuaStackValue& obj, const LuaStackValue& key, const std::string& val) {
	obj.as<TextField>().set_text(val);
}

static int textfield_get_max_length(const LuaStackValue& obj, const LuaStackValue& key) {
	return obj.as<TextField>().max_length();
}

LuaValue lua_textfieldmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "TextInput");

	LuaValue methods = luameta_constants(meta);
	LuaValue getters = luameta_getters(meta);
	LuaValue setters = luameta_setters(meta);

	getters["text"].bind_function(textfield_get_text);
	setters["text"].bind_function(textfield_set_text);
	getters["max_length"].bind_function(textfield_get_max_length);
	methods["step"].bind_function(textfield_step);
	methods["event_handle"].bind_function(textfield_handle_event);
	methods["clear"].bind_function(textfield_clear);
	methods["clear_keystate"].bind_function(textfield_clear_keystate);

	luameta_gc<TextField>(meta);

	return meta;
}

static void register_textfield(lua_State* L) {
	LuaValue textfield = luawrap::ensure_table(luawrap::globals(L)["TextInput"]);
	textfield["create"].bind_function(lua_newtextinput);
	luawrap::install_userdata_type<TextField, &lua_textfieldmetatable>();
}

/* END of TextField bindings */

static int get_events(lua_State* L) {
	GameState* gs = lua_api::gamestate(L);
	std::vector<SDL_Event>& events = gs->io_controller().get_events();
	luawrap::push(L, events);
	return 1;
}

static void register_input_table(lua_State* L) {
	LuaValue input = luawrap::ensure_table(luawrap::globals(L)["input"]);

	LuaValue meta = luameta_new(L, "<InputTable>");
	LuaValue getters = luameta_getters(meta);

	getters["events"].bind_function(get_events);

	input.push();
	meta.push();
	lua_setmetatable(L, -2);
	lua_pop(L, 1);
}

// Directory bindings

static FilenameList io_directory_subfiles(const char* path) {
	FilenameList list;
	DirListing listing = list_directory(path);
	for (int i = 0; i < listing.size(); i++) {
		if (!listing[i].is_directory) {
			list.push_back(listing[i].name);
		}
	}
	return list;
}

static FilenameList io_directory_subdirectories(const char* path) {
	FilenameList list;
	DirListing listing = list_directory(path);
	for (int i = 0; i < listing.size(); i++) {
		if (listing[i].is_directory) {
			list.push_back(listing[i].name);
		}
	}
	return list;
}

namespace lua_api {
	void register_io_api(lua_State* L) {
		luawrap::install_plaindata_type<SDL_Event>();

		register_lua_submodule(L, "core.io.SerializeBuffer", lua_serializebuffer_type(L));
		lua_register_lsound(L, register_lua_submodule_as_luamodule(L, "core.Sound"));
		register_input_table(L);

		register_textfield(L);

		// Files API
		LuaValue io = luawrap::ensure_table(luawrap::globals(L)["io"]);
		io["directory_subfiles"].bind_function(io_directory_subfiles);
		io["directory_subdirectories"].bind_function(io_directory_subdirectories);
		io["directory_search"].bind_function(search_directory);
	}
}
