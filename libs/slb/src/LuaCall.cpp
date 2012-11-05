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



#include <SLB/LuaCall.hpp>
#include <SLB/Debug.hpp>
#include <SLB/Error.hpp>
#include <sstream>

namespace SLB {

  LuaCallBase::LuaCallBase(lua_State *L, int index) : _lua_state(L) { SLB_DEBUG_CALL; getFunc(index); } 

  LuaCallBase::LuaCallBase(lua_State *L, const char *func) : _lua_state(L) 
  {
    SLB_DEBUG_CALL;
    lua_getglobal(L,func);
    getFunc(-1);
    lua_pop(L,1); 
  }

  LuaCallBase::~LuaCallBase()
  {
    SLB_DEBUG_CALL;
    luaL_unref(_lua_state, LUA_REGISTRYINDEX, _ref); 
  }

  void LuaCallBase::getFunc(int index)
  {
    SLB_DEBUG_CALL;
    lua_pushvalue(_lua_state,index);
    if (lua_type(_lua_state, -1) != LUA_TFUNCTION)
    {
      SLB_THROW(std::runtime_error( "No Lua function was found at the index you provided." ));
      SLB_CRITICAL_ERROR("No Lua function was found at the index you provided.");
    }
    _ref = luaL_ref(_lua_state, LUA_REGISTRYINDEX);
  }

  int LuaCallBase::errorHandler(lua_State *L)
  {
    SLB_DEBUG_CALL;
    std::ostringstream out; // Use lua pushfstring and so on...
    lua_Debug debug;

    out << "SLB Exception: "
      << std::endl << "-------------------------------------------------------"
      << std::endl;
    out << "Lua Error:" << std::endl << "\t" 
      <<  lua_tostring(L, -1) << std::endl
      << "Traceback:" << std::endl;
    for ( int level = 0; lua_getstack(L, level, &debug ); level++)
    {
      if (lua_getinfo(L, "Sln", &debug) )
      {
        //TODO use debug.name and debug.namewhat
        //make this more friendly
        out << "\t [ " << level << " (" << debug.what << ") ] ";
        if (debug.currentline > 0 )
        {
          out << debug.short_src << ":" << debug.currentline; 
        }
        if (debug.name)
        {
          out << " @ " << debug.name;
           if (debug.namewhat) out << "(" << debug.namewhat << ")";
        }
        out << std::endl;
      }
      else
      {
        out << "[ERROR using Lua DEBUG INTERFACE]" << std::endl;
      }
    }

    lua_pushstring(L, out.str().c_str()) ;
    return 1;
  }

  void LuaCallBase::execute(int numArgs, int numOutput, int /*top*/)
  {
    SLB_DEBUG_CALL;
    //TODO: Use Manager to retreive a defaultHandler
    DefaultErrorHandler handler;

    if(handler.call(_lua_state, numArgs, numOutput))
    {
      const char* msg = lua_tostring(_lua_state, -1);
      SLB_THROW(std::runtime_error( msg ? msg : "Unknown Error" ));
      SLB_CRITICAL_ERROR(msg ? msg : "Unknown Error" );
    }

  }

}
