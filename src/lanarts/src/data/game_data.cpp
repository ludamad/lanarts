/*
 * game_data.cpp:
 *  Handles loading of all the various game data that is described in yaml files
 */

#include <typeinfo>

#include <lua.hpp>

#include <luawrap/luawrap.h>
#include <luawrap/calls.h>

#include <lcommon/lua_lcommon.h>

#include "draw/fonts.h"

#include "lua_api/lua_api.h"

#include "stats/items/EquipmentEntry.h"
#include "stats/items/ItemEntry.h"
#include "stats/items/ProjectileEntry.h"
#include "stats/items/WeaponEntry.h"

#include "game_data.h"

/* Components of init_game_data */
void load_tile_data(const FilenameList& filenames);
void load_tileset_data(const FilenameList& filenames);
LuaValue load_sprite_data(lua_State* L, const FilenameList& filenames);

/// NEW EQUIPMENT CODE
LuaValue load_item_data(lua_State* L, const FilenameList& filenames);
void load_equipment_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable);
LuaValue load_projectile_data(lua_State* L, const FilenameList& filenames,
		LuaValue& itemstable);
void load_weapon_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemstable = NULL);
/// NEW EQUIPMENT CODE

LuaValue load_spell_data(lua_State* L, const FilenameList& filenames);

LuaValue load_itemgenlist_data(lua_State* L, const FilenameList& filenames);
LuaValue load_enemy_data(lua_State* L, const FilenameList& filenames);
LuaValue load_effect_data(lua_State* L, const FilenameList& filenames);
LuaValue load_dungeon_data(lua_State* L, const FilenameList& filenames);
LuaValue load_class_data(lua_State* L, const FilenameList& filenames);

void load_area_template_data(const FilenameList& filenames);

/* Definition of game data */

std::vector<ClassType> game_class_data;
std::vector<EffectEntry> game_effect_data;
std::vector<EnemyEntry> game_enemy_data;
std::vector<ItemGenList> game_itemgenlist_data;
std::vector<TileEntry> game_tile_data;
std::vector<TilesetEntry> game_tileset_data;
std::vector<SpellEntry> game_spell_data;
std::vector<SpriteEntry> game_sprite_data;
std::vector<LevelGenSettings> game_dungeon_yaml;

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
	case EquipmentEntry::PROJECTILE: {
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
sprite_id get_sprite_by_name(const char* name) {
	return get_X_by_name(game_sprite_data, name);
}
spell_id get_spell_by_name(const char* name) {
	return get_X_by_name(game_spell_data, name);
}
tile_id get_tile_by_name(const char* name) {
	return get_X_by_name(game_tile_data, name);
}

TileEntry& get_tile_entry(tile_id id) {
	return game_tile_data.at(id);
}

effect_id get_effect_by_name(const char* name) {
	return get_X_by_name(game_effect_data, name);
}
enemy_id get_enemy_by_name(const char* name, bool error_if_not_found) {
	return get_X_by_name(game_enemy_data, name, error_if_not_found);
}

itemgenlist_id get_itemgenlist_by_name(const char* name,
		bool error_if_not_found) {
	return get_X_by_name(game_itemgenlist_data, name, error_if_not_found);
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

LuaValue lua_sprites, lua_armours, lua_enemies, lua_effects, lua_weapons,
		lua_projectiles, lua_items, lua_dungeon, lua_classes, lua_spells;

LuaValue lua_settings;

static void update_loading_screen(lua_State* L, int percent) {
	lua_api::require(L, "start_menu/loading.lua"); // for system.loading_draw
	luawrap::globals(L)["system"].push();

	lua_getfield(L, -1, "loading_draw");
	if (!lua_isnil(L, -1)) {
		luawrap::call<void>(L, percent);
	}

	lua_pop(L, 1); /* pop system table */
}

void init_game_data(GameSettings& settings, lua_State* L) {
	LuaSpecialValue globals = luawrap::globals(L);

	DataFiles dfiles = load_datafilenames("res/datafiles.yaml");

//NB: Do not re-order the way resources are loaded unless you know what you're doing!
//For example, all item data types must be loaded before room generation data

	load_tile_data(dfiles.tile_files);
	lua_sprites = load_sprite_data(L, dfiles.sprite_files);
	lua_sprites.clear();
	load_tileset_data(dfiles.tileset_files);

	update_loading_screen(L, 0);
	// --- ITEM DATA ---
	lua_items = load_item_data(L, dfiles.item_files); //new
	update_loading_screen(L, 10);

	lua_projectiles = load_projectile_data(L, dfiles.projectile_files,
			lua_items);
	update_loading_screen(L, 20);

	lua_spells = load_spell_data(L, dfiles.spell_files);
	update_loading_screen(L, 30);

	load_weapon_data(L, dfiles.weapon_files, &lua_items);
	update_loading_screen(L, 40);

	load_equipment_data(L, dfiles.equipment_files, &lua_items);
	update_loading_screen(L, 10);
	// --- ITEM DATA ---

	lua_effects = load_effect_data(L, dfiles.effect_files);
	update_loading_screen(L, 10);
	lua_enemies = load_enemy_data(L, dfiles.enemy_files);
	update_loading_screen(L, 10);

	load_itemgenlist_data(L, dfiles.itemgenlist_files);
	update_loading_screen(L, 10);
	load_area_template_data(dfiles.level_template_files);
	update_loading_screen(L, 10);
	lua_dungeon = load_dungeon_data(L, dfiles.level_files);
	update_loading_screen(L, 10);
	lua_dungeon.clear();
	lua_classes = load_class_data(L, dfiles.class_files);
}

static void register_as_global(lua_State* L, LuaValue& value,
		const char* name) {
	if (!value.empty()) {
		value.push();
		lua_setglobal(L, name);
	}
}

template<class T>
static void __lua_init(lua_State* L, T& t) {
	for (int i = 0; i < t.size(); i++)
		t[i].init(L);
}

void init_lua_data(GameState* gs, lua_State* L) {
	//Lua configuration
	lua_lanarts_api(gs, L);

	register_as_global(L, lua_enemies, "enemies");
	register_as_global(L, lua_effects, "effects");
//	register_as_global(L, weapons, "weapons");
	register_as_global(L, lua_items, "items");
	register_as_global(L, lua_projectiles, "projectiles");
	register_as_global(L, lua_spells, "spells");
//	register_as_global(L, sprites, "sprites");
//	register_as_global(L, dungeon, "dungeon");
	register_as_global(L, lua_classes, "classes");

	lua_safe_dofile(L, "res/main.lua");
	__lua_init(L, game_enemy_data);
	__lua_init(L, game_effect_data);
	__lua_init(L, game_spell_data);

	for (int i = 0; i < game_item_data.size(); i++) {
		game_item_data[i]->init(L);
	}

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

void luayaml_push_levelinfo(lua_State* L, const char* name) {
	luayaml_push(lua_dungeon, L, name);
	LANARTS_ASSERT(!lua_isnil(L, -1));
}
