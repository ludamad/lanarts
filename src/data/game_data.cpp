/*
 * game_data.cpp:
 *  Handles loading of all the various game data that is described in yaml files
 */

#include <typeinfo>

#include <lua.hpp>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>

#include <lcommon/lua_lcommon.h>

#include "data/FilenameList.h"

#include "draw/fonts.h"

#include "lua_api/lua_api.h"

#include "stats/items/EquipmentEntry.h"
#include "stats/items/ItemEntry.h"
#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"
#include "stats/items/WeaponEntry.h"

#include "data/ResourceDataSet.h"

#include "game_data.h"

/* Components of init_game_data */
void load_tile_data(const FilenameList& filenames);
void lapi_data_create_enemy(const LuaStackValue& table);
void lapi_data_create_item(const LuaStackValue& table);
void lapi_data_create_class(const LuaStackValue& table);
void lapi_data_create_spell(const LuaStackValue& table);
void lapi_data_create_effect(const LuaStackValue& table);
void lapi_data_create_projectile(const LuaStackValue& table);
void lapi_data_create_equipment(const LuaStackValue& table);
void lapi_data_create_weapon(const LuaStackValue& table);

/* Definition of game data */

ResourceDataSet<ClassEntry> game_class_data;
ResourceDataSet<EffectEntry> game_effect_data;
ResourceDataSet<EnemyEntry> game_enemy_data;
ResourceDataSet<TileEntry> game_tile_data;
ResourceDataSet<TilesetEntry> game_tileset_data;
ResourceDataSet<SpellEntry> game_spell_data;
ResourceDataSet<SpriteEntry> game_sprite_data;

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
template<typename E>
static E& get_X_ref_by_name(std::vector<E>& t, const char* name) {
	for (int i = 0; i < t.size(); i++) {
		if (name == t.at(i).name) {
			return t.at(i);
		}
	}
	/*Error if resource not found*/
	fprintf(stderr, "Failed to load resource!\nname: %s, of type %s\n", name,
			typeid(t[0]).name());
	fflush(stderr);
	LANARTS_ASSERT(false /*resource not found*/);
	return t.at(-1); /* throws */
}

const char* equip_type_description(const ItemEntry& ientry) {
	const EquipmentEntry* eentry = dynamic_cast<const EquipmentEntry*>(&ientry);
	if (!eentry) {
		return "One-time Use";
	}

	switch (eentry->type) {
	case EquipmentEntry::BODY_ARMOUR:
		return "Armour";
	case EquipmentEntry::WEAPON:
		return "Weapon";
	case EquipmentEntry::AMMO: {
		const ProjectileEntry* pentry =
				dynamic_cast<const ProjectileEntry*>(eentry);
		if (pentry->is_standalone()) {
			return "Unarmed Projectile";
		} else {
			return "Projectile";
		}
	}
	case EquipmentEntry::NONE:
		return "One-time Use";
	}
	return "";
}

class_id get_class_by_name(const char* name) {
	return get_X_by_name(game_class_data, name);
}
sprite_id get_sprite_by_name(const char* name, bool error_if_not_found) {
	return get_X_by_name(game_sprite_data, name, error_if_not_found);
}
spell_id get_spell_by_name(const char* name) {
	return get_X_by_name(game_spell_data, name);
}
tile_id get_tile_by_name(const char* name) {
	return get_X_by_name(game_tile_data, name);
}

TileEntry& get_tile_entry(tile_id id) {
	return game_tile_data.get(id);
}

effect_id get_effect_by_name(const char* name) {
	return get_X_by_name(game_effect_data, name);
}
enemy_id get_enemy_by_name(const char* name, bool error_if_not_found) {
	return get_X_by_name(game_enemy_data, name, error_if_not_found);
}

tileset_id get_tileset_by_name(const char* name) {
	return get_X_by_name(game_tileset_data, name);
}

