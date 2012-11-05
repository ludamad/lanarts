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



#ifndef __SLB_LUACALL__
#define __SLB_LUACALL__

#include "lua.hpp"
#include "Export.hpp"
#include "SPP.hpp"
#include "Object.hpp"
#include "PushGet.hpp"
#include "Type.hpp"

#include <vector>
#include <typeinfo>
#include <iostream>
#include <stdexcept>

namespace SLB
{

  class SLB_EXPORT LuaCallBase 
  { 
  public:
    // this allows to store a luaCall, mainly used by
    // Hybrid classes...
    virtual ~LuaCallBase();
  protected: 
    LuaCallBase(lua_State *L, int index);
    LuaCallBase(lua_State *L, const char *func);
    void execute(int numArgs, int numOutput, int top);

    lua_State *_lua_state;
    int _ref; 
  private:
    void getFunc(int index);
    static int errorHandler(lua_State *L);
  }; 

  template<typename T>
  struct LuaCall;

  #define SLB_ARG(N) T##N arg_##N, 
  #define SLB_PUSH_ARGS(N) push<T##N>(_lua_state, arg_##N );

  #define SLB_REPEAT(N) \
  \
    /* LuaCall: functions that return something  */ \
    template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
    struct SLB_EXPORT LuaCall<R( SPP_ENUM_D(N,T) )> : public LuaCallBase\
    { \
      LuaCall(lua_State *L, int index) : LuaCallBase(L,index) {} \
      LuaCall(lua_State *L, const char *func) : LuaCallBase(L,func) {} \
      R operator()( SPP_REPEAT( N, SLB_ARG) char dummyARG = 0) /*TODO: REMOVE dummyARG */\
      { \
        int top = lua_gettop(_lua_state); \
        lua_rawgeti(_lua_state, LUA_REGISTRYINDEX,_ref); \
        SPP_REPEAT( N, SLB_PUSH_ARGS ); \
        execute(N, 1, top); \
        R result = get<R>(_lua_state, -1); \
        lua_settop(_lua_state,top); \
        return result; \
      } \
      bool operator==(const LuaCall& lc) { return (_lua_state == lc._lua_state && _ref == lc._ref); }\
    };
  SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
  #undef SLB_REPEAT

  #define SLB_REPEAT(N) \
  \
    /*LuaCall: functions that doesn't return anything */  \
    template<SPP_ENUM_D(N, class T)> \
    struct SLB_EXPORT LuaCall<void( SPP_ENUM_D(N,T) )> : public LuaCallBase\
    { \
      LuaCall(lua_State *L, int index) : LuaCallBase(L,index) {} \
      LuaCall(lua_State *L, const char *func) : LuaCallBase(L,func) {} \
      void operator()( SPP_REPEAT( N, SLB_ARG) char /*dummyARG*/ = 0) /*TODO: REMOVE dummyARG */\
      { \
        int top = lua_gettop(_lua_state); \
        lua_rawgeti(_lua_state, LUA_REGISTRYINDEX,_ref); \
        SPP_REPEAT( N, SLB_PUSH_ARGS ); \
        execute(N, 0, top); \
        lua_settop(_lua_state,top); \
      } \
      bool operator==(const LuaCall& lc) { return (_lua_state == lc._lua_state && _ref == lc._ref); }\
    }; \

  SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
  #undef SLB_REPEAT
  #undef SLB_ARG
  #undef SLB_PUSH_ARGS

} //end of SLB namespace

  //--------------------------------------------------------------------
  // Inline implementations:
  //--------------------------------------------------------------------
  
#endif
