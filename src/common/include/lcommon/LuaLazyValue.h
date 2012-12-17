/*
 * LuaLazyValue.h:
 *  Lazy-resolved lua expression.
 */

#ifndef LUALAZYVALUE_H_
#define LUALAZYVALUE_H_

#include <luawrap/LuaValue.h>

class LuaLazyValue {
public:
	explicit LuaLazyValue(const std::string& expr);
	explicit LuaLazyValue(const LuaValue& value);
	LuaLazyValue();
	~LuaLazyValue();

	// Grabs or initializes value
	LuaValue& get(lua_State* L);

	// Ensure value is initialized
	void initialize(lua_State* L);
	bool empty() const;

	bool is_initialized() const;
private:
	std::string _expression;
	LuaValue _value;
};

#endif
