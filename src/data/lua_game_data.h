/*
 * lua_game_data.h:
 *  Provides routines for getting game data from lua arguments
 */

#ifndef LUA_GAME_DATA_H_
#define LUA_GAME_DATA_H_

#include "game_data.h"

struct lua_State;

//Lua argument getters
item_id item_from_lua(lua_State* L, int idx);
class_id class_from_lua(lua_State* L, int idx);
scriptobj_id scriptobject_from_lua(lua_State* L, int idx);
sprite_id sprite_from_lua(lua_State* L, int idx);
spell_id spell_from_lua(lua_State* L, int idx);
tile_id tile_from_lua(lua_State* L, int idx);
effect_id effect_from_lua(lua_State* L, int idx);
enemy_id enemy_from_lua(lua_State* L, int idx);
projectile_id projectile_from_lua(lua_State* L, int idx);
weapon_id weapon_from_lua(lua_State* L, int idx);
tileset_id tileset_from_lua(lua_State* L, int idx);

extern LuaValue lua_sprites, lua_enemies, _lua_items, lua_dungeon, lua_classes, lua_spells;

#endif /* LUA_GAME_DATA_H_ */
