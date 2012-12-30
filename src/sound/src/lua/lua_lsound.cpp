/*
 * lua_lsound.cpp:
 *  Bindings for the lsound component
 */

#include <luawrap/luameta.h>
#include <luawrap/types.h>
#include <luawrap/functions.h>

#include "lsound.h"
#include "lua_lsound.h"

using namespace lsound;

static void play(const Sound& snd) {
	snd.play();
}

static void loop(const Sound& snd) {
	snd.loop();
}
static void clear(Sound& snd) {
	snd.clear();
}

LuaValue lua_soundmetatable(lua_State* L) {
	LuaValue meta = luameta_new(L, "Sound");
	LuaValue methods = luameta_methods(meta);

	methods["play"].bind_function(play);
	methods["loop"].bind_function(loop);
	methods["free"].bind_function(clear);

	luameta_gc<Sound>(meta);

	return meta;
}

void lua_register_lsound(lua_State* L) {
	luawrap::install_userdata_type<Sound, lua_soundmetatable>();
	LuaSpecialValue globals = luawrap::globals(L);
	globals["music_load"].bind_function(load_music);
	globals["sound_load"].bind_function(load_sound);
}
