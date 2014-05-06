/*
 * lua_drawoptions.h:
 *  Bindings for drawing options <-> lua table
 */

#ifndef LUA_DRAWOPTIONS_H_
#define LUA_DRAWOPTIONS_H_

#include "DrawOptions.h"

struct lua_State;
class LuaValue;

void lua_push_drawoptions(lua_State* L, const ldraw::DrawOptions& options);
ldraw::DrawOptions lua_get_drawoptions(lua_State* L, int idx);
bool lua_check_drawoptions(lua_State* L, int idx);

class LuaValue;
namespace ldraw {
// Register draworigin constants
void lua_register_drawoptions(lua_State* L, const LuaValue& module);

}
#endif /* LUA_DRAWOPTIONS_H_ */
