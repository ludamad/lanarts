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



#include <SLB/Manager.hpp>
#include <SLB/ClassInfo.hpp>
#include <SLB/lua.hpp>
#include <SLB/Debug.hpp>
#include <SLB/Hybrid.hpp>
#include <SLB/Mutex.hpp>
#include <SLB/Allocator.hpp>

#include <iostream>

namespace SLB {

  Mutex managerMutex;

  /* Global functions */
  int SLB_type(lua_State *L)
  {
    SLB_DEBUG_CALL;
    const ClassInfo *ci = Manager::getInstance(L)->getClass(L,-1);
    if (ci)
    {
      lua_pushstring(L, ci->getName().c_str());
      return 1;
    }
    return 0;
  }

  int SLB_rawptr(lua_State *L)
  {
    SLB_DEBUG_CALL;
    int top = lua_gettop(L);
    if (lua_getmetatable(L,1))
    {
      lua_getfield(L, -1, "__class_ptr");
      if (!lua_isnil(L,-1))
      {
        ClassInfo* ci = reinterpret_cast<ClassInfo*>( lua_touserdata(L,-1) );
        const void* raw = ci->get_const_ptr(L, 1);
        lua_settop(L, top);
        lua_pushinteger(L, (lua_Integer) raw);
        return 1;
      }
    }
    lua_settop(L, top);
    return 0;
  }

  int SLB_copy(lua_State *L)
  {
    SLB_DEBUG_CALL;
    int top = lua_gettop(L);
    if (lua_getmetatable(L,1))
    {
      lua_getfield(L, -1, "__class_ptr");
      if (!lua_isnil(L,-1))
      {
        ClassInfo* ci = reinterpret_cast<ClassInfo*>( lua_touserdata(L,-1) );
        lua_settop(L, top);
        ci->push_copy(L, ci->get_const_ptr(L,1));
        return 1;
      }
    }
    lua_settop(L, top);
    return 0;
  }

  int SLB_using_index(lua_State *L)
  {
    SLB_DEBUG_CALL;
    lua_pushnil(L);
    while( lua_next(L, lua_upvalueindex(1)) )
    {
      lua_pushvalue(L,2); // key
      lua_gettable(L, -2);
      if (!lua_isnil(L,-1))
      {
        return 1;
      }
      lua_pop(L,2);
    }
    return 0;
  }

  int SLB_using(lua_State *L)
  {
    SLB_DEBUG_CALL;
    int top = lua_gettop(L);
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, LUA_REGISTRYINDEX, "SLB_using");
    if ( lua_isnil(L,-1) )
    {
      // pop nil value
      lua_pop(L,1);

      // get or create _G's metatable
      // 1st recover _G
      lua_pushglobaltable(L); // [1:_G] --> _G = globals
      if(!lua_getmetatable(L, -1))
      {
        lua_newtable(L); // [2:_G,MT] --> MT = MetaTable (for globals)
        // set as metatable of _G (keeping a copy)
        lua_pushvalue(L,-1); //[3:_G,MT,MT]
        lua_setmetatable(L, -3); // [2:_G,MT]
      }
      else
      {
        luaL_error(L, "Can not use SLB.using,"
          " _G already has a metatable");
      }
      // [2:(_G)(MT)]
      lua_newtable(L); // [3:_G,MT,SU] SU --> create the "SLB_using" table
      lua_pushvalue(L,-1); // [4:_G,MT,SU,SU] --> keep a copy in registry
      lua_setfield(L, LUA_REGISTRYINDEX, "SLB_using"); // [3:_G,MT,SU]

      // push the using_index func with the SLB_using table as closure
      lua_pushvalue(L,-1); //[4:_G,MT,SU,SU]
      lua_pushcclosure(L, SLB_using_index, 1); // [4:_G,MT,SU,func] func -> push the closure
      // set this functions as "__index" of the metatable (at -3)
      lua_setfield(L, -3, "__index"); // 3[4:_G,MT_SU]

      // leave the SLB_using table at the top
    }
    lua_pushvalue(L,1); // get the first argument
    //TODO check not repeating tables...
    luaL_ref(L, -2); // add the table

