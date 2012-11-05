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



#include <SLB/Table.hpp>
#include <SLB/Debug.hpp>
#include <SLB/Allocator.hpp>
#include <SLB/TypeInfoWrapper.hpp>

namespace SLB {

  
  // BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG
  //------------->8------------------->8------------------>8--------------
  // BUG: cacheable Tables doesn't work, this is a temporal fix:
  //
  Table::Table(const String &sep, bool c) : _cacheable(true), _separator(sep) {SLB_DEBUG_CALL;}
  //Table::Table(const String &sep, bool c) : _cacheable(c), _separator(sep) {SLB_DEBUG_CALL;}
  //----------8<-----------------8<---------------------8<----------------
  // BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG BUG

  Table::~Table() {SLB_DEBUG_CALL;}
    
  Object* Table::rawGet(const String &name)
  {
    SLB_DEBUG_CALL;
    Elements::iterator i = _elements.find(name);
    if (i == _elements.end())
    {
      SLB_DEBUG(10, "Access Table(%p) [%s] (FAILED!)", this, name.c_str());
      return 0;
    }
    SLB_DEBUG(10, "Access Table(%p) [%s] (OK!)", this, name.c_str());
    return i->second.get();
  }

  inline void Table::rawSet(const String &name, Object *obj)
  {
    SLB_DEBUG_CALL;
    if (obj == 0)
    {
      SLB_DEBUG(6, "Table (%p) remove '%s'", this, name.c_str());
      _elements.erase(name);
    }
    else
    {
      SLB_DEBUG(6, "Table (%p) [%s] = %p", this, name.c_str(), obj);
      _elements[name] = obj;    
    }
  }

  Object* Table::get(const String &name)
  {
    SLB_DEBUG_CALL;
    TableFind t = getTable(name, false);
    if (t.first != 0) return t.first->rawGet(t.second);
    return 0;
  }

  void Table::erase(const String &name)
  {
    SLB_DEBUG_CALL;
    set(name, 0);
  }

  void Table::setCache(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_CLEAN_STACK(L,-2);
    SLB_DEBUG_STACK(8, L, "Table(%p) :: setCache BEGIN ", this);
    int top = lua_gettop(L);
    if (top < 2 ) luaL_error(L, "Not enough elements to perform Table::setCache");
    push(L); // push ::Table
    if (luaL_getmetafield(L,-1, "__indexCache"))
    {
      lua_insert(L, top - 1); // move the metatable above key,value
      lua_settop(L, top + 1); // remove everything else
      lua_rawset(L,-3);
    }
    else
    {
      luaL_error(L, "Invalid setCache;  %s:%d", __FILE__, __LINE__ );
    }
    SLB_DEBUG_STACK(8, L, "Table(%p) :: setCache END original top = %d", this, top);
    lua_settop(L, top - 2);
  }

  void Table::getCache(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_CLEAN_STACK(L,0);
    SLB_DEBUG_STACK(8, L, "Table(%p) :: getCache BEGIN ", this);
    int top = lua_gettop(L);
    if (top < 1 ) luaL_error(L, "Not enough elements to perform Table::getCache");
    push(L); // [+1] push ::Table 
    if (!luaL_getmetafield(L,-1, "__indexCache")) // [+1]
    {
      luaL_error(L, "Invalid setCache;  %s:%d", __FILE__, __LINE__ );
    }
    lua_pushvalue(L, top); // [+1] copy value
    lua_rawget(L, -2); // [-2]
    SLB_DEBUG_STACK(8, L, "Table(%p) :: getCache END (result is at top) top was = %d", this, top);
    lua_replace(L, top); // [-1,+1] as result
    lua_settop(L, top);
  }


  void Table::set(const String &name, Object *obj)
  {
    SLB_DEBUG_CALL;
    TableFind t = getTable(name, true);
    t.first->rawSet(t.second, obj);
  }

  Table::TableFind Table::getTable(const String &key, bool create)
  {
    SLB_DEBUG_CALL;
    if (_separator.empty()) return TableFind(this,key);

    String::size_type pos = key.find(_separator);
    if (pos != String::npos)
    {
      const String &base = key.substr(0, pos);
      const String &next = key.substr(pos+_separator.length());

      Table* subtable = slb_dynamic_cast<Table>(rawGet(base));
      
      if (subtable == 0)
      {
        if (create)
        {
          SLB_DEBUG(6, "Table (%p) create Subtable %s -> %s", this, 
            base.c_str(), next.c_str());

          subtable = new (Malloc(sizeof(Table))) Table(_separator, _cacheable);
          rawSet(base,subtable);
        }
        else
        {
          return TableFind((Table*)0,key); // not found
        }
      }

      return subtable->getTable(next, create); //< TODO: recursivity... replace with iterative version.
    }
    // else :
    return TableFind(this,key);
  }
  
