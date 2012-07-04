/*
 * game_data.cpp:
 *  Handles loading of all the various game data that is described in yaml files
 */

#include <typeinfo>

extern "C" {
#include <lua/lua.h>
#include <lua/lauxlib.h>
}

#include "game_data.h"

#include "../lua/lua_api.h"

/* Components of init_game_data */
void load_tile_data(const FilenameList& filenames);
void load_tileset_data(const FilenameList& filenames);
LuaValue load_sprite_data(lua_State* L, const FilenameList& filenames);
void load_armour_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable);
void load_weapon_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemstable = NULL);
LuaValue load_spell_data(lua_State* L, const FilenameList& filenames);
LuaValue load_projectile_data(lua_State* L, const FilenameList& filenames,
		LuaValue& itemstable);
void load_armour_item_entries();
void load_weapon_item_entries();
void load_projectile_item_entries();

LuaValue load_item_data(lua_State* L, const FilenameList& filenames);
LuaValue load_enemy_data(lua_State* L, const FilenameList& filenames);
LuaValue load_effect_data(lua_State* L, const FilenameList& filenames);
LuaValue load_dungeon_data(lua_State* L, const FilenameList& filenames);
LuaValue load_class_data(lua_State* L, const FilenameList& filenames);

/* Definition of game data */

std::vector<ArmourEntry> game_armour_data;
std::vector<ClassType> game_class_data;
std::vector<EffectEntry> game_effect_data;
std::vector<EnemyEntry> game_enemy_data;
std::vector<ItemEntry> game_item_data;
std::vector<TileEntry> game_tile_data;
std::vector<TilesetEntry> game_tileset_data;
std::vector<ProjectileEntry> game_projectile_data;
std::vector<SpellEntry> game_spell_data;
std::vector<SpriteEntry> game_sprite_data;
std::vector<LevelGenSettings> game_dungeon_yaml;
std::vector<WeaponEntry> game_weapon_data;

DungeonBranch game_dungeon_data[1] = { };

template<typename T>
static int get_X_by_name(const T& t, const char* name, bool error_if_not_found =
		true) {
	for (int i = 0; i < t.size(); i++) {
		if (name == t.at(i).name) {
			return i;
		}
	}
	if (error_if_not_found) {
		/*Error if resource not found*/
		fprintf(stderr, "Failed to load resource!\nname: %s, of type %s\n",
				name, typeid(t[0]).name());
		fflush(stderr);
		LANARTS_ASSERT(false /*resource not found*/);
	}
	return -1;
}

int get_armour_by_name(const char *name, bool error_if_not_found) {
	return get_X_by_name(game_armour_data, name, error_if_not_found);
}
const char* equip_type_description(ItemEntry::equip_type equipment_type) {
	switch (equipment_type) {
	case ItemEntry::ARMOUR:
		return "Armour";
	case ItemEntry::WEAPON:
		return "Weapon";
	case ItemEntry::PROJECTILE:
		return "Projectile";
	case ItemEntry::NONE:
		return "One-time Use";
	}
	return "";
}

item_id get_item_by_name(const char* name, bool error_if_not_found) {
	return get_X_by_name(game_item_data, name, error_if_not_found);
}
class_id get_class_by_name(const char* name) {
	return get_X_by_name(game_class_data, name);
}
sprite_id get_sprite_by_name(const char* name) {
	return get_X_by_name(game_sprite_data, name);
}
spell_id get_spell_by_name(const char* name) {
	return get_X_by_name(game_spell_data, name);
}
tile_id get_tile_by_name(const char* name) {
	return get_X_by_name(game_tile_data, name);
}
effect_id get_effect_by_name(const char* name) {
	return get_X_by_name(game_effect_data, name);
}
enemy_id get_enemy_by_name(const char* name, bool error_if_not_found) {
	return get_X_by_name(game_enemy_data, name, error_if_not_found);
}
projectile_id get_projectile_by_name(const char* name) {
	return get_X_by_name(game_projectile_data, name);
}
weapon_id get_weapon_by_name(const char* name) {
	return get_X_by_name(game_weapon_data, name);
}

tileset_id get_tileset_by_name(const char* name) {
	return get_X_by_name(game_tileset_data, name);
}

LuaValue sprites, armours, enemies, effects, weapons, projectiles, items,
		dungeon, classes, spells;
void init_game_data(lua_State* L) {
	DataFiles dfiles = load_datafiles_data("res/datafiles.yaml");

//NB: Do not re-order the way resources are loaded unless you know what you're doing
	load_tile_data(dfiles.tile_files);
	sprites = load_sprite_data(L, dfiles.sprite_files);
	sprites.deinitialize(L);
	load_tileset_data(dfiles.tileset_files);
	//TODO: make separate weapons table ?
	items = load_item_data(L, dfiles.item_files);

	projectiles = load_projectile_data(L, dfiles.projectile_files, items);
	load_projectile_item_entries();

	spells = load_spell_data(L, dfiles.spell_files);

	load_weapon_data(L, dfiles.weapon_files, &items);
	load_weapon_item_entries();

	load_armour_data(L, dfiles.armour_files, &items);
	load_armour_item_entries();

	effects = load_effect_data(L, dfiles.effect_files);
	enemies = load_enemy_data(L, dfiles.enemy_files);
	dungeon = load_dungeon_data(L, dfiles.level_files);
	dungeon.deinitialize(L);
	classes = load_class_data(L, dfiles.class_files);
}

static void register_as_global(lua_State* L, LuaValue& value,
		const char* name) {
	value.push(L);
	lua_setglobal(L, name);
}

template<class T>
static void __lua_init(lua_State* L, T& t) {
	for (int i = 0; i < t.size(); i++)
		t[i].init(L);
}

void init_lua_data(GameState* gs, lua_State* L) {
	//Lua configuration
	lua_lanarts_api(gs, L);

	register_as_global(L, enemies, "enemies");
	register_as_global(L, effects, "effects");
//	register_as_global(L, weapons, "weapons");
	register_as_global(L, items, "items");
	register_as_global(L, projectiles, "projectiles");
	register_as_global(L, spells, "spells");
//	register_as_global(L, sprites, "sprites");
//	register_as_global(L, dungeon, "dungeon");
	register_as_global(L, classes, "classes");

	luaL_dofile(L, "res/main.lua");

	__lua_init(L, game_enemy_data);
	__lua_init(L, game_effect_data);
	__lua_init(L, game_item_data);

//	lua_getglobal(L, "level_tests");
//	lua_call(L, 0, 0);
}

static void luayaml_push(LuaValue& value, lua_State *L, const char* name) {
	value.push(L);
	int tableind = lua_gettop(L);
	lua_getfield(L, tableind, name);
	lua_replace(L, tableind);
}
void luayaml_push_item(lua_State *L, const char* name) {
	luayaml_push(items, L, name);
}
void luayaml_push_sprites(lua_State *L, const char* name) {
	luayaml_push(sprites, L, name);
}
void luayaml_push_enemies(lua_State *L, const char* name) {
	luayaml_push(enemies, L, name);
}
void luayaml_push_levelinfo(lua_State *L, const char* name) {
	luayaml_push(dungeon, L, name);
}