//Lua argument getters
item_id item_from_lua(lua_State* L, int idx) {
	return get_item_by_name(lua_tostring(L, idx));
}
class_id class_from_lua(lua_State* L, int idx) {
	return get_X_by_name(game_class_data, lua_tostring(L, idx));
}
sprite_id sprite_from_lua(lua_State* L, int idx) {
	return get_X_by_name(game_sprite_data, lua_tostring(L, idx));
}
spell_id spell_from_lua(lua_State* L, int idx) {
	return get_X_by_name(game_spell_data, lua_tostring(L, idx));
}
tile_id tile_from_lua(lua_State* L, int idx) {
	return get_X_by_name(game_tile_data, lua_tostring(L, idx));
}
effect_id effect_from_lua(lua_State* L, int idx) {
	return get_X_by_name(game_effect_data, lua_tostring(L, idx));
}
enemy_id enemy_from_lua(lua_State* L, int idx) {
	return get_X_by_name(game_enemy_data, lua_tostring(L, idx));
}
projectile_id projectile_from_lua(lua_State* L, int idx) {
	return get_projectile_by_name(lua_tostring(L, idx));
}
weapon_id weapon_from_lua(lua_State* L, int idx) {
	return get_weapon_by_name(lua_tostring(L, idx));
}
tileset_id tileset_from_lua(lua_State* L, int idx) {
	return get_X_by_name(game_tileset_data, lua_tostring(L, idx));
}

LuaValue lua_sprites, lua_enemies, lua_items, lua_classes, lua_spells;

LuaValue lua_settings;

template<class T>
static void __lua_init(lua_State* L, T& t) {
	for (int i = 0; i < t.size(); i++)
		t[i].init(L);
}

LuaValue load_sprite_data(lua_State* L, const FilenameList& filenames);
void init_game_data(lua_State* L) {
    game_class_data.clear();
    game_effect_data.clear();
    game_enemy_data.clear();
    game_tile_data.clear();
    game_tileset_data.clear();
    game_spell_data.clear();
    game_sprite_data.clear();

	LuaSpecialValue globals = luawrap::globals(L);
        // D: The table that holds all the resource data
	LuaValue D = luawrap::ensure_table(globals["Data"]);


        // Compatibility
	DataFiles dfiles = load_datafilenames("datafiles.yaml");
	load_tile_data(dfiles.tile_files);
    load_sprite_data(L, dfiles.sprite_files);

    globals["sprites"].newtable();
    globals["enemies"].newtable();
    globals["items"].newtable();
    globals["classes"].newtable();
    globals["spells"].newtable();

    lua_sprites = globals["sprites"];
    lua_enemies = globals["enemies"];
    lua_items = globals["items"];
    lua_classes = globals["classes"];
    lua_spells = globals["spells"];

    D["enemy_create"].bind_function(lapi_data_create_enemy);
    D["class_create"].bind_function(lapi_data_create_class);
    D["item_create"].bind_function(lapi_data_create_item);
    D["spell_create"].bind_function(lapi_data_create_spell);
    D["effect_create"].bind_function(lapi_data_create_effect);
    D["projectile_create"].bind_function(lapi_data_create_projectile);
    D["equipment_create"].bind_function(lapi_data_create_equipment);
    D["weapon_create"].bind_function(lapi_data_create_weapon);
}

static void luayaml_push(LuaValue& value, lua_State* L, const char* name) {
	value.push();
	int tableind = lua_gettop(L);
	lua_getfield(L, tableind, name);
	lua_replace(L, tableind);
}
void luayaml_push_item(lua_State* L, const char* name) {
	luayaml_push(lua_items, L, name);
	LANARTS_ASSERT(!lua_isnil(L, -1));
}
void luayaml_push_sprites(lua_State* L, const char* name) {
	luayaml_push(lua_sprites, L, name);
	LANARTS_ASSERT(!lua_isnil(L, -1));
}
void luayaml_push_enemies(lua_State* L, const char* name) {
	luayaml_push(lua_enemies, L, name);
	LANARTS_ASSERT(!lua_isnil(L, -1));
}
