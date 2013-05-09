/*
 * parse_context.h:
 *  Global parse context.
 *  This is used so a lua_State does not have to be passed to many methods.
 */

#ifndef PARSE_CONTEXT_H_
#define PARSE_CONTEXT_H_

struct lua_State;

/* This controls the global parse context
 * when it leaves the scope, the context is unset. */
class ParseContext {
public:
	ParseContext(lua_State* L);
	~ParseContext();
	static lua_State* luastate();
};


#endif /* PARSE_CONTEXT_H_ */
