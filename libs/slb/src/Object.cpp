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



#include <SLB/Object.hpp>
#include <SLB/Debug.hpp>
#include <SLB/lua.hpp>
#include <SLB/Manager.hpp>

#include<assert.h>

namespace SLB {
    
  const char *objectsTable_name = "SLB_Objects";
  const char *refTable_name     = "SLB_References";

  Object::Object() : _refCounter(0)
  {
    SLB_DEBUG_CALL;
  }

  Object::~Object()
  {
    SLB_DEBUG_CALL;
  }
  
  void Object::initialize(lua_State *L) const
  {
    SLB_DEBUG_CALL;
    int top = lua_gettop(L);

    // create a new objects' table
    lua_newtable(L); 
    //make it weak(-value)
    lua_newtable(L);
    lua_pushstring(L, "v");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L,-2);
    lua_setfield(L, LUA_REGISTRYINDEX, objectsTable_name); 

    // create new references' table
    lua_newtable(L); // create a new table
    //make it weak(-key)
    lua_newtable(L);
    lua_pushstring(L, "k");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L,-2);
    lua_setfield(L, LUA_REGISTRYINDEX, refTable_name); 

    lua_settop(L, top);
  }
  
  void Object::push(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(3, "(L %p) Object::push(%p) [%s]", L, this, typeid(*this).name());
    int top = lua_gettop(L);

    // get the table of objects (top+1)
    lua_getfield(L, LUA_REGISTRYINDEX, objectsTable_name);
    if (lua_isnil(L, -1))
    {
      lua_pop(L, 1); // remove nil
      initialize(L);
      lua_getfield(L, LUA_REGISTRYINDEX, objectsTable_name);
    }
    // get Reference's table (top+2)
    lua_getfield(L, LUA_REGISTRYINDEX, refTable_name);

    // find the object with our id
    lua_pushlightuserdata(L, (void*) this );
    lua_rawget(L, top + 1);

    if (lua_isnil(L,-1))
    {
      lua_pop(L, 1); // remove nil

      int objpos = lua_gettop(L) + 1; // position of the user pushImplementation
      pushImplementation(L); // value (0)
      SLB_DEBUG(5, "\t-new object");

      // check, pushImplementation should return only one value
      if (lua_gettop(L) != objpos) 
      {
        luaL_error(L, "Error on Object::push the current stack "
          "has %d elments and should have only one.", 
          lua_gettop(L) - objpos - 1);
        lua_settop(L, top);
        return;
      }

      // make objects[key] = value
      lua_pushlightuserdata(L, (void*) this); // key
      lua_pushvalue(L, objpos); // copy of value (0)
      lua_rawset(L, top+1); // objects[key] = value  


      lua_pushvalue(L, objpos); // key 
      // create a reference of this object... 
      Object **objptr = reinterpret_cast<Object**>(
        lua_newuserdata(L, sizeof(Object*)));
      *objptr = this; // copy a pointer
      ref(); // manual reference

      // get or crate a metatable for the reference of this
      if(luaL_newmetatable(L, "SLB_ObjectPtr_GC"))
      {
        // create the __gc function
        lua_pushcfunction(L, GC_callback);
        lua_setfield(L, -2, "__gc"); // set the gc func of the obj
      }
      lua_setmetatable(L, -2); // metatable of reference object

      // make references[obj] = reference
      lua_rawset(L, top+2);

      // check object at top
      assert( lua_gettop(L) == objpos);
    }
    else
    {
      SLB_DEBUG(6, "\t-object exists");
    }

    // replace top+1 with the object
    lua_replace(L, top+1);
    lua_settop(L, top+1);
  }
  
  int Object::GC_callback(lua_State *L)
  {
    SLB_DEBUG_CALL;
    Object **ptr_obj = reinterpret_cast<Object**>(lua_touserdata(L, 1));
    Object *obj = *ptr_obj;
    SLB_DEBUG(2, "(L %p) GC object %p (refcount %d - 1) [%s]", L, obj, obj->referenceCount(), typeid(*obj).name());
    obj->onGarbageCollection(L);
    obj->unref(); // manual unref
    *ptr_obj = 0L;
    return 0;
  }

}
