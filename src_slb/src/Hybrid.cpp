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



#include <SLB/Hybrid.hpp>
#include <SLB/lua.hpp>
#include <SLB/Manager.hpp>
#include <SLB/Allocator.hpp>
#include <sstream>
#include <iostream>

namespace SLB {

  /*--- Invalid Method (exception) ----------------------------------------------*/
   InvalidMethod::InvalidMethod(const HybridBase *obj, const char *c)
  {
    SLB_DEBUG_CALL;
    const ClassInfo  *CI = obj->getClassInfo();
    std::ostringstream out;

    out << "Invalid Method '" << CI->getName() << "::" <<
      c << "' NOT FOUND!" << std::endl;
    
    _what = out.str();
  }
  /*--- Invalid Method (exception) ----------------------------------------------*/

  /*--- Internal Hybrid Subclasses ---------------------------------------*/
  struct InternalHybridSubclass : public Table
  {
    InternalHybridSubclass(ClassInfo *ci) : _CI(ci)
    {
      SLB_DEBUG_CALL;
      assert("Invalid ClassInfo" && _CI.valid());
    }

    int __newindex(lua_State *L)
    {
      SLB_DEBUG_CALL;
      SLB_DEBUG_CLEAN_STACK(L,-2);
      SLB_DEBUG_STACK(6,L, "Call InternalHybridSubclass(%p)::__nexindex", this);
      //1 = table
      //2 = string
      //3 = function
      luaL_checkstring(L,2);
      if (lua_type(L,3) != LUA_TFUNCTION)
      {
        luaL_error(L, "Only functions can be added to hybrid classes"
          " (invalid %s of type %s)", lua_tostring(L,2), lua_typename(L, 3));
      }
      SLB_DEBUG(4, "Added method to an hybrid-subclass:%s", lua_tostring(L,2));
      lua_pushcclosure(L, HybridBase::call_lua_method, 1); // replaces 
      setCache(L);
      return 0;
    }

    int __call(lua_State *L)
    {
      SLB_DEBUG_CALL;
      SLB_DEBUG_STACK(6,L, "Call InternalHybridSubclass(%p)::__call", this);
      // create new instance:
      ref_ptr<FuncCall> fc = _CI->getConstructor();
      assert("Invalid Constructor!" && fc.valid());
      fc->push(L);
      lua_replace(L,1); // table of metamethod __call
      lua_call(L, lua_gettop(L) -1 , LUA_MULTRET);
      {
        SLB_DEBUG_CLEAN_STACK(L,0);
        // at 1 we should have an HybridBase instance...
        HybridBase *obj = SLB::get<HybridBase*>(L,1);
        if (obj == 0) luaL_error(L, "Output(1) of constructor should be an HybridBase instance");
        // now our table... to find methods
        obj->_subclassMethods = this;
      }
      return lua_gettop(L);
    }

  private:
    ref_ptr<ClassInfo> _CI;
  };
  /*--- Internal Hybrid Subclasses ----------------------------------------------*/


  HybridBase::HybridBase() : _lua_state(0),
    _data(0)
  {
    SLB_DEBUG_CALL;
  }

  HybridBase::~HybridBase()
  {
    SLB_DEBUG_CALL;
    unAttach();
  }
  
  void HybridBase::attach(lua_State *L)
  {
    SLB_DEBUG_CALL;
    //TODO allow reattaching...
    if (_lua_state != 0 && _lua_state != L) {
      SLB_THROW(std::runtime_error("Trying to reattach an Hybrid instance"));
      SLB_CRITICAL_ERROR("Trying to reattach an Hybrid instance");
    }

    if (L)
    {
      SLB_DEBUG_CLEAN_STACK(L,0);
      _lua_state = L;
      // create a table to store internal data
      lua_newtable(_lua_state);
      _data = luaL_ref(_lua_state, LUA_REGISTRYINDEX);
    }
  }

  void HybridBase::unAttach()
  {
    SLB_DEBUG_CALL;
    clearMethodMap();
    _subclassMethods = 0;
    if (_lua_state && _data )
    {
      luaL_unref(_lua_state, LUA_REGISTRYINDEX, _data);
      _data = 0;
      _lua_state = 0;
    }
  }

  
  void HybridBase::clearMethodMap()
  {
    SLB_DEBUG_CALL;
    // delete the list of _methods
    for(MethodMap::iterator i = _methods.begin(); i != _methods.end(); i++ )
    {
      Free_T(&i->second);
    }
    _methods.clear();
  }
  
