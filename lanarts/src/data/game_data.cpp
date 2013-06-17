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

#include "game_data.h"

/* Components of init_game_data */
void load_tile_data(const FilenameList& filenames);
void load_tileset_data(const FilenameList& filenames);
LuaValue load_sprite_data(lua_State* L, const FilenameList& filenames);

LuaValue load_item_data(lua_State* L, const FilenameList& filenames);
void load_equipment_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemtable);
LuaValue load_projectile_data(lua_State* L, const FilenameList& filenames,
		LuaValue& itemstable);
void load_weapon_data(lua_State* L, const FilenameList& filenames,
		LuaValue* itemstable = NULL);

LuaValue load_spell_data(lua_State* L, const FilenameList& filenames);

LuaValue load_itemgenlist_data(lua_State* L, const FilenameList& filenames);
LuaValue load_enemy_data(lua_State* L, const FilenameList& filenames);
LuaValue load_effect_data(lua_State* L, const FilenameList& filenames);
LuaValue load_dungeon_data(lua_State* L, const FilenameList& filenames);
LuaValue load_class_data(lua_State* L, const FilenameList& filenames);

void load_area_template_data(const FilenameList& filenames);

/* Definition of game data */

std::vector<ClassEntry> game_class_data;
std::vector<EffectEntry> game_effect_data;
std::vector<EnemyEntry> game_enemy_data;
std::vector<ItemGenList> game_itemgenlist_data;
std::vector<TileEntry> game_tile_data;
std::vector<TilesetEntry> game_tileset_data;
std::vector<SpellEntry> game_spell_data;
std::vector<SpriteEntry> game_sprite_data;
std::vector<RoomGenSettings> game_dungeon_yaml;

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

static void update_loading_screen(lua_State* L, int percent, const char* task) {
	luawrap::globals(L)["Engine"].push();

	lua_getfield(L, -1, "loading_screen_draw");
	if (!lua_isnil(L, -1)) {
		luawrap::call<void>(L, percent, task);
	}

	/* pop engine table */
	lua_pop(L, 1);
}

template<class T>
static void __lua_init(lua_State* L, T& t) {
	for (int i = 0; i < t.size(); i++)
		t[i].init(L);
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

	update_loading_screen(L, 0, "Loading Items");
	// --- ITEM DATA ---
	lua_items = load_item_data(L, dfiles.item_files); //new
	update_loading_screen(L, 10, "Loading Projectiles");

	lua_projectiles = load_projectile_data(L, dfiles.projectile_files,
			lua_items);
	update_loading_screen(L, 20, "Loading Spells");

	lua_spells = load_spell_data(L, dfiles.spell_files);
	update_loading_screen(L, 30, "Loading Weapons");

	load_weapon_data(L, dfiles.weapon_files, &lua_items);
	update_loading_screen(L, 40, "Loading Equipment");

	load_equipment_data(L, dfiles.equipment_files, &lua_items);
	update_loading_screen(L, 50, "Loading Enemies");
	// --- ITEM DATA ---

	lua_effects = load_effect_data(L, dfiles.effect_files);
	lua_enemies = load_enemy_data(L, dfiles.enemy_files);
	update_loading_screen(L, 60, "Loading Item Generation Templates");

	load_itemgenlist_data(L, dfiles.itemgenlist_files);

	update_loading_screen(L, 70, "Loading Dungeon Areas");
	load_area_template_data(dfiles.room_template_files);
	update_loading_screen(L, 80, "Loading Dungeon Generation Templates");
	lua_dungeon = load_dungeon_data(L, dfiles.room_files);
	update_loading_screen(L, 90, "Loading Classes");
	lua_dungeon.clear();
	lua_classes = load_class_data(L, dfiles.class_files);
	update_loading_screen(L, 100, "Complete!");

	// TODO clean this up

	lua_api::require(L, "main");

	globals["enemies"] = lua_enemies;
	globals["effects"] = lua_effects;
	globals["items"] = lua_items;
	globals["projectiles"] = lua_projectiles;
	LuaValue weapons = luawrap::ensure_table(globals["weapons"]);
	int ind = 0;
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (dynamic_cast<WeaponEntry*>(&ientry)) {
			WeaponEntry& entry = get_weapon_entry(i);

			weapons[++ind] = lua_items[entry.name];
		}
	}
	LuaValue armour = luawrap::ensure_table(globals["armour"]);
	ind = 0;
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (dynamic_cast<EquipmentEntry*>(&ientry)) {
			if (dynamic_cast<WeaponEntry*>(&ientry) || dynamic_cast<ProjectileEntry*>(&ientry)) {
				continue;
			}
			ItemEntry& entry = get_item_entry(i);

			armour[++ind] = lua_items[entry.name];
		}
	}
	LuaValue consumables = luawrap::ensure_table(globals["consumable"]);
	for (int i = 0; i < game_item_data.size(); i++) {
		ItemEntry& ientry = get_item_entry(i);
		if (!dynamic_cast<EquipmentEntry*>(&ientry)) {
			ItemEntry& entry = get_item_entry(i);

			consumables[entry.name] = lua_items[entry.name];
		}
	}
	globals["spells"] = lua_spells;
	globals["classes"] = lua_classes;

	__lua_init(L, game_enemy_data);
	__lua_init(L, game_effect_data);

	for (int i = 0; i < game_item_data.size(); i++) {
		game_item_data[i]->initialize(L);
	}

	for (int i = 0; i < game_spell_data.size(); i++) {
		game_spell_data[i].initialize(L);
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
