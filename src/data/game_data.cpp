/*
 * game_data.cpp
 *
 *  Created on: Mar 25, 2012
 *      Author: 100397561
 */


#include "game_data.h"

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}
#include "../world/lua/lua_api.h"

std::vector<ClassType> game_class_data;
std::vector<ItemEntry> game_item_data;
std::vector<TileEntry> game_tile_data;
std::vector<TilesetEntry> game_tileset_data;
std::vector<SpellEntry> game_spell_data;
std::vector<SpriteEntry> game_sprite_data;
std::vector<EnemyEntry> game_enemy_data;
std::vector<LevelGenSettings> game_dungeon_yaml;
std::vector<WeaponEntry> game_weapon_data;

DungeonBranch game_dungeon_data[1] = {  };

int get_class_by_name(const char* name){
	for (int i = 0; i < game_class_data.size(); i++){
		if (strcmp(name, game_class_data[i].name) == 0){
			return i;
		}
	}
	LANARTS_ASSERT(false);
	return 0;
}
int get_tile_by_name(const char *name){
	for (int i = 0; i < game_tile_data.size(); i++){
		if (strcmp(name, game_tile_data[i].name) == 0){
			return i;
		}
	}
	LANARTS_ASSERT(false);
	return 0;
}

int get_tileset_by_name(const char* name){
	for (int i = 0; i < game_tileset_data.size(); i++){
		if (name == game_tileset_data[i].name){
			return i;
		}
	}
	LANARTS_ASSERT(false);
	return 0;
}

LuaValue sprites, enemies, weapons, items, dungeon, classes;
void init_game_data(lua_State* L){

//NB: Do not re-order the way resources are loaded unless you know what you're doing
	load_tile_data("res/tiles.yaml");
	sprites = load_sprite_data(L, "res/sprites.yaml");
	load_tileset_data("res/tileset.yaml");
	enemies = load_enemy_data(L, "res/enemies.yaml");
	//TODO: make separate weapons table
	items = load_item_data(L, "res/items.yaml");
	load_weapon_data(L, "res/weapons.yaml",  &items);
	//TODO: remove lua_state arg
	load_weapon_item_entries(L);
	dungeon = load_dungeon_data(L, "res/levels.yaml");
	classes = load_class_data(L, "res/classes.yaml");
}

static void register_as_global(lua_State* L, LuaValue& value, const char* name){
	lua_pushstring(L, name);
	value.push(L);
	lua_settable(L, LUA_GLOBALSINDEX);
}

void init_lua_data(GameState* gs, lua_State* L){
	//Lua configuration
	lua_lanarts_api(gs, L);

	register_as_global(L, enemies, "enemies");
//	register_as_global(L, weapons, "weaponss");
	register_as_global(L, items, "items");
	register_as_global(L, sprites, "sprites");
	register_as_global(L, dungeon, "dungeon");
	register_as_global(L, classes, "classes");

	luaL_dofile(L, "res/lua/defines.lua");

	for (int i = 0; i < game_enemy_data.size(); i++){
		game_enemy_data[i].init(L);
	}
	for (int i = 0; i < game_item_data.size(); i++){
		game_item_data[i].init(L);
	}

}

static void luayaml_push(LuaValue& value, lua_State *L, const char *name){
	value.push(L);
	int tableind = lua_gettop(L);
	lua_pushstring(L, name);
	lua_gettable(L, tableind);
	lua_replace(L, tableind);
}
void luayaml_push_item(lua_State *L, const char *name){
	luayaml_push(items, L, name);
}
void luayaml_push_sprites(lua_State *L, const char *name){
	luayaml_push(sprites, L, name);
}
void luayaml_push_enemies(lua_State *L, const char *name){
	luayaml_push(enemies, L, name);
}
void luayaml_push_levelinfo(lua_State *L, const char *name){
	luayaml_push(dungeon, L, name);
}
