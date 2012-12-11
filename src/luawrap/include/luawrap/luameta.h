/*
 * luacpp_metatable.h:
 *  Helpers for creating a flexible metatable for lua classes holding C++ data.
 *  Relies on luacpp_get
 */

#ifndef LUAWRAP_LUAMETA_H_
#define LUAWRAP_LUAMETA_H_

struct LuaState;

#include <luawrap/LuaValue.h>

/*
 * Creates a new metatable that has getters, setters, and methods cleanly separated.
 * Getters and setters are checked first, and then a method is looked-up.
 */
LuaValue luameta_new(lua_State* L, const char* classname);

/*
 * Used to create a new meta-table.
 * The meta-table is then cached using the function address as a key.
 * Note that the initializer does not need to be heavily optimized.
 * It will be lazily called, and only once.
 */
typedef LuaValue (*luameta_initializer)(lua_State* L);

/*
 * Pushes cached metatable if exists, or initializes cache and then pushes.
 */
void luameta_push(lua_State* L, luameta_initializer initfunc);

/*
 * Pushes a new, uninitialized userdata with the given metatable.
 * Note that the metatable is cached (via luameta_push).
 */
void* luameta_newuserdata(lua_State* L, luameta_initializer initfunc, size_t size);

/*
 * Retrieve the getter-table.
 */
LuaValue luameta_getters(lua_State* L, const LuaValue& metatable);

/*
 * Retrieve the setter-table.
 */
LuaValue luameta_setters(lua_State* L, const LuaValue& metatable);

/*
 * Retrieve the method (and class variable) table.
 */
LuaValue luameta_methods(lua_State* L, const LuaValue& metatable);

#endif /* LUAWRAP_LUAMETA_H_ */