  bool HybridBase::getMethod(const char *name) const
  {
    SLB_DEBUG_CALL;
    if (_lua_state == 0) {
      SLB_THROW(std::runtime_error("Hybrid instance not attached"));
      SLB_CRITICAL_ERROR("Hybrid instance not attached")
    }
    SLB_DEBUG_STACK(5,_lua_state, "HybridBase(%p)::getMethod '%s' (_lua_state = %p)", this, name, _lua_state); 
    int top = lua_gettop(_lua_state);

    // first try to find in _subclassMethods
    if (_subclassMethods.valid())
    {
      //TODO: NEED DEBUG...
      // ---- why not:
      // Object *m = obj->_subclassMethods->get(key);
      // if (m)
      // {
      //   SLB_DEBUG(5, "Hybrid subclassed instance, looking for '%s' method [OK]", key);
      //   m->push(L);
      //   return 1;
      // }
      // else SLB_DEBUG(5, "Hybrid subclassed instance, looking for '%s' method [FAIL!]", key);
      // ---- instead of: (even though this is quicker) but code above should work
      lua_pushstring(_lua_state,name); // [+1]
      _subclassMethods->getCache(_lua_state); // [-1, +1] will pop key's copy and return the cache 
      if (!lua_isnil(_lua_state,-1))
      {
        assert("Invalid Stack" && (lua_gettop(_lua_state) == top+1));
        return true;
      }
      lua_pop(_lua_state,1); // [-1] remove nil
      assert("Invalid Stack" && (lua_gettop(_lua_state) == top));
      //end TODO-------------------------------------------------------------------------------
    }

    ClassInfo *ci = getClassInfo();
    ci->push(_lua_state);
    lua_getmetatable(_lua_state,-1);
    lua_getfield(_lua_state,-1, "__hybrid");
    if (!lua_isnil(_lua_state,-1))
    {
      lua_pushstring(_lua_state,name);
      lua_rawget(_lua_state,-2);
      if (!lua_isnil(_lua_state,-1))
      {
        lua_replace(_lua_state,top+1);
        lua_settop(_lua_state,top+1);
        SLB_DEBUG(6, "HybridBase(%p-%s)::getMethod '%s' (_lua_state = %p) -> FOUND",
            this, ci->getName().c_str(),name, _lua_state); 
        assert("Invalid Stack" && (lua_gettop(_lua_state) == top+1));
        return true;
      }
      else SLB_DEBUG(6, "HybridBase(%p-%s)::getMethod '%s' (_lua_state = %p) -> *NOT* FOUND",
        this,ci->getName().c_str(), name, _lua_state); 
    }
    else SLB_DEBUG(4, "HybridBase(%p-%s) do not have any hybrid methods", this, ci->getName().c_str());

    // anyway... if not found:
    lua_settop(_lua_state,top);
    return false;
  }
  
  void HybridBase::setMethod(lua_State *L, ClassInfo *ci)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_CLEAN_STACK(L,-2);
    // key
    // value [top]
    int top = lua_gettop(L);
    // checks key, value
    assert( "Invalid key for method" && lua_type(L,top-1) == LUA_TSTRING);
    assert( "Invalid type of method" && lua_type(L,top) == LUA_TFUNCTION);