  int Table::__index(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_STACK(10,L,"Table::__index (%p)",this);
    int result = -1;
    
    { // Always tries to look in the cache first
      lua_pushvalue(L,2);
      lua_rawget(L, cacheTableIndex());
      if (lua_isnil(L,-1)) lua_pop(L,1); // remove nil
      else
      {
        SLB_DEBUG(10, "Access Table(%p) (In CACHE)", this);
        result = 1; // we found it
      }
    }

    if (result < 0)
    {
      if (lua_type(L, 2) == LUA_TSTRING)
      {
        Object *obj = get(lua_tostring(L,2));
        if (obj)
        {
          result = 1;
          obj->push(L);
          if (_cacheable)
          {
            // store in the cache...
            SLB_DEBUG(10, "L(%p) table(%p) key %s (->to cache)", L, this, lua_tostring(L,2));
            lua_pushvalue(L,2); // key
            lua_pushvalue(L,-2); // returned value
            lua_rawset(L, cacheTableIndex() ); // table
          }
        }
      }
    }

    return result;
  }
  
  int Table::__newindex(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_STACK(10,L,"Table::__newindex (%p)",this);
    luaL_error(L, "(%p)__newindex metamethod not implemented", (void*)this);
    return 0;
  }

  int Table::__call(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_STACK(10,L,"Table::__call (%p)",this);
    luaL_error(L, "(%p)__call metamethod not implemented", (void*)this);
    return 0;
  }

  int Table::__garbageCollector(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_STACK(10,L,"Table::__GC (%p)",this);
    luaL_error(L, "(%p) __gc metamethod not implemented", (void*)this);
    return 0;
  }

  int Table::__tostring(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_STACK(10,L,"Table::__tostring (%p)",this);
    int top = lua_gettop(L);
    lua_pushfstring(L, "Table(%p) [%s] with keys:", this, typeInfo().name());
    for(Elements::iterator i = _elements.begin(); i != _elements.end(); ++i)
    {
      lua_pushfstring(L, "\n\t%s -> %p [%s]",i->first.c_str(), i->second.get(), i->second->typeInfo().name());
    }
    lua_concat(L, lua_gettop(L) - top);
    return 1;
  }

  int Table::__eq(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_STACK(10,L,"Table::__eq (%p)",this);
    luaL_error(L, "__eq metamethod called but no implementation was given");
    return 0;
  }

  void Table::pushImplementation(lua_State *L)
  {
    SLB_DEBUG_CALL;

    lua_newtable(L); // an NEW empty table
    lua_newtable(L); // and its metatable:

    //----------------------------------------------------------------------
    // Set the metatable (empty) to the table to avoid calling __gc on the
    // table itself. This is new in lua 5.2
    lua_pushvalue(L,-1);
    lua_setmetatable(L,-3);
    //----------------------------------------------------------------------

    lua_newtable(L); // cache

    lua_pushvalue(L, -1); 
    lua_setfield(L, -3, "__indexCache");

    pushMeta(L, &Table::__indexProxy);
    lua_setfield(L,-3, "__index");
    pushMeta(L, &Table::__newindex);
    lua_setfield(L, -3, "__newindex");
    pushMeta(L, &Table::__tostring);
    lua_setfield(L, -3, "__tostring");
    pushMeta(L, &Table::__call);
    lua_setfield(L, -3, "__call");
    pushMeta(L, &Table::__garbageCollector);
    lua_setfield(L, -3, "__gc");
    pushMeta(L, &Table::__eq);
    lua_setfield(L, -3, "__eq");

    lua_pop(L,2); // remove the cache table and metatable
  }

  int Table::__indexProxy(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG(9, "---> __index search");
    SLB_DEBUG_STACK(10,L,"Table::__indexProxy (%p)",this);
    int result = __index(L);
    
    if (result < 0)
    {
      SLB_DEBUG(9, "Nothing found....");
      /* TODO Change the result behaviour, resturn 0 when nothing is found
       * and quit using result = -1 */
      lua_pushnil(L);
      result = 1;
    }
    SLB_DEBUG(9, "<--- __index result = %d", result);
    assert(result == 1 && "Result must be 1 when it gets here");
    return result;
  }

  
  int Table::__meta(lua_State *L)
  {
    SLB_DEBUG_CALL;
    SLB_DEBUG_STACK(10,L,"Table::__meta (static method)");
    // upvalue(1) is the cache table...
    void *table_raw = lua_touserdata(L, lua_upvalueindex(2));
    void *table_member_raw = lua_touserdata(L, lua_upvalueindex(3));
    Table *table = reinterpret_cast<Table*>(table_raw);
    TableMember member = *reinterpret_cast<TableMember*>(table_member_raw);

    return (table->*member)(L);
  }
  
  /** Pushmeta exepects a cache-table at the top, and creates a metamethod with 3 upvalues,
   * first the cache table, second the current object, and last the member to call */
  void Table::pushMeta(lua_State *L, Table::TableMember member) const
  {
    assert("Invalid pushMeta, expected a table at the top (the cache-table)" &&
      lua_type(L,-1) == LUA_TTABLE);
    lua_pushvalue(L,-1); // copy of cache table
    lua_pushlightuserdata(L, (void*) this);
    void *p = lua_newuserdata(L, sizeof(TableMember)); 
    memcpy(p,&member,sizeof(TableMember));

    lua_pushcclosure(L, __meta, 3 );
  }
  


}