    lua_settop(L,top);
    return 0;
  }

  int SLB_isA(lua_State *L)
  {
    SLB_DEBUG_CALL;
    int top = lua_gettop(L);
    if (top != 2)
      luaL_error(L, "Invalid number of arguments (instance, class)");

    ClassInfo* ci_a = 0;
    if (lua_getmetatable(L,1))
    {
      lua_getfield(L, -1, "__class_ptr");
      if (!lua_isnil(L,-1))
      {
        ci_a = reinterpret_cast<ClassInfo*>( lua_touserdata(L,-1) );
      }
    }
    ClassInfo* ci_b = 0;
    if (lua_getmetatable(L,2))
    {
      lua_getfield(L, -1, "__class_ptr");
      if (!lua_isnil(L,-1))
      {
        ci_b = reinterpret_cast<ClassInfo*>( lua_touserdata(L,-1) );
      }
    }
    lua_settop(L, top);
    if ( ci_a && ci_b )
    {
      lua_pushboolean(L, ci_a->isSubClassOf(ci_b) );
    }
    else
    {
      lua_pushboolean(L, false);
    }
    return 1;
  }

  int SLB_allTypes(lua_State *L)
  {
    SLB_DEBUG_CALL;
    Manager::ClassMap &map =  Manager::getInstance(L)->getClasses();

    lua_newtable(L);
    for(Manager::ClassMap::iterator i = map.begin(); i != map.end(); ++i)
    {
      lua_pushstring(L, i->second->getName().c_str());
      i->second->push(L);
      lua_rawset(L,-3);
    }
    return 1;
  }

  static const luaL_Reg SLB_funcs[] = {
    {"type", SLB_type},
    {"copy", SLB_copy},
    {"using", SLB_using},
    {"rawptr", SLB_rawptr},
    {"isA", SLB_isA},
    {"allTypes", SLB_allTypes},
    {NULL, NULL}
  };

  Manager* Manager::_default = 0;

  Manager::Manager()
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(0, "Manager initialization");
    _global = new (Malloc(sizeof(Namespace))) Namespace;
  }

  Manager::~Manager()
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(0, "Manager destruction");
  }

  void Manager::registerSLB(lua_State *L)
  {
    SLB_DEBUG_CALL;
    int top = lua_gettop(L);

    // Register global functions
    lua_newtable(L);
    luaL_register(L, NULL, SLB_funcs);
    // keep a copy of the SLB table before setting the global SLB
    lua_pushvalue(L,-1);
    lua_setglobal(L,"SLB");

    // metatable of "SLB"
    lua_newtable(L);
    lua_pushstring(L,"__index");
    _global->push(L);
    lua_rawset(L,-3);
    lua_setmetatable(L,-2); // SLB table

    lua_settop(L,top);

    // put the manager inside the lua_State
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "SLB::Manager");
  }

  const ClassInfo *Manager::getClass(const TypeInfoWrapper &ti) const
  {
    SLB_DEBUG_CALL;
    ActiveWaitCriticalSection lock(&managerMutex);
    //TODO: change this assert with a ti.valid()
    assert("Invalid type_info" && (&ti) );
    ClassMap::const_iterator i = _classes.find(ti);
    if ( i != _classes.end() ) return i->second.get();
    return 0;
  }

  const ClassInfo *Manager::getClass(const String &name) const
  {
    SLB_DEBUG_CALL;
    NameMap::const_iterator i = _names.find(name);
    if ( i != _names.end() )
      return getClass( i->second );
    return 0;
  }

  ClassInfo *Manager::getClass(lua_State *L, int pos) const
  {
    SLB_DEBUG_CALL;
    pos = lua_absindex(L,pos);
    int top = lua_gettop(L);
    ClassInfo* ci = 0L;
    if (lua_getmetatable(L,pos))
    {
      lua_getfield(L, -1, "__class_ptr");
      if (!lua_isnil(L,-1))
      {
        ci = reinterpret_cast<ClassInfo*>( lua_touserdata(L,-1) );
      }
    }
    lua_settop(L, top);
    return ci;
  }


  ClassInfo *Manager::getClass(const String &name)
  {
    SLB_DEBUG_CALL;
    NameMap::iterator i = _names.find(name);
    if ( i != _names.end() ) return getClass( i->second );
    return 0;
  }

  ClassInfo *Manager::getClass(const TypeInfoWrapper &ti)
  {
    SLB_DEBUG_CALL;
    ActiveWaitCriticalSection lock(&managerMutex);
    ClassInfo *result = 0;
    ClassMap::iterator i = _classes.find(ti);
    if ( i != _classes.end() ) result = i->second.get();
    SLB_DEBUG(6, "ClassInfo = %p", (void*) result);
    return result;
  }

  bool Manager::copy(lua_State *from, int pos, lua_State *to)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_CLEAN_STACK(from,0);

    if (from == to)
    {
      lua_pushvalue(from, pos);
      return true;
    }

    switch(lua_type(from, pos))
    {
      case LUA_TNIL:
        {
          SLB_DEBUG(20, "copy from %p(%d)->%p [nil]", from,pos,to);
          lua_pushnil(to);
          return true;
        }
      case LUA_TNUMBER:
        {
          SLB_DEBUG(20, "copy from %p(%d)->%p [number]", from,pos,to);
          lua_Number n = lua_tonumber(from,pos);
          lua_pushnumber(to, n);
          return true;
        }
      case LUA_TBOOLEAN:
        {
          SLB_DEBUG(20, "copy from %p(%d)->%p [boolean]", from,pos,to);
          int b = lua_toboolean(from,pos);
          lua_pushboolean(to,b);
          return true;
        }
      case LUA_TSTRING:
        {
          SLB_DEBUG(20, "copy from %p(%d)->%p [string]", from,pos,to);
          const char *s = lua_tostring(from,pos);
          lua_pushstring(to,s);
          return true;
        }
      case LUA_TTABLE:
        {
          SLB_DEBUG(0, "*WARNING* copy of tables Not yet Implemented!!!");
          return false;
        }
      case LUA_TUSERDATA:
        {
          SLB_DEBUG(20, "copy from %p(%d)->%p [Object]", from,pos,to);
          ClassInfo *ci = getClass(from, pos);
          if (ci != 0L)
          {
            const void* ptr = ci->get_const_ptr(from, pos);
            SLB_DEBUG(25, "making a copy of the object %p", ptr);
            // now copy it
            ci->push_copy(to,ptr);
            return true;
          }
          else
          {
            SLB_DEBUG(25, "Could not recognize the object");
            return false;
          }
        }
    }
    SLB_DEBUG(10,
      "Invalid copy from %p(%d)->%p %s",
      from,pos,to, lua_typename(from, lua_type(from,pos)));
    return false;
  }

  ClassInfo *Manager::getOrCreateClass(const TypeInfoWrapper &ti)
  {
    SLB_DEBUG_CALL;
    //TODO: change this assert with a ti.valid()
    assert("Invalid type_info" && (&ti) );
    ClassInfo *c = 0;
    {
      CriticalSection lock(&managerMutex);
      ClassMap::iterator i = _classes.find(ti);
      if ( i != _classes.end() )
      {
        c = i->second.get();
      }
    }
    if (c == 0)
    {
      CriticalSection lock(&managerMutex);
      c = new (Malloc(sizeof(ClassInfo))) ClassInfo(this,ti);
      _classes[ c->getTypeid() ] = c;
    }
    return c;
  }

  void Manager::set(const String &name, Object *obj)
  {
    SLB_DEBUG_CALL;
    ActiveWaitCriticalSection lock(&managerMutex);
    _global->set(name, obj);
  }

  void Manager::rename(ClassInfo *ci, const String &new_name)
  {
    SLB_DEBUG_CALL;
    CriticalSection lock(&managerMutex);
    const String old_name = ci->getName();

    NameMap::iterator i = _names.find(old_name);
    if ( i != _names.end() )
    {
      _global->erase(old_name);
      _names.erase(i);
    }

    _global->set(new_name, ci);
    _names[ new_name ] = ci->getTypeid();

  }

  Manager *Manager::getInstance(lua_State *L)
  {
    Manager *m = 0L;
    lua_getfield(L,LUA_REGISTRYINDEX, "SLB::Manager");
    if (lua_islightuserdata(L,-1))
    {
      m = reinterpret_cast<Manager*>(lua_touserdata(L,-1));
    }
    lua_pop(L,1);
    return m;
  }

  Manager *Manager::defaultManager()
  {
    if (_default == 0)
    {
      _default = new (Malloc(sizeof(Manager))) Manager();
    }
    return _default;
  }

  void Manager::destroyDefaultManager()
  {
    if (_default)
    {
      Free_T(&_default);
    }
  }


  void* Manager::recursiveConvert(const TypeInfoWrapper& C1, const TypeInfoWrapper &C2, const TypeInfoWrapper& prev, void *obj)
  {
    //This function does not support "diamond" inheritance patterns -- such inheritance patterns will
    // lead to infinite recursion of this function.  If you need to support "diamond" inheritance, then
    // a list of all previously traversed nodes must be passed around instead of just ensuring to not walk
    // into the previously traversed node alone.
    //
    //This function is O(N*N) worst case.  If there is a conversion that is hitting this code frequently
    // you can add additional .inherits<> or .static_inherits<> to the derived class, adding the direct
    // conversion being done to the _conversions map and bypassing this code.  For example if you have
    // Animal -> Dog -> Dalmation and you are converting between Animal and Dalmation a lot, in the Dalmation
    // wrapper add .inherits<Animal> along with .inherits<Dog>.  Now converting from Animal to Dalmation
    // is O(Log2(N)) and doesn't invoke this algorithm like it does if you don't have .inherits<Animal>.
    for (ConversionsMap::iterator it=_conversions.begin(); it!=_conversions.end(); ++it)
    {
      if (it->first.first == C1)
      {
        if (it->first.second == C2)
        {
          return it->second( obj );
        }
        else if (!(it->first.second == prev))
        {
          void *foundObj = recursiveConvert(it->first.second, C2, C1, it->second(obj));
          if (foundObj)
          {
            return foundObj;
          }
        }
      }
    }
    return 0;
  }
}

