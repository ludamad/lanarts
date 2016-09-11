/*
 * lua_lsound.cpp:
 *  Bindings for the lsound component
 */

#include <SDL_mixer.h>

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
	LuaValue methods = luameta_constants(meta);

	methods["play"].bind_function(play);
	methods["loop"].bind_function(loop);
	methods["free"].bind_function(clear);

	luameta_gc<Sound>(meta);

	return meta;
}

static void set_volume(LuaStackValue table, LuaStackValue key, double volume) {
	Mix_Volume(-1, volume / double(MIX_MAX_VOLUME));
}

static double get_volume(LuaStackValue table, LuaStackValue key) {
	return Mix_Volume(-1, -1) / double(MIX_MAX_VOLUME);
}

void lua_register_lsound(lua_State* L, const LuaModule& module) {
	luawrap::install_userdata_type<Sound, lua_soundmetatable>();

	module.values["music_load"].bind_function(load_music);
	module.values["sound_load"].bind_function(load_sound);
	module.setters["sound_volume"].bind_function(set_volume);
	module.getters["sound_volume"].bind_function(get_volume);
}
