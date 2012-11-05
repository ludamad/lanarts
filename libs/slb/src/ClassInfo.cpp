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



#include <SLB/ClassInfo.hpp>
#include <SLB/Manager.hpp>
#include <SLB/Hybrid.hpp>
#include <SLB/Debug.hpp>

namespace SLB {

  ClassInfo::ClassInfo(Manager *m, const TypeInfoWrapper &ti) :
    Namespace(true),
    _manager(m),
    __TIW(ti), 
    _name(""), 
    _instanceFactory(0),
    _isHybrid(false)
  {
    SLB_DEBUG_CALL;
    _name = __TIW.name();
  }

  ClassInfo::~ClassInfo()
  {
    SLB_DEBUG_CALL;
    _baseClasses.clear(); // force deletion of memory
    _properties.clear(); // force deletion of properties
    Free_T(&_instanceFactory); 
  }

  void ClassInfo::setName(const String& name)
  {
    SLB_DEBUG_CALL;
    // rename first in the manager...
    _manager->rename(this, name);
    _name = name;
  }
  
  void ClassInfo::setInstanceFactory( InstanceFactory *factory)
  {
    SLB_DEBUG_CALL;
    Free_T(&_instanceFactory); // delete old..
    _instanceFactory = factory;
  }

  void ClassInfo::setConstructor( FuncCall *constructor )
  {
    SLB_DEBUG_CALL;
    _constructor = constructor;
  }
  
