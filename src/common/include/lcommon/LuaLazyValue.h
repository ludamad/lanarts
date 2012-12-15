/*
 * LuaLazyValue.h:
 *  Lazy-resolved lua expression.
 */

#ifndef LUALAZYVALUE_H_
#define LUALAZYVALUE_H_

#include <luawrap/LuaValue.h>

class LuaLazyValue {
public:
	LuaLazyValue(const std::string& expr);
	LuaLazyValue(const LuaValue& value);
	~LuaLazyValue();

	// Grabs or initializes value
	const LuaValue& get(lua_State* L);

	// Ensure value is initialized
	void initialize(lua_State* L);

	bool is_initialized() const;
private:
	std::string _expression;
	LuaValue _value;
};

#endif
