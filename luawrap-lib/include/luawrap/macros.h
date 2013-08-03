/*
 * luawrap_macros.h:
 * 	Macros for use with luawrap.
 * 	The macros are ugly, and rely on naming conventions, but at least they should cut down on boilerplate.
 *	They should be avoided, unless an object has many small getters & setters.
 */

#ifndef LUAWRAP_MACROS_H_
#define LUAWRAP_MACROS_H_

#include <luawrap/luawrap.h>
#include <luawrap/functions.h>


#define LUAWRAP_SET_TYPE(x) typedef x LuawrapMacroTypeToWrap;

//// Allows for one line method wrapping
//// TODO: Argument typecheck
#define LUAWRAP_METHOD(table, func_name, body) \
	struct MacroGeneratedMethod_##func_name { \
		static int FUNC##func_name (lua_State* L) { \
			int nargs = lua_gettop(L); \
			LuawrapMacroTypeToWrap OBJ = luawrap::get<LuawrapMacroTypeToWrap>(L, 1); \
			body ; \
			return lua_gettop(L) - nargs; \
		} \
	}; \
	table[ #func_name ].bind_function( MacroGeneratedMethod_##func_name :: FUNC##func_name )

//// Allows for one line getter wrapping
//// TODO: Argument typecheck
#define LUAWRAP_GETTER(table, func_name, body) \
	struct MacroGeneratedGetter_##func_name { \
		static int FUNC##func_name  (lua_State* L) { \
			LuawrapMacroTypeToWrap OBJ = luawrap::get<LuawrapMacroTypeToWrap>(L, 1); \
			luawrap::push(L, body ); \
			return 1; \
		} \
	}; \
	table[ #func_name ].bind_function( MacroGeneratedGetter_##func_name :: FUNC##func_name  )

//// Allows for one line setter wrapping
//// TODO: Argument typecheck
#define LUAWRAP_SETTER(table, func_name, param_type, body) \
	struct MacroGeneratedSetter_##func_name { \
		static int FUNC##func_name  (lua_State* L) { \
			LuawrapMacroTypeToWrap OBJ = luawrap::get<LuawrapMacroTypeToWrap>(L, 1); \
			param_type VAL = luawrap::get<param_type>(L, 3); \
			body ; \
			return 0; \
		} \
	}; \
	table[ #func_name ].bind_function( MacroGeneratedSetter_##func_name :: FUNC##func_name  )

#endif /* LUAWRAP_MACROS_H_ */