    ci->push(L); // top +1
    lua_getmetatable(L,-1); // top +2
    lua_getfield(L,-1, "__hybrid"); // top +3
    // create if not exists
    if (lua_isnil(L,-1))
    {
      lua_pop(L,1); // remove nil
      lua_newtable(L); // top +3
      lua_pushstring(L, "__hybrid");
      lua_pushvalue(L,-2); // a copy for ClassInfo
      lua_rawset(L, top+2); // set to he metatable 
    }
    lua_insert(L,top-2); // put the __hybrid table below key,value
    lua_settop(L, top+1); // table, key, and value
    lua_rawset(L,top-2); // set elements
    lua_settop(L, top-2); // remove everything :)
  }

  void HybridBase::registerAsHybrid(ClassInfo *ci)
  {
    SLB_DEBUG_CALL;
    //TODO: Check first if the class already has __index, __newindex
    //Maybe this should be a feature at ClassInfo to warn in case
    //the user sets these functions more than once.
    ci->setClass__newindex( FuncCall::create(class__newindex) );
    ci->setClass__index( FuncCall::create(class__index) );
    ci->setObject__index( FuncCall::create(object__index) );
    ci->setObject__newindex( FuncCall::create(object__newindex) );
    ci->setHybrid();
  }
  

  const HybridBase* get_hybrid(lua_State *L, int pos)
  {
    SLB_DEBUG_CALL;
    //TODO: Generalize this to be used in all SLB
    const HybridBase *obj = get<const HybridBase*>(L,pos);
    if (!obj)
    {
      if (lua_type(L,pos) == LUA_TUSERDATA)
      {
        void *dir = lua_touserdata(L,pos);
        // try to get the class info:
        ClassInfo *ci = Manager::getInstance(L)->getClass(L,pos);
        if (ci == 0)
        {
          luaL_error(L, "Invalid Hybrid object (index=%d) "
          "'%s' %p", pos, ci->getName().c_str(), dir);
        }
        else
        {
          luaL_error(L, "Invalid Hybrid object (index=%d) "
          "userdata (NOT REGISTERED WITH SLB) %p", pos, dir);
        }
      }
      else
      {
        luaL_error(L, "Invalid Hybrid object (index=%d) found %s", pos, luaL_typename(L,pos));
      }
    }
    return obj;
  }


  int HybridBase::call_lua_method(lua_State *L)
  {
    SLB_DEBUG_CALL;
    const HybridBase *hb = get_hybrid( L, 1 );
    if (hb->_lua_state == 0) luaL_error(L, "Instance(%p) not attached to any lua_State...", hb);
    if (hb->_lua_state != L) luaL_error(L, "This instance(%p) is attached to another lua_State(%p)", hb, hb->_lua_state);
    
    // get the real function to call
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L,1); //put the target function at 1
    SLB_DEBUG_STACK(10, L, "Hybrid(%p)::call_lua_method ...", hb);
    lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);

    return lua_gettop(L);
  }

  int HybridBase::class__newindex(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_CLEAN_STACK(L,-2);
    // 1 - obj (table with classInfo)
    ClassInfo *ci = Manager::getInstance(L)->getClass(L,1);
    if (ci == 0) luaL_error(L, "Invalid Class at #1");

    // 2 - key (string)
    const int key = 2;

    // 3 - value (func)
    const int value = 3;

    if (lua_isstring(L,key) && lua_isfunction(L,value))
    {
      // create a closure with the function to call
      lua_pushcclosure(L, HybridBase::call_lua_method, 1); // replaces [value]
      setMethod(L, ci);  
    }
    else
    {
      luaL_error(L,
        "hybrid instances can only have new methods (functions) "
        "indexed by strings ( called with: class[ (%s) ] = (%s) )",
        lua_typename(L, lua_type(L,key)), lua_typename(L, lua_type(L,value))
        );
    }
    return 0;
  }

  int HybridBase::object__index(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_CLEAN_STACK(L,+1);
    SLB_DEBUG(4, "HybridBase::object__index");
    // 1 - obj (table with classInfo)
    HybridBase* obj = get<HybridBase*>(L,1);
    if (obj == 0) luaL_error(L, "Invalid instance at #1");
    if (!obj->_lua_state) luaL_error(L, "Hybrid instance not attached or invalid method");
    if (obj->_lua_state != L) luaL_error(L, "Can not use that object outside its lua_state(%p)", obj->_lua_state);

    // 2 - key (string) (at top)
    const char *key = lua_tostring(L,2);

    // first look into _data table to see if the key is valid there
    lua_rawgeti(L, LUA_REGISTRYINDEX, obj->_data);
    lua_getfield(L, -1, key);
    if (lua_isnil(L,-1))
    {
      lua_pop(L,2); // remove _data table + nil
    }
    else 
    {
      lua_remove(L,-2); // remove _data table
      return 1; // result found.
    }

    // getMethod of hybrid (basic)
    if(obj->getMethod(key)) return 1;

    // nothing found..
    lua_pushnil(L);
    return 1;
  }

  int HybridBase::object__newindex(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_CLEAN_STACK(L,-2); // we pop two values...
    SLB_DEBUG(4, "HybridBase::object__newindex");
    // 1 - obj (table with classInfo)
    HybridBase* obj = get<HybridBase*>(L,1);
    if (obj == 0) luaL_error(L, "Invalid instance at #1");
    if (!obj->_lua_state) luaL_error(L, "Hybrid instance not attached or invalid method");
    if (obj->_lua_state != L) luaL_error(L, "Can not use that object outside its lua_state(%p)", obj->_lua_state);

    // 2 - key (string)
    // 3 - value (top)

    // get the _data table and put the value there
    lua_rawgeti(L, LUA_REGISTRYINDEX, obj->_data);
    lua_replace(L,1); // replace top element, so key, value will be at the top
    // set the value
    lua_settable(L,1);

    return 0;
  }
  
  int HybridBase::class__index(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_CLEAN_STACK(L,+1);
    SLB_DEBUG_STACK(6, L, "Call class__index");
    // trying to traverse the class... create a new InternalHybridSubclass
    ClassInfo *ci = Manager::getInstance(L)->getClass(L,1);
    if (ci == 0) luaL_error(L, "Expected a valid class.");
    luaL_checkstring(L,2); // only valid with strings
    if (!ci->hasConstructor())
    {
      luaL_error(L, "Hybrid Class(%s) doesn't have constructor."
        " You can not subclass(%s) from it", ci->getName().c_str(),
        lua_tostring(L,2));
    }

    ref_ptr<InternalHybridSubclass> subc = new (Malloc(sizeof(InternalHybridSubclass))) InternalHybridSubclass(ci) ;
    subc->push(L);

    // -- set cache...
    lua_pushvalue(L,2);  // [+1] key
    lua_pushvalue(L,-2); // [+1] copy of new InternalHybrid...
    ci->setCache(L);     // [-2] keep a copy in the cache
    // -- set cache done

    return 1;
  }
}
