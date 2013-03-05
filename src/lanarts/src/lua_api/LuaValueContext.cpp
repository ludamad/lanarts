/*
 * LuaValueContext.cpp:
 *  A LuaValue along with the context of how it was computed.
 *  Useful for decoding object tables as C++ objects.
 */

#include <lcommon/strformat.h>

#include "LuaValueContext.h"

LuaValueContext::LuaValueContext(const LuaValue& root, const char* fmt) :
				_child(root),
				_parent(NULL),
				_integer_idx(-1),
				_string_idx(NULL),
				_fmt(fmt) {
	if (!_fmt) {
		_fmt = "Non-existent member 'object%s'\n";
	}
}

LuaValueContext::LuaValueContext(const LuaValueContext* parent,
		const char* string_idx, const char* fmt) :
				_child(parent->value()[string_idx]),
				_parent(parent),
				_integer_idx(-1),
				_string_idx(string_idx),
				_fmt(fmt) {
	if (_child.isnil()) {
		throw_index_error();
	}
}

LuaValueContext::LuaValueContext(const LuaValueContext* parent, int integer_idx,
		const char* fmt) :
				_child(parent->value()[integer_idx]),
				_parent(parent),
				_integer_idx(integer_idx),
				_string_idx(NULL),
				_fmt(fmt) {
	if (_child.isnil()) {
		throw_index_error();
	}
}

LuaValueContext LuaValueContext::operator [](const char* string_idx) const {
	return LuaValueContext(this, string_idx, _fmt);
}

LuaValueContext LuaValueContext::operator [](int integer_idx) const {
	return LuaValueContext(this, integer_idx, _fmt);
}

void LuaValueContext::trace_object_path(std::string& object_path) const {
	if (_parent) {
		_parent->trace_object_path(object_path);
		if (_string_idx != NULL) {
			object_path += format("[\"%s\"]", _string_idx);
		} else {
			object_path += format("[%d]", _integer_idx);
		}
	}
}

void LuaValueContext::throw_index_error() const {
	std::string object_path;
	trace_object_path(object_path);

	throw LuaValueContextIndexError(format(_fmt, object_path.c_str()));
}
