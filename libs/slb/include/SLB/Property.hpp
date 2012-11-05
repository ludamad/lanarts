/*
    SLB - Simple Lua Binder
    Copyright (C) 2007-2011 Jose L. Hidalgo Valiño (PpluX)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  
  Jose L. Hidalgo (www.pplux.com)
  pplux@pplux.com
*/



#ifndef __SLB_PROPERTY__
#define __SLB_PROPERTY__

#include "Export.hpp"
#include "Object.hpp"
#include "PushGet.hpp"
#include "ref_ptr.hpp"
#include <map>

#include <iostream>

struct lua_State;

namespace SLB {

  class SLB_EXPORT BaseProperty: public Object
  {
  public:
    typedef SLB_Map( String, ref_ptr<BaseProperty> ) Map;

    template<class T, class M>
    static BaseProperty* create(M T::*prop);

    /** gets the property from the object at index idx */
    virtual void set(lua_State *, int idx);
    /** sets the property of the object located at index idx, poping
        an element from the stack */
    virtual void get(lua_State *, int idx);

  
  protected:
    BaseProperty();
    virtual ~BaseProperty();

    virtual void pushImplementation(lua_State *);
    SLB_CLASS(BaseProperty, Object);
  };

  template<class T, class M>
  class Property: public virtual BaseProperty
  {
  public:
    typedef M T::*MemberPtr;
    Property( MemberPtr m) : _m(m) 
    {
      
    }
  protected:
    virtual ~Property() {}

    virtual void get(lua_State *L, int idx)
    {
      // get object at T
      const T *obj = SLB::get<const T*>(L,idx);
      if (obj == 0L) luaL_error(L, "Invalid object to get a property from");
      // push the property
      SLB::push(L, obj->*_m);
    }
    virtual void set(lua_State *L, int idx)
    {
      // get object at T
      T *obj = SLB::get<T*>(L,idx);
      if (obj == 0L) luaL_error(L, "Invalid object to set a property from");
      // set the property
      obj->*_m = SLB::get<M>(L,-1);

      lua_pop(L,1); // remove the last element
    }
  private:
    MemberPtr _m;
    typedef Property<T,M> T_This;
    SLB_CLASS(T_This, BaseProperty);
  };


  template<class T, class M>
  inline BaseProperty* BaseProperty::create(M T::*p)
  {
    typedef Property<T,M> Prop;
    Prop *nprop = new (SLB::Malloc(sizeof(Prop))) Prop(p);
    return nprop;
  }

}


#endif
