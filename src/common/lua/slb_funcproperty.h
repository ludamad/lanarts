/*
 * slb_funcproperty.h:
 *  Utility for binding arbitrary properties
 */

#ifndef SLB_FUNCPROPERTY_H_
#define SLB_FUNCPROPERTY_H_

#include <SLB/Property.hpp>
#include <SLB/ClassInfo.hpp>

typedef void (*lua_getsetf)(lua_State *L, int idx);

template<typename T, typename V, V (T::*func)() const>
inline void lua_get_membfunc(lua_State *L, int idx) {
	// get object at T
	const T* obj = SLB::get<const T*>(L, idx);
	if (obj == 0L)
		luaL_error(L, "Invalid object to get a property from");
	// push the property
	SLB::push(L, (obj->*func)());
}

namespace SLB {

class FuncProperty: public BaseProperty {
public:
	FuncProperty(lua_getsetf getf, lua_getsetf setf = NULL) :
			getf(getf), setf(setf) {
	}
	/** gets the property from the object at index idx */
	virtual void set(lua_State *L, int idx) {
		if (setf == 0L) {
			luaL_error(L, "Attempt to assign to read-only property.");
		} else {
			setf(L, idx);
		}
	}
	/** sets the property of the object located at index idx, poping
	 an element from the stack */
	virtual void get(lua_State *L, int idx) {
		if (getf == 0L) {
			luaL_error(L, "Attempt to lookup write-only property.");
		} else {
			getf(L, idx);
		}
	}
private:
	lua_getsetf getf, setf;
};

template<typename T, typename V, V (T::*func)() const>
void addFuncProperty(ClassInfo* class_info, const char* name) {
	class_info->addProperty(name,
			new SLB::FuncProperty(lua_get_membfunc<T, V, func>));
}
}

#endif /* SLB_FUNCPROPERTY_H_ */
