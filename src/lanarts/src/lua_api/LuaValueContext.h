/*
 * LuaValueContext.h:
 *  A LuaValue along with the context of how it was computed.
 *  Meant to be used -only- as stack values!
 *  Useful for decoding object tables as C++ objects.
 */

#ifndef LUAVALUECONTEXT_H_
#define LUAVALUECONTEXT_H_

#include <stdexcept>
#include <string>

#include <luawrap/LuaValue.h>

/* Thrown when an invalid index is passed */
class LuaValueContextIndexError: public std::runtime_error {
public:
	LuaValueContextIndexError(const std::string& msg) :
					runtime_error(msg) {
	}
};

class LuaValueContext {
public:
	/* Construct the context root */
	LuaValueContext(const LuaValue& root, const char* fmt = NULL);

	/* These can throw an error message. */
	LuaValueContext(const LuaValueContext* parent, const char* string_idx,
			const char* fmt);
	LuaValueContext(const LuaValueContext* parent, int integer_idx,
			const char* fmt);

	/* syntax sugar is important for data parsing */
	const LuaValue* operator->() const {
		return &_child;
	}
	const LuaValue& operator*() const {
		return _child;
	}
	const LuaValue& value() const {
		return _child;
	}
	bool has(const char* string_idx) const {
		return !_child[string_idx].isnil();
	}
	template <typename T>
	T defaulted(const char* string_idx, const T& default_value) const {
		return _child[string_idx].defaulted(default_value);
	}
	LuaValueContext operator[](const char* string_idx) const;
	LuaValueContext operator[](int integer_idx) const;
private:
	void trace_object_path(std::string& object_path) const;
	void throw_index_error() const;

	const LuaValue _child;
	const LuaValueContext* _parent;

	int _integer_idx;

	const char* _string_idx;

	const char* _fmt;
};

#endif /* LUAVALUECONTEXT_H_ */
