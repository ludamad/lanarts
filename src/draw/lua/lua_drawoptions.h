/*
 * lua_drawoptions.h:
 *  Bindings for drawing options <-> lua table
 */

#ifndef LUA_DRAWOPTIONS_H_
#define LUA_DRAWOPTIONS_H_

#include <common/lua/luacpp_wrap.h>

#include "../DrawOptions.h"

void lua_push_drawoptions(lua_State* L, const ldraw::DrawOptions& options);
ldraw::DrawOptions lua_get_drawoptions(lua_State* L, int idx);
bool lua_check_drawoptions(lua_State* L, int idx);

LUACPP_TYPE_WRAP(ldraw::DrawOptions, lua_push_drawoptions,
		lua_get_drawoptions, lua_check_drawoptions);

class LuaValue;
namespace ldraw {
// Register draworigin constants
void lua_register_draworigin_constants(lua_State* L, const LuaValue& module);

}
#endif /* LUA_DRAWOPTIONS_H_ */