  void ClassInfo::setClass__index( FuncCall *func )
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(2, "ClassInfo(%p) '%s' set Class __index -> %p", this, _name.c_str(), func);
    _meta__index[0] = func;
  }

  void ClassInfo::setClass__newindex( FuncCall *func )
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(2, "ClassInfo(%p) '%s' set Class __newindex -> %p", this, _name.c_str(), func);
    _meta__newindex[0] = func;
  }

  void ClassInfo::setObject__index( FuncCall *func )
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(2, "ClassInfo(%p) '%s' set Object __index -> %p", this, _name.c_str(), func);
    _meta__index[1] = func;
  }

  void ClassInfo::setObject__newindex( FuncCall *func )
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(2, "ClassInfo(%p) '%s' set Object __newindex -> %p", this, _name.c_str(), func);
    _meta__newindex[1] = func;
  }

  void ClassInfo::set__eq( FuncCall *func )
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(2, "ClassInfo(%p) '%s' set __eq -> %p", this, _name.c_str(), func);
    _meta__eq = func;
  }

  void ClassInfo::pushImplementation(lua_State *L)
  {
    Table::pushImplementation(L);
    lua_getmetatable(L, -1);
    
    // To a void GC collect the ClassInfo we need to make
    // a reference back to it's metatable. Objects will only use
    // the METATABLe and not this TABLE... but the metatable its
    // linked to this table...
    lua_pushstring(L,"__back_reference");
    lua_pushvalue(L,-3);
    lua_rawset(L,-3);

    /*
    lua_pushstring(L, "__objects");
    lua_newtable(L);
    lua_rawset(L, -3);
    */

    lua_pushstring(L, "__class_ptr");
    lua_pushlightuserdata(L, (void*)this);
    lua_rawset(L, -3);

    lua_pop(L,1); // remove metatable
  }

  void ClassInfo::pushInstance(lua_State *L, InstanceBase *instance)
  {
    SLB_DEBUG_CALL;
    int top = lua_gettop(L);
    if (instance == 0)
    {
      SLB_DEBUG(7, "Push(%s) Invalid!!", _name.c_str());
      lua_pushnil(L);
      return;
    }
    
    InstanceBase **obj = reinterpret_cast<InstanceBase**>
      (lua_newuserdata(L, sizeof(InstanceBase*))); // top+1
    *obj = instance;

    SLB_DEBUG(7, "Push(%s) InstanceHandler(%p) -> ptr(%p) const_ptr(%p)",
        _name.c_str(), instance, instance->get_ptr(), instance->get_const_ptr());

    // get metatable (class table's metatable)
    push(L);  // (table) top+2
    lua_getmetatable(L,-1);
    lua_replace(L,-2);

    lua_pushvalue(L,-1);
    lua_setmetatable(L, top+1);

    /* // keep a copy
    if (instance->keepCopyAsCache())
    {
      lua_getfield(L, top+2, "__objects");
      lua_pushlightuserdata(L, const_cast<void*>(instance->get_const_ptr()) );
      lua_pushvalue(L,top+1);
      lua_rawset(L, -3);
    }
    */

    lua_settop(L, top+1);

  }

  void *ClassInfo::get_ptr(lua_State *L, int pos) const
  {
    SLB_DEBUG_CALL;
    pos = lua_absindex(L,pos);
    void *obj = 0;
    InstanceBase *i = getInstance(L, pos);
    if (i)
    {
      ClassInfo *i_ci = i->getClass();
      assert("Invalid ClassInfo" && (i_ci != 0));
      obj = _manager->convert( 
          i_ci->getTypeid(), 
          getTypeid(), 
          i->get_ptr()
        );
    }
    SLB_DEBUG(7, "Class(%s) get_ptr at %d -> %p", _name.c_str(), pos, obj);
    return obj;
  }

  const void* ClassInfo::get_const_ptr(lua_State *L, int pos) const
  {
    SLB_DEBUG_CALL;
    pos = lua_absindex(L,pos);
    const void *obj = 0;
    InstanceBase *i = getInstance(L, pos);
    if (i)
    {
      ClassInfo *i_ci = i->getClass();
      assert("Invalid ClassInfo" && (i_ci != 0));
      obj = _manager->convert( 
          i_ci->getTypeid(),
          getTypeid(),
          i->get_const_ptr() );
    }
    SLB_DEBUG(7, "Class(%s) get_const_ptr -> %p", _name.c_str(), obj);
    return obj;
  }

  InstanceBase* ClassInfo::getInstance(lua_State *L, int pos) const
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(10, "L=%p; Pos = %i (abs = %i)",L, pos, lua_absindex(L,pos) );
    pos = lua_absindex(L,pos);
    InstanceBase *instance = 0;
    int top = lua_gettop(L);
    if (lua_getmetatable(L, pos))
    {
      lua_getfield(L, -1, "__class_ptr");
      if (!lua_isnil(L,-1))
      {
        void *obj = lua_touserdata(L, pos);
        if (obj == 0) 
        {
          luaL_error(L,
            "Expected object of type %s at #%d", 
            _name.c_str(), pos);
        }
        instance = *reinterpret_cast<InstanceBase**>(obj);
      }
    }
    lua_settop(L, top);
    SLB_DEBUG(10, "Instance(%p) -> %p (%s)",
      instance,
      instance? instance->get_const_ptr() : 0,
      instance? (instance->get_const_ptr() ? "const" : "non_const") : "nil");
    return instance;
  }

  BaseProperty* ClassInfo::getProperty(const String &key)
  {
    BaseProperty::Map::iterator prop = _properties.find(key);
    BaseProperty::Map::const_iterator end = _properties.end();
    for(BaseClassMap::iterator i = _baseClasses.begin(); (prop == end) && i != _baseClasses.end(); ++i)
    {
      prop = i->second->_properties.find(key);
      end = i->second->_properties.end();
    }

    if (prop != end) return prop->second.get();
    return 0;
  }

  void ClassInfo::push_ref(lua_State *L, void *ref )
  {
    SLB_DEBUG_CALL;
    if (_instanceFactory)
    {
      if (ref)
      {
        pushInstance(L, _instanceFactory->create_ref(_manager, ref) );
        SLB_DEBUG(7, "Class(%s) push_ref -> %p", _name.c_str(), ref);
      }
      else
      {
        luaL_error(L, "Can not push a NULL reference of class %s", _name.c_str());
      }
    }
    else
    {
      luaL_error(L, "Unknown class %s (push_reference)", _name.c_str());
    }
  }

  void ClassInfo::push_ptr(lua_State *L, void *ptr)
  {
    SLB_DEBUG_CALL;
    if (_instanceFactory)
    {
      pushInstance(L, _instanceFactory->create_ptr(_manager, ptr) );
    }
    else
    {
      luaL_error(L, "Can not push a ptr of class %s", _name.c_str());
    }
  }

  void ClassInfo::push_const_ptr(lua_State *L, const void *const_ptr)
  {
    SLB_DEBUG_CALL;
    if (_instanceFactory)
    {
      pushInstance(L, _instanceFactory->create_const_ptr(_manager, const_ptr) );
      SLB_DEBUG(7, "Class(%s) push const_ptr -> %p", _name.c_str(), const_ptr);
    }
    else
    {
      luaL_error(L, "Can not push a const_ptr of class %s", _name.c_str());
    }
  }

  void ClassInfo::push_copy(lua_State *L, const void *ptr)
  {
    SLB_DEBUG_CALL;
    if (_instanceFactory)
    {
      if (ptr)
      {
        pushInstance(L, _instanceFactory->create_copy(_manager, ptr) );
        SLB_DEBUG(7, "Class(%s) push copy -> %p", _name.c_str(), ptr);
      }
      else
      {
        luaL_error(L, "Can not push copy from NULL of class %s", _name.c_str());
      }
    }
    else
    {
      luaL_error(L, "Can not push a copy of class %s", _name.c_str());
    }
  }
  
  int ClassInfo::__index(lua_State *L)
  {
    SLB_DEBUG_CALL;
    int result = Table::__index(L); // default implementation
    if ( result < 1 )
    {
      // type == 0 -> class __index
      // type == 1 -> object __index
      const int type = lua_istable(L,1)? 0 : 1; 
      SLB_DEBUG(4, "Called ClassInfo(%p) '%s' __index %s", this, _name.c_str(), type? "OBJECT" : "CLASS");

      // if looking for a class method, using an string...
      if (lua_isstring(L,2))
      {
        const char *key = lua_tostring(L,2);
        Object *obj = 0;
        for(BaseClassMap::iterator i = _baseClasses.begin(); obj == 0L && i != _baseClasses.end(); ++i)
        {
          Table *table = (i->second);
          obj = table->get(key);
        }
        if (obj)
        {
          obj->push(L);
          result = 1;
        }
        else
        {
          BaseProperty *prop = getProperty(key);
          if (prop)
          {
            prop->get(L,1);
            // properties can neber be cached by table
            // so here we return directly the result value.
            return 1;
          }
        } // end of properties
      }


      if (result < 1) // still not found...
      {
        if (_meta__index[type].valid())
        {
          // 1 - func to call
          _meta__index[type]->push(L);
          lua_insert(L,1);  
          // 2 - object/table
          // 3 - key
          SLB_DEBUG_STACK(8,L,  "Class(%s) __index {%s} metamethod -> %p",
            _name.c_str(), type? "OBJECT" : "CLASS", (void*)_meta__index[type].get() );

          assert("Error in number of stack elements" && lua_gettop(L) == 3);
          lua_call(L,lua_gettop(L)-1, LUA_MULTRET);
          result = lua_gettop(L);
        }
        else
        {
          SLB_DEBUG(4, "ClassInfo(%p) '%s' doesn't have __index[%s] implementation.",
            this, _name.c_str(), type? "OBJECT" : "CLASS");
        }
      }
    }

    return result;
  }

  int ClassInfo::__newindex(lua_State *L)
  {
    SLB_DEBUG_CALL;
    // 0 = class __index
    // 1 = object __index
    int type = lua_istable(L,1)? 0 : 1; 
    SLB_DEBUG(4, "Called ClassInfo(%p) '%s' __newindex %s", this, _name.c_str(), type? "OBJECT" : "CLASS");

    // search for properties
    if (type == 1 && lua_isstring(L,2))
    {
      const char *key = lua_tostring(L,2);
      BaseProperty *prop = getProperty(key);
      if (prop)
      {
        prop->set(L,1);
        // as with property-get we can not cache results with properties, so
        // instead of calling Table::__newindex we just return.
        return 1;
      }
    }

    if (_meta__newindex[type].valid())
    {
      // 1 - func to call
      _meta__newindex[type]->push(L);
      lua_insert(L,1);
      // 2 - object
      // 3 - key/table
      // 4 - value
      SLB_DEBUG_STACK(8,L,  "Class(%s) __index {%s} metamethod -> %p",
        _name.c_str(), type?  "OBJECT" :"CLASS", (void*)_meta__newindex[type].get() );

      assert("Error in number of stack elements" && lua_gettop(L) == 4);
      lua_call(L,lua_gettop(L)-1, LUA_MULTRET);
      return lua_gettop(L);
    }
    else
    {
      SLB_DEBUG(4, "ClassInfo(%p) '%s' doesn't have __newindex[%s] implementation.",
        this, _name.c_str(), type? "OBJECT" : "CLASS");
    }
    return Table::__newindex(L);
  }
  
  int ClassInfo::__garbageCollector(lua_State *L)
  {
    SLB_DEBUG_CALL;
    void *raw_ptr = lua_touserdata(L,1);
  
    if (raw_ptr == 0) luaL_error(L, "SLB check ERROR: __garbageCollector called on a no-instance value");
    InstanceBase* instance = 
      *reinterpret_cast<InstanceBase**>(raw_ptr);
    
    //Compiler pointer shifts from the virtual inheritance, must
    // put pointer back to allocation address before deleting it.
    const void* addr = instance->memoryRawPointer();
    instance->~InstanceBase();
    Free(const_cast<void*>(addr));

    return 0;
  }

  int ClassInfo::__call(lua_State *L)
  {
    SLB_DEBUG_CALL;
    if ( _constructor.valid() )
    {
      _constructor->push(L);
      lua_replace(L, 1); // remove ClassInfo table
      lua_call(L, lua_gettop(L)-1 , LUA_MULTRET);

      // Mark the last instance as MustFreeMem..
      InstanceBase* instance_base = getInstance(L, -1);
      if (instance_base) {
        instance_base->setMustFreeMemFlag();
        // if is Hybrid and we just have build a new Instance
        if (_isHybrid)
        {
          int top = lua_gettop(L);
          //suppose that this state will hold the Object..
          HybridBase *hb = SLB::get<HybridBase*>(L,top);
          if (!hb) assert("Invalid push of hybrid object" && false);
          if (!hb->isAttached()) hb->attach(L);
          // check... just in case
          assert("Invalid lua stack..." && (top == lua_gettop(L)));
        }
      }

      return lua_gettop(L);
    }
    else
    {
      luaL_error(L,
        "ClassInfo '%s' is abstract ( or doesn't have a constructor ) ", _name.c_str());
    }
    return 0;
  }

  int ClassInfo::__tostring(lua_State *L)
  {
    SLB_DEBUG_CALL;
    int top = lua_gettop(L);
    lua_pushfstring(L, "Class(%s) [%s]", _name.c_str(), getInfo().c_str());
    ;
    for(BaseClassMap::iterator i = _baseClasses.begin(); i != _baseClasses.end(); ++i)
    {
        lua_pushfstring(L, "\n\tinherits from %s (%p)",i->second->getName().c_str(), (Object*) i->second);
    }
    for(Elements::iterator i = _elements.begin(); i != _elements.end(); ++i)
    {
      Object *obj = i->second.get();
      FuncCall *fc = slb_dynamic_cast<FuncCall>(obj);
      if (fc)
      {
        lua_pushfstring(L, "\n\tfunction (%s) [%s]",i->first.c_str(), obj->getInfo().c_str() );
        for (size_t i = 0; i < fc->getNumArguments(); ++i)
        {
          lua_pushfstring(L, "\n\t\t[%d] (%s) [%s]",i,
            fc->getArgType(i).name(),
            fc->getArgComment(i).c_str()
            );
        }
        //TODO, print return type
      }
      else
      {
        lua_pushfstring(L, "\n\t%s -> %p [%s] [%s]",i->first.c_str(), obj, obj->typeInfo().name(), obj->getInfo().c_str() );
      }
    }
    lua_concat(L, lua_gettop(L) - top);
    return 1;
  }

  bool ClassInfo::isSubClassOf( const ClassInfo *base )
  {
    SLB_DEBUG_CALL;
    if (base == this) return true;
    BaseClassMap::iterator i = _baseClasses.find( base->__TIW );
    if (i != _baseClasses.end()) return true;
    // make it recursive
    for(BaseClassMap::iterator i = _baseClasses.begin(); i != _baseClasses.end(); ++i)
    {
      if (i->second->isSubClassOf(base)) return true;
    }
    // no, definitely is not a subclass
    return false;
  }
  
  int ClassInfo::__eq(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(4, "Called ClassInfo(%p) '%s' __eq", this, _name.c_str() );
    if (_meta__eq.valid())
    {
      // 1 - func to call
      _meta__eq->push(L);
      lua_insert(L,1);
      lua_call(L, lua_gettop(L)-1 , LUA_MULTRET);
      return lua_gettop(L);
    }
    // default do the same as Table
    else return Table::__eq(L);
  }


}
